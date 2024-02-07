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
#include "WorkloadTypeByDevice.h"
#include "StringConverter.h"

using namespace std;

namespace WorkloadTypeByDevice
{
	string toString(Type type)
	{
		switch (type)
		{
		case Gpu:
			return "GPU";
		case Combo:
			return "Combo";
		case Cpu:
			return "CPU";
		default:
			return Constants::InvalidString;
		}
	}

	Type fromString(const string& type)
	{
		const string parsedValue = StringConverter::toUpper(StringConverter::trimWhitespace(type));
		if (parsedValue == StringConverter::toUpper(toString(Gpu)))
		{
			return Gpu;
		}

		if (parsedValue == StringConverter::toUpper(toString(Combo)))
		{
			return Combo;
		}
		
		if (parsedValue == StringConverter::toUpper(toString(Cpu)))
		{
			return Cpu;
		}
		
		return InvalidMax;
	}

	Type fromUInt32(const UInt32& value)
	{
		switch (value)
		{
		case Gpu:
			return Gpu;
		case Combo:
			return Combo;
		case Cpu:
			return Cpu;
		default:
			return InvalidMax;
		}
	}
}
