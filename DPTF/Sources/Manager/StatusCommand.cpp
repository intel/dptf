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
#include "StatusCommand.h"
#include "DptfManagerInterface.h"
#include "DptfStatusInterface.h"
#include "WorkItemQueueManagerInterface.h"

using namespace std;

StatusCommand::StatusCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

StatusCommand::~StatusCommand()
{
}

std::string StatusCommand::getCommandName() const
{
	return "status";
}

void StatusCommand::execute(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);

	eAppStatusCommand command = (eAppStatusCommand)(UInt32)*(UInt32 *)arguments[1].getData().get();
	UInt8 groupId = 0;
	UInt8 moduleId = 0;
	UInt32 appStatusIn = 0;
	if (arguments.size() > 2) {
		groupId = (UInt8)*(UInt8 *)arguments[2].getData().get();
		appStatusIn = groupId;
	}
	if (arguments.size() > 3) {
		moduleId = (UInt8)*(UInt8 *)arguments[3].getData().get();
		appStatusIn = appStatusIn << 16;
		appStatusIn = appStatusIn | moduleId;
	}

	std::pair<std::string, eEsifError> statusResult = m_dptfManager->getDptfStatus()->getStatus(command, appStatusIn);

	eEsifError resultCode = statusResult.second;
	setResultCode(resultCode);
	if (resultCode == ESIF_OK)
	{
		setResultMessage(statusResult.first);
	}
}



void StatusCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		string description = string("Invalid argument count given.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

}
