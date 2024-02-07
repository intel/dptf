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
#include "ToleranceLevel.h"
#include "StringConverter.h"

using namespace std;

namespace ToleranceLevel
{
	std::string toString(Level level)
	{
		switch (level)
		{
		case ToleranceLevel::Low:
			return "Low";
		case ToleranceLevel::Medium:
			return "Medium";
		case ToleranceLevel::High:
			return "High";
		default:
			return Constants::InvalidString;
		}
	}

	Level fromString(const std::string& levelName)
	{
		const string cleanedValue = StringConverter::toUpper(StringConverter::trimWhitespace(levelName));
		if (cleanedValue == StringConverter::toUpper(toString(Low)))
		{
			return ToleranceLevel::Low;
		}

		if (cleanedValue == StringConverter::toUpper(toString(Medium)))
		{
			return ToleranceLevel::Medium;
		}

		if (cleanedValue == StringConverter::toUpper(toString(High)))
		{
			return ToleranceLevel::High;
		}

		return ToleranceLevel::Invalid;
	}
}
