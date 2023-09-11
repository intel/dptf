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

#pragma once
#include "Dptf.h"
#include <string>

class dptf_export EnvironmentProfile
{
public:
	EnvironmentProfile();
	EnvironmentProfile(UInt64 cpuId);
	EnvironmentProfile(UInt64 cpuId, const Power& socBasePower);
	~EnvironmentProfile() = default;

	std::string toString() const;

	UInt64 cpuIdValue;
	UInt64 cpuIdWithoutSteppingValue;
	Power socBasePower;
	std::string cpuId;
	std::string cpuIdWithoutStepping;
	std::string platformGeneration;

private:
	static std::string findPlatformGeneration(UInt64 cpuIdWithoutSteppingValue);
};