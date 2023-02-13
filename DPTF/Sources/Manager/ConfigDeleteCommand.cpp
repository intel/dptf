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
#include "ConfigDeleteCommand.h"
#include "DptfManagerInterface.h"
#include "DataManager.h"
using namespace std;

ConfigDeleteCommand::ConfigDeleteCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

ConfigDeleteCommand::~ConfigDeleteCommand()
{
}

string ConfigDeleteCommand::getCommandName() const
{
	return "delete";
}

void ConfigDeleteCommand::execute(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);

	auto dvName = arguments[1].getDataAsString();
	auto key = arguments[2].getDataAsString();

	// config delete override /shared/tables/itmt/test
	m_dptfManager->getDataManager()->deleteConfigKey(DataVaultType::ToType(dvName), key);
	setResultCode(ESIF_OK);
}

void ConfigDeleteCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 3)
	{
		string description = string(
			"Invalid argument count given to 'config delete' command. "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if ((arguments[0].isDataTypeString() == false) || (arguments[1].isDataTypeString() == false)
		|| (arguments[2].isDataTypeString() == false))
	{
		string description = string(
			"Invalid argument type given to 'config delete' command. "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}
