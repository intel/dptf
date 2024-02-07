/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "esif_ccb.h"
#include "esif_sdk_data.h"
#include "CommandArgument.h"
#include <vector>

#define DEFAULT_COMMAND_OUTPUT_BUFFER_SIZE 512

class dptf_export CommandArguments
{
public:
	CommandArguments();
	CommandArguments(const std::vector<std::string>& arguments);
	CommandArguments(const std::vector<CommandArgument>& arguments);
	static CommandArguments parse(UInt32 argCount, EsifDataArray argData);
	virtual ~CommandArguments();

	UInt32 size() const;
	CommandArgument operator[](UInt32 index) const;
	void remove(UInt32 argumentNumber);

private:
	std::vector<CommandArgument> m_arguments;
};