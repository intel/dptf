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

#include "ControlFactoryType.h"

std::string ControlFactoryType::ToString(Type factoryType)
{
	switch (factoryType)
	{
	case ControlFactoryType::Active:
		return "Active";
	case ControlFactoryType::ConfigTdp:
		return "ConfigTdp";
	case ControlFactoryType::Core:
		return "Core";
	case ControlFactoryType::Display:
		return "Display";
	case ControlFactoryType::Performance:
		return "Performance";
	case ControlFactoryType::PlatformPower:
		return "PlatformPower";
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
	case ControlFactoryType::TccOffsetControl:
		return "TccOffsetControl";
	default:
		throw dptf_exception("Invalid control factory type.");
	}
}
