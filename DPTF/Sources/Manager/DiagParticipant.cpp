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
#include "DiagParticipantCommand.h"
#include "FileIo.h"
#include "DptfManagerInterface.h"
#include "ParticipantManagerInterface.h"

using namespace std;

DiagParticipantCommand::DiagParticipantCommand(
	DptfManagerInterface* dptfManager,
	const shared_ptr<IFileIo>& fileIo,
	const shared_ptr<TimeStampGenerator>& timeStampGenerator)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
	, m_timeStampGenerator(timeStampGenerator)
{
}

string DiagParticipantCommand::getCommandName() const
{
	return "part"s;
}

void DiagParticipantCommand::execute(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);
	throwIfParticipantNotExist(arguments);
	throwIfReportNameIsInvalid(arguments);

	const auto diagnostics = getParticipantDiagnosticReport(arguments);
	const auto fullReportPath = generateReportPath(arguments);
	m_fileIo->write(fullReportPath, diagnostics);
}

string DiagParticipantCommand::getParticipantDiagnosticReport(const CommandArguments& arguments) const
{
	const auto participantName = arguments[1].getDataAsString();
	const auto participant = m_dptfManager->getParticipantManager()->getParticipant(participantName);
	return participant->getDiagnosticsAsXml();
}

string DiagParticipantCommand::generateReportPath(const CommandArguments& arguments) const
{
	const auto reportPath = m_dptfManager->getDttLogDirectoryPath();
	string reportName;
	if (reportNameProvided(arguments))
	{
		reportName = arguments[2].getDataAsString();
	}
	else
	{
		reportName = m_timeStampGenerator->generateAsString() + ".xml"s;
	}

	return FileIo::generatePathWithTrailingSeparator(reportPath) + reportName;
}

Bool DiagParticipantCommand::reportNameProvided(const CommandArguments& arguments)
{
	return (arguments.size() > 2) && (arguments[2].isDataTypeString());
}

void DiagParticipantCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		const auto description = string("Invalid argument count given to 'diag participant' command."s);
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if ((arguments[0].isDataTypeString() == false) || (arguments[1].isDataTypeString() == false))
	{
		const auto description = string("Invalid argument type given.  Expected a string."s);
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void DiagParticipantCommand::throwIfParticipantNotExist(const CommandArguments& arguments)
{
	const auto participantExists = m_dptfManager->getParticipantManager()->participantExists(arguments[1].getDataAsString());
	if (participantExists == false)
	{
		const auto description = string("The participant specified was not found."s);
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
			const auto description = string("Invalid argument type given for report name.  Expected a string."s);
			setResultMessage(description);
			throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
		}

		const auto reportName = arguments[2].getDataAsString();
		if (IFileIo::fileNameContainsIllegalCharacters(reportName))
		{
			const auto description = string("Invalid characters used in report name given."s);
			setResultMessage(description);
			throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
		}
	}
}
