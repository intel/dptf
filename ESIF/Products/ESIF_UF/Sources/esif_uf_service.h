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

#ifndef _ESIF_UF_SERVICE_
#define _ESIF_UF_SERVICE_

#include "esif.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ESIF Services  Interface Functions */
eEsifError ESIF_CALLCONV EsifSvcConfigGet(const esif_handle_t esifHandle,
							const EsifDataPtr nameSpacePtr,
							const EsifDataPtr elementPathPtr,
							EsifDataPtr elementValuePtr);

eEsifError ESIF_CALLCONV EsifSvcConfigSet(const esif_handle_t esifHandle,
							const EsifDataPtr nameSpacePtr,
							const EsifDataPtr elementPathPtr,
							const EsifDataPtr elementValuePtr,
							const EsifFlags elementFlags);

eEsifError ESIF_CALLCONV EsifSvcPrimitiveExec(const esif_handle_t esifHandle,
								const esif_handle_t participantId,
								const esif_handle_t domainId,
								const EsifDataPtr requestPtr,
								EsifDataPtr responsePtr,
								const ePrimitiveType primitive,
								const UInt8 instance);

eEsifError ESIF_CALLCONV EsifSvcWriteLog(const esif_handle_t esifHandle,
						   const esif_handle_t participantId,
						   const esif_handle_t domainId,
						   const EsifDataPtr messagePtr,
						   const eLogType logType);

eEsifError ESIF_CALLCONV EsifSvcEventRegister(const esif_handle_t esifHandle,
								const esif_handle_t participantId,
								const esif_handle_t domainId,
								const EsifDataPtr eventGuid);

eEsifError ESIF_CALLCONV EsifSvcEventUnregister(const esif_handle_t esifHandle,
								  const esif_handle_t participantId,
								  const esif_handle_t domainId,
								  const EsifDataPtr eventGuid);

eEsifError ESIF_CALLCONV EsifSvcEventReceive(const esif_handle_t esifHandle,
								  const esif_handle_t participantId,
								  const esif_handle_t domainId,
								  const EsifDataPtr eventData,
								  const EsifDataPtr eventGuid);

eEsifError ESIF_CALLCONV EsifSvcCommandReceive(const esif_handle_t esifHandle,
								  const UInt32 argc,
								  const EsifDataPtr argv,
								  EsifDataPtr response);

/* Event handler used by the ESIF services on behalf of applications */
eEsifError ESIF_CALLCONV EsifSvcEventCallback(
	esif_handle_t esifHandle,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_SERVICE_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

