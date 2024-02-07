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

#include "ConditionType.h"

namespace ConditionType
{
	std::string ToString(ConditionType::Type type)
	{
		switch (type)
		{
		case ConditionType::PlatformOrientation:
			return "Platform Orientation";
		case ConditionType::UserProximity:
			return "User Proximity";
		case ConditionType::InMotion:
			return "In Motion";
		case ConditionType::DockMode:
			return "Dock Mode";
		case ConditionType::Workload:
			return "Workload";
		case ConditionType::CoolingMode:
			return "Cooling Mode";
		case ConditionType::PowerSource:
			return "Power Source";
		case ConditionType::AggregateBatteryPercentage:
			return "Aggregate Battery Percentage";
		case ConditionType::LidState:
			return "Lid State";
		case ConditionType::PlatformType:
			return "Platform Type";
		case ConditionType::PlatformSku:
			return "Platform SKU";
		case ConditionType::Utilization:
			return "Utilization";
		case ConditionType::DutyCycle:
			return "Duty Cycle";
		case ConditionType::Power:
			return "Power";
		case ConditionType::TemperatureWithHysteresis:
			return "Temperature With Hysteresis";
		case ConditionType::DisplayOrientation:
			return "Display Orientation";
		case ConditionType::OemVariable0:
			return "OEM Variable 0";
		case ConditionType::OemVariable1:
			return "OEM Variable 1";
		case ConditionType::OemVariable2:
			return "OEM Variable 2";
		case ConditionType::OemVariable3:
			return "OEM Variable 3";
		case ConditionType::OemVariable4:
			return "OEM Variable 4";
		case ConditionType::OemVariable5:
			return "OEM Variable 5";
		case ConditionType::MaxBatteryPower:
			return "Battery Max Peak Power (PMAX)";
		case ConditionType::PlatformPowerSource:
			return "Platform Power Source (PSRC)";
		case ConditionType::AdapterPowerRating:
			return "Adapter Power Rating (ARTG)";
		case ConditionType::ChargerType:
			return "Charger Type (CTYP)";
		case ConditionType::PlatformRestOfPower:
			return "Rest of Platform Power (PROP)";
		case ConditionType::PlatformBatterySteadyState:
			return "Battery Sustained Peak Power (PBSS)";
		case ConditionType::BatteryState:
			return "Battery State";
		case ConditionType::BatteryPresentRate:
			return "Battery Present Rate";
		case ConditionType::BatteryRemainingCapacity:
			return "Battery Remaining Capacity";
		case ConditionType::BatteryPresentVoltage:
			return "Battery Present Voltage";
		case ConditionType::BatteryCycleCount:
			return "Battery Cycle Count";
		case ConditionType::BatteryLastFullChargeCapacity:
			return "Battery Last Full Charge Capacity";
		case ConditionType::BatteryDesignCapacity:
			return "Battery Design Capacity";
		case ConditionType::PowerSchemePersonality:
			return "Power Scheme Personality";
		case ConditionType::ScreenState:
			return "Screen State";
		case ConditionType::ACNominalVoltage:
			return "AC Voltage (AVOL)";
		case ConditionType::ACOperationalCurrent:
			return "AC Current (ACUR)";
		case ConditionType::AC1msPercentageOverload:
			return "AC Peak Percentage 1ms (AP01)";
		case ConditionType::AC2msPercentageOverload:
			return "AC Peak Percentage 2ms (AP02)";
		case ConditionType::AC10msPercentageOverload:
			return "AC Peak Percentage 10ms (AP10)";
		case ConditionType::Time:
			return "Time";
		case ConditionType::TemperatureWithoutHysteresis:
			return "Temperature Without Hysteresis";
		case ConditionType::MixedRealityMode:
			return "Mixed Reality Mode";
		case ConditionType::UserPresence:
			return "User Presence";
		case ConditionType::BatteryHighFrequencyImpedance:
			return "Battery High Frequency Impedance (RBHF)";
		case ConditionType::BatteryNoLoadVoltage:
			return "Battery Instantaneous No-Load Voltage (VBNL)";
		case ConditionType::BatteryMaxPeakCurrent:
			return "Battery Max Peak Current (CMPP)";
		case ConditionType::BatteryPercentage:
			return "Battery Percentage";
		case ConditionType::BatteryCount:
			return "Battery Count";
		case ConditionType::PowerSlider:
			return "Power Slider";
		case ConditionType::SocWorkloadClassification:
			return "Soc Workload Classification";
		case ConditionType::GameMode:
			return "Game Mode";
		case ConditionType::PlatformUserPresence:
			return "Platform User Presence";
		case ConditionType::UserInteraction:
			return "User Interaction";
		case ConditionType::NumberOfMonitors:
			return "Number of Monitors";
		case ConditionType::NumberOfExternalMonitors:
			return "Number of External Monitors";
		case ConditionType::NumberOfExternalWiredMonitors:
			return "Number of External Wired Monitors";
		case ConditionType::NumberOfExternalWirelessMonitors:
			return "Number of External Wireless Monitors";
		case ConditionType::HighestDisplayResolution:
			return "Highest Display Resolution";
		case ConditionType::HighestExternalDisplayResolution:
			return "Highest External Display Resolution";
		case ConditionType::HighestExternalWiredDisplayResolution:
			return "Highest External Wired Display Resolution";
		case ConditionType::HighestExternalWirelessDisplayResolution:
			return "Highest External Wireless Display Resolution";
		case ConditionType::TotalExternalDisplayResolution:
			return "Total External Display Resolution";
		case ConditionType::TpgPowerState:
			return "TPG Power State";
		case ConditionType::SwOemVariable0:
			return "SW OEM Variable 0";
		case ConditionType::SwOemVariable1:
			return "SW OEM Variable 1";
		case ConditionType::SwOemVariable2:
			return "SW OEM Variable 2";
		case ConditionType::SwOemVariable3:
			return "SW OEM Variable 3";
		case ConditionType::SwOemVariable4:
			return "SW OEM Variable 4";
		case ConditionType::SwOemVariable5:
			return "SW OEM Variable 5";
		case ConditionType::CollaborationMode:
			return "Collaboration Mode";
		case ConditionType::ExtendedWorkloadPrediction:
			return "Extended Workload Prediction";
		case ConditionType::FanOperatingMode:
			return "Fan Operating Mode";
		case ConditionType::SocPowerFloor:
			return "SoC Power Floor";
		case ConditionType::SocBasePower:
			return "SoC Base Power";
		case ConditionType::SystemInBag:
			return "System In Bag";
		case ConditionType::Max:
		case ConditionType::DoNotCare:
		case ConditionType::ParticipantConditionBaseId:
		case ConditionType::OemConditionBaseId:
		case ConditionType::MaxStaticConditionId:
		default:
			return Constants::InvalidString;
		}
	}

