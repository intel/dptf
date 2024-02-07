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

#include "LogPolicyStateStatusCommand.h"
#include "DptfManagerInterface.h"
#include "WorkItemQueueManagerInterface.h"
#include "PolicyStateLogger.h"

using namespace std;
constexpr size_t argumentCount{1};
#define LOGGER_STATUS_NAME "status"s

LogPolicyStateStatusCommand::LogPolicyStateStatusCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

string LogPolicyStateStatusCommand::getCommandName() const
{
	return LOGGER_STATUS_NAME;
}

void LogPolicyStateStatusCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfInvalidArguments(arguments);
		statusLogger();
	}
	catch (command_failure& ex)
	{
		setResultCode(ex.getErrorCode());
		setResultMessage(ex.getDescription());
	}
}

void LogPolicyStateStatusCommand::throwIfInvalidArguments(const CommandArguments& arguments)
{
	throwIfInvalidArgumentCount(arguments);
	throwIfInvalidArgumentName(arguments);
}

void LogPolicyStateStatusCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() != argumentCount)
	{
		throw command_failure(
			ESIF_E_INVALID_ARGUMENT_COUNT, "Invalid number of arguments for command: "s + LOGGER_STATUS_NAME);
	}
}

void LogPolicyStateStatusCommand::throwIfInvalidArgumentName(const CommandArguments& arguments)
{
	if (arguments[0].getDataAsString() != LOGGER_STATUS_NAME)
	{
		throw command_failure(
			ESIF_E_INVALID_REQUEST_TYPE, "Invalid argument given: "s + arguments[0].getDataAsString());
	}
}

void LogPolicyStateStatusCommand::statusLogger()
{
	string resultMessage = m_dptfManager->getPolicyStateLogger()->getStatus();
	setResultCode(ESIF_OK);
	setResultMessage(string(resultMessage));
}
