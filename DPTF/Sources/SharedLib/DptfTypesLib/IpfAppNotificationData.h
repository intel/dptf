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

const Guid DttToOemAppCommunicationGuid(0x8C, 0xF9, 0x06, 0x02, 0x3D, 0xB0, 0x49, 0xD0, 0xBC, 0x21, 0xFC, 0xB1, 0xC0, 0x0D, 0x0F, 0x57); // 8cf90602-3db0-49d0-bc21-fcb1c00d0f57

namespace DttToOemSwNotificationData
{
	typedef struct OemSwNotificationData
	{
		esif_guid_t guidOemDttAppCommunication;
		UInt32 dataPayloadLength;
		UInt32 dataPayload;
	} OemSwNotificationData, *OemSwNotificationDataPtr;
}
