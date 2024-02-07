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
#include "CaptureCommand.h"
#include "DptfManagerInterface.h"
#include "ParticipantCaptureDataGenerator.h"
#include "ParticipantListCaptureDataGenerator.h"
#include "PolicyListCaptureDataGenerator.h"
#include "WorkItemQueueManagerInterface.h"
#include "RealTimeStampGenerator.h"
#include "PolicyTablesCaptureDataGenerator.h"

using namespace std;

CaptureCommand::CaptureCommand(
	DptfManagerInterface* dptfManager,
	const shared_ptr<IFileIo>& fileIo,
	const shared_ptr<TimeStampGenerator>& timeStampGenerator)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
	, m_timeStampGenerator(timeStampGenerator)
{
	m_dataGenerators.emplace_back(make_shared<PolicyListCaptureDataGenerator>(dptfManager));
	m_dataGenerators.emplace_back(make_shared<ParticipantListCaptureDataGenerator>(dptfManager));
	m_dataGenerators.emplace_back(make_shared<ParticipantCaptureDataGenerator>(dptfManager));
	m_dataGenerators.emplace_back(make_shared<PolicyTablesCaptureDataGenerator>(dptfManager));
}

string CaptureCommand::getCommandName() const
{
	return "capture";
}

void CaptureCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfBadArgumentCount(arguments);
		throwIfBadFileNameGiven(arguments);
		const auto exportFilePath = generateExportPath(arguments);
		const auto captureData = generateCaptureData();
		m_fileIo->write(exportFilePath, captureData);
		setResultCode(ESIF_OK);
		setResultMessage(string("Successfully exported settings to " + exportFilePath));
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}

string CaptureCommand::generateExportPath(const CommandArguments& arguments) const
{
	const auto exportPath = m_dptfManager->getDttLogDirectoryPath();
	const auto exportFileName = getExportFileName(arguments);
	return FileIo::generatePathWithTrailingSeparator(exportPath) + exportFileName;
}

Bool CaptureCommand::exportFileNameProvided(const CommandArguments& arguments)
{
	return (arguments.size() > 1) && (arguments[1].isDataTypeString());
}

string CaptureCommand::getExportFileName(const CommandArguments& arguments) const
{
	string exportFileName = Constants::EmptyString;

	if (exportFileNameProvided(arguments))
	{
		exportFileName = arguments[1].getDataAsString();
	}
	else
	{
		exportFileName = m_timeStampGenerator->generateAsString() + ".txt";
	}

	return exportFileName;
}

string CaptureCommand::generateCaptureData() const
{
	stringstream compiledData;
	for (const auto& dataGenerator : m_dataGenerators)
	{
		compiledData << dataGenerator->generate()->toString();
	}
	return compiledData.str();
}

void CaptureCommand::throwIfBadArgumentCount(const CommandArguments& arguments)
{
	// capture <optional file name>
	if (arguments.size() > 2)
	{
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, "Invalid argument count.");
	}
}

void CaptureCommand::throwIfBadFileNameGiven(const CommandArguments& arguments)
{
	// capture <optional file name>
	if ((arguments.size() == 2) && (IFileIo::fileNameContainsIllegalCharacters(arguments[1].getDataAsString())))
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Invalid file name given.");
	}
}
