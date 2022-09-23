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
#include "TableObjectSetCommand.h"
#include "DptfManagerInterface.h"
#include "DataManager.h"
#include "esif_ccb_string.h"
#include "ParticipantManagerInterface.h"
#include "TableObjectBinaryTableParser.h"
using namespace std;

#define REVISION_INDICATOR_LENGTH 2
#define MODE_INDICATOR_LENGTH 2
#define COLUMN_MAX_SIZE 64

TableObjectSetCommand::TableObjectSetCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

TableObjectSetCommand::~TableObjectSetCommand()
{
}

string TableObjectSetCommand::getCommandName() const
{
	return "set";
}

void TableObjectSetCommand::execute(const CommandArguments& arguments)
{
	setDefaultResultMessage();

	throwIfBadArguments(arguments);
	throwIfTableObjectNotExist(arguments);

	setTableObjectXmlString(arguments);
	setResultCode(ESIF_OK);
}

void TableObjectSetCommand::setTableObjectXmlString(const CommandArguments& arguments)
{
	auto tableName = arguments[1].getDataAsString();
	auto tableString = arguments[2].getDataAsString();
	string uuid = Constants::EmptyString;
	string dvName = Constants::EmptyString;
	string key = Constants::EmptyString;
	UIntN participantIndex = Constants::Esif::NoParticipant;
	Bool isParticipantTable = m_dptfManager->getDataManager()->isParticipantTable(TableObjectType::ToType(tableName));

	if (isParticipantTable)
	{
		// tableobject set vsct VIR1 "value"
		throwIfBadArgumentsForParticipantTable(arguments);
		throwIfParticipantNotExist(arguments);

		string participantName = arguments[2].getDataAsString();
		participantIndex =
			m_dptfManager->getParticipantManager()->getParticipant(participantName)->getParticipantIndex();
		tableString = arguments[3].getDataAsString();
	}
	else if (arguments.size() == 4)
	{
		// tableobject set apat "value" uuid
		uuid = arguments[3].getDataAsString();
	}
	else if (arguments.size() > 4)
	{
		// tableobject set itmt "value" override /shared/tables/itmt/test
		dvName = arguments[3].getDataAsString();
		key = arguments[4].getDataAsString();
	}
	auto textInput = tableString.c_str();

	if (esif_ccb_strlen(textInput, REVISION_INDICATOR_LENGTH + 1) > 0)
	{
		convertToBinaryAndSet(TableObjectType::ToType(tableName), textInput, uuid, dvName, key, participantIndex);
	}
}

void TableObjectSetCommand::convertToBinaryAndSet(
	TableObjectType::Type tableType,
	const char* textInput,
	string uuid,
	string dvName,
	string key,
	UIntN participantIndex)
{
	auto table = m_dptfManager->getDataManager()->getTableObjectMap().find(tableType)->second;
	char revisionNumString[REVISION_INDICATOR_LENGTH + 1];
	UInt64 revisionNumInt = 0;

	if (table.hasRevisionField())
	{
		esif_ccb_memcpy(&revisionNumString, textInput, REVISION_INDICATOR_LENGTH);
		revisionNumString[REVISION_INDICATOR_LENGTH] = '\0';
		revisionNumInt = extractInteger(revisionNumString);
	}

	if (table.isUsingEsifDataVariant(revisionNumInt))
	{
		convertToEsifDataVariantBinaryAndSet(tableType, textInput, uuid, dvName, key, participantIndex);
	}
	else
	{
		convertToNonEsifDataVariantBinaryAndSet(tableType, textInput, uuid, dvName, key, participantIndex);
	}

}

