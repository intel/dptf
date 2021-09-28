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

#include "PpmPackage.h"
#include "StringConverter.h"
#include "StringParser.h"

namespace PpmPackage
{
	std::string toString(PpmPackage::Type package)
	{
		switch (package)
		{
		case PpmPackage::P1:
			return "P1";
		case PpmPackage::P2:
			return "P2";
		case PpmPackage::P3:
			return "P3";
		case PpmPackage::P4:
			return "P4";
		case PpmPackage::P5:
			return "P5";
		case PpmPackage::P6:
			return "P6";
		default:
			return Constants::InvalidString;
		}
	}

	PpmPackage::Type toState(std::string state)
	{
		if (StringConverter::toUInt32(state) == 1)
		{
			return PpmPackage::P1;
		}
		if (StringConverter::toUInt32(state) == 2)
		{
			return PpmPackage::P2;
		}
		if (StringConverter::toUInt32(state) == 3)
		{
			return PpmPackage::P3;
		}
		if (StringConverter::toUInt32(state) == 4)
		{
			return PpmPackage::P4;
		}
		if (StringConverter::toUInt32(state) == 5)
		{
			return PpmPackage::P5;
		}
		if (StringConverter::toUInt32(state) == 6)
		{
			return PpmPackage::P6;
		}
		throw dptf_exception("PPM Package input value is invalid");
	}

	PpmPackage::Type fromString(const std::string package)
	{
		const std::string packageName = StringConverter::toUpper(package);

		if (packageName == StringConverter::toUpper(toString(PpmPackage::P1)))
		{
			return PpmPackage::P1;
		}
		if (packageName == StringConverter::toUpper(toString(PpmPackage::P2)))
		{
			return PpmPackage::P2;
		}
		if (packageName == StringConverter::toUpper(toString(PpmPackage::P3)))
		{
			return PpmPackage::P3;
		}
		if (packageName == StringConverter::toUpper(toString(PpmPackage::P4)))
		{
			return PpmPackage::P4;
		}
		if (packageName == StringConverter::toUpper(toString(PpmPackage::P5)))
		{
			return PpmPackage::P5;
		}
		if (packageName == StringConverter::toUpper(toString(PpmPackage::P6)))
		{
			return PpmPackage::P6;
		}

		return PpmPackage::Invalid;
	}

	PpmPackage::Type toState(UInt32 state)
	{
		if (state == 1)
		{
			return PpmPackage::P1;
		}
		if (state == 2)
		{
			return PpmPackage::P2;
		}
		if (state == 3)
		{
			return PpmPackage::P3;
		}
		if (state == 4)
		{
			return PpmPackage::P4;
		}
		if (state == 5)
		{
			return PpmPackage::P5;
		}
		if (state == 6)
		{
			return PpmPackage::P6;
		}
		throw dptf_exception("PPM Package input value is invalid");
	}

	std::string toStringBasedOnOS(PpmPackage::Type package, OsPowerSource::Type osPowerSource)
	{
		if (osPowerSource == OsPowerSource::AC)
		{
			switch (package)
			{
			case PpmPackage::P1:
				return toString(PpmPackage::P1AC);
			case PpmPackage::P2:
				return toString(PpmPackage::P2AC);
			case PpmPackage::P3:
				return toString(PpmPackage::P3AC);
			case PpmPackage::P4:
				return toString(PpmPackage::P4AC);
			case PpmPackage::P5:
				return toString(PpmPackage::P5AC);
			case PpmPackage::P6:
				return toString(PpmPackage::P6AC);
			default:
				return Constants::InvalidString;
			}
		}
		return toString(package);
	}
}
