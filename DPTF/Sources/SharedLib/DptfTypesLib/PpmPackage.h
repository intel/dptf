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

#pragma once

#include "Dptf.h"
#include "OsPowerSource.h"

namespace PpmPackage
{
	struct PpmParam
	{
		UIntN ppmType;
		esif_guid_t subgroupGuid;
		esif_guid_t paramGuid;
		UIntN paramValue;
		UIntN betterBattery;
		UIntN betterPerformance;
		UIntN powerSource;
	};

	enum Type
	{
		P1 = 1,
		P2 = 2,
		P3 = 3,
		P4 = 4,
		P5 = 5,
		P6 = 6,
		P1AC = 7,
		P2AC = 8,
		P3AC = 9,
		P4AC = 10,
		P5AC = 11,
		P6AC = 12,
		Invalid = 0
	};

	std::string toString(PpmPackage::Type package);
	PpmPackage::Type toState(std::string state);
	PpmPackage::Type fromString(const std::string package);
	PpmPackage::Type toState(UInt32 state);
	std::string toStringBasedOnOS(PpmPackage::Type package, OsPowerSource::Type osPowerSource);
}
