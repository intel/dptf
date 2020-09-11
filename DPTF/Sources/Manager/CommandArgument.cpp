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

#include "CommandArgument.h"

CommandArgument::CommandArgument(const std::string& argString)
{
	m_dataType = ESIF_DATA_STRING;
	m_data.put(0, (UInt8*)argString.c_str(), (UInt32)argString.size());
}

CommandArgument::CommandArgument(EsifData argData)
{
	m_dataType = argData.type;
	m_data.put(0, (UInt8*)argData.buf_ptr, std::min(argData.buf_len, argData.data_len));
}

CommandArgument::~CommandArgument()
{
}

bool CommandArgument::isDataTypeString() const
{
	return m_dataType == ESIF_DATA_STRING;
}

std::string CommandArgument::getDataAsString() const
{
	if (isDataTypeString())
	{
		auto data = std::string((char*)m_data.get(), m_data.size());
		auto positionOfNull = data.find('\0');
		while (positionOfNull != std::string::npos)
		{
			data.erase(positionOfNull);
			positionOfNull = data.find('\0');
		}
		return data;
	}
	else
	{
		throw invalid_data_type("Command argument is not of type 'string'");
	}
}

const DptfBuffer& CommandArgument::getData() const
{
	return m_data;
}
