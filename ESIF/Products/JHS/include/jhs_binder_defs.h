/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include <stdlib.h>

#define ACTION_TYPE__BQC 0x4351425f
#define ACTION_TYPE__BCM 0x4d43425f
#define ACTION_TYPE__TMP 0x504d545f
#define ACTION_TYPE_SPPC 0x43505053
#define ACTION_TYPE_SHDN 0x4e444853

#define ESIA_ID_LEN 8

namespace jhs {
	typedef struct {
		uint32_t mParticipantId;
	} JhsParticipantHandle;

	typedef struct {
		uint32_t mActionType;
		uint32_t mDomainId;
		uint32_t mInstanceId;
		uint32_t mReplyDataType;
		uint32_t mReplyBufferSize;
	} ActionParameters;

	typedef struct {
		uint32_t mJhsDataType; // Question - do we really need this field?
		uint32_t mActualDataSize;
	} JhsReplyHeader;

	typedef char* JhsReplyPayload;

	typedef struct {
		char mEisaId[ESIA_ID_LEN]; // NULL terminated ESIA ID, such as INT3408 for modem
		uint32_t mPtype; // sub-participant type to distinguish devices sharing same ESIA ID 
	} DptfParticipant;

	typedef struct {
		uint32_t mEventType;
		uint32_t mEventParticipant;
		uint32_t mEventSubType;
		uint32_t mEventSubTypeParam;
	} JhsEvent;
}

