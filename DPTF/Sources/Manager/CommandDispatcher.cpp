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
#include "CommandDispatcher.h"
#include <map>
#include <string>
using namespace std;

std::string defaultCommandMessage = "The command completed successfully.";
std::string errorNoCommandGiven = "No command to process.";
std::string errorCommandNotString = "Invalid command type.  String type is required for commands.";
std::string errorInvalidCommand = "Command not supported. Type 'app cmd dptf help' for available commands.";

CommandDispatcher::CommandDispatcher()
	: m_registeredCommands()
	, m_lastSuccessfulCommandMessage(Constants::EmptyString)
	, m_lastCommandReturnCode(ESIF_OK)
{
}

CommandDispatcher::~CommandDispatcher()
{
}

void CommandDispatcher::dispatch(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);
	m_lastSuccessfulCommandMessage = defaultCommandMessage;
	m_lastCommandReturnCode = ESIF_OK;
	std::string command = arguments[0].getDataAsString();
	auto it = m_registeredCommands.find(command);
	if (it != m_registeredCommands.end())
	{
		it->second->execute(arguments);
		m_lastCommandReturnCode = it->second->getLastExecutionResultCode();

		if (m_lastCommandReturnCode == ESIF_OK)
		{
			m_lastSuccessfulCommandMessage = it->second->getLastExecutionMessage();
		}
	}
	else
	{
		std::string message = command + std::string(" not supported.");
		throw command_failure(ESIF_E_NOT_SUPPORTED, message);
	}
}

void CommandDispatcher::registerHandler(const std::string commandName, std::shared_ptr<CommandHandler> handler)
{
	m_registeredCommands[commandName] = handler;
}

void CommandDispatcher::unregisterHandler(const std::string commandName)
{
	auto it = m_registeredCommands.find(commandName);
	if (it != m_registeredCommands.end())
	{
		m_registeredCommands.erase(it);
	}
}

std::string CommandDispatcher::getLastSuccessfulCommandMessage() const
{
	return m_lastSuccessfulCommandMessage;
}

eEsifError CommandDispatcher::getLastReturnCode() const
{
	return m_lastCommandReturnCode;
}

void CommandDispatcher::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 1)
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, errorNoCommandGiven);
	}

	if (arguments[0].isDataTypeString() == false)
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, errorCommandNotString);
	}

	std::string command = arguments[0].getDataAsString();
	auto it = m_registeredCommands.find(command);
	if (it == m_registeredCommands.end())
	{
		throw command_failure(ESIF_E_NOT_IMPLEMENTED, errorInvalidCommand);
	}
}
