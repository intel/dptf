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
#pragma once

#include "jhs_binder_defs.h"
#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>

#include <utils/String16.h>

#define JHS_SERVICE_NAME "java_helper_service"
#define JHS_META_INTERFACE_NAME "com.intel.dptf.jhs.binder.IJhsService"
#define JHS_CLIENT_SERVICE_NAME "java_helper_client_service"
#define JHS_CLIENT_META_INTERFACE_NAME "com.intel.dptf.jhs.binder.IJhsClientService"

using namespace android;

namespace jhs {
	// IJhsService is the service provided by JHS to DPTF
        enum JHS_API_ENUM {
                GET_VALUE=IBinder::FIRST_CALL_TRANSACTION, SET_VALUE
        };

	class IJhsService : public IInterface
	{
		public:
		DECLARE_META_INTERFACE(JhsService);
		virtual int32_t getValue(JhsParticipantHandle handle, ActionParameters params, JhsReplyHeader *header, JhsReplyPayload payload) = 0;
		virtual int32_t setValue(JhsParticipantHandle handle, ActionParameters params, int32_t value) = 0;
	};


	class BpJhsService : public BpInterface<IJhsService>
	{
		public:
		BpJhsService(const sp<IBinder>& impl);
		virtual int32_t getValue(JhsParticipantHandle handle, ActionParameters params, JhsReplyHeader *header, JhsReplyPayload payload);
		virtual int32_t setValue(JhsParticipantHandle handle, ActionParameters params, int32_t value);
	};

	// IJhsClientService is the service provided by DPTF to JHS
	enum JHCS_API_ENUM {
		PART_ACTIVE=IBinder::FIRST_CALL_TRANSACTION, PART_INACTIVE, SEND_EVENT
	};

	class IJhsClientService : public IInterface
	{
		public:
		DECLARE_META_INTERFACE(JhsClientService);
		virtual uint32_t participantActive(DptfParticipant participant) = 0;
		virtual status_t participantInActive(JhsParticipantHandle handle) = 0;
		virtual status_t sendEvent(JhsEvent event) = 0;
	};

	class BpJhsClientService : public BpInterface<IJhsClientService>
	{
		public:
		BpJhsClientService(const sp<IBinder>& impl);
		uint32_t participantActive(DptfParticipant participant) { return 0; }
		status_t participantInActive(JhsParticipantHandle handle) { return 0; }
		status_t sendEvent(JhsEvent event) { return 0; }
	};


	class BnJhsClientService : public BnInterface<IJhsClientService>
	{
		public:
		virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel *reply, uint32_t flags = 0);
	};

	class JhsClientService : public BnJhsClientService {
		public:
		virtual uint32_t participantActive(DptfParticipant participant);
		virtual status_t participantInActive(JhsParticipantHandle handle);
		virtual status_t sendEvent(JhsEvent event);
	};

}