void TableObjectSetCommand::convertToEsifDataVariantBinaryAndSet(
	TableObjectType::Type tableType,
	const char* textInput,
	string uuid,
	string dvName,
	string key,
	UIntN participantIndex)
{
	TableObject table = findTableObjectByType(tableType);
	auto fieldsMap = table.getFieldsMap();
	vector<TableObjectField> fields;
	size_t numberOfFields = 0;
	int fieldIndex = 0;
	char revisionNumString[REVISION_INDICATOR_LENGTH + 1];
	UInt64 revisionNumInt = 0;
	char modeNumString[MODE_INDICATOR_LENGTH + 1];
	UInt64 modeNumInt = 0;
	UInt32 totalDataOutputLength = 0;
	UInt32 stringType = ESIF_DATA_STRING;
	UInt32 numberType = ESIF_DATA_UINT64;
	UInt8* tableData = NULL;
	UInt8* newTableData = NULL;
	UInt8* dataOutput = NULL;
	UInt64 counter = 0;
	char* tableRow = NULL;
	char* rowToken = NULL;
	char rowDelimiter[] = "!";
	char* tableColumn = NULL;
	char* columnToken = NULL;
	char columnDelimiter[] = ",";
	auto tempDelimiter = "'";
	char* tableColumnValue = NULL;
	UInt64 columnValueNumber = 0;
	char* temp;

	try
	{
		if (table.hasRevisionField())
		{
			esif_ccb_memcpy(&revisionNumString, textInput, REVISION_INDICATOR_LENGTH);
			revisionNumString[REVISION_INDICATOR_LENGTH] = '\0';
			textInput += REVISION_INDICATOR_LENGTH + 1;
			revisionNumInt = extractInteger(revisionNumString);
			totalDataOutputLength += sizeof(numberType) + sizeof(revisionNumInt);
			tableData = (u8*)esif_ccb_malloc(totalDataOutputLength);
			throwIfFailedToAllocateMemory(tableData);

			dataOutput = tableData;
			esif_ccb_memcpy(dataOutput, &numberType, sizeof(numberType));
			dataOutput += sizeof(numberType);
			counter += sizeof(numberType);
			esif_ccb_memcpy(dataOutput, &revisionNumInt, sizeof(revisionNumInt));
			dataOutput += sizeof(revisionNumInt);
			counter += sizeof(revisionNumInt);

			if (fieldsMap.find(revisionNumInt) != fieldsMap.end())
			{
				fields = fieldsMap.find(revisionNumInt)->second;
			}
		}
		else if (fieldsMap.find(0) != fieldsMap.end())
		{
			fields = fieldsMap.find(0)->second;
		}
		else
		{
			string description = string(
				"Invalid/Unsupported revision detected"
				"Run 'dptf help' command for more information.");
			setResultMessage(description);
			throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
		}

		if (table.hasModeField())
		{
			esif_ccb_memcpy(&modeNumString, textInput, MODE_INDICATOR_LENGTH);
			modeNumString[MODE_INDICATOR_LENGTH] = '\0';
			textInput += MODE_INDICATOR_LENGTH + 1;

			modeNumInt = extractInteger(modeNumString);
			totalDataOutputLength += sizeof(numberType) + (UInt32)sizeof(modeNumInt);
			newTableData = (u8*)esif_ccb_realloc(tableData, totalDataOutputLength);
			throwIfFailedToAllocateMemory(newTableData);
			tableData = newTableData;
			dataOutput = tableData;
			dataOutput += counter;
			esif_ccb_memcpy(dataOutput, &numberType, sizeof(numberType));
			dataOutput += sizeof(numberType);
			counter += sizeof(numberType);
			esif_ccb_memcpy(dataOutput, &modeNumInt, sizeof(modeNumInt));
			dataOutput += sizeof(modeNumInt);
			counter += sizeof(modeNumInt);
		}

		numberOfFields = fields.size();
		tableRow = esif_ccb_strtok((char*)textInput, rowDelimiter, &rowToken);

		while (tableRow != NULL)
		{
			fieldIndex = -1;
			tableColumn = esif_ccb_strtok(tableRow, columnDelimiter, &columnToken);
			while (tableColumn != NULL)
			{
				size_t columnValueLength = esif_ccb_strlen(tableColumn, COLUMN_MAX_SIZE - 1) + 1;
				tableColumnValue = (char*)esif_ccb_malloc(columnValueLength);
				throwIfFailedToAllocateMemory(tableColumnValue);
				esif_ccb_strcpy(tableColumnValue, tableColumn, columnValueLength);
				fieldIndex++;

				if (fieldIndex < (int)numberOfFields)
				{
					if ((temp = esif_ccb_strstr(tableColumnValue, tempDelimiter)) != NULL)
					{
						*temp = 0;
					}

					esif_data_type targetType;
					if (fields[fieldIndex].m_isPowerLimit)
					{
						targetType = (extractInteger(tableColumnValue) > 0 || strcmp(tableColumnValue, "0") == 0)
										 ? ESIF_DATA_UINT64
										 : ESIF_DATA_STRING;
					}
					else
					{
						targetType = fields[fieldIndex].m_fieldDataType;
					}

					switch (targetType)
					{
					case ESIF_DATA_STRING:
						totalDataOutputLength +=
							sizeof(stringType) + (u32)sizeof(columnValueLength) + (u32)columnValueLength;
						newTableData = (UInt8*)esif_ccb_realloc(tableData, totalDataOutputLength);
						throwIfFailedToAllocateMemory(newTableData);
						tableData = newTableData;
						dataOutput = tableData;
						dataOutput += counter;
						esif_ccb_memcpy(dataOutput, &stringType, sizeof(stringType));
						dataOutput += sizeof(stringType);
						counter += sizeof(stringType);
						esif_ccb_memcpy(dataOutput, &columnValueLength, sizeof(columnValueLength));
						dataOutput += sizeof(columnValueLength);
						counter += sizeof(columnValueLength);
						esif_ccb_memcpy(dataOutput, tableColumnValue, (size_t)columnValueLength);
						dataOutput += columnValueLength;
						counter += columnValueLength;
						break;
					case ESIF_DATA_UINT32:
					case ESIF_DATA_UINT64:
						columnValueNumber = extractInteger(tableColumnValue);
						totalDataOutputLength += sizeof(numberType) + (UInt32)sizeof(columnValueNumber);
						newTableData = (UInt8*)esif_ccb_realloc(tableData, totalDataOutputLength);
						throwIfFailedToAllocateMemory(newTableData);
						tableData = newTableData;
						dataOutput = tableData;
						dataOutput += counter;
						esif_ccb_memcpy(dataOutput, &numberType, sizeof(numberType));
						dataOutput += sizeof(numberType);
						counter += sizeof(numberType);
						esif_ccb_memcpy(dataOutput, &columnValueNumber, sizeof(columnValueNumber));
						dataOutput += sizeof(columnValueNumber);
						counter += sizeof(columnValueNumber);
						break;
					default:
						throw command_failure(ESIF_E_NOT_FOUND, "TableObject field type not recognized."s);
					}
				}

				esif_ccb_free(tableColumnValue);
				tableColumnValue = NULL;

				// Get next column
				tableColumn = esif_ccb_strtok(NULL, columnDelimiter, &columnToken);
			}

			// Get next row
			tableRow = esif_ccb_strtok(NULL, rowDelimiter, &rowToken);
		}

		if (tableData)
		{
			DptfBuffer tableDataBuffer(totalDataOutputLength);
			tableDataBuffer.put(0, tableData, totalDataOutputLength);
			setTableData(tableDataBuffer, tableType, uuid, dvName, key, participantIndex);
		}
	}
	catch (command_failure& cmd_failure)
	{
		auto desc = cmd_failure.getDescription();
		auto errorCode = cmd_failure.getErrorCode();
		esif_ccb_free(tableData);
		esif_ccb_free(tableColumnValue);
		throw command_failure(errorCode, desc);
	}

	esif_ccb_free(tableData);
	esif_ccb_free(tableColumnValue);
}

