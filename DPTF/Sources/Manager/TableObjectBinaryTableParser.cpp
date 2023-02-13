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
#include "TableObjectBinaryTableParser.h"
#include "StringConverter.h"

using namespace std;


TableObjectBinaryTableParser::TableObjectBinaryTableParser(const TableObject& tableFormat, const std::string& input)
	: m_tableFormat(tableFormat)
	, m_input(input)
{
}

DptfBuffer TableObjectBinaryTableParser::extractTable() const
{
	DptfBuffer tableData;
	tableData.append(extractRevision());
	tableData.append(extractMode());
	tableData.append(extractRows());
	return tableData;
}

DptfBuffer TableObjectBinaryTableParser::extractRevision() const
{
	DptfBuffer tableData;
	if (m_tableFormat.hasRevisionField())
	{
		UInt32 reserved = 4;
		tableData.append((UInt8*)&reserved, sizeof(reserved));
		UInt64 revision = StringConverter::toUInt64(m_input.getRevision());
		tableData.append((UInt8*)&revision, sizeof(revision));
	}
	return tableData;
}

DptfBuffer TableObjectBinaryTableParser::extractMode() const
{
	DptfBuffer tableData;
	if (m_tableFormat.hasModeField())
	{
		UInt32 reserved = 4;
		tableData.append((UInt8*)&reserved, sizeof(reserved));
		UInt64 mode = StringConverter::toUInt64(m_input.getMode());
		tableData.append((UInt8*)&mode, sizeof(mode));
	}
	return tableData;
}

DptfBuffer TableObjectBinaryTableParser::extractRows() const
{
	DptfBuffer tableData;
	const auto tableObjectFields = m_tableFormat.getFieldsMap().find(getRevisionValue());
	if (tableObjectFields != m_tableFormat.getFieldsMap().end())
	{
		const auto inputRows = m_input.getRows();
		for (auto& inputRow : inputRows)
		{
			tableData.append(extractRow(inputRow, tableObjectFields->second));
		}
	}
	return tableData;
}

DptfBuffer TableObjectBinaryTableParser::extractRow(
	const std::vector<std::string>& inputRow,
	const std::vector<TableObjectField>& fieldDefinitions) const
{
	DptfBuffer rowData;
	if (inputRow.size() == fieldDefinitions.size())
	{
		for (UInt32 i = 0; i < inputRow.size(); ++i)
		{
			rowData.append(extractField(inputRow[i], fieldDefinitions[i]));
		}
	}
	return rowData;
}

DptfBuffer TableObjectBinaryTableParser::extractField(
	const std::string& fieldValue,
	const TableObjectField& fieldDefinition) const
{
	if (fieldDefinition.m_fieldDataType == ESIF_DATA_STRING)
	{
		return extractFieldString(fieldValue, fieldDefinition);
	}
	else
	{
		return extractFieldNumber(fieldValue, fieldDefinition);
	}
}

DptfBuffer TableObjectBinaryTableParser::extractFieldString(
	const std::string& fieldValue,
	const TableObjectField& fieldDefinition) const
{
	DptfBuffer fieldData(fieldDefinition.m_fieldLength);
	fieldData.put(0, (UInt8*)fieldValue.data(), (UInt32)fieldValue.size());
	return fieldData;
}

DptfBuffer TableObjectBinaryTableParser::extractFieldNumber(
	const std::string& fieldValue,
	const TableObjectField& fieldDefinition) const
{
	if (fieldDefinition.m_fieldLength <= 8)
	{
		UInt64 fieldDataValue = StringConverter::toUInt64(fieldValue);
		DptfBuffer fieldData =
			DptfBuffer::fromExistingByteArray((UInt8*)&fieldDataValue, fieldDefinition.m_fieldLength);
		return fieldData;
	}
	else
	{
		return {};
	}
}

UInt64 TableObjectBinaryTableParser::getRevisionValue() const
{
	UInt64 revision = 0;
	if (m_tableFormat.hasRevisionField())
	{
		revision = StringConverter::toUInt64(m_input.getRevision());
	}
	return revision;
}
