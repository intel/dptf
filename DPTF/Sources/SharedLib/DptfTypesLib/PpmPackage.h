/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

namespace PpmPackage
{
	struct PpmParam
	{
		UIntN ppmType;
		esif_guid_t subgroupGuid;
		esif_guid_t paramGuid;
		UIntN paramValue;
	};

	enum Type
	{
		Performance = 1,
		Responsiveness = 2,
		BatteryLife = 3,
		PerformanceAutonomous = 4,
		ResponsivenessAutonomous = 5,
		BatteryLifeAutonomous = 6,
		
		Invalid
	};

	std::string toString(PpmPackage::Type package);
	PpmPackage::Type toState(std::string state);
	PpmPackage::Type fromString(const std::string package);
	PpmPackage::Type toState(UInt32 state);
}
