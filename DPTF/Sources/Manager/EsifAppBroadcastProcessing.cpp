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

#include "EsifAppBroadcastProcessing.h"
#define MAX_BYTES_SW_OEM_VAR_DATA 8192

EsifAppBroadcastProcessing::EsifAppBroadcastProcessing()
{
}

EsifAppBroadcastProcessing::~EsifAppBroadcastProcessing()
{
}

std::shared_ptr<WorkItem> EsifAppBroadcastProcessing::FindAppBroadcastIdAndCreateWorkItem(
	DptfManagerInterface* dptfManager,
	const EsifDataPtr esifEventDataPtr)
{
	EsifAppBroadcastHeader* broadcastNotificationDataHeader = (EsifAppBroadcastHeader*)esifEventDataPtr->buf_ptr;
	Guid broadcastGuid(broadcastNotificationDataHeader->UUID);
	std::shared_ptr<WorkItem> wi= nullptr;
	if (IGCC_BROADCAST_GUID == broadcastGuid)
	{
		IgccBroadcastData::IgccToDttNotificationPackage* igccNotificationData =
			(IgccBroadcastData::IgccToDttNotificationPackage*)esifEventDataPtr->buf_ptr;
		wi = std::make_shared<WIDptfIgccBroadcastReceived>(dptfManager, *igccNotificationData);
	}
	else if (SW_OEM_VAR_GUID == broadcastGuid)
	{
		UInt8* eventData = (UInt8*)(broadcastNotificationDataHeader->UUID + sizeof(EsifAppBroadcastHeader));
		DptfBuffer swOemVariablesData;
		if (broadcastNotificationDataHeader->dataLen >= MAX_BYTES_SW_OEM_VAR_DATA)
		{
			swOemVariablesData = DptfBuffer::fromExistingByteArray(eventData, MAX_BYTES_SW_OEM_VAR_DATA);
		}
		else
		{
			swOemVariablesData = DptfBuffer::fromExistingByteArray(eventData, broadcastNotificationDataHeader->dataLen);
		}
		wi = std::make_shared<WIDptfSwOemVariablesBroadcastReceived>(dptfManager, swOemVariablesData);
	}
	return wi;
}
