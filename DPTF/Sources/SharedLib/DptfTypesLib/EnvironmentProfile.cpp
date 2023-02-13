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

#include "EnvironmentProfile.h"
#include "StringConverter.h"
using namespace std;

constexpr UInt64 SINGLE_HEXADECIMAL_LENGTH = 4;
const string unknownPlatformGeneration = "Unknown"s;
const map<UInt64, string> platformGenerations
{
	{0x906A, "ADL"s},
	{0x9067, "ADL"s},
	{0xB06A, "RPL"s},
	{0xB067, "RPL"s},
	{0xB06F, "RPL"s},
	{0xA06A, "MTL"s},
	{0xA06C, "MTL"s},
};

EnvironmentProfile::EnvironmentProfile(UInt64 cpuIdRaw)
	: cpuIdValue(cpuIdRaw)
	, cpuIdWithoutSteppingValue(cpuIdRaw >> SINGLE_HEXADECIMAL_LENGTH)
	, cpuId(StringConverter::toHexString(cpuIdValue))
	, cpuIdWithoutStepping(StringConverter::toHexString(cpuIdWithoutSteppingValue))
	, platformGeneration(findPlatformGeneration(cpuIdWithoutSteppingValue))
{
	
}

std::string EnvironmentProfile::toString() const
{
	stringstream stream;
	stream << "Platform: "s << platformGeneration << ". "s;
	stream << "CPUID: "s << cpuId << ". "s;
	return stream.str();
}

string EnvironmentProfile::findPlatformGeneration(UInt64 cpuIdWithoutSteppingValue)
{
	try
	{
		const auto result = platformGenerations.find(cpuIdWithoutSteppingValue);
		if (result == platformGenerations.end())
		{
			return unknownPlatformGeneration;
		}
		else
		{
			return result->second;
		}
	}
	catch (const exception&)
	{
		return unknownPlatformGeneration;
	}
}
