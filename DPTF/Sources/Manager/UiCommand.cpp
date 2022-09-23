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
#include "UiCommand.h"
#include "PolicyManagerInterface.h"
#include "CommandDispatcher.h"
#include "UiGetGroupsCommand.h"
#include "UiGetModuleDataCommand.h"
#include "UiGetModulesInGroupCommand.h"

using namespace std;

UiCommand::UiCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
	m_commandDispatcher = make_shared<CommandDispatcher>();
	createSubCommands();
	registerSubCommands();
}

UiCommand::~UiCommand()
{
	m_subCommands.clear();
}

void UiCommand::createSubCommands()
{
	m_subCommands.push_back(make_shared<UiGetGroupsCommand>(m_dptfManager));
	m_subCommands.push_back(make_shared<UiGetModulesInGroupCommand>(m_dptfManager));
	m_subCommands.push_back(make_shared<UiGetModuleDataCommand>(m_dptfManager));
}

void UiCommand::registerSubCommands()
{
	for (auto c = m_subCommands.begin(); c != m_subCommands.end(); ++c)
	{
		m_commandDispatcher->registerHandler((*c)->getCommandName(), *c);
	}
}

string UiCommand::getCommandName() const
{
	return "ui";
}

void UiCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfInvalidArgumentCount(arguments);
		throwIfInvalidArgumentData(arguments);
		throwIfInvalidCommand(arguments);
		CommandArguments subArguments = arguments;
		subArguments.remove(0);
		m_commandDispatcher->dispatch(subArguments);
		setResultCode(m_commandDispatcher->getLastReturnCode());
		setResultMessage(m_commandDispatcher->getLastSuccessfulCommandMessage());
	}
	catch (const command_failure& cf) 
	{
		setResultCode(cf.getErrorCode());
		setResultMessage(cf.getDescription());
		throw cf;
	}
}

void UiCommand::throwIfInvalidCommand(const CommandArguments& arguments) const
{
	const auto subCommandText = arguments[1].getDataAsString();
	if (!m_commandDispatcher->isCommandRegistered(subCommandText))
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Subcommand given for ui command not found.");
	}
}

void UiCommand::throwIfInvalidArgumentData(const CommandArguments& arguments)
{
	if (arguments[1].isDataTypeString() == false)
	{
		throw command_failure(
			ESIF_E_COMMAND_DATA_INVALID, "Invalid argument type given to 'ui' command.  Expected a string.");
	}
}

void UiCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	// arguments[0] = 'ui', arguments[1] = the intended sub command.  So expect minimum of 2 arguments.
	if (arguments.size() < 2)
	{
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, "Too few arguments: expected a valid sub-command but none given.");
	}
}
