/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "PlatformPowerLimitType.h"

std::string PlatformPowerLimitType::ToString(Type limitType)
{
	switch (limitType)
	{
	case PSysPL1:
		return "PSys PL1";
	case PSysPL2:
		return "PSys PL2";
	case PSysPL3:
		return "PSys PL3";
	default:
		throw dptf_exception("Invalid platform power limit type requested for ToString");
	}
}

std::string PlatformPowerLimitType::ToXmlString(Type limitType)
{
	switch (limitType)
	{
	case PSysPL1:
		return "PSys_PL1";
	case PSysPL2:
		return "PSys_PL2";
	case PSysPL3:
		return "PSys_PL3";
	default:
		throw dptf_exception("Invalid platform power limit type requested for ToXmlString");
	}
}
