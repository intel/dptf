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
#include "esif_uf_arbmgr.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define MSGBUF_SIZE 1024


typedef enum EventCategory_e {
	EVENT_CATEGORY_NONE = 0,
	EVENT_CATEGORY_APP,
	EVENT_CATEGORY_PARTICIPANT,
	EVENT_CATEGORY_DOMAIN,
	EVENT_CATEGORY_MATCH_ANY,
	EVENT_CATEGORY_MAX
} EventCategory, *EventCategoryPtr;


static ESIF_INLINE EventCategory CategorizeEvent(
	const esif_handle_t participantId,
	const esif_handle_t domainHandle
	);


/* Provide read access to ESIF configuration space */
eEsifError ESIF_CALLCONV EsifSvcConfigGet(
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpacePtr,
	const EsifDataPtr elementPathPtr,
	EsifDataPtr elementValuePtr
	)
{
	UNREFERENCED_PARAMETER(esifHandle);

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
					 "ESIF Handle          : " ESIF_HANDLE_FMT "n"
					 "Name Space           : %s\n"
					 "Element Path         : %s\n\n",
					 esif_ccb_handle2llu(esifHandle),
					 (EsifString)nameSpacePtr->buf_ptr,
					 (EsifString)elementPathPtr->buf_ptr);

	return EsifConfigGet(nameSpacePtr, elementPathPtr, elementValuePtr);
}


/* Provide write access to ESIF configuration space */
eEsifError ESIF_CALLCONV EsifSvcConfigSet(
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpacePtr,
	const EsifDataPtr elementPathPtr,
	const EsifDataPtr elementValuePtr,
	const EsifFlags elementFlags
	)
{
	UNREFERENCED_PARAMETER(esifHandle);

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
					 "ESIF Handle          : " ESIF_HANDLE_FMT "\n"
					 "Name Space           : %s\n"
					 "Element Path         : %s\n"
					 "Element Value        : %s\n"
					 "Element Flags        : %08x\n\n",
					 esif_ccb_handle2llu(esifHandle),
					 (EsifString)nameSpacePtr->buf_ptr,
					 (EsifString)elementPathPtr->buf_ptr,
					 (EsifString)elementValuePtr->buf_ptr,
					 elementFlags);

	return EsifConfigSet(nameSpacePtr,
		elementPathPtr,
		elementFlags,
		elementValuePtr);
}


