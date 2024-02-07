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

#pragma once
#include "IApplicationTimerSettings.h"
#include "EventObserverInterface.h"
#include "DptfManagerInterface.h"
#include "FileIo.h"
#include "TimeStampGenerator.h"

class dptf_export IPolicyStateLogger : public EventObserverInterface
{
public:
	virtual void start(
		const std::vector<std::string>& policies,
		const std::string& logFileName,
		const TimeSpan& pollingPeriod) = 0;
	virtual void stop() = 0;
	void update(FrameworkEvent::Type event, const DptfBuffer& eventPayload) override = 0;
	virtual std::string getStatus() const = 0;
	virtual bool isActive() const = 0;
};

class dptf_export PolicyStateLogger : public IPolicyStateLogger
{
public:
	PolicyStateLogger(
		DptfManagerInterface* manager,
		const std::shared_ptr<IFileIo>& fileIo,
		const std::shared_ptr<TimeStampGenerator>& timeStampGenerator,
		const std::shared_ptr<IApplicationTimerSettings>& applicationTimerSettings);
	void start(
		const std::vector<std::string>& policyNames,
		const std::string& logFileName,
		const TimeSpan& pollingPeriod) override;
	void stop() override;
	void update(FrameworkEvent::Type event, const DptfBuffer& eventPayload) override;
	std::string getStatus() const override;
	bool isActive() const override;

private:
	DptfManagerInterface* m_manager;
	std::shared_ptr<IFileIo> m_fileIo;
	std::shared_ptr<TimeStampGenerator> m_timeStampGenerator;
	std::shared_ptr<IApplicationTimerSettings> m_applicationTimerSettings;
	Bool m_isActive;

	std::vector<std::string> m_policies;
	std::string m_logFileName;
	TimeSpan m_pollingPeriod;

	std::map<std::string, std::set<std::string>> m_expectedColumnNames;

	std::map<std::string, std::map<std::string, std::string>> readPolicyStateLogData() const;
	void setExpectedColumnNamesIfNeeded(const std::map<std::string, std::map<std::string, std::string>>& sampleData);
	static std::map<std::string, std::string> getDataForPolicy(
		const std::string& policyName,
		const std::map<std::string, std::map<std::string, std::string>>& sampleData);
	std::map<std::string, std::string> createOutputDataForPolicy(
		const std::string& policyName,
		const std::map<std::string, std::map<std::string, std::string>>& sampleData) const;
	std::map<std::string, std::string> generateDataForOutput(
		const std::map<std::string, std::map<std::string, std::string>>& sampleData);
	void removeFromWorkItemQueue() const;
	static std::map<std::string, std::string> generateUpdatedColumnNames(
		const std::map<std::string, std::string>& logData,
		const std::string& policyName);
	bool columnNamesMatchExpectedVersion(
		const std::string& policyName,
		const std::set<std::string>& columnNames) const;
	std::map<std::string, std::string> generateEmptyValues(
		const std::string& policyName) const;
	static std::string captureDataForHeader(const std::map<std::string, std::string>& content);
	std::string generateTimeStamp() const;
	std::string captureDataForRow(const std::map<std::string, std::string>& content) const;
	std::string driverDataFilePath(const std::string& fileName) const;
	void writeToFile(const std::map<std::string, std::string>& sampleData) const;
	void writeHeader(const std::map<std::string, std::string>& content) const;
	void writeRow(const std::map<std::string, std::string>& content) const;
	void queueNextLoggerUpdate();
	void setActive();
	void setInactive();
	std::string generateActiveStatusMessage() const;
	static std::string generateInactiveStatusMessage();
	void createLogFolderIfNotPresent(const std::string& directoryPath) const;
	void throwIfAlreadyActive() const;
	void throwIfNotActive() const;
	void throwIfInvalidArguments(
		const std::vector<std::string>& policyNames,
		const std::string& logFileName,
		const TimeSpan& pollingPeriod) const;
	void throwIfInvalidLogFileName(const std::string& logFileName) const;
	static void throwIfInvalidPollingPeriod(const TimeSpan& pollingPeriod);
	void throwIfInvalidPolicyNames(const std::vector<std::string>& policyNames) const;
	static bool isSamplePeriodInvalid(const TimeSpan& pollingPeriod);
	static std::vector<std::string> cleanPolicyNames(const std::vector<std::string>& policyNames);
	void throwIfColumnNamesDoNotMatchExpectations(
		const std::string& policyName,
		const std::set<std::string>& columnNames) const;
};
