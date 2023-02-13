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

#include "OsPowerSchemePersonality.h"

// GUID_MIN_POWER_SAVINGS (8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c)
#define GUID_HIGH_PERFORMANCE_MODE                                                                                     \
	{                                                                                                                  \
		0xDA, 0x7F, 0x5E, 0x8C, 0xBF, 0xE8, 0x96, 0x4A, 0x9A, 0x85, 0xA6, 0xE2, 0x3A, 0x8C, 0x63, 0x5C                 \
	}

// GUID_MAX_POWER_SAVINGS (a1841308-3541-4fab-bc81-f71556f20b4a)
#define GUID_POWER_SAVER_MODE                                                                                          \
	{                                                                                                                  \
		0x08, 0x13, 0x84, 0xA1, 0x41, 0x35, 0xAB, 0x4F, 0xBC, 0x81, 0xF7, 0x15, 0x56, 0xF2, 0x0B, 0x4A                 \
	}

// GUID_TYPICAL_POWER_SAVINGS (381b4222-f694-41f0-9685-ff5bb260df2e)
#define GUID_BALANCED_MODE                                                                                             \
	{                                                                                                                  \
		0x22, 0x42, 0x1B, 0x38, 0x94, 0xF6, 0xF0, 0x41, 0x96, 0x85, 0xFF, 0x5B, 0xB2, 0x60, 0xDF, 0x2E                 \
	}

namespace OsPowerSchemePersonality
{
	std::string toString(OsPowerSchemePersonality::Type osPowerSchemePersonality)
	{
		switch (osPowerSchemePersonality)
		{
		case HighPerformance:
			return "HighPerformance";
		case PowerSaver:
			return "PowerSaver";
		case Balanced:
			return "Balanced";
		default:
			throw dptf_exception("OsPowerSchemePersonality::Type is invalid");
		}
	}

	OsPowerSchemePersonality::Type toType(Guid osPowerSchemePersonality)
	{
		if (osPowerSchemePersonality == Guid(GUID_HIGH_PERFORMANCE_MODE))
		{
			return OsPowerSchemePersonality::HighPerformance;
		}
		else if (osPowerSchemePersonality == Guid(GUID_POWER_SAVER_MODE))
		{
			return OsPowerSchemePersonality::PowerSaver;
		}
		else if (osPowerSchemePersonality == Guid(GUID_BALANCED_MODE))
		{
			return OsPowerSchemePersonality::Balanced;
		}
		else
		{
			return OsPowerSchemePersonality::Invalid;
		}
	}
}
