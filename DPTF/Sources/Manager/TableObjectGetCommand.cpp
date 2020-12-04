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
#include "TableObjectGetCommand.h"
#include "DptfManagerInterface.h"
#include "DataManager.h"
using namespace std;

TableObjectGetCommand::TableObjectGetCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

TableObjectGetCommand::~TableObjectGetCommand()
{
}

string TableObjectGetCommand::getCommandName() const
{
	return "get";
}

void TableObjectGetCommand::execute(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);
	throwIfTableObjectNotExist(arguments);

	auto results = getTableObjectXmlString(arguments);
	setResultMessage(results);
	setResultCode(ESIF_OK);
}

string TableObjectGetCommand::getTableObjectXmlString(const CommandArguments& arguments)
{
	auto tableName = arguments[1].getDataAsString();
	auto data = m_dptfManager->getDataManager()->getTableObject(TableObjectType::ToType(tableName));
	return data.getXmlString();
}

void TableObjectGetCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		string description = string(
			"Invalid argument count given to 'tableobject get' command.  "
			"Run 'help' command for more information");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if ((arguments[0].isDataTypeString() == false) || (arguments[1].isDataTypeString() == false))
	{
		string description = string(
			"Invalid argument type given to 'tableobject get' command.  "
			"Run 'help' command for more information");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void TableObjectGetCommand::throwIfTableObjectNotExist(const CommandArguments& arguments)
{
	auto tableName = arguments[1].getDataAsString();
	auto tableExists = m_dptfManager->getDataManager()->tableObjectExists(TableObjectType::ToType(tableName));
	if (tableExists == false)
	{
		string description = string("The table object specified was not found.");
		setResultMessage(description);
		throw command_failure(ESIF_E_NOT_FOUND, description);
	}
}
