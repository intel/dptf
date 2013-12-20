/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

// #define ESIF_TRACE_DEBUG_DISABLED

#include "esif_uf.h"			/* Upper Framework */
#include "esif_uf_appmgr.h"		/* Application Manager */
#include "esif_uf_cfgmgr.h"		/* Configuration Manager */
#include "esif_uf_log.h"		/* Logging */
#include "esif_uf_esif_iface.h"	/* ESIF Service Interface */
#include "esif_uf_service.h"	/* ESIF Service */
#include "esif_uf_primitive.h"	/* ESIF Primitive Execution */
#include "esif_dsp.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* DEBUG? */
#define POST_LOG_PACKAGE "Svc"
#define POST_LOG_MODULE  "Iface"

typedef enum _t_eCategoryType {
	eCategoryTypeNone,
	eCategoryTypeApplication,
	eCategoryTypeParticipant,
	eCategoryTypeDomain
} eCategoryType;

static ESIF_INLINE eCategoryType Categorize (
	const void *appHandle,
	const void *participantHandle,
	const void *domainHandle
	)
{
	eCategoryType category = eCategoryTypeNone;

	if (NULL != appHandle && NULL == participantHandle && NULL == domainHandle) {
		category = eCategoryTypeApplication;
	} else if (NULL != appHandle && NULL != participantHandle && NULL == domainHandle) {
		category = eCategoryTypeParticipant;
	} else if (NULL != appHandle && NULL != participantHandle && NULL != domainHandle) {
		category = eCategoryTypeDomain;
	} else {
		// This should never happen
	}
	return category;
}


