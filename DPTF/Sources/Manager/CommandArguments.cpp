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

#include "CommandArguments.h"

CommandArguments::CommandArguments()
	: m_arguments()
{
}

CommandArguments::CommandArguments(const std::vector<std::string>& arguments)
{
	for (auto arg = arguments.begin(); arg != arguments.end(); ++arg)
	{
		m_arguments.push_back(CommandArgument(*arg));
	}
}

CommandArguments::CommandArguments(const std::vector<CommandArgument>& arguments)
	: m_arguments(arguments)
{
}

CommandArguments::~CommandArguments()
{
}

UInt32 CommandArguments::size() const
{
	return (UInt32)m_arguments.size();
}

CommandArgument CommandArguments::operator[](UInt32 index) const
{
	if (index < m_arguments.size())
	{
		return m_arguments[index];
	}
	else
	{
		throw dptf_out_of_range("Argument index out of range");
	}
}

void CommandArguments::remove(UInt32 argumentNumber)
{
	if (argumentNumber < size())
	{
		m_arguments.erase(m_arguments.begin() + argumentNumber);
	}
	else
	{
		throw dptf_out_of_range("Argument index out of range");
	}
}

CommandArguments CommandArguments::parse(UInt32 argCount, EsifDataArray argData)
{
	std::vector<CommandArgument> arguments;
	for (UInt32 argNum = 0; argNum < argCount; ++argNum)
	{
		arguments.push_back(CommandArgument(argData[argNum]));
	}
	return CommandArguments(arguments);
}
