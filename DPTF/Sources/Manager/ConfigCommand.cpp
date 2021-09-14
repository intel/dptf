/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
}

void ConfigCommand::registerSubCommands()
{
	for (auto c = m_subCommands.begin(); c != m_subCommands.end(); ++c)
	{
		m_configCommandDispatcher->registerHandler((*c)->getCommandName(), *c);
	}
}

string ConfigCommand::getCommandName() const
{
	return "config";
}

void ConfigCommand::execute(const CommandArguments& arguments)
{
	throwIfInvalidArgumentCount(arguments);
	throwIfInvalidArgumentData(arguments);
	throwIfInvalidCommand(arguments);

	CommandArguments subArguments = arguments;
	subArguments.remove(0);
	m_configCommandDispatcher->dispatch(subArguments);
	setResultMessage(m_configCommandDispatcher->getLastSuccessfulCommandMessage());
}

void ConfigCommand::throwIfInvalidCommand(const CommandArguments& arguments)
{
	auto subCommandText = arguments[1].getDataAsString();
	Bool commandExists = false;
	for (auto it = m_subCommands.begin(); it != m_subCommands.end(); ++it)
	{
		if (subCommandText == (*it)->getCommandName())
		{
			commandExists = true;
			break;
		}
	}
	if (commandExists == false)
	{
		string description = string("Subcommand given for 'config' command not found.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void ConfigCommand::throwIfInvalidArgumentData(const CommandArguments& arguments)
{
	if (arguments[1].isDataTypeString() == false)
	{
		string description = string("Invalid argument type given to 'config' command. Expected a string.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void ConfigCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		string description = string(
			"Invalid argument count given to 'config' command. "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}
