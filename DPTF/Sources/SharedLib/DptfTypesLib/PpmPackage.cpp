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

#include "PpmPackage.h"
#include "StringConverter.h"
#include "StringParser.h"

namespace PpmPackage
{
	std::string toString(PpmPackage::Type package)
	{
		switch (package)
		{
		case PpmPackage::Performance:
			return "Performance";
		case PpmPackage::Responsiveness:
			return "Responsiveness";
		case PpmPackage::BatteryLife:
			return "Battery Life";
		case PpmPackage::PerformanceAutonomous:
			return "Performance Autonomous";
		case PpmPackage::ResponsivenessAutonomous:
			return "Responsiveness Autonomous";
		case PpmPackage::BatteryLifeAutonomous:
			return "Battery Life Autonomous";
		default:
			return Constants::InvalidString;
		}
	}

	PpmPackage::Type toState(std::string state)
	{
		if (StringConverter::toUInt32(state) == 1)
		{
			return PpmPackage::Performance;
		}
		if (StringConverter::toUInt32(state) == 2)
		{
			return PpmPackage::Responsiveness;
		}
		if (StringConverter::toUInt32(state) == 3)
		{
			return PpmPackage::BatteryLife;
		}
		if (StringConverter::toUInt32(state) == 4)
		{
			return PpmPackage::PerformanceAutonomous;
		}
		if (StringConverter::toUInt32(state) == 5)
		{
			return PpmPackage::ResponsivenessAutonomous;
		}
		if (StringConverter::toUInt32(state) == 6)
		{
			return PpmPackage::BatteryLifeAutonomous;
		}
		throw dptf_exception("PPM Package input value is invalid");
	}

	PpmPackage::Type fromString(const std::string package)
	{
		const std::string packageName = StringConverter::toUpper(package);

		if (packageName == StringConverter::toUpper(toString(PpmPackage::Performance)))
		{
			return PpmPackage::Performance;
		}
		if (packageName == StringConverter::toUpper(toString(PpmPackage::Responsiveness)))
		{
			return PpmPackage::Responsiveness;
		}

		// Need to remove the extra whitespace
		const std::string battLife = StringConverter::toUpper(toString(PpmPackage::BatteryLife));
		const std::string battLifeNoSpace = StringParser::removeAll(battLife, ' ');
		
		if (packageName == battLifeNoSpace || packageName == battLife)
		{
			return PpmPackage::BatteryLife;
		}

		// Need to remove the extra whitespace
		const std::string perfAuto = StringConverter::toUpper(toString(PpmPackage::PerformanceAutonomous));
		const std::string perfAutoNoSpace = StringParser::removeAll(perfAuto, ' ');
		
		if (packageName == perfAutoNoSpace || packageName == perfAuto)
		{
			return PpmPackage::PerformanceAutonomous;
		}

		// Need to remove the extra whitespace
		const std::string respAuto = StringConverter::toUpper(toString(PpmPackage::ResponsivenessAutonomous));
		const std::string respAutoNoSpace = StringParser::removeAll(respAuto, ' ');
		
		if (packageName == respAutoNoSpace || packageName == respAuto)
		{
			return PpmPackage::ResponsivenessAutonomous;
		}

		// Need to remove the extra whitespace
		const std::string battLifeAuto = StringConverter::toUpper(toString(PpmPackage::BatteryLifeAutonomous));
		const std::string battLifeAutoNoSpace = StringParser::removeAll(battLifeAuto, ' ');
		
		if (packageName == battLifeAutoNoSpace || packageName == battLifeAuto)
		{
			return PpmPackage::BatteryLifeAutonomous;
		}
		
		return PpmPackage::Invalid;
	}

	PpmPackage::Type toState(UInt32 state)
	{
		if (state == 1)
		{
			return PpmPackage::Performance;
		}
		if (state == 2)
		{
			return PpmPackage::Responsiveness;
		}
		if (state == 3)
		{
			return PpmPackage::BatteryLife;
		}
		if (state == 4)
		{
			return PpmPackage::PerformanceAutonomous;
		}
		if (state == 5)
		{
			return PpmPackage::ResponsivenessAutonomous;
		}
		if (state == 6)
		{
			return PpmPackage::BatteryLifeAutonomous;
		}
		throw dptf_exception("PPM Package input value is invalid");
	}
}
