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
#include <string>
#include <vector>
#include "Dptf.h"

class dptf_export TableObjectInputString
{
public:
	TableObjectInputString(const std::string& input);
	virtual ~TableObjectInputString() = default;

	std::string getRevision() const;
	std::string getMode() const;
	std::vector<std::vector<std::string>> getRows() const;

private:
	std::string m_input;
};
