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
#include "ipf_version.h"
#include "esif_sdk_data.h"
#include "esif_ccb_string.h"
#include "ipfipc_trace.h"




eEsifError ESIF_CALLCONV IpfIpc_WriteLog(
	const esif_handle_t esifHandle,		/* ESIF provided context handle */
	const esif_handle_t participantHandle,	/* optional may be ESIF_HANDLE_PRIMARY_PARTICIPANT used to augment log detail*/
	const esif_handle_t domainHandle,	/* optional may be ESIF_INVALID_HANDLE used to augment log detail*/
	const EsifDataPtr message,	/* Message For Log */
	const eLogType logType		/* Log Type e.g. crticial, debug, info,e tc */
	)
{
	EsifData uniStr = { ESIF_DATA_UNICODE };

	UNREFERENCED_PARAMETER(esifHandle);
	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(domainHandle);
	UNREFERENCED_PARAMETER(logType);

	if (message && message->type == ESIF_DATA_STRING && message->buf_ptr && message->data_len) {

	}

	esif_ccb_free(uniStr.buf_ptr);
	return ESIF_OK;
}