/* Provide read access to ESIF configuration space */
eEsifError EsifSvcConfigGet (
	const void *esifHandle,
	const void *appHandle,
	const EsifDataPtr nameSpacePtr,
	const EsifDataPtr elementPathPtr,
	EsifDataPtr elementValuePtr
	)
{
	if (NULL == esifHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == nameSpacePtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == elementPathPtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == elementValuePtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("%s\n\n"
					 "ESIF Handle          : %p\n"
					 "Application Handle   : %p\n"
					 "Name Space           : %s\n"
					 "Element Path         : %s\n\n",
					 ESIF_FUNC,
					 esifHandle,
					 appHandle,
					 (EsifString)nameSpacePtr->buf_ptr,
					 (EsifString)elementPathPtr->buf_ptr);

	return EsifConfigGet(nameSpacePtr, elementPathPtr, elementValuePtr);
}


/* Provide write access to ESIF configuration space */
eEsifError EsifSvcConfigSet (
	const void *esifHandle,
	const void *appHandle,
	const EsifDataPtr nameSpacePtr,
	const EsifDataPtr elementPathPtr,
	const EsifDataPtr elementValuePtr,
	const UInt32 elementFlags
	)
{
	if (NULL == esifHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == nameSpacePtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == elementPathPtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == elementValuePtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("%s\n\n"
					 "ESIF Handle          : %p\n"
					 "Application Handle   : %p\n"
					 "Name Space           : %s\n"
					 "Element Path         : %s\n"
					 "Element Value        : %s\n"
					 "Element Flags        : %08x\n\n",
					 ESIF_FUNC,
					 esifHandle,
					 appHandle,
					 (EsifString)nameSpacePtr->buf_ptr,
					 (EsifString)elementPathPtr->buf_ptr,
					 (EsifString)elementValuePtr->buf_ptr,
					 elementFlags);

	return EsifConfigSet(nameSpacePtr, elementPathPtr, elementFlags,
						 elementValuePtr);
}


/* Lookup participant data for a handle */
static AppParticipantDataMapPtr find_participant_data_map_from_handle (
	const EsifAppPtr appPtr,
	const void *participantHandle
	)
{
	UInt8 i = 0;
	AppParticipantDataMapPtr participant_data_map_ptr = NULL;

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++)
		if (appPtr->fParticipantData[i].fAppParticipantHandle ==
			participantHandle) {
			participant_data_map_ptr = &appPtr->fParticipantData[i];
			break;
		}

	return participant_data_map_ptr;
}


/* Lookup domain data for a handle */
static AppDomainDataMapPtr find_domain_data_from_handle (
	const AppParticipantDataMapPtr participantPtr,
	const void *domainHandle
	)
{
	UInt8 i = 0;
	AppDomainDataMapPtr domain_data_map_ptr = NULL;

	for (i = 0; i < MAX_DOMAIN_ENTRY; i++)
		if (participantPtr->fDomainData[i].fAppDomainHandle == domainHandle) {
			domain_data_map_ptr = &participantPtr->fDomainData[i];
			break;
		}

	return domain_data_map_ptr;
}


/* Provide execute action for ESIF primitive */
eEsifError EsifSvcPrimitiveExec (
	const void *esifHandle,
	const void *appHandle,
	const void *participantHandle,
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
	EsifAppPtr app_ptr = NULL;

	if (NULL == esifHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == requestPtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	if (NULL == responsePtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	/* Callbacks must always provide this */
	app_ptr = GetAppFromHandle(appHandle);
	if (NULL == app_ptr) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	/* Lookup our Participant ID from the provided participant handle */
	/* If the participantHandle is NULL use particiapnt 0 as agreed with DPTF */
	if (NULL == participantHandle) {
		participant_id = 0;
		participant_data_map_ptr = &app_ptr->fParticipantData[participant_id];
	} else {
		participant_data_map_ptr = find_participant_data_map_from_handle(app_ptr, participantHandle);
		if (NULL != participant_data_map_ptr) {
			participant_id = participant_data_map_ptr->fUpPtr->fInstance;
		}
	}

	if (NULL == participant_data_map_ptr) {
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
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		domain_id     = domain_data_map_ptr->fAppDomainId;
		qualifier_str = domain_data_map_ptr->fQualifier;
	}

	/* Share what we have learned so far */
	ESIF_TRACE_DEBUG("%s\n\n"
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
					 ESIF_FUNC,
					 esifHandle,
					 appHandle,
					 participantHandle,
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
eEsifError EsifSvcWriteLog (
	const void *esifHandle,
	const void *appHandle,
	const void *participantHandle,
	const void *domainHandle,
	const EsifDataPtr messagePtr,
	const eLogType logType
	)
{
	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(domainHandle);

	if (NULL == esifHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == messagePtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("%s\n\n"
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
					 ESIF_FUNC,
					 esifHandle,
					 appHandle,
					 participantHandle,
					 domainHandle,
					 messagePtr,
					 esif_data_type_str(messagePtr->type), messagePtr->type,
					 messagePtr->buf_ptr,
					 messagePtr->buf_len,
					 esif_log_type_str(logType), logType,
					 (esif_string)messagePtr->buf_ptr);

	POST_LOG_TYPE(logType, "%s", (esif_string)messagePtr->buf_ptr);
	return ESIF_OK;
}


/*
** Event Registration / Unregistration
*/

static ESIF_INLINE void esif_guid_to_ms_guid (esif_guid_t *guid)
{
#ifdef ESIF_ATTR_OS_WINDOWS
	u8 *ptr = (u8*)guid;
	u8 b[ESIF_GUID_LEN] = {0};

	ESIF_TRACE_DEBUG("%s:\n", ESIF_FUNC);
	esif_ccb_memcpy(&b, ptr, ESIF_GUID_LEN);

	*(ptr + 0) = b[3];
	*(ptr + 1) = b[2];
	*(ptr + 2) = b[1];
	*(ptr + 3) = b[0];
	*(ptr + 4) = b[5];
	*(ptr + 5) = b[4];
	*(ptr + 6) = b[7];
	*(ptr + 7) = b[6];
#endif
}


/* TODO Move To Header */
eEsifError register_for_power_notification (const esif_guid_t *guid);

static eEsifError EsifSvcEventRegisterAppByGroup (
	const EsifAppPtr appPtr,
	const struct esif_fpc_event *event_ptr
	)
{
	eEsifError rc  = ESIF_OK;
	UInt64 bitMask = 1;	/* must be one */

	char guid_str[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guid_str);

	ESIF_TRACE_DEBUG("%s: Event Alias: %s\n", ESIF_FUNC, event_ptr->name);
	ESIF_TRACE_DEBUG("%s: Event GUID: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_guid, guid_str));
	ESIF_TRACE_DEBUG("%s: Event Type: %s(%d)\n", ESIF_FUNC, esif_event_type_str(event_ptr->esif_event), event_ptr->esif_event);
	ESIF_TRACE_DEBUG("%s: Event Data: %s(%d)\n", ESIF_FUNC,
					 esif_data_type_str(event_ptr->esif_group_data_type), event_ptr->esif_group_data_type);
	ESIF_TRACE_DEBUG("%s: Event Key: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_key, guid_str));

	/*
	 * Set the mask here so in case we get an immediate callback from the OS with state while registering.
	 */
	bitMask = bitMask << (UInt32) event_ptr->esif_event;
	ESIF_TRACE_DEBUG("%s: s %016llx\n", ESIF_FUNC, bitMask);
	appPtr->fRegisteredEvents |= bitMask;
	ESIF_TRACE_DEBUG("%s: events %016llx\n", ESIF_FUNC, appPtr->fRegisteredEvents);

	switch (event_ptr->esif_group) {
	case ESIF_EVENT_GROUP_ACPI:
		ESIF_TRACE_DEBUG("%s: ACPI\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_CODE:
		ESIF_TRACE_DEBUG("%s: CODE\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_DPTF:
		ESIF_TRACE_DEBUG("%s: DPTF\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_POWER:
	{
		esif_guid_t power_guid = {0};
		esif_ccb_memcpy(&power_guid, &event_ptr->event_key, ESIF_GUID_LEN);

		ESIF_TRACE_DEBUG("%s: POWER\n", ESIF_FUNC);
		esif_guid_to_ms_guid(&power_guid);
		ESIF_TRACE_DEBUG("%s: Power Event Key: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)&power_guid, guid_str));
#ifdef ESIF_ATTR_OS_WINDOWS
		register_for_power_notification(&power_guid);
#endif
		break;
	}
	}
	return rc;
}


static eEsifError EsifSvcEventUnregisterAppByGroup (
	const EsifAppPtr appPtr,
	const struct esif_fpc_event *event_ptr
	)
{
	eEsifError rc  = ESIF_OK;
	UInt64 bitMask = 1;	/* must be one */
	char guid_str[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guid_str);

	ESIF_TRACE_DEBUG("%s: Event Alias: %s\n", ESIF_FUNC, event_ptr->name);
	ESIF_TRACE_DEBUG("%s: Event GUID: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_guid, guid_str));
	ESIF_TRACE_DEBUG("%s: Event Type: %s(%d)\n", ESIF_FUNC, esif_event_type_str(event_ptr->esif_event), event_ptr->esif_event);
	ESIF_TRACE_DEBUG("%s: Event Data: %s(%d)\n", ESIF_FUNC,
					 esif_data_type_str(event_ptr->esif_group_data_type), event_ptr->esif_group_data_type);
	ESIF_TRACE_DEBUG("%s: Event Key: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_key, guid_str));

	switch (event_ptr->esif_group) {
	case ESIF_EVENT_GROUP_ACPI:
		ESIF_TRACE_DEBUG("%s: ACPI\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_CODE:
		ESIF_TRACE_DEBUG("%s: CODE\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_DPTF:
		ESIF_TRACE_DEBUG("%s: DPTF\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_POWER:
		ESIF_TRACE_DEBUG("%s: POWER\n", ESIF_FUNC);
		break;
	}

	bitMask = bitMask << (UInt32) event_ptr->esif_event;
	bitMask = ~bitMask;

	ESIF_TRACE_DEBUG("%s: bitMask %016llx\n", ESIF_FUNC, bitMask);
	appPtr->fRegisteredEvents &= bitMask;
	ESIF_TRACE_DEBUG("%s: events %016llx\n", ESIF_FUNC, appPtr->fRegisteredEvents);

	return rc;
}


static eEsifError EsifSvcEventRegisterParticipantByGroup (
	const AppParticipantDataMapPtr participant_ptr,
	const struct esif_fpc_event *event_ptr
	)
{
	eEsifError rc  = ESIF_OK;
	UInt64 bitMask = 1;	/* must be one */
	char guid_str[ESIF_GUID_PRINT_SIZE];
	UNREFERENCED_PARAMETER(guid_str);

	ESIF_TRACE_DEBUG("%s: Event Alias: %s\n", ESIF_FUNC, event_ptr->name);
	ESIF_TRACE_DEBUG("%s: Event GUID: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_guid, guid_str));
	ESIF_TRACE_DEBUG("%s: Event Type: %s(%d)\n", ESIF_FUNC, esif_event_type_str(event_ptr->esif_event), event_ptr->esif_event);
	ESIF_TRACE_DEBUG("%s: Event Data: %s(%d)\n", ESIF_FUNC,
					 esif_data_type_str(event_ptr->esif_group_data_type), event_ptr->esif_group_data_type);
	ESIF_TRACE_DEBUG("%s: Event Key: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_key, guid_str));

	/*
	 * Set the mask here so in case we get an immediate callback from the OS with state while registering.
	 */
	bitMask = bitMask << (UInt32) event_ptr->esif_event;
	ESIF_TRACE_DEBUG("%s: bitMask %016llx\n", ESIF_FUNC, bitMask);
	participant_ptr->fRegisteredEvents |= bitMask;
	ESIF_TRACE_DEBUG("%s: events %016llx\n", ESIF_FUNC, participant_ptr->fRegisteredEvents);

	switch (event_ptr->esif_group) {
	case ESIF_EVENT_GROUP_ACPI:
		ESIF_TRACE_DEBUG("%s: ACPI\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_CODE:
		ESIF_TRACE_DEBUG("%s: CODE\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_DPTF:
		ESIF_TRACE_DEBUG("%s: DPTF\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_POWER:
		ESIF_TRACE_DEBUG("%s: POWER\n", ESIF_FUNC);
		break;
	}

	return rc;
}


static eEsifError EsifSvcEventUnregisterParticipantByGroup (
	const AppParticipantDataMapPtr participant_ptr,
	const struct esif_fpc_event *event_ptr
	)
{
	eEsifError rc  = ESIF_OK;
	UInt64 bitMask = 1;	/* must be one */
	char guid_str[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guid_str);

	ESIF_TRACE_DEBUG("%s: Event Alias: %s\n", ESIF_FUNC, event_ptr->name);
	ESIF_TRACE_DEBUG("%s: Event GUID: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_guid, guid_str));
	ESIF_TRACE_DEBUG("%s: Event Type: %s(%d)\n", ESIF_FUNC, esif_event_type_str(event_ptr->esif_event), event_ptr->esif_event);
	ESIF_TRACE_DEBUG("%s: Event Data: %s(%d)\n", ESIF_FUNC,
					 esif_data_type_str(event_ptr->esif_group_data_type), event_ptr->esif_group_data_type);
	ESIF_TRACE_DEBUG("%s: Event Key: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_key, guid_str));

	switch (event_ptr->esif_group) {
	case ESIF_EVENT_GROUP_ACPI:
		ESIF_TRACE_DEBUG("%s: ACPI\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_CODE:
		ESIF_TRACE_DEBUG("%s: CODE\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_DPTF:
		ESIF_TRACE_DEBUG("%s: DPTF\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_POWER:
		ESIF_TRACE_DEBUG("%s: POWER\n", ESIF_FUNC);
		break;
	}

	bitMask = bitMask << (UInt32) event_ptr->esif_event;
	bitMask = ~bitMask;

	ESIF_TRACE_DEBUG("%s: bitMask %016llx\n", ESIF_FUNC, bitMask);
	participant_ptr->fRegisteredEvents &= bitMask;
	ESIF_TRACE_DEBUG("%s: events %016llx\n", ESIF_FUNC, participant_ptr->fRegisteredEvents);

	return rc;
}


static eEsifError EsifSvcEventRegisterDomainByGroup (
	const AppDomainDataMapPtr domain_ptr,
	const struct esif_fpc_event *event_ptr
	)
{
	eEsifError rc  = ESIF_OK;
	UInt64 bitMask = 1;	/* must be one */
	char guid_str[ESIF_GUID_PRINT_SIZE];
	UNREFERENCED_PARAMETER(guid_str);

	ESIF_TRACE_DEBUG("%s: Event Alias: %s\n", ESIF_FUNC, event_ptr->name);
	ESIF_TRACE_DEBUG("%s: Event GUID: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_guid, guid_str));
	ESIF_TRACE_DEBUG("%s: Event Type: %s(%d)\n", ESIF_FUNC, esif_event_type_str(event_ptr->esif_event), event_ptr->esif_event);
	ESIF_TRACE_DEBUG("%s: Event Data: %s(%d)\n", ESIF_FUNC,
					 esif_data_type_str(event_ptr->esif_group_data_type), event_ptr->esif_group_data_type);
	ESIF_TRACE_DEBUG("%s: Event Key: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_key, guid_str));

	/*
	 * Set the mask here so in case we get an immediate callback from the OS with state while registering.
	 */
	bitMask = bitMask << (UInt32) event_ptr->esif_event;
	ESIF_TRACE_DEBUG("%s: bitMask %016llx\n", ESIF_FUNC, bitMask);
	domain_ptr->fRegisteredEvents |= bitMask;
	ESIF_TRACE_DEBUG("%s: events %016llx\n", ESIF_FUNC, domain_ptr->fRegisteredEvents);

	switch (event_ptr->esif_group) {
	case ESIF_EVENT_GROUP_ACPI:
		ESIF_TRACE_DEBUG("%s: ACPI\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_CODE:
		ESIF_TRACE_DEBUG("%s: CODE\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_DPTF:
		ESIF_TRACE_DEBUG("%s: DPTF\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_POWER:
		ESIF_TRACE_DEBUG("%s: POWER\n", ESIF_FUNC);
		break;
	}

	return rc;
}


static eEsifError EsifSvcEventUnregisterDomainByGroup (
	const AppDomainDataMapPtr domain_ptr,
	const struct esif_fpc_event *event_ptr
	)
{
	eEsifError rc  = ESIF_OK;
	UInt64 bitMask = 1;	/* must be one */
	char guid_str[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guid_str);

	ESIF_TRACE_DEBUG("%s: Event Alias: %s\n", ESIF_FUNC, event_ptr->name);
	ESIF_TRACE_DEBUG("%s: Event GUID: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_guid, guid_str));
	ESIF_TRACE_DEBUG("%s: Event Type: %s(%d)\n", ESIF_FUNC, esif_event_type_str(event_ptr->esif_event), event_ptr->esif_event);
	ESIF_TRACE_DEBUG("%s: Event Data: %s(%d)\n", ESIF_FUNC,
					 esif_data_type_str(event_ptr->esif_group_data_type), event_ptr->esif_group_data_type);
	ESIF_TRACE_DEBUG("%s: Event Key: %s\n", ESIF_FUNC, esif_guid_print((esif_guid_t*)event_ptr->event_key, guid_str));

	switch (event_ptr->esif_group) {
	case ESIF_EVENT_GROUP_ACPI:
		ESIF_TRACE_DEBUG("%s: ACPI\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_CODE:
		ESIF_TRACE_DEBUG("%s: CODE\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_DPTF:
		ESIF_TRACE_DEBUG("%s: DPTF\n", ESIF_FUNC);
		break;

	case ESIF_EVENT_GROUP_POWER:
		ESIF_TRACE_DEBUG("%s: POWER\n", ESIF_FUNC);
		break;
	}

	bitMask = bitMask << (UInt32) event_ptr->esif_event;
	bitMask = ~bitMask;

	ESIF_TRACE_DEBUG("%s: bitMask %016llx\n", ESIF_FUNC, bitMask);
	domain_ptr->fRegisteredEvents &= bitMask;
	ESIF_TRACE_DEBUG("%s: events %016llx\n", ESIF_FUNC, domain_ptr->fRegisteredEvents);

	return rc;
}


/* Provide registration for ESIF event */
eEsifError EsifSvcEventRegister (
	const void *esifHandle,
	const void *appHandle,
	const void *participantHandle,
	const void *domainHandle,
	const EsifDataPtr eventGuid
	)
{
	eEsifError rc = ESIF_OK;
	char guid_str[ESIF_GUID_PRINT_SIZE];
	UNREFERENCED_PARAMETER(guid_str);

	if (NULL == esifHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == eventGuid || NULL == eventGuid->buf_ptr || ESIF_DATA_GUID != eventGuid->type) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("%s\n\n"
					 "ESIF Handle          : %p\n"
					 "App Handle           : %p\n"
					 "Participant Handle   : %p\n"
					 "Domain Handle        : %p\n"
					 "Event GUID           : %s\n\n",
					 ESIF_FUNC,
					 esifHandle,
					 appHandle,
					 participantHandle,
					 domainHandle,
					 esif_guid_print((esif_guid_t*)eventGuid->buf_ptr, guid_str));

	/* Determine what to do based on provided parameters */
	switch (Categorize(appHandle, participantHandle, domainHandle)) {
	case eCategoryTypeApplication:
	{
		EsifAppPtr app_ptr = NULL;
		EsifUpPtr up_ptr   = NULL;
		struct esif_fpc_event *event_ptr = NULL;

		/* Find Our Participant 0 */
		up_ptr = EsifUpManagerGetAvailableParticipantByInstance(0);
		if (NULL == up_ptr) {
			rc = ESIF_E_UNSPECIFIED;
			goto exit;
		}
		ESIF_TRACE_DEBUG("%s: Using Participant %s\n", ESIF_FUNC, up_ptr->fMetadata.fName);

		event_ptr = up_ptr->fDspPtr->get_event_by_guid(up_ptr->fDspPtr, *(esif_guid_t*)eventGuid->buf_ptr);
		if (NULL == event_ptr) {
			ESIF_TRACE_DEBUG("%s: EVENT NOT FOUND/SUPPORTED\n", ESIF_FUNC);
			rc = ESIF_E_NOT_FOUND;
			goto exit;
		}

		app_ptr = GetAppFromHandle(appHandle);
		if (NULL == app_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		rc = EsifSvcEventRegisterAppByGroup(app_ptr, event_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case eCategoryTypeParticipant:
	{
		/* Register with participant */
		AppParticipantDataMapPtr participant_ptr = NULL;
		EsifAppPtr app_ptr = NULL;
		struct esif_fpc_event *event_ptr = NULL;

		app_ptr = GetAppFromHandle(appHandle);
		if (NULL == app_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		participant_ptr = find_participant_data_map_from_handle(app_ptr, participantHandle);
		if (NULL == participant_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		ESIF_TRACE_DEBUG("%s: Using Participant %s\n", ESIF_FUNC, participant_ptr->fUpPtr->fMetadata.fName);

		/* Find The Event Associated With Participant  */
		event_ptr = participant_ptr->fUpPtr->fDspPtr->get_event_by_guid(participant_ptr->fUpPtr->fDspPtr, *(esif_guid_t*)eventGuid->buf_ptr);
		if (NULL == event_ptr) {
			ESIF_TRACE_DEBUG("%s: EVENT NOT FOUND/SUPPORTED\n", ESIF_FUNC);
			rc = ESIF_E_NOT_FOUND;
			goto exit;
		}

		rc = EsifSvcEventRegisterParticipantByGroup(participant_ptr, event_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case eCategoryTypeDomain:
	{
		/* Register with domain */
		AppParticipantDataMapPtr participant_ptr = NULL;
		AppDomainDataMapPtr domain_ptr   = NULL;
		EsifAppPtr app_ptr = NULL;
		struct esif_fpc_event *event_ptr = NULL;

		app_ptr = GetAppFromHandle(appHandle);
		if (NULL == app_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		participant_ptr = find_participant_data_map_from_handle(app_ptr, participantHandle);
		if (NULL == participant_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		domain_ptr = find_domain_data_from_handle(participant_ptr, domainHandle);
		if (NULL == domain_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		ESIF_TRACE_DEBUG("%s: Using Participant %s\n",
						 ESIF_FUNC, participant_ptr->fUpPtr->fMetadata.fName);

		/* Find The Event Associated With Participant  */
		event_ptr = participant_ptr->fUpPtr->fDspPtr->get_event_by_guid(participant_ptr->fUpPtr->fDspPtr, *(esif_guid_t*)eventGuid->buf_ptr);
		if (NULL == event_ptr) {
			ESIF_TRACE_DEBUG("%s: EVENT NOT FOUND/SUPPORTED\n", ESIF_FUNC);
			rc = ESIF_E_NOT_FOUND;
			goto exit;
		}

		rc = EsifSvcEventRegisterDomainByGroup(domain_ptr, event_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	default:
		rc = ESIF_E_INVALID_HANDLE;
		break;
	}
exit:
	return rc;
}


/* Provide unregistration for previously registered ESIF event */
eEsifError EsifSvcEventUnregister (
	const void *esifHandle,
	const void *appHandle,
	const void *participantHandle,
	const void *domainHandle,
	const EsifDataPtr eventGuid
	)
{
	eEsifError rc = ESIF_OK;
	char guid_str[ESIF_GUID_PRINT_SIZE];
	UNREFERENCED_PARAMETER(guid_str);

	if (NULL == esifHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == eventGuid || NULL == eventGuid->buf_ptr || ESIF_DATA_GUID != eventGuid->type) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("%s\n\n"
					 "ESIF Handle          : %p\n"
					 "App Handle           : %p\n"
					 "Participant Handle   : %p\n"
					 "Domain Handle        : %p\n"
					 "Event GUID           : %s\n\n",
					 ESIF_FUNC,
					 esifHandle,
					 appHandle,
					 participantHandle,
					 domainHandle,
					 esif_guid_print((esif_guid_t*)eventGuid->buf_ptr, guid_str));

	/* Determine what to do based on provided parameters */
	switch (Categorize(appHandle, participantHandle, domainHandle)) {
	case eCategoryTypeApplication:
	{
		/* Register with application */
		EsifAppPtr app_ptr = NULL;
		EsifUpPtr up_ptr   = NULL;
		struct esif_fpc_event *event_ptr = NULL;

		/* Find Our Participant 0 */
		up_ptr = EsifUpManagerGetAvailableParticipantByInstance(0);
		if (NULL == up_ptr) {
			rc = ESIF_E_UNSPECIFIED;
			goto exit;
		}
		ESIF_TRACE_DEBUG("%s: Using Participant %s\n", ESIF_FUNC, up_ptr->fMetadata.fName);

		/* Find The Event Associated With Participant 0 */
		event_ptr = up_ptr->fDspPtr->get_event_by_guid(up_ptr->fDspPtr, *(esif_guid_t*)eventGuid->buf_ptr);
		if (NULL == event_ptr) {
			ESIF_TRACE_DEBUG("%s: EVENT NOT FOUND/SUPPORTED\n", ESIF_FUNC);
			rc = ESIF_E_NOT_FOUND;
			goto exit;
		}

		app_ptr = GetAppFromHandle(appHandle);
		if (NULL == app_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		rc = EsifSvcEventUnregisterAppByGroup(app_ptr, event_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case eCategoryTypeParticipant:
	{
		/* Register with participant */
		EsifAppPtr app_ptr = NULL;
		AppParticipantDataMapPtr participant_ptr = NULL;
		struct esif_fpc_event *event_ptr = NULL;

		app_ptr = GetAppFromHandle(appHandle);
		if (NULL == app_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		participant_ptr = find_participant_data_map_from_handle(app_ptr, participantHandle);
		if (NULL == participant_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		ESIF_TRACE_DEBUG("%s: Using Participant %s\n", ESIF_FUNC, participant_ptr->fUpPtr->fMetadata.fName);

		/* Find The Event Associated With Participant  */
		event_ptr = participant_ptr->fUpPtr->fDspPtr->get_event_by_guid(participant_ptr->fUpPtr->fDspPtr, *(esif_guid_t*)eventGuid->buf_ptr);
		if (NULL == event_ptr) {
			ESIF_TRACE_WARN("%s: EVENT NOT FOUND/SUPPORTED\n", ESIF_FUNC);
			rc = ESIF_E_NOT_FOUND;
			goto exit;
		}

		rc = EsifSvcEventUnregisterParticipantByGroup(participant_ptr, event_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case eCategoryTypeDomain:
	{
		/* Register with domain */
		AppParticipantDataMapPtr participant_ptr = NULL;
		AppDomainDataMapPtr domain_ptr   = NULL;
		EsifAppPtr app_ptr = NULL;
		struct esif_fpc_event *event_ptr = NULL;

		app_ptr = GetAppFromHandle(appHandle);
		if (NULL == app_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		participant_ptr = find_participant_data_map_from_handle(app_ptr, participantHandle);
		if (NULL == participant_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		domain_ptr = find_domain_data_from_handle(participant_ptr, domainHandle);
		if (NULL == domain_ptr) {
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		ESIF_TRACE_DEBUG("%s: Using Participant %s\n",
						 ESIF_FUNC, participant_ptr->fUpPtr->fMetadata.fName);

		/* Find The Event Associated With Participant  */
		event_ptr = participant_ptr->fUpPtr->fDspPtr->get_event_by_guid(participant_ptr->fUpPtr->fDspPtr, *(esif_guid_t*)eventGuid->buf_ptr);
		if (NULL == event_ptr) {
			ESIF_TRACE_DEBUG("%s: EVENT NOT FOUND/SUPPORTED\n", ESIF_FUNC);
			rc = ESIF_E_NOT_FOUND;
			goto exit;
		}

		rc = EsifSvcEventUnregisterDomainByGroup(domain_ptr, event_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	default:
		break;
	}
exit:
	return rc;
}


eEsifError EsifSvcInit ()
{
	return ESIF_OK;
}


void EsifSvcExit ()
{
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
