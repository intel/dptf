/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#include "TimeOps.h"
#include "StringParser.h"
#include "EsifDataString.h"
using namespace std;

DiagPolicyCommand::DiagPolicyCommand(DptfManagerInterface* dptfManager, shared_ptr<IFileIo> fileIo)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
{
}

DiagPolicyCommand::~DiagPolicyCommand()
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

	auto diagnostics = getPolicyDiagnosticReport(arguments);
	auto fullReportPath = generateReportPath(arguments);
	string message = string("Wrote policy diagnostics to ") + fullReportPath;
	m_fileIo->write(fullReportPath, diagnostics);
	setResultCode(ESIF_OK);
	setResultMessage(message);
}

string DiagPolicyCommand::getPolicyDiagnosticReport(const CommandArguments& arguments)
{
	auto policyName = arguments[1].getDataAsString();
	auto policy = m_dptfManager->getPolicyManager()->getPolicy(policyName);
	return policy->getDiagnosticsAsXml();
}

string DiagPolicyCommand::generateReportPath(const CommandArguments& arguments)
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
		string description = string(
			"Invalid argument count given to 'diag policy' command.  "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if ((arguments[0].isDataTypeString() == false) || (arguments[1].isDataTypeString() == false))
	{
		string description = string(
			"Invalid argument type given to 'diag policy' command.  "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void DiagPolicyCommand::throwIfPolicyNotExist(const CommandArguments& arguments)
{
	auto policyExists = m_dptfManager->getPolicyManager()->policyExists(arguments[1].getDataAsString());
	if (policyExists == false)
	{
		string description = string("The policy specified was not found.");
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
			string description = string("Invalid argument type given for report name.");
			setResultMessage(description);
			throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
		}

		auto reportName = arguments[2].getDataAsString();
		if (IFileIo::fileNameContainsIllegalCharacters(reportName))
		{
			string description = string("Invalid characters used in report name given to 'diag policy' command.");
			setResultMessage(description);
			throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
		}
	}
}
