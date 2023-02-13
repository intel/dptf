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
#include "ConfigCommand.h"
#include "ConfigDeleteCommand.h"
#include "ConfigFilterDbCommand.h"
#include "ConfigListCommand.h"
#include "ConfigPrintCommand.h"
#include "ConfigPrintDbCommand.h"
#include "ConfigReloadCommand.h"
using namespace std;

ConfigCommand::ConfigCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
	m_configCommandDispatcher = make_shared<CommandDispatcher>();
	createSubCommands();
	registerSubCommands();
}

ConfigCommand::~ConfigCommand()
{
	m_subCommands.clear();
}

void ConfigCommand::createSubCommands()
{
	m_subCommands.push_back(make_shared<ConfigDeleteCommand>(m_dptfManager));
	m_subCommands.push_back(make_shared<ConfigListCommand>(m_dptfManager));
	m_subCommands.push_back(make_shared<ConfigPrintCommand>(m_dptfManager));
	m_subCommands.push_back(make_shared<ConfigPrintDbCommand>(m_dptfManager));
	m_subCommands.push_back(make_shared<ConfigFilterDbCommand>(m_dptfManager));
	m_subCommands.push_back(make_shared<ConfigReloadCommand>(m_dptfManager));
}

void ConfigCommand::registerSubCommands() const
{
	for (const auto& subCommand : m_subCommands)
	{
		m_configCommandDispatcher->registerHandler(subCommand->getCommandName(), subCommand);
	}
}

string ConfigCommand::getCommandName() const
{
	return "config";
}

void ConfigCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfInvalidArgumentCount(arguments);
		throwIfInvalidArgumentData(arguments);
		throwIfInvalidCommand(arguments);

		CommandArguments subArguments = arguments;
		subArguments.remove(0);
		m_configCommandDispatcher->dispatch(subArguments);

		setResultCode(ESIF_OK);
		setResultMessage(m_configCommandDispatcher->getLastSuccessfulCommandMessage());
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}

void ConfigCommand::throwIfInvalidCommand(const CommandArguments& arguments) const
{
	const auto subCommandText = arguments[1].getDataAsString();
	Bool commandExists = false;
	for (const auto& subCommand : m_subCommands)
	{
		if (subCommandText == subCommand->getCommandName())
		{
			commandExists = true;
			break;
		}
	}
	if (commandExists == false)
	{
		const auto description = "Subcommand given for 'config' command not found."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void ConfigCommand::throwIfInvalidArgumentData(const CommandArguments& arguments)
{
	if (arguments[1].isDataTypeString() == false)
	{
		const auto description = "Invalid argument type given to 'config' command. Expected a string."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void ConfigCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		const auto description = R"(\
			Invalid argument count given to 'config' command. Run 'dptf help' command for more information.)";
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}
