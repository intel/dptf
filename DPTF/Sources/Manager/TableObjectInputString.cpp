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
#include "TableObjectInputString.h"
#include "StringParser.h"
using namespace std;

#define DELIMITER_ROW '!'
#define DELIMITER_FIELD ','
#define DELIMITER_REVISION_MODE ':'

TableObjectInputString::TableObjectInputString(const std::string& input)
	: m_input(input)
{

}

std::string TableObjectInputString::getRevision() const
{
	const auto pieces = StringParser::split(m_input, DELIMITER_REVISION_MODE);
	if (pieces.empty())
	{
		return {};
	}
	else
	{
		return pieces[0];
	}
}

std::string TableObjectInputString::getMode() const
{
	const auto pieces = StringParser::split(m_input, DELIMITER_REVISION_MODE);
	if (pieces.size() > 2)
	{
		return pieces[1];
	}
	else
	{
		return {};
	}
}

vector<vector<string>> TableObjectInputString::getRows() const
{
	const size_t inputRowStartPosition = std::min(m_input.find_last_of(DELIMITER_REVISION_MODE) + 1, m_input.size());
	const auto rows = StringParser::split(m_input.substr(inputRowStartPosition), DELIMITER_ROW);
	vector<vector<string>> rowsAndFields(rows.size());
	for (auto& row : rows)
	{
		rowsAndFields.push_back(StringParser::split(row, DELIMITER_FIELD));
	}
	return rowsAndFields;
}