void TableObjectSetCommand::convertToNonEsifDataVariantBinaryAndSet(
	TableObjectType::Type tableType,
	const char* textInput,
	string uuid,
	string dvName,
	string key,
	UIntN participantIndex)
{
	const auto tableObjectMap = m_dptfManager->getDataManager()->getTableObjectMap();
	const auto table = tableObjectMap.find(tableType);
	if (table != tableObjectMap.end())
	{
		TableObjectBinaryTableParser tableParser(table->second, textInput);
		DptfBuffer tableData = tableParser.extractTable();
		setTableData(tableData, tableType, uuid, dvName, key, participantIndex);
	}
	else
	{
		throw dptf_exception("Unable to find table object definition for "s + TableObjectType::ToString(tableType));
	}
}

UInt32 TableObjectSetCommand::extractInteger(const esif_string str)
{
	UInt32 val = 0;
	if (str)
	{
		if (esif_ccb_strncmp(str, "0x", 2) == 0)
		{
			esif_ccb_sscanf(str + 2, "%x", &val);
		}
		else
		{
			esif_ccb_sscanf(str, "%d", (Int32*)&val);
		}
	}
	return val;
}

void TableObjectSetCommand::setTableData(
	const DptfBuffer& tableData,
	TableObjectType::Type tableType,
	string uuid,
	string dvName,
	string key,
	UIntN participantIndex)
{
	if (dvName != Constants::EmptyString && key != Constants::EmptyString)
	{
		auto dvType = DataVaultType::ToType(dvName);
		m_dptfManager->getDataManager()->setTableObjectBasedOnAlternativeDataSourceAndKey(
			tableData, tableType, dvType, key);
	}
	else
	{
		m_dptfManager->getDataManager()->setTableObject(
			tableData, tableType, uuid, participantIndex);
	}
}

