/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "TimeOps.h"
#include "DiagAllCommand.h"
#include "DiagPolicyCommand.h"
#include "DiagParticipantCommand.h"
#include "CommandDispatcher.h"

using namespace std;

DiagCommand::DiagCommand(DptfManagerInterface* dptfManager, std::shared_ptr<IFileIO> fileIo)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
{
	m_diagCommandDispatcher = std::make_shared<CommandDispatcher>();
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
	m_subCommands.push_back(make_shared<DiagPolicyCommand>(m_dptfManager, m_fileIo));
	m_subCommands.push_back(make_shared<DiagParticipantCommand>(m_dptfManager, m_fileIo));
}

void DiagCommand::registerSubCommands()
{
	for (auto c = m_subCommands.begin(); c != m_subCommands.end(); ++c)
	{
		m_diagCommandDispatcher->registerHandler((*c)->getCommandName(), *c);
	}
}

std::string DiagCommand::getCommandName() const
{
	return "diag";
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
		string description = string("Subcommand given for diagnostics command not found.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void DiagCommand::throwIfInvalidArgumentData(const CommandArguments& arguments)
{
	if (arguments[1].isDataTypeString() == false)
	{
		string description = string("Invalid argument type given to 'diag' command.  Expected a string.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void DiagCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		string description = string("Invalid argument count.  Expect >= 2.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}
