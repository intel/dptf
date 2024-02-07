/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#include "PolicyStateLogger.h"
#include "WorkItemQueueManagerInterface.h"
#include "StringParser.h"
#include "StringConverter.h"
#include "WIDptfPolicyStateLogger.h"
#include "EventNotifierInterface.h"
#include "MapOps.h"
#include "PolicyManagerInterface.h"

using namespace std;

const TimeSpan minPollingPeriod{TimeSpan::createFromMilliseconds(100)};
const TimeSpan maxPollingPeriod{TimeSpan::createFromMilliseconds(600000)};
const TimeSpan timerResolution{TimeSpan::createFromMilliseconds(4)};
const TimeSpan pollingPeriodOffset{timerResolution / 2}; // to compensate for timer drift
const string itemSeparator{","s};
const string columnNamePartition{" - "s};
const string newLine{"\n"s};
const string columnTitleTimestamp{"TimeStamp"s};

PolicyStateLogger::PolicyStateLogger(
	DptfManagerInterface* manager,
	const shared_ptr<IFileIo>& fileIo,
	const shared_ptr<TimeStampGenerator>& timeStampGenerator,
	const shared_ptr<IApplicationTimerSettings>& applicationTimerSettings)
	: m_manager(manager)
	, m_fileIo(fileIo)
	, m_timeStampGenerator(timeStampGenerator)
	, m_applicationTimerSettings(applicationTimerSettings)
	, m_isActive(false)
{
}

void PolicyStateLogger::start(
	const vector<string>& policyNames,
	const string& logFileName,
	const TimeSpan& pollingPeriod)
{
	throwIfAlreadyActive();
	const auto cleanedPolicyNames = cleanPolicyNames(policyNames);
	throwIfInvalidArguments(cleanedPolicyNames, logFileName, pollingPeriod);
	m_policies = cleanedPolicyNames;
	m_logFileName = logFileName;
	m_pollingPeriod = pollingPeriod;
	m_expectedColumnNames.clear();
	m_applicationTimerSettings->setMinimumResolutionForTimers(timerResolution);

	const auto loggerWorkItem = make_shared<WIDptfPolicyStateLogger>(m_manager);
	m_manager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(loggerWorkItem);

	setActive();
}

void PolicyStateLogger::stop()
{
	throwIfNotActive();
	removeFromWorkItemQueue();
	setInactive();
	m_applicationTimerSettings->clearMinimumResolutionForTimers();
}

void PolicyStateLogger::update(FrameworkEvent::Type event, const DptfBuffer& eventPayload)
{
	if (event == FrameworkEvent::DptfPolicyStateLogger && isActive())
	{
		queueNextLoggerUpdate();
		const auto sampleData = readPolicyStateLogData();
		setExpectedColumnNamesIfNeeded(sampleData);
		const auto outputData = generateDataForOutput(sampleData);
		writeToFile(outputData);
	}
}

string PolicyStateLogger::getStatus() const
{
	return isActive() ? generateActiveStatusMessage() : generateInactiveStatusMessage();
}

Bool PolicyStateLogger::isActive() const
{
	return m_isActive;
}

string PolicyStateLogger::generateActiveStatusMessage() const
{
	stringstream argumentString;
	argumentString << "Status: Started"s << endl;
	argumentString << "File Name : "s << m_manager->getDttLogDirectoryPath() << m_logFileName << endl;
	argumentString << "Sample Period: "s << m_pollingPeriod.toStringMilliseconds() << endl;

	auto policyArgument = StringParser::join(m_policies, itemSeparator[0]);
	policyArgument.erase(0, 1);
	policyArgument.erase(policyArgument.size() - 1);
	argumentString << "Policy List: "s << policyArgument << endl;
	return argumentString.str();
}

string PolicyStateLogger::generateInactiveStatusMessage()
{
	stringstream argumentString;
	argumentString << "Status: Stopped"s << endl;
	return argumentString.str();
}

map<string, map<string, string>> PolicyStateLogger::readPolicyStateLogData() const
{
	map<string, map<string, string>> sampleData;
	const auto policyManager = m_manager->getPolicyManager();
	for (const auto& policyName : m_policies)
	{
		try
		{
			const auto policy = policyManager->getPolicy(policyName);
			const auto logData = policy->getPolicyStateLogData();
			sampleData[policyName] = logData;
		}
		catch (...)
		{
		}
	}
	return sampleData;
}

void PolicyStateLogger::setExpectedColumnNamesIfNeeded(const map<string, map<string, string>>& sampleData)
{
	if (m_expectedColumnNames.empty())
	{
		for (const auto& [policyName, data] : sampleData)
		{
			m_expectedColumnNames[policyName] = MapOps<string, string>::getKeys(data);
		}
	}
}

