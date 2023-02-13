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

namespace ControlKnobType
{
	enum Type
	{
		Invalid = 0,
		PowerControlPl1 = 0x00010000, // PL1 in mW
		PowerControlPl2 = 0x00010001, // PL2 in mW
		PowerControlPl3 = 0x00010002, // PL3 in mW
		PowerControlPl4 = 0x00010003, // PL4 in mW
		PerformanceControlPerfFrequency = 0x00020000, // Performance Target Freq(in % units. Indicates % from Max Turbo)
		PerformanceControlThrottleFrequency =
			0x00020001, // Throttle Target Freq (Indicates % from LFM/MFM for IA domain only)
		DbptControlIccMax = 0x00030000, // Current Limit in mA
		CoreControlLpo = 0x00040000, // Number of logical processors/exec units to be offlined
		DisplayControlBrightness = 0x00050000, // Display brightness in percentage units
		PercentageFanControl = 0x00060000, // Target fan speed in rpm
		TauControlPl1 = 0x00080000, // PL1 Tau in ms
		TauControlPl2 = 0x00080001, // PL2 Tau in ms
		TauControlPl3 = 0x00080002, // PL3 Tau in ms
		DataThroughput = 0x00090000, // Desired data throughput in kbps
		PerformanceControlPerfIndex = 0x000A0000, // Performance Target Index reported by PPSS or _PSS
		PSysPowerControlPl1 = 0x000B0000, // PSys PL1 in mW
		PSysPowerControlPl2 = 0x000B0001, // PSys PL2 in mW
		PSysPowerControlPl3 = 0x000B0002, // PSys PL3 in mW
		RPMFanControl = 0x000C0000 // RPM fan Control in Integer
	};

	std::string toString(ControlKnobType::Type type);
	ControlKnobType::Type fromString(const std::string& typeName);
}
