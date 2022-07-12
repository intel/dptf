/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "ControlFactoryType.h"

std::string ControlFactoryType::toString(Type factoryType)
{
	switch (factoryType)
	{
	case ControlFactoryType::Active:
		return "Active";
	case ControlFactoryType::Core:
		return "Core";
	case ControlFactoryType::Display:
		return "Display";
	case ControlFactoryType::EnergyControl:
		return "EnergyControl";
	case ControlFactoryType::Performance:
		return "Performance";
	case ControlFactoryType::SystemPower:
		return "SystemPower";
	case ControlFactoryType::PlatformPowerStatus:
		return "PlatformPowerStatus";
	case ControlFactoryType::PowerControl:
		return "PowerControl";
	case ControlFactoryType::PowerStatus:
		return "PowerStatus";
	case ControlFactoryType::Priority:
		return "Priority";
	case ControlFactoryType::RfProfileControl:
		return "RfProfileControl";
	case ControlFactoryType::RfProfileStatus:
		return "RfProfileStatus";
	case ControlFactoryType::Temperature:
		return "Temperature";
	case ControlFactoryType::Utilization:
		return "Utilization";
	case ControlFactoryType::GetSpecificInfo:
		return "GetSpecificInfo";
	case ControlFactoryType::SetSpecificInfo:
		return "SetSpecificInfo";
	case ControlFactoryType::ActivityStatus:
		return "ActivityStatus";
	case ControlFactoryType::PeakPowerControl:
		return "PeakPowerControl";
	case ControlFactoryType::ProcessorControl:
		return "ProcessorControl";
	case ControlFactoryType::BatteryStatus:
		return "BatteryStatus";
	case ControlFactoryType::SocWorkloadClassification:
		return "SocWorkloadClassification";
	case ControlFactoryType::DynamicEpp:
		return "DynamicEpp";
	default:
		throw dptf_exception("Invalid control factory type.");
	}
}

std::string ControlFactoryType::getArbitratorString(Type factoryType)
{
	switch (factoryType)
	{
	case ControlFactoryType::Active:
		return "Active Control Arbitrator";
	case ControlFactoryType::Core:
		return "Core Control Arbitrator";
	case ControlFactoryType::Display:
		return "Display Control Arbitrator";
	case ControlFactoryType::PeakPowerControl:
		return "Peak Power Control Arbitrator";
	case ControlFactoryType::Performance:
		return "Performance Control Arbitrator";
	case ControlFactoryType::PowerControl:
		return "Power Control Arbitrator";
	case ControlFactoryType::ProcessorControl:
		return "Processor Control Arbitrator";
	case ControlFactoryType::SystemPower:
		return "System Power Control Arbitrator";
	case ControlFactoryType::Temperature:
		return "Temperature Control Arbitrator";
	case ControlFactoryType::ActivityStatus:
	case ControlFactoryType::BatteryStatus:
	case ControlFactoryType::EnergyControl:
	case ControlFactoryType::GetSpecificInfo:
	case ControlFactoryType::PlatformPowerStatus:
	case ControlFactoryType::PowerStatus:
	case ControlFactoryType::Priority:
	case ControlFactoryType::RfProfileControl:
	case ControlFactoryType::RfProfileStatus:
	case ControlFactoryType::SetSpecificInfo:
	case ControlFactoryType::Utilization:
	case ControlFactoryType::MAX:
	default:
		throw dptf_exception("Does not have an arbitrator.");
	}
}

const Guid ControlFactoryType::getArbitratorFormatId(Type factoryType)
{
	Guid formatId;

	switch (factoryType)
	{
	case ControlFactoryType::Active:
		// clang-format off
		formatId = Guid(0xAC, 0x0A, 0x0E, 0x91, 0x09, 0xA8, 0x69, 0x4E, 0x87, 0x5F, 0xE9, 0x49, 0xF7, 0x97, 0x53, 0xA0);
		// clang-format on
		break;
	case ControlFactoryType::Core:
		// clang-format off
		formatId = Guid(0xB7, 0x7F, 0xDD, 0xEB, 0xDA, 0x0C, 0xDA, 0x4B, 0xA0, 0x6C, 0x9B, 0xC3, 0x75, 0xCD, 0xC7, 0xE3);
		// clang-format on
		break;
	case ControlFactoryType::Display:
		// clang-format off
		formatId = Guid(0xBD, 0x18, 0x39, 0x7E, 0x40, 0x94, 0x3E, 0x4D, 0x95, 0x15, 0x9E, 0x5F, 0xEF, 0xFB, 0xCD, 0x3F);
		// clang-format on
		break;
	case ControlFactoryType::PeakPowerControl:
		// clang-format off
		formatId = Guid(0x45, 0x48, 0xAA, 0xD1, 0x75, 0xBF, 0x24, 0x47, 0x9A, 0x5D, 0x31, 0xAD, 0xEA, 0x4D, 0x7C, 0x82);
		// clang-format on
		break;
	case ControlFactoryType::Performance:
		// clang-format off
		formatId = Guid(0xAC, 0x27, 0x24, 0x21, 0x6E, 0xD8, 0xA2, 0x49, 0x88, 0x60, 0x94, 0x13, 0xC9, 0x17, 0xD3, 0x19);
		// clang-format on
		break;
	case ControlFactoryType::PowerControl:
		// clang-format off
		formatId = Guid(0xFF, 0x5A, 0xB4, 0x72, 0x1B, 0x29, 0x08, 0x4D, 0x8D, 0x54, 0x33, 0x58, 0xB7, 0xC8, 0x82, 0x1D);
		// clang-format on
		break;
	case ControlFactoryType::ProcessorControl:
		// clang-format off
		formatId = Guid(0xF3, 0xD5, 0x03, 0xF3, 0xE6, 0x64, 0x93, 0x47, 0x9A, 0x42, 0x4A, 0x79, 0x10, 0x6F, 0x78, 0xD7);
		// clang-format on
		break;
	case ControlFactoryType::SystemPower:
		// clang-format off
		formatId = Guid(0xF0, 0xD0, 0xDC, 0x6D, 0xAE, 0x95, 0xF2, 0x42, 0xB5, 0x48, 0x98, 0xCE, 0xF6, 0x86, 0x52, 0xDE);
		// clang-format on
		break;
	case ControlFactoryType::Temperature:
		// clang-format off
		formatId = Guid(0x7B, 0x86, 0x29, 0xA9, 0x09, 0xAE, 0x6E, 0x40, 0xAB, 0x8F, 0x8C, 0x4C, 0x6A, 0xF8, 0x7F, 0x69);
		// clang-format on
		break;
	case ControlFactoryType::ActivityStatus:
	case ControlFactoryType::BatteryStatus:
	case ControlFactoryType::EnergyControl:
	case ControlFactoryType::GetSpecificInfo:
	case ControlFactoryType::PlatformPowerStatus:
	case ControlFactoryType::PowerStatus:
	case ControlFactoryType::Priority:
	case ControlFactoryType::RfProfileControl:
	case ControlFactoryType::RfProfileStatus:
	case ControlFactoryType::SetSpecificInfo:
	case ControlFactoryType::Utilization:
	case ControlFactoryType::MAX:
	default:
		throw dptf_exception("Does not have an arbitrator.");
	}

	return formatId;
}
