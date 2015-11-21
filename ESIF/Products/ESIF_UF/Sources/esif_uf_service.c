/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID ESIF_TRACEMODULE_SERVICE

#include "esif_uf.h"			/* Upper Framework */
#include "esif_uf_app.h"		/* Application Manager */
#include "esif_uf_appmgr.h"		/* Application Manager */
#include "esif_uf_cfgmgr.h"		/* Configuration Manager */
#include "esif_uf_log.h"		/* Logging */
#include "esif_sdk_iface_esif.h"	/* ESIF Service Interface */
#include "esif_uf_service.h"	/* ESIF Service */
#include "esif_uf_primitive.h"	/* ESIF Primitive Execution */
#include "esif_dsp.h"
#include "esif_uf_ccb_system.h"
#include "esif_uf_eventmgr.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define MSGBUF_SIZE 1024

/* Provide read access to ESIF configuration space */
eEsifError ESIF_CALLCONV EsifSvcConfigGet(
	const void *esifHandle,
	const void *appHandle,
	const EsifDataPtr nameSpacePtr,
	const EsifDataPtr elementPathPtr,
	EsifDataPtr elementValuePtr
	)
{
	if (NULL == esifHandle) {
		ESIF_TRACE_ERROR("Invalid esif handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		ESIF_TRACE_ERROR("Invalid app handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == nameSpacePtr) {
		ESIF_TRACE_ERROR("Invalid namespace pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == elementPathPtr) {
		ESIF_TRACE_ERROR("Invalid element path pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == elementValuePtr) {
		ESIF_TRACE_ERROR("Invalid element value pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("\n\n"
					 "ESIF Handle          : %p\n"
					 "Application Handle   : %p\n"
					 "Name Space           : %s\n"
					 "Element Path         : %s\n\n",
					 esifHandle,
					 appHandle,
					 (EsifString)nameSpacePtr->buf_ptr,
					 (EsifString)elementPathPtr->buf_ptr);

	return EsifConfigGet(nameSpacePtr, elementPathPtr, elementValuePtr);
}


/* Provide write access to ESIF configuration space */
eEsifError ESIF_CALLCONV EsifSvcConfigSet(
	const void *esifHandle,
	const void *appHandle,
	const EsifDataPtr nameSpacePtr,
	const EsifDataPtr elementPathPtr,
	const EsifDataPtr elementValuePtr,
	const UInt32 elementFlags
	)
{
	if (NULL == esifHandle) {
		ESIF_TRACE_ERROR("Invalid esif handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		ESIF_TRACE_ERROR("Invalid app handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == nameSpacePtr) {
		ESIF_TRACE_ERROR("Invalid namespace pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == elementPathPtr) {
		ESIF_TRACE_ERROR("Invalid element path pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == elementValuePtr) {
		ESIF_TRACE_ERROR("Invalid element value pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("\n\n"
					 "ESIF Handle          : %p\n"
					 "Application Handle   : %p\n"
					 "Name Space           : %s\n"
					 "Element Path         : %s\n"
					 "Element Value        : %s\n"
					 "Element Flags        : %08x\n\n",
					 esifHandle,
					 appHandle,
					 (EsifString)nameSpacePtr->buf_ptr,
					 (EsifString)elementPathPtr->buf_ptr,
					 (EsifString)elementValuePtr->buf_ptr,
					 elementFlags);

	return EsifConfigSet(nameSpacePtr, elementPathPtr, elementFlags,
						 elementValuePtr);
}



/* Lookup domain data for a handle */
static AppDomainDataMapPtr find_domain_data_from_handle(
	const AppParticipantDataMapPtr participantPtr,
	const void *domainHandle
	)
{
	UInt8 i = 0;
	AppDomainDataMapPtr domain_data_map_ptr = NULL;

	for (i = 0; i < MAX_DOMAIN_ENTRY; i++) {
		if (participantPtr->fDomainData[i].fAppDomainHandle == domainHandle) {
			domain_data_map_ptr = &participantPtr->fDomainData[i];
			break;
		}
	}

	return domain_data_map_ptr;
}


/* Provide execute action for ESIF primitive */
eEsifError ESIF_CALLCONV EsifSvcPrimitiveExec(
	const void *esifHandle,
	const void *appHandle,
	const void *upHandle,
	const void *domainHandle,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr,
	const ePrimitiveType primitive,
	const UInt8 instance
	)
{
	eEsifError rc   = ESIF_OK;
	UInt8 participant_id = 0;
	UInt8 domain_id = 0;

	EsifString qualifier_str = "D0";
	AppParticipantDataMapPtr participant_data_map_ptr = NULL;
	AppDomainDataMapPtr domain_data_map_ptr = NULL;
	EsifAppPtr appPtr = NULL;

	if (NULL == esifHandle) {
		ESIF_TRACE_ERROR("Invalid esif handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		ESIF_TRACE_ERROR("Invalid app handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == requestPtr) {
		ESIF_TRACE_ERROR("Invalid request buffer pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == responsePtr) {
		ESIF_TRACE_ERROR("Invalid response buffer pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	/* Callbacks must always provide this */
	appPtr = GetAppFromHandle(appHandle);
	if (NULL == appPtr) {
		ESIF_TRACE_ERROR("The app data was not found from app handle\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	/* Lookup our Participant ID from the provided participant handle */
	/* If the upHandle is NULL use particiapnt 0 as agreed with DPTF */
	if (NULL == upHandle) {
		participant_id = 0;
		participant_data_map_ptr = &appPtr->fParticipantData[participant_id];
	} else {
		participant_data_map_ptr = EsifApp_GetParticipantDataMapFromHandle(appPtr, upHandle);
		if (NULL != participant_data_map_ptr) {
			participant_id = EsifUp_GetInstance(participant_data_map_ptr->fUpPtr);
		}
	}

	if (NULL == participant_data_map_ptr) {
		ESIF_TRACE_ERROR("The participant data was not found from participant handle\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	/* Lookup or Domain Qualifier from the provided domain handle */
	/* If the domainHandle is NULL use domain 0 as agreed with DPTF */
	if (NULL == participant_data_map_ptr || NULL == domainHandle) {
		domain_id     = 0;
		qualifier_str = "D0";
	} else {
		domain_data_map_ptr = find_domain_data_from_handle(participant_data_map_ptr, domainHandle);
		if (NULL == domain_data_map_ptr) {
			ESIF_TRACE_ERROR("The domain data was not found from domain handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		domain_id     = domain_data_map_ptr->fAppDomainId;
		qualifier_str = domain_data_map_ptr->fQualifier;
	}

	/* Share what we have learned so far */
	ESIF_TRACE_DEBUG("\n\n"
					 "ESIF Handle          : %p\n"
					 "Application Handle   : %p\n"
					 "Participant Handle   : %p\n"
					 "Participant ID       : %u\n"
					 "Domain Handle        : %p\n"
					 "Domain ID            : %u\n"
					 "Request              : %p\n"
					 "  Data Type:         : %s(%u)\n"
					 "  Data Buffer        : %p\n"
					 "  Data Length        : %u\n"
					 "Response             : %p\n"
					 "  Data Type          : %s(%u)\n"
					 "  Data Buffer        : %p\n"
					 "  Data Length        : %u\n"
					 "Primitive            : %s(%u)\n"
					 "Qualifier            : %s\n"
					 "Instance             : %u\n\n",
					 esifHandle,
					 appHandle,
					 upHandle,
					 participant_id,
					 domainHandle,
					 domain_id,
					 requestPtr,
					 esif_data_type_str(requestPtr->type), requestPtr->type,
					 requestPtr->buf_ptr,
					 requestPtr->buf_len,
					 responsePtr,
					 esif_data_type_str(responsePtr->type), responsePtr->type,
					 responsePtr->buf_ptr,
					 responsePtr->buf_len,
					 esif_primitive_str((enum esif_primitive_type)primitive), primitive,
					 qualifier_str,
					 instance);

	rc = EsifExecutePrimitive(
			participant_id, primitive, qualifier_str, instance,
			requestPtr, responsePtr);
exit:
	return rc;
}

/* Provide write access to ESIF log object */
eEsifError ESIF_CALLCONV EsifSvcWriteLog(
	const void *esifHandle,
	const void *appHandle,
	const void *upHandle,
	const void *domainHandle,
	const EsifDataPtr messagePtr,
	const eLogType logType
	)
{
	char *msg   = NULL;

	UNREFERENCED_PARAMETER(upHandle);
	UNREFERENCED_PARAMETER(domainHandle);

	if (NULL == esifHandle) {
		ESIF_TRACE_ERROR("Invalid esif handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		ESIF_TRACE_ERROR("Invalid app handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == messagePtr) {
		ESIF_TRACE_ERROR("Invalid message pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("\n\n"
					 "ESIF Handle          : %p\n"
					 "Application Handle   : %p\n"
					 "Participant Handle   : %p\n"
					 "Domain Handle        : %p\n"
					 "Message              : %p\n"
					 "  Data Type:         : %s(%u)\n"
					 "  Data Buffer:       : %p\n"
					 "  Data Length:       : %u\n"
					 "Log Type             : %s(%u)\n"
					 "Message              : %s\n\n",
					 esifHandle,
					 appHandle,
					 upHandle,
					 domainHandle,
					 messagePtr,
					 esif_data_type_str(messagePtr->type), messagePtr->type,
					 messagePtr->buf_ptr,
					 messagePtr->buf_len,
					 esif_log_type_str(logType), logType,
					 (esif_string)messagePtr->buf_ptr);

	msg = (esif_string)messagePtr->buf_ptr;
	if (msg != NULL && *msg=='\n')
		msg++;

	// Always Steer Application Messages to Target Routes if DPTF module enabled, regardless of global trace level
	switch (logType) {
	case eLogTypeFatal:
	case eLogTypeError:
		ESIF_TRACE_IFENABLED(ESIF_TRACEMODULE_DPTF, ESIF_TRACELEVEL_ERROR, "%s", msg);
		break;
	case eLogTypeWarning:
		ESIF_TRACE_IFENABLED(ESIF_TRACEMODULE_DPTF, ESIF_TRACELEVEL_WARN, "%s", msg);
		break;
	case eLogTypeInfo:
		ESIF_TRACE_IFENABLED(ESIF_TRACEMODULE_DPTF, ESIF_TRACELEVEL_INFO, "%s", msg);
		break;
	case eLogTypeDebug:
		ESIF_TRACE_IFENABLED(ESIF_TRACEMODULE_DPTF, ESIF_TRACELEVEL_DEBUG, "%s", msg);
		break;
	default:
		break;
	}
	return ESIF_OK;
}


/*
** Event Registration / Unregistration
*/
eEsifError ESIF_CALLCONV EsifSvcEventRegister(
	const void *esifHandle,
	const void *appHandle,
	const void *upHandle,
	const void *domainHandle,
	const EsifDataPtr eventGuidPtr
	)
{
	return EsifApp_RegisterEvent(esifHandle, appHandle, upHandle, domainHandle, eventGuidPtr);
}


/* Provide unregistration for previously registered ESIF event */
eEsifError ESIF_CALLCONV EsifSvcEventUnregister(
	const void *esifHandle,
	const void *appHandle,
	const void *upHandle,
	const void *domainHandle,
	const EsifDataPtr eventGuidPtr
	)
{
	return EsifApp_UnregisterEvent(esifHandle, appHandle, upHandle, domainHandle, eventGuidPtr);
}


eEsifError EsifSvcInit()
{
	return ESIF_OK;
}


void EsifSvcExit()
{
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
