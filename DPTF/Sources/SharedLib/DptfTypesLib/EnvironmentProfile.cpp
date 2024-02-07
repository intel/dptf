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
	{0xB06D, "LNL"s},
	{0xB065, "ARL"s},
	{0xC065, "ARL"s},
	{0xC066, "ARL"s},
	{0xC06C, "PTL"s},
	{0xC06D, "PTL"s},
	{0xB06E, "TWL"s},
	{0xD065, "WCL"s},
};

EnvironmentProfile::EnvironmentProfile()
	: EnvironmentProfile(0, Power::createInvalid())
{
}

EnvironmentProfile::EnvironmentProfile(UInt64 cpuIdRaw)
	: EnvironmentProfile(cpuIdRaw, Power::createInvalid())
{
}

EnvironmentProfile::EnvironmentProfile(UInt64 cpuIdRaw, const Power& socBasePower)
	: cpuIdValue(cpuIdRaw)
	, cpuIdWithoutSteppingValue(cpuIdRaw >> SINGLE_HEXADECIMAL_LENGTH)
	, socBasePower(socBasePower)
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
	stream << "SocBasePower: "s << socBasePower.toStringAsWatts(0) << "W. "s;
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
		return result->second;
	}
	catch (const exception&)
	{
		return unknownPlatformGeneration;
	}
}
