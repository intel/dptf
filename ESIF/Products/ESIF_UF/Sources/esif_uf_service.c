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

	ESIF_TRACE_DEBUG("\n"
		"Name Space           : %s\n"
		"Element Path         : %s\n"
		"App Handle           : " ESIF_HANDLE_FMT "\n",
		(EsifString)nameSpacePtr->buf_ptr,
		(EsifString)elementPathPtr->buf_ptr,
		esif_ccb_handle2llu(esifHandle));

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

	ESIF_TRACE_DEBUG("\n"
					 "Name Space           : %s\n"
					 "Element Path         : %s\n"
					 "Element Value        : %s\n"
					 "Element Flags        : %08x\n\n"
					 "App Handle           : " ESIF_HANDLE_FMT "\n",
					 (EsifString)nameSpacePtr->buf_ptr,
					 (EsifString)elementPathPtr->buf_ptr,
					 (EsifString)elementValuePtr->buf_ptr,
					 elementFlags,
					 esif_ccb_handle2llu(esifHandle));

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

	/* Banned Primitives that may not be called via the ESIF Interface */
	if (primitive == GET_CONFIG) {
		rc = ESIF_E_NOT_SUPPORTED;
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
	ESIF_TRACE_DEBUG("\n"
		"Primitive            : %s(%u)\n"
		"Qualifier            : %s\n"
		"Instance             : %u\n"
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
		"App Handle           : " ESIF_HANDLE_FMT "\n",
		esif_primitive_str((enum esif_primitive_type)primitive), primitive,
		qualifier_str,
		instance,
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
		esif_ccb_handle2llu(esifHandle));
	
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

	ESIF_TRACE_DEBUG("\n"
		"Message              : %s\n"
		"Log Type             : %s(%u)\n"
		"ESIF Data Pointer    : %p\n"
		"  Data Type:         : %s(%u)\n"
		"  Data Buffer:       : %p\n"
		"  Data Length:       : %u\n"
		"  App Handle         : " ESIF_HANDLE_FMT "\n",
		(esif_string)messagePtr->buf_ptr,
		esif_log_type_str(logType), logType,
		messagePtr,
		esif_data_type_str(messagePtr->type), messagePtr->type,
		messagePtr->buf_ptr,
		messagePtr->buf_len,
		esif_ccb_handle2llu(esifHandle));

	msg = (esif_string)messagePtr->buf_ptr;
	if (msg != NULL && *msg=='\n')
		msg++;

	/* If msg starts with "<EVENTLOG>", change fmt from "%s" to "<EVENTLOG>%s" so we can
	** detect it and remove it in the Trace Logging functions. This is necessary since the
	** logging functions use va_list arguments so we can't access the msg string directly.
	*/
	const char prefix[] = ESIF_SERVICE_ROUTE_EVENTLOG;
	char fmt[sizeof(prefix) + 2] = "%s";
	if (msg && esif_ccb_strnicmp(msg, prefix, sizeof(prefix) - 1) == 0) {
		msg += sizeof(prefix) - 1;
		esif_ccb_sprintf(sizeof(fmt), fmt, "%s%%s", prefix);
	}

	// Always Steer Application Messages to Target Routes if DPTF module enabled, regardless of global trace level
	switch (logType) {
	case eLogTypeFatal:
	case eLogTypeError:
		ESIF_TRACE_IFENABLED(ESIF_TRACEMODULE_DPTF, ESIF_TRACELEVEL_ERROR, fmt, msg);
		break;
	case eLogTypeWarning:
		ESIF_TRACE_IFENABLED(ESIF_TRACEMODULE_DPTF, ESIF_TRACELEVEL_WARN, fmt, msg);
		break;
	case eLogTypeInfo:
		ESIF_TRACE_IFENABLED(ESIF_TRACEMODULE_DPTF, ESIF_TRACELEVEL_INFO, fmt, msg);
		break;
	case eLogTypeDebug:
		ESIF_TRACE_IFENABLED(ESIF_TRACEMODULE_DPTF, ESIF_TRACELEVEL_DEBUG, fmt, msg);
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

	ESIF_TRACE_INFO("\n"
		"Registering Event\n"
		"  Event Type           : %d (%s)\n"
		"  Participant Handle   : " ESIF_HANDLE_FMT "\n"
		"  Domain Handle        : " ESIF_HANDLE_FMT "\n"
		"  App Handle			: " ESIF_HANDLE_FMT "\n"
		"  Event GUID           : %s\n",
		eventType, esif_event_type_str(eventType),
		esif_ccb_handle2llu(participantId),
		esif_ccb_handle2llu(domainHandle),
		esif_ccb_handle2llu(esifHandle),
		esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr));

	/* Validate the app calling the interface */
	appPtr = EsifAppMgr_GetAppFromHandle(esifHandle);
	if (NULL == appPtr) {
		// This can happen during a restart of all apps
		ESIF_TRACE_WARN("The app data was not found from app handle\n");
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

	ESIF_TRACE_INFO("\n"
		"Unregistering App event:\n"
		"  Event Type           : %d (%s)\n"
		"  Participant Handle   : " ESIF_HANDLE_FMT "\n"
		"  Domain Handle        : " ESIF_HANDLE_FMT "\n"
		"  App Handle           : " ESIF_HANDLE_FMT "\n"
		"  Event GUID           : %s\n",
		eventType, esif_event_type_str(eventType),
		esif_ccb_handle2llu(participantId),
		esif_ccb_handle2llu(domainHandle),
		esif_ccb_handle2llu(esifHandle),
		esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr));

	/* Validate the app calling the interface */
	appPtr = EsifAppMgr_GetAppFromHandle(esifHandle);
	if (NULL == appPtr) {
		// This can happen during a restart of all apps
		ESIF_TRACE_WARN("The app data was not found from app handle\n");
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

	ESIF_TRACE_DEBUG("Received Event\n"
		"Event GUID           : %s\n\n"
		"Participant Handle   : " ESIF_HANDLE_FMT "\n"
		"Domain Handle        : " ESIF_HANDLE_FMT "\n"
		"Event Data           : %p\n"
		"App Handle           : " ESIF_HANDLE_FMT "\n",
		esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr),
		esif_ccb_handle2llu(participantId),
		esif_ccb_handle2llu(domainHandle),
		eventDataPtr,
		esif_ccb_handle2llu(esifHandle));

	/* Validate the app calling the interface */
	appPtr = EsifAppMgr_GetAppFromHandle(esifHandle);
	if (NULL == appPtr) {
		// This can happen during a restart of all apps
		ESIF_TRACE_WARN("The app data was not found from app handle\n");
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

		ESIF_TRACE_INFO("\n"
			"Received APP event\n"
			"  Type: %s(%d)\n",
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

		ESIF_TRACE_INFO("\n"
			"Received PART Event\n"
			"  Type: %s(%d)\n"
			"  Participant: " ESIF_HANDLE_FMT "\n",
			esif_event_type_str(eventType), eventType,
			esif_ccb_handle2llu(localParticipantId));

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

		ESIF_TRACE_INFO("\n"
			"Received DOMAIN event\n"
			"  Type:          %s(%d)\n"
			"  Participant:   " ESIF_HANDLE_FMT "\n",
			"  Domain Handle: " ESIF_HANDLE_FMT "\n",
			esif_event_type_str(eventType), eventType,
			esif_ccb_handle2llu(localParticipantId),
			esif_ccb_handle2llu(domainHandle));

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

/*
** Worker Thread to run "start" commands received via ESIF Interface on a separate thread
*/
static void *ESIF_CALLCONV EsifSvcCommand_StartWorker(void *ctx)
{
	char *command = (char *)ctx;
	if (command) {
		// Skip "start" prefix from Intercepted commands, if any
		char start_prefix[] = "start ";
		size_t prefix_len = 0;
		if (esif_ccb_strnicmp(command, start_prefix, sizeof(start_prefix) - 1) == 0) {
			prefix_len = sizeof(start_prefix) - 1;
		}
		parse_cmd(command + prefix_len, ESIF_FALSE, ESIF_FALSE);
		esif_ccb_free(command);
	}
	return 0;
}

/*
** Checks whether the given multi-statement command is permitted or must run asynchronously
*/
static esif_error_t EsifSvcCommand_VerifyCommand(char *cmdbuf)
{
	esif_error_t rc = ESIF_OK;	// Command permitted and may run on current thread synchronously
	static struct {
		char *cmd;
		Bool banned;
	} cmd_mapping[] = {
		{ "$start" },			// $ = only allowed for first command in multi-command string ("start command one && command two")
		{ "config gddv set" },
		{ "config gddv reset" },
		{ "config gddv restore" },
		{ "getp_part ietm get_config", ESIF_TRUE },
		{ "getp_part ietm 344", ESIF_TRUE },
		{ "getp get_config", ESIF_TRUE },
		{ "getp 344", ESIF_TRUE },
		{ NULL }
	};
	int cmdnum = 0;

	// Check whether any of the multi-statement commands are asynchronous
	// Multi-Format: "[start] command one && command two && command three"
	while (cmdbuf) {
		while (isspace(*cmdbuf)) {
			cmdbuf++;
		}
		for (int j = 0; cmd_mapping[j].cmd; j++) {
			char *cmdthis = cmd_mapping[j].cmd;
			if (*cmdthis == '$') {
				if (cmdnum) {
					continue;
				}
				cmdthis++;
			}
			size_t cmdlen = esif_ccb_strlen(cmdthis, MAX_LINE);
			if (esif_ccb_strnicmp(cmdthis, cmdbuf, cmdlen) == 0) {
				if (cmdbuf[cmdlen] == 0 || isspace(cmdbuf[cmdlen])) {
					if (cmd_mapping[j].banned) {
						rc = ESIF_E_SESSION_PERMISSION_DENIED;	// Command not permitted via ESIF Interface
					}
					else {
						rc = ESIF_I_AGAIN;	// Command permitted but must run on another thread asynchronously
					}
					return rc;
				}
			}
		}
		char cmdsep[] = " && ";
		cmdbuf = esif_ccb_strstr(cmdbuf, cmdsep);
		if (cmdbuf) {
			cmdbuf += sizeof(cmdsep) - 1;
			cmdnum++;
		}
	}
	return rc;
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

	ESIF_TRACE_DEBUG("Command received\n");

	// Intercept some commands so they can be executed on a different thread
	if (argc == 1 && argv[0].type == ESIF_DATA_STRING && argv[0].buf_ptr) {
		char *command = NULL;

		// Intercept commands that need to run on a different thread to avoid deadlocks caused by restarting all ESIF apps
		rc = EsifSvcCommand_VerifyCommand((esif_string)argv[0].buf_ptr);

		if (rc == ESIF_I_AGAIN) {
			command = esif_ccb_strdup((esif_string)argv[0].buf_ptr);
		}
		else if (rc != ESIF_OK) {
			if (response && response->type == ESIF_DATA_STRING && response->buf_ptr && response->buf_len) {
				response->data_len = esif_ccb_sprintf(response->buf_len, response->buf_ptr, "%s\n", esif_rc_str(rc)) + 1;
			}
			ESIF_TRACE_DEBUG("Command verification failure = %d\n", rc);
			return rc;
		}

		// Run Intercepted commands on a different thread
		if (command) {
			esif_thread_t cmdThread = ESIF_THREAD_NULL;
			ESIF_TRACE_DEBUG("Executing command: %s\n", command);

			if ((rc = esif_ccb_thread_create(&cmdThread, EsifSvcCommand_StartWorker, command)) == ESIF_OK) {
				esif_ccb_thread_close(&cmdThread);
			}
			else {
				esif_ccb_free(command);
			}
			// Send Result of Thread Create, not Command Output
			if (response && response->type == ESIF_DATA_STRING && response->buf_ptr && response->buf_len) {
				response->data_len = esif_ccb_sprintf(response->buf_len, response->buf_ptr, "%s\n", esif_rc_str(rc)) + 1;
			}
			ESIF_TRACE_DEBUG("Exit Code = %d\n", rc);
			return rc;
		}
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
		if (argc == 1 && argv[0].type == ESIF_DATA_STRING) {
			static char fmthint[] = "<XML>";
			Bool isRest = ESIF_FALSE;
			if (response->type == ESIF_DATA_STRING && response->buf_ptr && response->buf_len >= response->data_len && response->data_len >= (u32)sizeof(fmthint) && esif_ccb_stricmp(response->buf_ptr, fmthint) == 0) {
				esif_ccb_memset(response->buf_ptr, 0, sizeof(fmthint));
				response->data_len = 0;
				isRest = ESIF_TRUE;
			}
			esif_shell_exec_command((char *)argv[0].buf_ptr, argv[0].data_len, isRest, ESIF_FALSE);
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
			if (response_len > response->buf_len || response->buf_ptr == NULL) {
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
	ESIF_TRACE_DEBUG("Exit Code = %d\n", rc);
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
	char guidStr[ESIF_GUID_PRINT_SIZE] = { 0 };

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

	ESIF_TRACE_DEBUG(
		"Sending app event:\n"
		"  App:            " ESIF_HANDLE_FMT "\n"
		"  App Context:    %p\n"
		"  Participant ID: " ESIF_HANDLE_FMT "\n"
		"  Domain:         0x%04X\n"
		"  Event Type:     %s\n"
		"  Event GUID:     %s\n",
		esif_ccb_handle2llu(appPtr->fHandle),
		appPtr->fAppCtxHandle,
		esif_ccb_handle2llu(participantId),
		domainId,
		esif_event_type_str(fpcEventPtr->esif_event),
		esif_guid_print((esif_guid_t *)dataGuid.buf_ptr, guidStr));

	rc = EsifApp_SendEvent(
		appPtr,
		participantId,
		domainId,
		eventDataPtr,
		fpcEventPtr->esif_event,
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
