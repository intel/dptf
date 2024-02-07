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
#include "ExportConfigCommand.h"
#include "PolicyInterface.h"
#include "StringParser.h"

#define EXPORT_COMMAND_NAME "config"s
namespace ArgumentIndex
{
	enum ArgumentIndexEnum
	{
		CommandName = 0, 
		PolicyName = 1,
		FileName = 2,
		TotalArguments
	};
}

using namespace std;

ExportConfigCommand::ExportConfigCommand(DptfManagerInterface* dptfManager, const shared_ptr<IFileIo>& fileIo)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
{
}

ExportConfigCommand::~ExportConfigCommand()
{
}

string ExportConfigCommand::getCommandName() const
{
	return EXPORT_COMMAND_NAME;
}

void ExportConfigCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfInvalidArguments(arguments);
		exportPolicyConfiguration(
			arguments[ArgumentIndex::PolicyName].getDataAsString(),
			arguments[ArgumentIndex::FileName].getDataAsString());
		setResultCode(ESIF_OK);
		setResultMessage(getResultString(arguments[ArgumentIndex::FileName].getDataAsString()));
	}
	catch (command_failure& ex)
	{
		setResultCode(ex.getErrorCode());
		setResultMessage("Export Failed: " + ex.getDescription());
	}
}

void ExportConfigCommand::exportPolicyConfiguration(const string& policyName, const string& fileName) const
{
	try
	{
		const auto policyManager = m_dptfManager->getPolicyManager();
		const auto policy = policyManager->getPolicy(policyName);
		const auto jsonString = policy->getConfigurationForExport();
		m_fileIo->write(driverDataFilePath(fileName), jsonString);
	}
	catch (const dptf_exception& ex)
	{
		throw command_failure(ESIF_E_INVALID_REQUEST_TYPE, ex.getDescription());
	}
}

string ExportConfigCommand::getResultString(const string& fileName) const
{
return "Export Successful! Config file can be found at: "
		   + driverDataFilePath(fileName);
}

void ExportConfigCommand::throwIfInvalidArguments(const CommandArguments& arguments) const
{
	throwIfInvalidArgumentCount(arguments);
	throwIfInvalidArgumentData(arguments);
	throwIfInvalidArgumentSubData(arguments);
	throwIfInvalidCommand(arguments);
	throwIfInvalidSubCommand(arguments);
}

void ExportConfigCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() != ArgumentIndex::TotalArguments)
	{
		const auto description = R"(
			Invalid argument count given to 'export config' command. Run 'dptf help' command for more information.)";
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}

void ExportConfigCommand::throwIfInvalidArgumentData(const CommandArguments& arguments)
{
	if (arguments[ArgumentIndex::CommandName].isDataTypeString() == false)
	{
		const auto description = "Invalid argument type given to 'export config' command. Expected a string."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void ExportConfigCommand::throwIfInvalidArgumentSubData(const CommandArguments& arguments)
{
	if (arguments[ArgumentIndex::PolicyName].isDataTypeString() == false
		|| arguments[ArgumentIndex::FileName].isDataTypeString() == false)
	{
		const auto description =
			"Invalid sub-command argument type given to 'export config' command. Expected a string."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void ExportConfigCommand::throwIfInvalidCommand(const CommandArguments& arguments) const
{
	if (arguments[ArgumentIndex::CommandName].getDataAsString() != getCommandName())
	{
		const auto description = "Invalid command given for 'export config', command name is different from supplied command."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void ExportConfigCommand::throwIfInvalidSubCommand(const CommandArguments& arguments) const
{
	throwIfInvalidPolicyName(arguments[ArgumentIndex::PolicyName].getDataAsString());
	throwIfInvalidFileNameArgument(arguments[ArgumentIndex::FileName].getDataAsString());
}

void ExportConfigCommand::throwIfInvalidPolicyName(const string& policyName) const
{
	if (!m_dptfManager->getPolicyManager()->policyExists(policyName))
	{
		throw command_failure(
			ESIF_E_INVALID_REQUEST_TYPE, "Given policy " + policyName + " is not active"s);
	}
}

void ExportConfigCommand::throwIfInvalidFileNameArgument(const string& fileName) const
{
	if (IFileIo::fileNameContainsIllegalCharacters(fileName))
	{
		throw command_failure(
			ESIF_E_INVALID_REQUEST_TYPE, "Invalid character used in filename: "s + fileName);
	}

	if (m_fileIo->pathExists(driverDataFilePath(fileName)))
	{
		throw command_failure(
			ESIF_E_INVALID_REQUEST_TYPE,
			"The file already exists at '"s + driverDataFilePath(fileName) + "'"s);
	}
}

string ExportConfigCommand::driverDataFilePath(const string& fileName) const
{
	const auto exportPath = m_dptfManager->getDttLogDirectoryPath();
	return FileIo::generatePathWithTrailingSeparator(exportPath) + fileName;
}