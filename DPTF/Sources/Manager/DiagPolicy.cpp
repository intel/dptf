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
#include "DiagPolicyCommand.h"
#include "DptfManagerInterface.h"
#include "PolicyManagerInterface.h"
#include "FileIo.h"
#include "EsifDataString.h"
using namespace std;

DiagPolicyCommand::DiagPolicyCommand(
	DptfManagerInterface* dptfManager,
	const shared_ptr<IFileIo>& fileIo,
	const shared_ptr<TimeStampGenerator>& timeStampGenerator)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
	, m_timeStampGenerator(timeStampGenerator)
{
}

string DiagPolicyCommand::getCommandName() const
{
	return "policy";
}

void DiagPolicyCommand::execute(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);
	throwIfPolicyNotExist(arguments);
	throwIfReportNameIsInvalid(arguments);

	const auto diagnostics = getPolicyDiagnosticReport(arguments);
	const auto fullReportPath = generateReportPath(arguments);
	const string message = string("Wrote policy diagnostics to "s) + fullReportPath;
	m_fileIo->write(fullReportPath, diagnostics);
	setResultCode(ESIF_OK);
	setResultMessage(message);
}

string DiagPolicyCommand::getPolicyDiagnosticReport(const CommandArguments& arguments) const
{
	const auto policyName = arguments[1].getDataAsString();
	const auto policy = m_dptfManager->getPolicyManager()->getPolicy(policyName);
	return policy->getDiagnosticsAsXml();
}

string DiagPolicyCommand::generateReportPath(const CommandArguments& arguments) const
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

Bool DiagPolicyCommand::reportNameProvided(const CommandArguments& arguments)
{
	return arguments.size() > 2;
}

void DiagPolicyCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		const string description = {
			"Invalid argument count given to 'diag policy' command.  "s +
			"Run 'dptf help' command for more information."s};
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if ((arguments[0].isDataTypeString() == false) || (arguments[1].isDataTypeString() == false))
	{
		const string description = {
			"Invalid argument type given to 'diag policy' command.  "s +
			"Run 'dptf help' command for more information."s};
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void DiagPolicyCommand::throwIfPolicyNotExist(const CommandArguments& arguments)
{
	const auto policyExists = m_dptfManager->getPolicyManager()->policyExists(arguments[1].getDataAsString());
	if (policyExists == false)
	{
		const auto description = string("The policy specified was not found."s);
		setResultMessage(description);
		throw command_failure(ESIF_E_NOT_FOUND, description);
	}
}

void DiagPolicyCommand::throwIfReportNameIsInvalid(const CommandArguments& arguments)
{
	if (arguments.size() > 2)
	{
		if (arguments[2].isDataTypeString() == false)
		{
			const auto description = string("Invalid argument type given for report name."s);
			setResultMessage(description);
			throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
		}

		const auto reportName = arguments[2].getDataAsString();
		if (IFileIo::fileNameContainsIllegalCharacters(reportName))
		{
			const auto description = string("Invalid characters used in report name given to 'diag policy' command."s);
			setResultMessage(description);
			throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
		}
	}
}
