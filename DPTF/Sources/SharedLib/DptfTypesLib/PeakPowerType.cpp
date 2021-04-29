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

#include "PeakPowerType.h"

std::string PeakPowerType::ToString(Type peakPowerType)
{
	switch (peakPowerType)
	{
	case PL4ACPower:
		return "PL4 AC Power";
	case PL4DCPower:
		return "PL4 DC Power";
	default:
		throw dptf_exception("Invalid peak power type requested for ToString");
	}
}

std::string PeakPowerType::ToXmlString(Type peakPowerType)
{
	switch (peakPowerType)
	{
	case PL4ACPower:
		return "PL4_AC_Power";
	case PL4DCPower:
		return "PL4_DC_Power";
	default:
		throw dptf_exception("Invalid peak power type requested for ToXmlString");
	}
}