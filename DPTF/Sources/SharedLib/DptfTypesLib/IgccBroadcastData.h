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

#pragma once

#include "Dptf.h"

const Guid IGCC_BROADCAST_GUID(0xD6, 0x24,0xDB,0xA2,0xEC,0xD8,0x48,0x2C,0xAB,0x7F,0xC9, 0xA8, 0x61, 0xAA, 0x6D, 0x72); // d624dba2-ecd8-482c-ab7f-c9a861aa6d72
const Guid IGCC_SEND_GUID(0xB3, 0xB3, 0xCF, 0xE6, 0xA5, 0x9C, 0x4C, 0xB6, 0xBD, 0x99, 0x7E, 0xDF, 0x4F, 0xE1, 0xF4, 0xBB); // b3b3cfe6-a59c-4cb6-bd99-7edf4fe1f4bb
const UInt32 IGCC_DATA_PAYLOAD_LENGTH = 3;
const UInt8 IGCC_TOTAL_AVAILABLE_LEVELS = 6;
const UInt8 INVALID_PPM = 0;

//
const UInt8 ENDURANCE_GAMING_ON = 1;
const UInt8 ENDURANCE_GAMING_OFF = 0;
const UInt8 ENDURANCE_GAMING_ERROR_AC = 3;
const UInt8 ENDURANCE_GAMING_ERROR_FAIL_SET_PPM = 2;

namespace IgccBroadcastData
{
#pragma pack(push, 1)
	struct IgccToDttNotificationPackage
	{
		esif_guid_t uuid; // d624dba2-ecd8-482c-ab7f-c9a861aa6d72
		UInt32 dataPayloadLength;
		UInt8 enduranceGamingStatus; // (EG status: 0:off, 1:on, 2:fail set ppm, 3: off for AC mode)
		UInt8 ppmSettingDirection; // (Up:1, Down:0)
		UInt8 ppmSettingLevel; // How many levels to move based on PPM Direction P1-P6)
	};

	struct DttToIgccSendPackage
	{
		esif_guid_t uuid; // b3b3cfe6-a59c-4cb6-bd99-7edf4felf4bb
		UInt32 dataPayloadLength;
		UInt8 enduranceGamingStatus;
		UInt8 currentPpmSettingLevel;
		UInt8 totalAvailableLevels;
	};

#pragma pack(pop)
}
