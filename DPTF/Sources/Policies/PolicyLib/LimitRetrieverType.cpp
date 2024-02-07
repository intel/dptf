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

#include "LimitRetrieverType.h"
#include "StringConverter.h"
using namespace std;

namespace LimitRetrieverType
{
	std::string ToString(LimitRetrieverType::Type type)
	{
		switch (type)
		{
		case LimitRetrieverType::Maximum:
			return "MAX";
		case LimitRetrieverType::Minimum:
			return "MIN";
		case LimitRetrieverType::Fixed:
			return "";
		default:
			throw dptf_exception("LimitRetrieverType::Type is invalid.");
		}
	}
	LimitRetrieverType::Type FromString(const std::string& value)
	{
		const string cleanedValue = StringConverter::toUpper(StringConverter::trimWhitespace(value));
		if (cleanedValue == "MAX"s)
		{
			return LimitRetrieverType::Maximum;
		}
		else if (cleanedValue == "MIN"s)
		{
			return LimitRetrieverType::Minimum;
		}
		else
		{
			return LimitRetrieverType::Fixed;
		}
	}
}
