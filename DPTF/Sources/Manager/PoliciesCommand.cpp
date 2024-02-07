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
#include "PoliciesCommand.h"
#include "PoliciesEnabledCommand.h"
#include <algorithm>

using namespace std;

PoliciesCommand::PoliciesCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
	m_configCommandDispatcher = make_shared<CommandDispatcher>();
	createSubCommands();
	registerSubCommands();
}

PoliciesCommand::~PoliciesCommand()
{
	m_subCommands.clear();
}

void PoliciesCommand::createSubCommands()
{
	m_subCommands.push_back(make_shared<PoliciesEnabledCommand>(m_dptfManager));
}

void PoliciesCommand::registerSubCommands() const
{
	for (const auto& subCommand : m_subCommands)
	{
		m_configCommandDispatcher->registerHandler(subCommand->getCommandName(), subCommand);
	}
}

string PoliciesCommand::getCommandName() const
{
	return "policies";
}

void PoliciesCommand::execute(const CommandArguments& arguments)
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

bool PoliciesCommand::subCommandExists(const string& subCommandText) const
{
	return any_of(
		m_subCommands.begin(),
		m_subCommands.end(),
		[subCommandText](const shared_ptr<CommandHandler>& h) { return subCommandText == h->getCommandName(); });
}

void PoliciesCommand::throwIfInvalidCommand(const CommandArguments& arguments) const
{
	const auto subCommandText = arguments[1].getDataAsString();
	if (!subCommandExists(subCommandText))
	{
		stringstream description;
		description << "Subcommand "s << subCommandText << " given to"s;
		description << " "s << getCommandName() << " "s;
		description << "command not found. Run 'dptf help' command for more information."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description.str());
	}
}

void PoliciesCommand::throwIfInvalidArgumentData(const CommandArguments& arguments) const
{
	if (arguments[1].isDataTypeString() == false)
	{
		stringstream description;
		description << "Invalid argument type given to"s;
		description << " "s << getCommandName() << " "s;
		description << "command. Run 'dptf help' command for more information."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description.str());
	}
}

void PoliciesCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments) const
{
	if (arguments.size() < 2)
	{
		stringstream description;
		description << "Invalid argument count given to"s;
		description << " "s << getCommandName() << " "s;
		description << "command. Run 'dptf help' command for more information."s;
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description.str());
	}
}
