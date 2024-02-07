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
#include "DiagCommand.h"
#include "PolicyManagerInterface.h"
#include "DiagAllCommand.h"
#include "DiagPolicyCommand.h"
#include "DiagParticipantCommand.h"
#include "CommandDispatcher.h"

using namespace std;

DiagCommand::DiagCommand(
	DptfManagerInterface* dptfManager,
	const shared_ptr<IFileIo>& fileIo,
	const shared_ptr<TimeStampGenerator>& timeStampGenerator)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
	, m_timeStampGenerator(timeStampGenerator)
{
	m_diagCommandDispatcher = make_shared<CommandDispatcher>();
	createSubCommands();
	registerSubCommands();
}

DiagCommand::~DiagCommand()
{
	m_subCommands.clear();
}

void DiagCommand::createSubCommands()
{
	m_subCommands.push_back(make_shared<DiagAllCommand>(m_dptfManager, m_fileIo));
	m_subCommands.push_back(make_shared<DiagPolicyCommand>(m_dptfManager, m_fileIo, m_timeStampGenerator));
	m_subCommands.push_back(make_shared<DiagParticipantCommand>(m_dptfManager, m_fileIo, m_timeStampGenerator));
}

void DiagCommand::registerSubCommands() const
{
	for (const auto& subCommand : m_subCommands)
	{
		m_diagCommandDispatcher->registerHandler(subCommand->getCommandName(), subCommand);
	}
}

string DiagCommand::getCommandName() const
{
	return "diag"s;
}

void DiagCommand::execute(const CommandArguments& arguments)
{
	throwIfInvalidArgumentCount(arguments);
	throwIfInvalidArgumentData(arguments);
	throwIfInvalidCommand(arguments);
	CommandArguments subArguments = arguments;
	subArguments.remove(0);
	m_diagCommandDispatcher->dispatch(subArguments);
	setResultMessage(m_diagCommandDispatcher->getLastSuccessfulCommandMessage());
}

void DiagCommand::throwIfInvalidCommand(const CommandArguments& arguments)
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
		const auto description = string("Subcommand given for diagnostics command not found."s);
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void DiagCommand::throwIfInvalidArgumentData(const CommandArguments& arguments)
{
	if (arguments[1].isDataTypeString() == false)
	{
		const auto description = string("Invalid argument type given to 'diag' command.  Expected a string."s);
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void DiagCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		const auto description = string("Invalid argument count.  Expect >= 2."s);
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}
