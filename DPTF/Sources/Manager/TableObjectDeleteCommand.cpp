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
#include "TableObjectDeleteCommand.h"
#include "DptfManagerInterface.h"
#include "DataManager.h"
using namespace std;

TableObjectDeleteCommand::TableObjectDeleteCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

TableObjectDeleteCommand::~TableObjectDeleteCommand()
{
}

string TableObjectDeleteCommand::getCommandName() const
{
	return "delete";
}

void TableObjectDeleteCommand::execute(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);
	throwIfTableObjectNotExist(arguments);

	auto tableName = arguments[1].getDataAsString();
	string uuid = Constants::EmptyString;
	string all = Constants::EmptyString;
	if (arguments.size() > 3)
	{
		if (arguments[3].getDataAsString() == "all")
		{
			// tableobject delete apat d4cafb01-01cb-4b39-b33f-1ac68f0d7903 all
			uuid = arguments[2].getDataAsString();
			m_dptfManager->getDataManager()->deleteAllTableObject(TableObjectType::ToType(tableName), uuid);
		}
	}
	else if (arguments.size() > 2)
	{
		if (arguments[2].getDataAsString() == "all")
		{
			// tableobject delete apat all
			m_dptfManager->getDataManager()->deleteAllTableObject(TableObjectType::ToType(tableName), uuid);
		}
		else
		{
			// tableobject delete apat d4cafb01-01cb-4b39-b33f-1ac68f0d7903
			uuid = arguments[2].getDataAsString();
			m_dptfManager->getDataManager()->deleteTableObject(TableObjectType::ToType(tableName), uuid);
		}
	}
	else
	{
		// tableobject delete apat
		m_dptfManager->getDataManager()->deleteTableObject(TableObjectType::ToType(tableName), uuid);
	}
	setResultCode(ESIF_OK);
}

void TableObjectDeleteCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		string description = string(
			"Invalid argument count given to 'tableobject delete' command.  "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if ((arguments[0].isDataTypeString() == false) || (arguments[1].isDataTypeString() == false))
	{
		string description = string(
			"Invalid argument type given to 'tableobject delete' command.  "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void TableObjectDeleteCommand::throwIfTableObjectNotExist(const CommandArguments& arguments)
{
	auto tableName = arguments[1].getDataAsString();
	auto tableExists = m_dptfManager->getDataManager()->tableObjectExists(TableObjectType::ToType(tableName));
	if (tableExists == false)
	{
		string description = string("TableObject schema not found.");
		setResultMessage(description);
		throw command_failure(ESIF_E_NOT_FOUND, description);
	}
}
