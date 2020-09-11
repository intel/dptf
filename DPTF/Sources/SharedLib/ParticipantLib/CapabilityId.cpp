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

#include "Dptf.h"
#include "CapabilityId.h"

#define CASE(eventType)                                                                                                \
	case eventType:                                                                                                    \
		return ((UInt32)(0x1 << eventType));

namespace Capability
{
	UInt32 ToCapabilityId(eEsifCapabilityType capabilityType)
	{
		switch (capabilityType)
		{
			CASE(ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_CORE_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_ENERGY_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY)
			CASE(ESIF_CAPABILITY_TYPE_PERF_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_POWER_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_POWER_STATUS)
			CASE(ESIF_CAPABILITY_TYPE_TEMP_STATUS)
			CASE(ESIF_CAPABILITY_TYPE_UTIL_STATUS)
			CASE(ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS)
			CASE(ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD)
			CASE(ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS)
			CASE(ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_CURRENT_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_PSYS_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL)
			CASE(ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL)
		default:
			throw dptf_exception("Capability::Type is invalid.");
		}
	}

	esif_data getEsifDataFromCapabilityData(EsifCapabilityData* capability)
	{
		esif_data eventData = {
			esif_data_type::ESIF_DATA_STRUCTURE, capability, sizeof(*capability), sizeof(*capability)};
		return eventData;
	}
}
