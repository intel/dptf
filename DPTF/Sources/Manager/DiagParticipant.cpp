/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "DiagParticipantCommand.h"
#include "FileIO.h"
#include "DptfManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "TimeOps.h"

using namespace std;

DiagParticipantCommand::DiagParticipantCommand(DptfManagerInterface* dptfManager, shared_ptr<IFileIO> fileIo)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
{
}

DiagParticipantCommand::~DiagParticipantCommand()
{
}

string DiagParticipantCommand::getCommandName() const
{
	return "part";
}

void DiagParticipantCommand::execute(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);
	throwIfParticipantNotExist(arguments);
	throwIfReportNameIsInvalid(arguments);

	auto diagnostics = getParticipantDiagnosticReport(arguments);
	auto fullReportPath = generateReportPath(arguments);
	m_fileIo->writeData(fullReportPath, diagnostics);
}

string DiagParticipantCommand::getParticipantDiagnosticReport(const CommandArguments& arguments)
{
	auto participantName = arguments[1].getDataAsString();
	auto participant = m_dptfManager->getParticipantManager()->getParticipant(participantName);
	return participant->getDiagnosticsAsXml();
}

string DiagParticipantCommand::generateReportPath(const CommandArguments& arguments)
{
	auto reportPath = m_dptfManager->getDptfReportDirectoryPath();
	string reportName;
	if (reportNameProvided(arguments))
	{
		reportName = arguments[2].getDataAsString();
	}
	else
	{
		reportName = TimeOps::generateTimestampNowAsString() + ".xml";
	}

	return FileIO::generatePathWithTrailingSeparator(reportPath) + reportName;
}

Bool DiagParticipantCommand::reportNameProvided(const CommandArguments& arguments) const
{
	return (arguments.size() > 2) && (arguments[2].isDataTypeString());
}

void DiagParticipantCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		string description = string("Invalid argument count given to 'diag participant' command.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if ((arguments[0].isDataTypeString() == false) || (arguments[1].isDataTypeString() == false))
	{
		string description = string("Invalid argument type given.  Expected a string.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void DiagParticipantCommand::throwIfParticipantNotExist(const CommandArguments& arguments)
{
	auto participantExists = m_dptfManager->getParticipantManager()->participantExists(arguments[1].getDataAsString());
	if (participantExists == false)
	{
		string description = string("The participant specified was not found.");
		setResultMessage(description);
		throw command_failure(ESIF_E_NOT_FOUND, description);
	}
}

void DiagParticipantCommand::throwIfReportNameIsInvalid(const CommandArguments& arguments)
{
	if (arguments.size() > 2)
	{
		if (arguments[2].isDataTypeString() == false)
		{
			string description = string("Invalid argument type given for report name.  Expected a string.");
			setResultMessage(description);
			throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
		}

		auto reportName = arguments[2].getDataAsString();
		if (IFileIO::fileNameHasIllegalChars(reportName))
		{
			string description = string("Invalid characters used in report name given.");
			setResultMessage(description);
			throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
		}
	}
}
