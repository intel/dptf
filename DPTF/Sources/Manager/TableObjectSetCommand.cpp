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
#include "esif_ccb_memtrace.h"
using namespace std;

#define REVISION_INDICATOR_LENGTH 2
#define COLUMN_MAX_SIZE 64
#define MIN_BUFFER_LENGTH 1

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

	if (arguments.size() == 4)
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

	if (esif_ccb_strlen(textInput, REVISION_INDICATOR_LENGTH + 1) > REVISION_INDICATOR_LENGTH)
	{
		convertToBinaryAndSet(TableObjectType::ToType(tableName), textInput, uuid, dvName, key);
	}
}

void TableObjectSetCommand::convertToBinaryAndSet(
	TableObjectType::Type tableType,
	const char* textInput,
	string uuid,
	string dvName,
	string key)
{
	auto table = m_dptfManager->getDataManager()->getTableObjectMap().find(tableType)->second;
	auto fields = table.getFields();
	size_t numberOfFields = 0;
	int fieldIndex = 0;
	char revisionNumString[REVISION_INDICATOR_LENGTH + 1];
	UInt64 revisionNumInt = 0;
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
		esif_ccb_memcpy(&revisionNumString, textInput, REVISION_INDICATOR_LENGTH);
		revisionNumString[REVISION_INDICATOR_LENGTH] = '\0';
		textInput += REVISION_INDICATOR_LENGTH + 1;
		revisionNumInt = extractInteger(revisionNumString);
		totalDataOutputLength += sizeof(numberType) + sizeof(revisionNumInt);
		tableData = (u8*)esif_ccb_malloc(totalDataOutputLength);
		throwIfFailedToAllocateMemory(tableData);

		numberOfFields = fields.size();
		dataOutput = tableData;
		esif_ccb_memcpy(dataOutput, &numberType, sizeof(numberType));
		dataOutput += sizeof(numberType);
		counter += sizeof(numberType);
		esif_ccb_memcpy(dataOutput, &revisionNumInt, sizeof(revisionNumInt));
		dataOutput += sizeof(revisionNumInt);
		counter += sizeof(revisionNumInt);

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

					auto targetType = fields[fieldIndex].m_fieldDataType;

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
						break;
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

		setTableData(totalDataOutputLength, tableData, tableType, uuid, dvName, key);
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
	UInt32 totalDataOutputLength,
	UInt8* tableData,
	TableObjectType::Type tableType,
	string uuid,
	string dvName,
	string key)
{
	UInt32 tableDataLength = MIN_BUFFER_LENGTH;
	UInt8* finalTableData = NULL;

	if (totalDataOutputLength)
	{
		tableDataLength = totalDataOutputLength;
	}

	finalTableData = (u8*)esif_ccb_malloc(tableDataLength);
	throwIfFailedToAllocateMemory(finalTableData);
	esif_ccb_memcpy((u8*)finalTableData, (u8*)tableData, tableDataLength);

	if (dvName != Constants::EmptyString && key != Constants::EmptyString)
	{
		auto dvType = DataVaultType::ToType(dvName);
		m_dptfManager->getDataManager()->setTableObjectBasedOnAlternativeDataSourceAndKey(
			tableDataLength, finalTableData, tableType, dvType, key);
	}
	else
	{
		m_dptfManager->getDataManager()->setTableObject(tableDataLength, finalTableData, tableType, uuid);
	}

	esif_ccb_free(finalTableData);
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
