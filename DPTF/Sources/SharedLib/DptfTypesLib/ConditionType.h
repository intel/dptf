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

#pragma once

#include "Dptf.h"

namespace ConditionType
{
	enum Type
	{
		DoNotCare = 1,
		PlatformOrientation = 2,
		UserProximity = 3,
		InMotion = 4,
		DockMode = 5,
		Workload = 6,
		CoolingMode = 7,
		PowerSource = 8,
		AggregateBatteryPercentage = 9,
		LidState = 10,
		PlatformType = 11,
		PlatformSku = 12,
		Utilization = 13,
		DutyCycle = 15,
		Power = 16,
		TemperatureWithHysteresis = 17,
		DisplayOrientation = 18,
		OemVariable0 = 19,
		OemVariable1 = 20,
		OemVariable2 = 21,
		OemVariable3 = 22,
		OemVariable4 = 23,
		OemVariable5 = 24,
		MaxBatteryPower = 25,
		PlatformPowerSource = 26,
		AdapterPowerRating = 27,
		ChargerType = 28,
		PlatformRestOfPower = 29,
		BatteryState = 32,
		BatteryPresentRate = 33,
		BatteryRemainingCapacity = 34,
		BatteryPresentVoltage = 35,
		PlatformBatterySteadyState = 36,
		BatteryCycleCount = 37,
		BatteryLastFullChargeCapacity = 38,
		PowerSchemePersonality = 39,
		BatteryDesignCapacity = 40,
		ScreenState = 41,
		ACNominalVoltage = 42,
		ACOperationalCurrent = 43,
		AC1msPercentageOverload = 44,
		AC2msPercentageOverload = 45,
		AC10msPercentageOverload = 46,
		Time = 47,
		TemperatureWithoutHysteresis = 48,
		MixedRealityMode = 49,
		UserPresence = 50,
		BatteryHighFrequencyImpedance = 51,
		BatteryNoLoadVoltage = 52,
		BatteryMaxPeakCurrent = 53,
		BatteryPercentage = 54,
		BatteryCount = 55,
		PowerSlider = 56,
		SocWorkloadClassification = 57,
		GameMode = 58,
		PlatformUserPresence = 59,
		UserInteraction = 60,
		NumberOfMonitors = 61,
		NumberOfExternalMonitors = 62,
		NumberOfExternalWiredMonitors = 63,
		NumberOfExternalWirelessMonitors = 64,
		HighestDisplayResolution = 65,
		HighestExternalDisplayResolution = 66,
		HighestExternalWiredDisplayResolution = 67,
		HighestExternalWirelessDisplayResolution = 68,
		TotalExternalDisplayResolution = 69,
		TpgPowerState = 70,
		SwOemVariable0 = 71,
		SwOemVariable1 = 72,
		SwOemVariable2 = 73,
		SwOemVariable3 = 74,
		SwOemVariable4 = 75,
		SwOemVariable5 = 76,
		CollaborationMode = 77,
		ExtendedWorkloadPrediction = 78,
		FanOperatingMode = 79,
		SocPowerFloor = 80,
		SocBasePower = 81,
		SystemInBag = 82,
		Max = 83,
		MaxStaticConditionId = 4095,
		OemConditionBaseId = 4096,
		SwOemConditionBaseId = 8192,
		ParticipantConditionBaseId = 65536
	};

	std::string ToString(ConditionType::Type type);
	Bool IsParticipantCondition(ConditionType::Type type);
	Bool IsCustomizedParticipantCondition(ConditionType::Type type);
}