map<string, string> PolicyStateLogger::generateDataForOutput(const map<string, map<string, string>>& sampleData)
{
	map<string, string> outputData;
	for (const auto& [policyName, _] : m_expectedColumnNames)
	{
		const auto policyOutputData = createOutputDataForPolicy(policyName, sampleData);
		outputData.insert(policyOutputData.begin(), policyOutputData.end());
	}
	return outputData;
}

void PolicyStateLogger::writeToFile(const map<string, string>& sampleData) const
{
	createLogFolderIfNotPresent(m_manager->getDttLogDirectoryPath());
	writeHeader(sampleData);
	writeRow(sampleData);
}

void PolicyStateLogger::queueNextLoggerUpdate()
{
	const auto loggerWorkItem = make_shared<WIDptfPolicyStateLogger>(m_manager);
	m_manager->getWorkItemQueueManager()->enqueueDeferredWorkItem(
		loggerWorkItem, m_pollingPeriod - pollingPeriodOffset);
}

map<string, string> PolicyStateLogger::createOutputDataForPolicy(
	const string& policyName,
	const map<string, map<string, string>>& sampleData) const
{
	map<string, string> outputData;
	try
	{
		const auto policySampleData = getDataForPolicy(policyName, sampleData);
		const auto columnNames = MapOps<string, string>::getKeys(policySampleData);
		throwIfColumnNamesDoNotMatchExpectations(policyName, columnNames);
		const auto updatedLogData = generateUpdatedColumnNames(policySampleData, policyName);
		outputData.insert(updatedLogData.begin(), updatedLogData.end());
	}
	catch (...)
	{
		const auto emptyValues = generateEmptyValues(policyName);
		const auto updatedLogData = generateUpdatedColumnNames(emptyValues, policyName);
		outputData.insert(updatedLogData.begin(), updatedLogData.end());
	}
	return outputData;
}

map<string, string> PolicyStateLogger::getDataForPolicy(
	const string& policyName,
	const map<string, map<string, string>>& sampleData)
{
	const auto entry = sampleData.find(policyName);
	if (entry == sampleData.end())
	{
		throw dptf_exception("Expected policy not found in sample data"s);
	}
	const auto [_, data] = *entry;
	return data;
}

map<string, string> PolicyStateLogger::generateUpdatedColumnNames(
	const map<string, string>& logData,
	const string& policyName)
{
	map<string, string> updatedLogData;
	for (const auto& [columnName, value] : logData)
	{
		stringstream stream;
		stream << policyName << columnNamePartition << columnName;
		updatedLogData.insert({stream.str(), value});
	}
	return updatedLogData;
}

map<string, string> PolicyStateLogger::generateEmptyValues(const string& policyName) const
{
	const auto entry = m_expectedColumnNames.find(policyName);
	if (entry == m_expectedColumnNames.end())
	{
		return {};
	}

	const auto& [_, columnNames] = *entry;
	map<string, string> emptyValues;
	for (const auto& columnName : columnNames)
	{
		emptyValues[columnName] = ""s;
	}
	return emptyValues;
}

vector<string> PolicyStateLogger::cleanPolicyNames(const vector<string>& policyNames)
{
	vector<string> policyNamesWithNoSpaces;
	policyNamesWithNoSpaces.resize(policyNames.size());
	transform(policyNames.begin(), policyNames.end(), policyNamesWithNoSpaces.begin(), StringConverter::trimWhitespace);
	set policySet(policyNamesWithNoSpaces.begin(), policyNamesWithNoSpaces.end());
	return {policyNamesWithNoSpaces.begin(), policyNamesWithNoSpaces.end()};
}

void PolicyStateLogger::removeFromWorkItemQueue() const
{
	try
	{
		WorkItemMatchCriteria criteria;
		criteria.addFrameworkEventTypeToMatchList(FrameworkEvent::DptfPolicyStateLogger);
		m_manager->getWorkItemQueueManager()->removeIfMatches(criteria);
	}
	catch (...)
	{

	}
}

string PolicyStateLogger::captureDataForHeader(const map<string, string>& content)
{
	stringstream compiledData;
	compiledData << columnTitleTimestamp << itemSeparator;
	for (const auto& [key, value] : content)
	{
		compiledData << key << itemSeparator;
	}
	string result = StringParser::removeLastCharacter(compiledData.str());
	result.append(newLine);
	return result;
}

