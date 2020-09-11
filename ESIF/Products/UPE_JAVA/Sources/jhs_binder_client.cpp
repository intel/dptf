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
#include "upe.h"
#include "jhs_binder_service.h"
#include <binder/IPCThreadState.h>
#include <binder/Parcel.h>

namespace jhs {

	IMPLEMENT_META_INTERFACE(JhsService, JHS_META_INTERFACE_NAME);

	BpJhsService::BpJhsService(const sp<IBinder>& impl) : BpInterface<IJhsService>(impl)
	{
		UPE_TRACE_DEBUG("JHS: Instantiating BpJhsService...");
	}

	int32_t BpJhsService::getValue(JhsParticipantHandle handle, ActionParameters params, JhsReplyHeader *header, JhsReplyPayload payload)
	{
		uint32_t code = GET_VALUE;
		Parcel request, response;
		int32_t dummy = 0;
		int32_t rc = 0;

		request.writeInterfaceToken(IJhsService::getInterfaceDescriptor());
		request.writeInt32(50); // FIXME: remove magic number for this padding (required)
		request.writeUint32(handle.mParticipantId);
		request.writeInt32(60); // FIXME: remove magic number for this padding (required)
		request.writeUint32(params.mActionType);
		request.writeUint32(params.mDomainId);
		request.writeUint32(params.mInstanceId);
		request.writeUint32(params.mReplyDataType);
		request.writeUint32(params.mReplyBufferSize);

		UPE_TRACE_DEBUG("JHS: BpJhsService: issue getValue() binder call");
		remote()->transact(code, request, &response);
		// FIXME: Copy response to reply
		// The first 3 int32 values returned are metadata (meaning TBD)
		status_t status = response.readInt32(&dummy);
		status = response.readInt32(&dummy);
		status = response.readInt32(&dummy);

		// Real data comes after
		status = response.readUint32(&(header->mJhsDataType));
		UPE_TRACE_DEBUG("JHS: status = %d, header->mDataType = %d", status, header->mJhsDataType);
		status = response.readUint32(&(header->mActualDataSize));
		UPE_TRACE_DEBUG("JHS: status = %d, reply->mActualDataSize = %d", status, header->mActualDataSize);

		if (header->mActualDataSize > params.mReplyBufferSize) {
			rc = -1; //FIXME - set with real ESIF error code
			goto exit;
		}

		status = response.readInt32(&dummy);
		UPE_TRACE_DEBUG("JHS: dummy: %d", dummy);
		status = response.readInt32(&dummy);
		UPE_TRACE_DEBUG("JHS: dummy: %d", dummy);

		status = response.read(payload, header->mActualDataSize);
exit:
		return rc;
	}

	int BpJhsService::setValue(JhsParticipantHandle handle, ActionParameters params, int32_t value)
	{
		uint32_t code = SET_VALUE;
		Parcel request;

		request.writeInterfaceToken(IJhsService::getInterfaceDescriptor());
		request.writeInt32(50); // FIXME: remove magic number for this padding (required)
		request.writeUint32(handle.mParticipantId);
		request.writeInt32(60); // FIXME: remove magic number for this padding (required)
		request.writeUint32(params.mActionType);
		request.writeUint32(params.mDomainId);
		request.writeUint32(params.mInstanceId);
		request.writeUint32(params.mReplyDataType);
		request.writeUint32(params.mReplyBufferSize);
		request.writeUint32(value);
		UPE_TRACE_DEBUG("JHS: BpJhsService: issue setValue() binder call");
		remote()->transact(code, request, NULL);

		return 0;
	}
}