/* Provide execute action for ESIF primitive */
eEsifError ESIF_CALLCONV EsifSvcPrimitiveExec(
	const esif_handle_t esifHandle,
	const esif_handle_t participantId,
	const esif_handle_t domainHandle,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr,
	const ePrimitiveType primitive,
	const UInt8 instance
	)
{
	eEsifError rc   = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	esif_handle_t localParticipantId = ESIF_INVALID_HANDLE;
	EsifString qualifier_str = NULL;

	if (NULL == requestPtr) {
		ESIF_TRACE_ERROR("Invalid request buffer pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == responsePtr) {
		ESIF_TRACE_ERROR("Invalid response buffer pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	/* Callbacks must always provide this */
	appPtr = EsifAppMgr_GetAppFromHandle(esifHandle);
	if (NULL == appPtr) {
		ESIF_TRACE_ERROR("The app was not found from handle\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	/* Lookup our Participant ID from the provided participant handle */
	/* If the participantId is invalid use particiapnt 0 as agreed with DPTF */
	if (EsifUpPm_IsPrimaryParticipantId(participantId) || (ESIF_INVALID_HANDLE == participantId)) {
		localParticipantId = ESIF_HANDLE_PRIMARY_PARTICIPANT;
		qualifier_str = "D0";
	} else {
		localParticipantId = participantId;

		/* Lookup or Domain Qualifier from the provided domain handle */
		/* If the domainHandle is NULL use domain 0 as agreed with DPTF */
		if (ESIF_INVALID_HANDLE == domainHandle) {
			qualifier_str = "D0";
		} else {
			qualifier_str = EsifApp_GetDomainQalifierByHandle(appPtr, participantId, domainHandle);
			if (NULL == qualifier_str) {
				ESIF_TRACE_WARN("The domain data was not found from domain handle\n");
				rc = ESIF_E_INVALID_HANDLE;
				goto exit;
			}
		}
	}

	/* Share what we have learned so far */
	ESIF_TRACE_DEBUG("\n\n"
					 "ESIF Handle          : " ESIF_HANDLE_FMT "\n"
					 "Participant Handle   : " ESIF_HANDLE_FMT "\n"
					 "Participant ID       : " ESIF_HANDLE_FMT "\n"
					 "Domain Handle        : " ESIF_HANDLE_FMT "\n"
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
					 esif_ccb_handle2llu(esifHandle),
					 esif_ccb_handle2llu(participantId),
					 esif_ccb_handle2llu(localParticipantId),
					 esif_ccb_handle2llu(domainHandle),
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
	
	rc = EsifArbMgr_ExecutePrimitive(
		esifHandle,
		localParticipantId, primitive, qualifier_str, instance,
		requestPtr, responsePtr);
exit:
	EsifAppMgr_PutRef(appPtr);
	return rc;
}

/* Provide write access to ESIF log object */
eEsifError ESIF_CALLCONV EsifSvcWriteLog(
	const esif_handle_t esifHandle,
	const esif_handle_t participantId,
	const esif_handle_t domainHandle,
	const EsifDataPtr messagePtr,
	const eLogType logType
	)
{
	char *msg   = NULL;

	UNREFERENCED_PARAMETER(esifHandle);
	UNREFERENCED_PARAMETER(participantId);
	UNREFERENCED_PARAMETER(domainHandle);

	if (NULL == messagePtr) {
		ESIF_TRACE_ERROR("Invalid message pointer\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("\n\n"
					 "ESIF Handle          : " ESIF_HANDLE_FMT "\n"
					 "Participant Handle   : " ESIF_HANDLE_FMT "\n"
					 "Domain Handle        : " ESIF_HANDLE_FMT "\n"
					 "Message              : %p\n"
					 "  Data Type:         : %s(%u)\n"
					 "  Data Buffer:       : %p\n"
					 "  Data Length:       : %u\n"
					 "Log Type             : %s(%u)\n"
					 "Message              : %s\n\n",
					 esif_ccb_handle2llu(esifHandle),
					 esif_ccb_handle2llu(participantId),
					 esif_ccb_handle2llu(domainHandle),
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
	const esif_handle_t esifHandle,
	const esif_handle_t participantId,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuidPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	esif_handle_t localParticipantId = ESIF_INVALID_HANDLE;
	UInt16 domainId = 0;
	eEsifEventType eventType = 0;
	char guidStr[ESIF_GUID_PRINT_SIZE] = { 0 };

	UNREFERENCED_PARAMETER(guidStr);

	if ((NULL == eventGuidPtr) || (NULL == eventGuidPtr->buf_ptr) || (ESIF_DATA_GUID != eventGuidPtr->type)) {
		ESIF_TRACE_WARN("Invalid event GUID\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (eventGuidPtr->buf_len < sizeof(esif_guid_t)) {
		ESIF_TRACE_WARN("GUID buffer too small\n");
		rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		goto exit;
	}

	if (!esif_event_map_guid2type(eventGuidPtr->buf_ptr, &eventType)) {
		ESIF_TRACE_WARN("Event type not found\n");
		rc = ESIF_E_EVENT_NOT_FOUND;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Registering App Event\n\n"
		"ESIF Handle          : " ESIF_HANDLE_FMT "\n"
		"Participant Handle   : " ESIF_HANDLE_FMT "\n"
		"Domain Handle        : " ESIF_HANDLE_FMT "\n"
		"Event GUID           : %s\n"
		"Event Type           : %d (%s)\n\n",
		esif_ccb_handle2llu(esifHandle),
		esif_ccb_handle2llu(participantId),
		esif_ccb_handle2llu(domainHandle),
		esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr),
		eventType, esif_event_type_str(eventType));

	/* Validate the app calling the interface */
	appPtr = EsifAppMgr_GetAppFromHandle(esifHandle);
	if (NULL == appPtr) {
		ESIF_TRACE_ERROR("The app data was not found from app handle\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	/* Determine what to do based on provided parameters */
	switch (CategorizeEvent(participantId, domainHandle)) {
	case EVENT_CATEGORY_APP:
	{
		ESIF_TRACE_DEBUG("Using Participant %d\n", ESIF_HANDLE_PRIMARY_PARTICIPANT);

		rc = EsifEventMgr_RegisterEventByType(eventType, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, (EVENT_OBSERVER_CALLBACK)EsifSvcEventCallback, (esif_context_t)esifHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_PARTICIPANT:
	{
		/* Register with participant */
		localParticipantId = participantId;

		ESIF_TRACE_DEBUG("Using Participant " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(localParticipantId));

		rc = EsifEventMgr_RegisterEventByType(eventType, localParticipantId, EVENT_MGR_MATCH_ANY_DOMAIN, (EVENT_OBSERVER_CALLBACK)EsifSvcEventCallback, esifHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_DOMAIN:
	{
		/* Register with domain */
		localParticipantId = participantId;

		rc = EsifApp_GetDomainIdByHandle(appPtr, participantId, domainHandle, &domainId);
		if (rc != ESIF_OK) {
			goto exit;
		}

		ESIF_TRACE_DEBUG("Using Participant " ESIF_HANDLE_FMT " Domain 0x%X\n", esif_ccb_handle2llu(localParticipantId), domainId);

		rc = EsifEventMgr_RegisterEventByType(eventType, localParticipantId, domainId, (EVENT_OBSERVER_CALLBACK)EsifSvcEventCallback, esifHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_MATCH_ANY:
	{
		ESIF_TRACE_DEBUG("Registering for matching any participant event\n");

		rc = EsifEventMgr_RegisterEventByType(eventType, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, (EVENT_OBSERVER_CALLBACK)EsifSvcEventCallback, esifHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;
	default:
		ESIF_TRACE_ERROR("Unknown category type\n");
		rc = ESIF_E_INVALID_HANDLE;
		break;
	}
exit:
	EsifAppMgr_PutRef(appPtr);
	ESIF_TRACE_DEBUG("Exit Code = %d\n", rc);
	return rc;
}


/* Provide unregistration for previously registered ESIF event */
eEsifError ESIF_CALLCONV EsifSvcEventUnregister(
	const esif_handle_t esifHandle,
	const esif_handle_t participantId,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuidPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	esif_handle_t localParticipantId = ESIF_INVALID_HANDLE;
	UInt16 domainId = 0;
	eEsifEventType eventType = 0;
	char guidStr[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guidStr);

	if ((NULL == eventGuidPtr) || (NULL == eventGuidPtr->buf_ptr) || (ESIF_DATA_GUID != eventGuidPtr->type)) {
		ESIF_TRACE_ERROR("Invalid event GUID\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (eventGuidPtr->buf_len < sizeof(esif_guid_t)) {
		ESIF_TRACE_WARN("GUID buffer too small\n");
		rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		goto exit;
	}
	if (!esif_event_map_guid2type(eventGuidPtr->buf_ptr, &eventType)) {
		ESIF_TRACE_WARN("Event type not found\n");
		rc = ESIF_E_EVENT_NOT_FOUND;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Unregistering App event:\n\n"
		"  ESIF Handle          : " ESIF_HANDLE_FMT "\n"
		"  Participant Handle   : " ESIF_HANDLE_FMT "\n"
		"  Domain Handle        : " ESIF_HANDLE_FMT "\n"
		"  Event GUID           : %s\n"
		"  Event Type           : %d (%s)\n\n",
		esif_ccb_handle2llu(esifHandle),
		esif_ccb_handle2llu(participantId),
		esif_ccb_handle2llu(domainHandle),
		esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr),
		eventType, esif_event_type_str(eventType));

	/* Validate the app calling the interface */
	appPtr = EsifAppMgr_GetAppFromHandle(esifHandle);
	if (NULL == appPtr) {
		ESIF_TRACE_ERROR("The app data was not found from app handle\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	/* Determine what to do based on provided parameters */
	switch (CategorizeEvent(participantId, domainHandle)) {
	case EVENT_CATEGORY_APP:
	{
		/* Unregister app  */
		ESIF_TRACE_DEBUG("Using Participant " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(ESIF_HANDLE_PRIMARY_PARTICIPANT));

		rc = EsifEventMgr_UnregisterEventByType(eventType, ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, (EVENT_OBSERVER_CALLBACK)EsifSvcEventCallback, (esif_context_t)esifHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_PARTICIPANT:
	{
		/*Unregister participant */
		localParticipantId = participantId;

		ESIF_TRACE_DEBUG("Using Participant " ESIF_HANDLE_FMT "\n", esif_ccb_handle2llu(localParticipantId));

		rc = EsifEventMgr_UnregisterEventByType(eventType, localParticipantId, EVENT_MGR_MATCH_ANY_DOMAIN, (EVENT_OBSERVER_CALLBACK)EsifSvcEventCallback, (esif_context_t)esifHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_DOMAIN:
	{
		/* Unregister domain */
		localParticipantId = participantId;

		rc = EsifApp_GetDomainIdByHandle(appPtr, participantId, domainHandle, &domainId);
		if (rc != ESIF_OK) {
			goto exit;
		}
		ESIF_TRACE_DEBUG("Using Participant " ESIF_HANDLE_FMT " Domain 0x%X\n", esif_ccb_handle2llu(localParticipantId), domainId);

		rc = EsifEventMgr_UnregisterEventByType(eventType, localParticipantId, domainId, (EVENT_OBSERVER_CALLBACK)EsifSvcEventCallback, (esif_context_t)esifHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_MATCH_ANY:
	{
		ESIF_TRACE_DEBUG("Unregistering from matching any participant event\n");

		rc = EsifEventMgr_UnregisterEventByType(eventType, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, (EVENT_OBSERVER_CALLBACK)EsifSvcEventCallback, (esif_context_t)esifHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	default:
		ESIF_TRACE_ERROR("Unknown category type\n");
		break;
	}
exit:
	EsifAppMgr_PutRef(appPtr);
	ESIF_TRACE_DEBUG("Exit Code = %d\n", rc);
	return rc;
}

/*
** Provide interface for App to send event to ESIF
*/
eEsifError ESIF_CALLCONV EsifSvcEventReceive(
	const esif_handle_t esifHandle,
	const esif_handle_t participantId,
	const esif_handle_t domainHandle,	
	const EsifDataPtr eventDataPtr,
	const EsifDataPtr eventGuidPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	esif_handle_t localParticipantId = ESIF_INVALID_HANDLE;
	UInt16 domainId = 0;
	eEsifEventType eventType = (eEsifEventType)ESIF_INVALID_ENUM_VALUE;
	char guidStr[ESIF_GUID_PRINT_SIZE] = { 0 };

	if ((NULL == eventGuidPtr) || (NULL == eventGuidPtr->buf_ptr) || (ESIF_DATA_GUID != eventGuidPtr->type)) {
		ESIF_TRACE_ERROR("Invalid event GUID\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("Received Event\n\n"
		"ESIF Handle          : " ESIF_HANDLE_FMT "\n"
		"Participant Handle   : " ESIF_HANDLE_FMT "\n"
		"Domain Handle        : " ESIF_HANDLE_FMT "\n"
		"Event Data           : %p\n"
		"Event GUID           : %s\n\n",
		esif_ccb_handle2llu(esifHandle),
		esif_ccb_handle2llu(participantId),
		esif_ccb_handle2llu(domainHandle),
		eventDataPtr,
		esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr));


	/* Validate the app calling the interface */
	appPtr = EsifAppMgr_GetAppFromHandle(esifHandle);
	if (NULL == appPtr) {
		ESIF_TRACE_ERROR("The app data was not found from app handle\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	/* Determine what to do based on provided parameters */
	switch (CategorizeEvent(participantId, domainHandle)) {
	case EVENT_CATEGORY_APP:
	{
		if (esif_event_map_guid2type(eventGuidPtr->buf_ptr, &eventType) == ESIF_FALSE) {
			rc = ESIF_E_EVENT_NOT_FOUND;
			goto exit;
		}

		ESIF_TRACE_DEBUG(
			"Received APP event\n"
			"  Participant: " ESIF_HANDLE_FMT "\n"
			"  GUID: %s\n"
			"  Type: %s(%d)\n",
			esif_ccb_handle2llu(localParticipantId),
			esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr),
			esif_event_type_str(eventType), eventType);

		rc = EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, eventType, eventDataPtr);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_PARTICIPANT:
	{
		localParticipantId = participantId;

		if (esif_event_map_guid2type(eventGuidPtr->buf_ptr, &eventType) == ESIF_FALSE) {
			rc = ESIF_E_EVENT_NOT_FOUND;
			goto exit;
		}

		ESIF_TRACE_DEBUG(
			"Received Participant event\n"
			"  Participant: " ESIF_HANDLE_FMT "\n"
			"  GUID: %s\n"
			"  Type: %s(%d)\n",
			esif_ccb_handle2llu(localParticipantId),
			esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr),
			esif_event_type_str(eventType), eventType);

		rc = EsifEventMgr_SignalEvent(localParticipantId, EVENT_MGR_MATCH_ANY_DOMAIN, eventType, eventDataPtr);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_DOMAIN:
	{
		localParticipantId = participantId;

		rc = EsifApp_GetDomainIdByHandle(appPtr, participantId, domainHandle, &domainId);
		if (rc != ESIF_OK) {
			goto exit;
		}

		ESIF_TRACE_DEBUG("Using Participant " ESIF_HANDLE_FMT " Domain 0x%X\n", esif_ccb_handle2llu(localParticipantId), domainId);

		if (esif_event_map_guid2type(eventGuidPtr->buf_ptr, &eventType) == ESIF_FALSE) {
			ESIF_TRACE_ERROR("esif_event_map_guid2type() failed");
			rc = ESIF_E_EVENT_NOT_FOUND;
			goto exit;
		}

		ESIF_TRACE_DEBUG(
			"Received Domain event\n"
			"  Participant: " ESIF_HANDLE_FMT "\n",
			"  GUID: %s\n"
			"  Type: %s(%d)\n",
			esif_ccb_handle2llu(localParticipantId),
			esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr),
			esif_event_type_str(eventType), eventType);

		rc = EsifEventMgr_SignalEvent(localParticipantId, domainId, eventType, eventDataPtr);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	default:
		ESIF_TRACE_ERROR("Unknown category type\n");
		rc = ESIF_E_INVALID_HANDLE;
		break;
	}

exit:
	EsifAppMgr_PutRef(appPtr);
	ESIF_TRACE_DEBUG("Exit Code = %d\n", rc);
	return rc;
}

// Worker Thread to run "start" commands received via ESIF Interface
static void *ESIF_CALLCONV startcmd_worker(void *ctx)
{
	char *command = (char *)ctx;
	if (command) {
		parse_cmd(command, ESIF_FALSE, ESIF_FALSE);
		esif_ccb_free(command);
	}
	return 0;
}

/*
** Provide interface for App to send Shell Command to ESIF
*/
eEsifError ESIF_CALLCONV EsifSvcCommandReceive(
	const esif_handle_t esifHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response)
{
	eEsifError rc = ESIF_E_NOT_IMPLEMENTED;
	UInt32 response_len = 0;
	int shell_argc = 0;
	char **shell_argv = NULL;

	UNREFERENCED_PARAMETER(esifHandle);

	// Intercept "start" command and create a new thread to run it to avoid shell deadlocks
	char start_prefix[] = "start ";
	if (argc == 1 && argv[0].type == ESIF_DATA_STRING && esif_ccb_strnicmp((char *)argv[0].buf_ptr, start_prefix, sizeof(start_prefix) - 1) == 0) {
		esif_thread_t cmdThread = ESIF_THREAD_NULL;
		char *command = esif_ccb_strdup((esif_string)argv[0].buf_ptr + sizeof(start_prefix) - 1);
		if ((rc = esif_ccb_thread_create(&cmdThread, startcmd_worker, command)) == ESIF_OK) {
			esif_ccb_thread_close(&cmdThread);
		}
		else {
			esif_ccb_free(command);
		}
		return rc;
	}

	esif_uf_shell_lock();

	g_outbuf[0] = '\0';

	if (argc > 0 && argv != NULL && response != NULL) {
		UInt32 j = 0;
		shell_argc = 0;
		shell_argv = esif_ccb_malloc((size_t)argc * sizeof(char *));

		if (shell_argv == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		// If argv[0] is a singleton string, parse it as a command line otherwise execute argc/argv
		if (argc == 1 && argv[0].type == ESIF_DATA_STRING && esif_ccb_strchr((char *)argv[0].buf_ptr, ' ') != NULL) {
			parse_cmd((char *)argv[0].buf_ptr, ESIF_FALSE, ESIF_FALSE);
			rc = ESIF_OK;
		}
		else {
			for (j = 0; j < argc; j++) {
				if (argv[j].buf_ptr != NULL && argv[j].type == ESIF_DATA_STRING) {
					shell_argv[shell_argc++] = (char *)argv[j].buf_ptr;
				}
			}
			rc = esif_shell_dispatch(shell_argc, shell_argv, &g_outbuf);
		}

		// Copy output to response unless buffer is too small
		if (rc == ESIF_OK) {
			response_len = (UInt32)esif_ccb_strlen(g_outbuf, g_outbuf_len) + 1;
			response->data_len = response_len;
			if (response_len > response->buf_len) {
				rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			else if (response->buf_ptr != g_outbuf) {
				esif_ccb_strcpy((char *)response->buf_ptr, g_outbuf, response->buf_len);
			}
		}
	}

exit:
	esif_uf_shell_unlock();
	esif_ccb_free(shell_argv);
	return rc;
}


/* Event handler for events registered for by the application */
eEsifError ESIF_CALLCONV EsifSvcEventCallback(
	esif_handle_t esifHandle,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	esif_guid_t guid = { 0 };
	EsifData dataGuid = { ESIF_DATA_GUID, &guid, sizeof(guid), sizeof(guid) };

	if (NULL == fpcEventPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (!esif_event_map_type2guid(&guid, &fpcEventPtr->esif_event)) {
		rc = ESIF_E_EVENT_NOT_FOUND;
		goto exit;
	}

	appPtr = EsifAppMgr_GetAppFromHandle(esifHandle);
	if (appPtr == NULL) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	rc = EsifApp_SendEvent(
		appPtr,
		participantId,
		domainId,
		eventDataPtr,
		&dataGuid);
exit:
	EsifAppMgr_PutRef(appPtr);
	return rc;
}


static ESIF_INLINE EventCategory CategorizeEvent(
	const esif_handle_t participantId,
	const esif_handle_t domainId
	)
{
	EventCategory category = EVENT_CATEGORY_NONE;
	Bool isValidParticipantHandle = !(EsifUpPm_IsPrimaryParticipantId(participantId) || (ESIF_INVALID_HANDLE == participantId));
	Bool isValidDomainHandle = !((ESIF_HANDLE_DEFAULT == domainId) || (ESIF_INVALID_HANDLE == domainId) || (EVENT_MGR_MATCH_ANY == domainId));
	Bool isMatchAnyHandle = (participantId == ESIF_HANDLE_MATCH_ANY_EVENT);

	if (isMatchAnyHandle) {
		category = EVENT_CATEGORY_MATCH_ANY;
	} 
	else if (!isValidParticipantHandle && !isValidDomainHandle) {
		category = EVENT_CATEGORY_APP;
	}
	else if (isValidParticipantHandle && !isValidDomainHandle) {
		category = EVENT_CATEGORY_PARTICIPANT;
	}
	else if (isValidParticipantHandle && isValidDomainHandle) {
		category = EVENT_CATEGORY_DOMAIN;
	}
	else {
		// This should never happen
	}
	return category;
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