string PolicyStateLogger::captureDataForRow(const map<string, string>& content) const
{
	stringstream compiledData;
	compiledData << generateTimeStamp() << itemSeparator;
	for (const auto& [key, value] : content)
	{
		compiledData << value << itemSeparator;
	}
	string result = StringParser::removeLastCharacter(compiledData.str());
	result.append(newLine);
	return result;
}

string PolicyStateLogger::generateTimeStamp() const
{
	return m_timeStampGenerator->generateAsHumanFriendlyString();
}

string PolicyStateLogger::driverDataFilePath(const string& fileName) const
{
	const auto exportPath = m_manager->getDttLogDirectoryPath();
	return FileIo::generatePathWithTrailingSeparator(exportPath) + fileName;
}

void PolicyStateLogger::createLogFolderIfNotPresent(const string& directoryPath) const
{
	if (!m_fileIo->pathExists(directoryPath))
	{
		m_fileIo->createDirectoryPath(directoryPath);
	}
}

void PolicyStateLogger::writeHeader(const map<string, string>& content) const
{
	if (!m_fileIo->pathExists(driverDataFilePath(m_logFileName)))
	{
		const auto dataHeader = captureDataForHeader(content);
		m_fileIo->append(driverDataFilePath(m_logFileName), dataHeader);
	}
}

void PolicyStateLogger::writeRow(const map<string, string>& content) const
{
	const auto data = captureDataForRow(content);
	m_fileIo->append(driverDataFilePath(m_logFileName), data);
}

void PolicyStateLogger::setActive()
{
	m_isActive = true;
}

void PolicyStateLogger::setInactive()
{
	m_isActive = false;
}

void PolicyStateLogger::throwIfInvalidArguments(
	const vector<string>& policyNames,
	const string& logFileName,
	const TimeSpan& pollingPeriod) const
{
	throwIfInvalidPolicyNames(policyNames);
	throwIfInvalidLogFileName(logFileName);
	throwIfInvalidPollingPeriod(pollingPeriod);
}

void PolicyStateLogger::throwIfInvalidPolicyNames(const vector<string>& policyNames) const
{
	const auto policyManager = m_manager->getPolicyManager();
	for (const auto& policyName : policyNames)
	{
		if (!policyManager->policyExists(policyName))
		{
			throw command_failure(
				ESIF_E_INVALID_REQUEST_TYPE,
				"Given policy is either invalid or not active: "s + "'"s + policyName + "'"s);
		}
	}
}

void PolicyStateLogger::throwIfInvalidLogFileName(const string& logFileName) const
{
	if (IFileIo::fileNameContainsIllegalCharacters(logFileName))
	{
		throw command_failure(ESIF_E_INVALID_REQUEST_TYPE, "Invalid character used in filename: "s + logFileName);
	}

	if (m_fileIo->pathExists(driverDataFilePath(logFileName)))
	{
		throw command_failure(ESIF_E_INVALID_REQUEST_TYPE, "File already exists: "s + logFileName);
	}
}

void PolicyStateLogger::throwIfInvalidPollingPeriod(const TimeSpan& pollingPeriod)
{
	if (isSamplePeriodInvalid(pollingPeriod))
	{
		throw command_failure(
			ESIF_E_INVALID_REQUEST_TYPE,
			"Given sample period must lie within "s + minPollingPeriod.toStringMilliseconds() + " and "s
			+ maxPollingPeriod.toStringMilliseconds() + " milliseconds."s);
	}
}

Bool PolicyStateLogger::isSamplePeriodInvalid(const TimeSpan& pollingPeriod)
{
	if (pollingPeriod > maxPollingPeriod || pollingPeriod < minPollingPeriod)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void PolicyStateLogger::throwIfColumnNamesDoNotMatchExpectations(
	const string& policyName,
	const set<string>& columnNames) const
{
	if (!columnNamesMatchExpectedVersion(policyName, columnNames))
	{
		throw dptf_exception("Column names do not match expected version for policy "s + policyName);
	}
}

bool PolicyStateLogger::columnNamesMatchExpectedVersion(const string& policyName, const set<string>& columnNames) const
{
	if (m_expectedColumnNames.empty())
	{
		return true;
	}

	const auto entry = m_expectedColumnNames.find(policyName);
	if (entry == m_expectedColumnNames.end())
	{
		return false;
	}

	const auto& [expectedPolicyName, expectedColumnNames] = *entry;
	return columnNames == expectedColumnNames;
}

void PolicyStateLogger::throwIfAlreadyActive() const
{
	if (isActive())
	{
		throw dptf_exception("Policy Logger already active."s);
	}
}

void PolicyStateLogger::throwIfNotActive() const
{
	if (!isActive())
	{
		throw dptf_exception("Policy Logger is not currently active."s);
	}
}