TableObject TableObjectSetCommand::findTableObjectByType(TableObjectType::Type type) const
{
	auto dataManager = m_dptfManager->getDataManager();
	if (dataManager)
	{
		auto tableObjectMap = dataManager->getTableObjectMap();
		auto tableObject = tableObjectMap.find(type);
		if (tableObject != tableObjectMap.end())
		{
			return tableObject->second;
		}
	}

	throw dptf_exception("Could not find TableObject for type "s + TableObjectType::ToString(type));
}

void TableObjectSetCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 3)
	{
		string description = string(
			"Invalid argument count given to 'tableobject set' command.  "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if ((arguments[0].isDataTypeString() == false) || (arguments[1].isDataTypeString() == false)
		|| (arguments[2].isDataTypeString() == false))
	{
		string description = string(
			"Invalid argument type given to 'tableobject set' command.  "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void TableObjectSetCommand::throwIfTableObjectNotExist(const CommandArguments& arguments)
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

void TableObjectSetCommand::throwIfFailedToAllocateMemory(char* tableValue)
{
	if (tableValue == NULL)
	{
		string description = string("Failed to allocate memory in 'tableobject set' command.");
		setResultMessage(description);
		throw command_failure(ESIF_E_NO_MEMORY, description);
	}
}

void TableObjectSetCommand::throwIfFailedToAllocateMemory(UInt8* tableData)
{
	if (tableData == NULL)
	{
		string description = string("Failed to allocate memory in 'tableobject set' command.");
		setResultMessage(description);
		throw command_failure(ESIF_E_NO_MEMORY, description);
	}
}

void TableObjectSetCommand::throwIfParticipantNotExist(const CommandArguments& arguments)
{
	auto participantExists = m_dptfManager->getParticipantManager()->participantExists(arguments[2].getDataAsString());
	if (participantExists == false)
	{
		string description = string("The participant specified was not found.");
		setResultMessage(description);
		throw command_failure(ESIF_E_NOT_FOUND, description);
	}
}

void TableObjectSetCommand::throwIfBadArgumentsForParticipantTable(const CommandArguments& arguments)
{
	if (arguments.size() < 4)
	{
		string description = string(
			"Invalid argument count given to 'tableobject set' command for a participant table. "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}