	Bool IsParticipantCondition(ConditionType::Type type)
	{
		if (IsCustomizedParticipantCondition(type))
		{
			return true;
		}

		switch (type)
		{
		case ConditionType::DutyCycle:
		case ConditionType::Power:
		case ConditionType::TemperatureWithHysteresis:
		case ConditionType::MaxBatteryPower:
		case ConditionType::PlatformPowerSource:
		case ConditionType::AdapterPowerRating:
		case ConditionType::ChargerType:
		case ConditionType::PlatformRestOfPower:
		case ConditionType::BatteryPresentRate:
		case ConditionType::BatteryPresentVoltage:
		case ConditionType::PlatformBatterySteadyState:
		case ConditionType::BatteryCycleCount:
		case ConditionType::ACNominalVoltage:
		case ConditionType::ACOperationalCurrent:
		case ConditionType::AC1msPercentageOverload:
		case ConditionType::AC2msPercentageOverload:
		case ConditionType::AC10msPercentageOverload:
		case ConditionType::TemperatureWithoutHysteresis:
		case ConditionType::BatteryHighFrequencyImpedance:
		case ConditionType::BatteryNoLoadVoltage:
		case ConditionType::BatteryMaxPeakCurrent:
		case ConditionType::BatteryState:
		case ConditionType::BatteryRemainingCapacity:
		case ConditionType::BatteryLastFullChargeCapacity:
		case ConditionType::BatteryDesignCapacity:
		case ConditionType::BatteryPercentage:
		case ConditionType::SocWorkloadClassification:
			return true;

		default:
			return false;
		}
	}

	Bool IsCustomizedParticipantCondition(ConditionType::Type type)
	{
		if (type >= ConditionType::ParticipantConditionBaseId)
		{
			return true;
		}

		return false;
	}
}
