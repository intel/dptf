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
#include "CommandDispatcher.h"
#include <map>
#include <string>
using namespace std;

string defaultCommandMessage = "The command completed successfully.";
string errorNoCommandGiven = "No command to process.";
string errorCommandNotString = "Invalid command type.  String type is required for commands.";
string errorInvalidCommand = "Command not supported. Type 'dptf help' for available commands.";

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
	const string command = arguments[0].getDataAsString();
	const auto it = m_registeredCommands.find(command);
	if (it != m_registeredCommands.end())
	{
		it->second->execute(arguments);
		m_lastCommandReturnCode = it->second->getLastExecutionResultCode();
		m_lastSuccessfulCommandMessage = it->second->getLastExecutionMessage();
		if ((m_lastCommandReturnCode == ESIF_OK) && (m_lastSuccessfulCommandMessage.empty()))
		{
			m_lastSuccessfulCommandMessage = defaultCommandMessage;
		}
	}
	else
	{
		const string message = command + string(" not supported.");
		throw command_failure(ESIF_E_NOT_SUPPORTED, message);
	}
}

void CommandDispatcher::registerHandler(const string commandName, shared_ptr<CommandHandler> handler)
{
	m_registeredCommands[commandName] = handler;
}

void CommandDispatcher::unregisterHandler(const string commandName)
{
	auto it = m_registeredCommands.find(commandName);
	if (it != m_registeredCommands.end())
	{
		m_registeredCommands.erase(it);
	}
}

string CommandDispatcher::getLastSuccessfulCommandMessage() const
{
	return m_lastSuccessfulCommandMessage;
}

eEsifError CommandDispatcher::getLastReturnCode() const
{
	return m_lastCommandReturnCode;
}

bool CommandDispatcher::isCommandRegistered(const std::string& command)
{
	const auto findResult = m_registeredCommands.find(command);
	return findResult != m_registeredCommands.end();
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

	string command = arguments[0].getDataAsString();
	auto it = m_registeredCommands.find(command);
	if (it == m_registeredCommands.end())
	{
		throw command_failure(ESIF_E_NOT_IMPLEMENTED, errorInvalidCommand);
	}
}
