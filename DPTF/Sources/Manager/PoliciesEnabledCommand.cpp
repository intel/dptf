/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#include "PoliciesEnabledCommand.h"

#include "DptfManagerInterface.h"
#include "PolicyManagerInterface.h"

using namespace std;

PoliciesEnabledCommand::PoliciesEnabledCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{

}

PoliciesEnabledCommand::~PoliciesEnabledCommand()
{

}

string PoliciesEnabledCommand::getCommandName() const
{
	return "enabled";
}

void PoliciesEnabledCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfBadArguments(arguments);
		const auto enabledPolicyGuids = m_dptfManager->getPolicyManager()->getSupportedPolicyList()->getGuids();
		stringstream message;
		for (const auto& guid : enabledPolicyGuids)
		{
			message << guid.toClassicString() << endl;
		}
		setResultCode(ESIF_OK);
		setResultMessage(message.str());
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}

void PoliciesEnabledCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() != 1)
	{
		const auto description = R"(
			Invalid argument count given to 'policies enabled' command. Run 'dptf help' command for more information.)";
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}