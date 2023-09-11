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

#include "EsifAppBroadcastProcessing.h"
#include "NptWwanBandBroadcastData.h"
#include "esif_sdk.h"

using namespace std;

constexpr u32 MAX_BYTES_SW_OEM_VAR_DATA = 8192;
constexpr u32 MAX_BYTES_NPT_WWAN_DATA = 10 * sizeof(esif_data_rfprofile);

std::shared_ptr<WorkItem> EsifAppBroadcastProcessing::createWorkItem(
	DptfManagerInterface* dptfManager,
	const EsifDataPtr esifEventDataPtr)
{
	const auto broadcastHeader = static_cast<EsifAppBroadcastHeader*>(esifEventDataPtr->buf_ptr);
	throwIfSizeMismatch(esifEventDataPtr, broadcastHeader);

	const Guid broadcastGuid(broadcastHeader->UUID);
	if (IGCC_BROADCAST_GUID == broadcastGuid)
	{
		const auto igccNotificationData = static_cast<IgccBroadcastData::IgccToDttNotificationPackage*>(esifEventDataPtr->buf_ptr);
		return make_shared<WIDptfIgccBroadcastReceived>(dptfManager, *igccNotificationData);
	}

	if (SW_OEM_VAR_GUID == broadcastGuid)
	{
		const auto eventData = reinterpret_cast<UInt8*>(broadcastHeader) + sizeof(EsifAppBroadcastHeader);
		const auto eventDataLength = std::min(MAX_BYTES_SW_OEM_VAR_DATA, broadcastHeader->dataLen);
		DptfBuffer swOemVariablesData = DptfBuffer::fromExistingByteArray(eventData, eventDataLength);
		return make_shared<WIDptfSwOemVariablesBroadcastReceived>(dptfManager, swOemVariablesData);
	}

	if (NPT_WWAN_BAND_BROADCAST_GUID == broadcastGuid)
	{
		const auto eventData = reinterpret_cast<UInt8*>(broadcastHeader) + sizeof(EsifAppBroadcastHeader);
		const auto eventDataLength = std::min(MAX_BYTES_NPT_WWAN_DATA, broadcastHeader->dataLen);
		DptfBuffer nptWwanBandBroadcastData = DptfBuffer::fromExistingByteArray(eventData, eventDataLength);
		return make_shared<WIDptfNptWwanBandBroadcastReceived>(dptfManager, nptWwanBandBroadcastData);
	}
	return nullptr;
}

void EsifAppBroadcastProcessing::throwIfSizeMismatch(
	const EsifDataPtr esifEventDataPtr,
	const EsifAppBroadcastHeader* broadcastHeader)
{
	const auto expectedSize = esifEventDataPtr->data_len;
	const auto actualSize = sizeof(*broadcastHeader) + broadcastHeader->dataLen;
	if (actualSize != expectedSize)
	{
		throw dptf_exception("App Broadcast data length does not match expected length"s);
	}
}