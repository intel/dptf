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
#include "LogPoliciesCommand.h"
#include "LogPolicyStateStartCommand.h"
#include "LogPolicyStateStopCommand.h"
#include "LogPolicyStateStatusCommand.h"

using namespace std;

constexpr auto indexOfCommandName = 0;
constexpr auto indexOfSubCommand = 1;
constexpr size_t minimumArgumentCount{2};
constexpr size_t maximumArgumentCount{5};

LogPoliciesCommand::LogPoliciesCommand(DptfManagerInterface* dptfManager, const shared_ptr<IFileIo>& fileIo)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
{
	m_logPoliciesCommandDispatcher = make_shared<CommandDispatcher>();
	createSubCommands();
	registerSubCommands();
}

LogPoliciesCommand::~LogPoliciesCommand()
{
	m_subCommands.clear();
}

void LogPoliciesCommand::createSubCommands()
{
	m_subCommands.push_back(make_shared<LogPolicyStateStartCommand>(m_dptfManager, m_fileIo));
	m_subCommands.push_back(make_shared<LogPolicyStateStopCommand>(m_dptfManager));
	m_subCommands.push_back(make_shared<LogPolicyStateStatusCommand>(m_dptfManager));
}

void LogPoliciesCommand::registerSubCommands() const
{
	for (const auto& subCommand : m_subCommands)
	{
		m_logPoliciesCommandDispatcher->registerHandler(subCommand->getCommandName(), subCommand);
	}
}

string LogPoliciesCommand::getCommandName() const
{
	return "policies";
}

void LogPoliciesCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfInvalidArguments(arguments);
		executeSubCommand(arguments);
		setResultCode(m_logPoliciesCommandDispatcher->getLastReturnCode());
		setResultMessage(m_logPoliciesCommandDispatcher->getLastSuccessfulCommandMessage());
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage("Command Failed: " + e.getDescription());
	}
}

void LogPoliciesCommand::executeSubCommand(const CommandArguments& arguments) const
{
	CommandArguments subArguments = arguments;
	subArguments.remove(0);
	m_logPoliciesCommandDispatcher->dispatch(subArguments);
}

void LogPoliciesCommand::throwIfInvalidArguments(const CommandArguments& arguments) const
{
	throwIfInvalidArgumentCount(arguments);
	throwIfInvalidArgumentData(arguments);
	throwIfInvalidArgumentSubData(arguments);
	throwIfInvalidCommand(arguments);
	throwIfInvalidSubCommand(arguments);
}

void LogPoliciesCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() < minimumArgumentCount || arguments.size() > maximumArgumentCount)
	{
		const auto description = R"(
			Invalid argument count given to 'log policies' command. Run 'dptf help' command for more information.)";
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}

void LogPoliciesCommand::throwIfInvalidArgumentData(const CommandArguments& arguments)
{
	if (arguments[indexOfCommandName].isDataTypeString() == false)
	{
		const auto description = "Invalid argument type given to 'log policies' command. Expected a string."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void LogPoliciesCommand::throwIfInvalidArgumentSubData(const CommandArguments& arguments)
{
	if (arguments[indexOfSubCommand].isDataTypeString() == false)
	{
		const auto description =
			"Invalid sub-command argument type given to 'log policies' command. Expected a string."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void LogPoliciesCommand::throwIfInvalidCommand(const CommandArguments& arguments) const
{
	if (arguments[indexOfCommandName].getDataAsString() != getCommandName())
	{
		const auto description = "Invalid command given for 'log policies', command name is different from supplied command."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}

	const auto subCommandText = arguments[indexOfSubCommand].getDataAsString();
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
		const auto description = "Invalid sub-command given for 'log policies', sub-command not found."s;
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void LogPoliciesCommand::throwIfInvalidSubCommand(const CommandArguments& arguments) const
{
	const auto subCommandText = arguments[indexOfSubCommand].getDataAsString();
	for (const auto& subCommand : m_subCommands)
	{
		if (subCommandText == subCommand->getCommandName())
		{
			return;
		}
	}
	const auto description = "Invalid sub-command given for 'log policies', sub-command not found."s;
	throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
}