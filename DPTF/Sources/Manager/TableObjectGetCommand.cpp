/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "ParticipantManagerInterface.h"
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

	try
	{
		auto results = getTableObjectXmlString(arguments);
		setResultMessage(results);
		setResultCode(ESIF_OK);
	}
	catch (const command_failure& cf)
	{
		throw cf;
	}
	catch (...)
	{
		string message = string("Error generating TableObject XML.");
		setResultMessage(message);
		throw command_failure(ESIF_E_UNSPECIFIED, message);
	}
}

string TableObjectGetCommand::getTableObjectXmlString(const CommandArguments& arguments)
{
	auto tableName = arguments[1].getDataAsString();
	string uuid = Constants::EmptyString;
	auto dataManager = m_dptfManager->getDataManager();
	auto tableType = TableObjectType::ToType(tableName);
	Bool isParticipantTable = m_dptfManager->getDataManager()->isParticipantTable(TableObjectType::ToType(tableName));

	if (isParticipantTable)
	{
		// tableobject get vsct VIR1
		throwIfBadArgumentsForParticipantTable(arguments);
		throwIfParticipantNotExist(arguments);

		string participantName = arguments[2].getDataAsString();
		UIntN participantIndex =
			m_dptfManager->getParticipantManager()->getParticipant(participantName)->getParticipantIndex();

		return dataManager->getTableObject(tableType, uuid, participantIndex).getXmlString();
	}
	else if (arguments.size() > 3)
	{
		auto dvName = arguments[2].getDataAsString();
		auto dvType = DataVaultType::ToType(dvName);
		auto key = arguments[3].getDataAsString();

		// tableobject get itmt override /shared/tables/itmt/test
		return dataManager->getTableObjectBasedOnAlternativeDataSourceAndKey(tableType, dvType, key).getXmlString();
	}
	else if (arguments.size() == 3)
	{
		uuid = arguments[2].getDataAsString();

		// tableobject get apat uuid
		return dataManager->getTableObject(tableType, uuid).getXmlString();
	}
	else
	{
		// tableobject get apat
		return dataManager->getTableObject(tableType, uuid).getXmlString();
	}
}

void TableObjectGetCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		string description = string(
			"Invalid argument count given to 'tableobject get' command.  "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if ((arguments[0].isDataTypeString() == false) || (arguments[1].isDataTypeString() == false))
	{
		string description = string(
			"Invalid argument type given to 'tableobject get' command.  "
			"Run 'dptf help' command for more information.");
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
		string description = string("TableObject schema not found.");
		setResultMessage(description);
		throw command_failure(ESIF_E_NOT_FOUND, description);
	}
}

void TableObjectGetCommand::throwIfParticipantNotExist(const CommandArguments& arguments)
{
	auto participantExists = m_dptfManager->getParticipantManager()->participantExists(arguments[2].getDataAsString());
	if (participantExists == false)
	{
		string description = string("The participant specified was not found.");
		setResultMessage(description);
		throw command_failure(ESIF_E_NOT_FOUND, description);
	}
}

void TableObjectGetCommand::throwIfBadArgumentsForParticipantTable(const CommandArguments& arguments)
{
	if (arguments.size() < 3)
	{
		string description = string(
			"Invalid argument count given to 'tableobject get' command for a participant table. "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}