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
#pragma once
#include <string>
#include <vector>
#include "Dptf.h"
#include "TableObject.h"
#include "TableObjectInputString.h"

class dptf_export TableObjectBinaryTableParser
{
public:
	TableObjectBinaryTableParser(const TableObject& tableFormat, const std::string& input);
	virtual ~TableObjectBinaryTableParser() = default;

	DptfBuffer extractTable() const;

private:
	TableObject m_tableFormat;
	TableObjectInputString m_input;

	DptfBuffer extractRevision() const;
	DptfBuffer extractMode() const;
	DptfBuffer extractRows() const;
	DptfBuffer extractRow(const std::vector<std::string>& inputRow, const std::vector<TableObjectField>& fieldDefinitions) const;
	DptfBuffer extractField(const std::string& fieldValue, const TableObjectField& fieldDefinition) const;
	DptfBuffer extractFieldString(const std::string& fieldValue, const TableObjectField& fieldDefinition) const;
	DptfBuffer extractFieldNumber(const std::string& fieldValue, const TableObjectField& fieldDefinition) const;
	UInt64 getRevisionValue() const;
};
