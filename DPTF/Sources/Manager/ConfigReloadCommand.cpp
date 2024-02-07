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
#include "ConfigReloadCommand.h"
#include "DptfManagerInterface.h"

using namespace std;

ConfigReloadCommand::ConfigReloadCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

ConfigReloadCommand::~ConfigReloadCommand()
{
}

string ConfigReloadCommand::getCommandName() const
{
	return "reload";
}

void ConfigReloadCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfBadArguments(arguments);
		reloadConfigurations();
		setResultMessage("Reload complete"s);
		setResultCode(ESIF_OK);
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}

void ConfigReloadCommand::reloadConfigurations() const
{
	throwIfInvalidConfigurationManager(m_dptfManager->getConfigurationManager());
	const auto configManager = m_dptfManager->getConfigurationManager();
	configManager->loadFiles();
}

void ConfigReloadCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() != 1)
	{
		const string description = string(
			"Invalid argument count given to 'config reload' command. "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}

void ConfigReloadCommand::throwIfInvalidConfigurationManager(
	const shared_ptr<ConfigurationFileManagerInterface>& configurationManager) const
{
	if (!configurationManager)
	{
		throw logic_error("Invalid configuration manager");
	}
}