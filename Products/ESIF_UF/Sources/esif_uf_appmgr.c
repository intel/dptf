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

#define ESIF_TRACE_DEBUG_DISABLED // NOTE: Was Enabled for Windows, Disabled for Linux

#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_appmgr.h"	/* Application Manager */

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Friends */
extern int g_dst;
EsifAppMgr g_appMgr = {0};

EsifAppPtr GetAppFromHandle (const void *appHandle)
{
	u8 i = 0;
	EsifAppPtr a_app_ptr = NULL;

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		a_app_ptr = &g_appMgr.fEntries[i];

		if (NULL == a_app_ptr->fLibNamePtr) {
			continue;
		}

		if (a_app_ptr->fHandle == appHandle) {
			return a_app_ptr;
		}
	}
	return NULL;
}


static EsifAppPtr GetAppFromName (
	EsifAppMgr *THIS,
	EsifString lib_name
	)
{
	u8 i = 0;
	EsifAppPtr a_app_ptr = NULL;

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		a_app_ptr = &THIS->fEntries[i];

		if (NULL == a_app_ptr->fLibNamePtr) {
			continue;
		}

		if (!strcmp(a_app_ptr->fLibNamePtr, lib_name)) {
			return a_app_ptr;
		}
	}
	return NULL;
}


static eEsifError GetPrompt (
	EsifAppMgr *THIS,
	EsifDataPtr promptPtr
	)
{
	enum esif_rc rc = ESIF_OK;
	EsifAppPtr a_app_ptr = THIS->fSelectedAppPtr;
	if (NULL == a_app_ptr) {
		esif_ccb_sprintf(promptPtr->buf_len, (esif_string)promptPtr->buf_ptr, "esif(%u)->", g_dst);
	} else {
		ESIF_DATA(data_prompt, ESIF_DATA_STRING, promptPtr->buf_ptr, promptPtr->buf_len);
		rc = a_app_ptr->fInterface.fAppGetPromptFuncPtr(a_app_ptr->fHandle, &data_prompt);
	}
	return rc;
}


static char*esif_primitive_domain_str (
	u16 domain,
	char *str,
	u8 str_len
	)
{
	u8 *ptr = (u8*)&domain;
	esif_ccb_sprintf(str_len, str, "%c%c", *(ptr + 1), *ptr);
	return str;
}


static ESIF_INLINE void ms_guid_to_esif_guid (esif_guid_t *guid)
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


eEsifError EsifAppsEvent (
	UInt8 participantId,
	UInt16 domainId,
	enum esif_event_type eventType,
	EsifDataPtr eventData
	)
{
	EsifAppPtr app_ptr = NULL;
	u8 i = 0;
	char domain_str[8] = "";

	UNREFERENCED_PARAMETER(domain_str);

	if (NULL == eventData) {
		ESIF_TRACE_DEBUG("%s: APPLICATION_EVENT_NO_DATA\n"
				   "ParticipantID: %u\n"
				   "Domain:        %s(%04X)\n"
				   "EventType:     %s(%d)\n",
				   ESIF_FUNC,
				   participantId,
				   esif_primitive_domain_str(domainId, domain_str, 8),
				   domainId,
				   esif_event_type_str(eventType), eventType);
	} else {
		ESIF_TRACE_DEBUG("%s: APPLICATION_EVENT\n"
				   "ParticipantID: %u\n"
				   "Domain:        %s(%04X)\n"
				   "EventType:     %s(%d)\n"
				   "EventDataType: %s(%d)\n"
				   "EventData:     %p\n"
				   "  buf_ptr      %p\n"
				   "  buf_len      %d\n"
				   "  data_len     %d\n",
				   ESIF_FUNC,
				   participantId,
				   esif_primitive_domain_str(domainId, domain_str, 8),
				   domainId,
				   esif_event_type_str(eventType), eventType,
				   esif_data_type_str(eventData->type), eventData->type,
				   eventData,
				   eventData->buf_ptr, eventData->buf_len, eventData->data_len
				   );

		if (ESIF_DATA_STRUCTURE == eventData->type && NULL != eventData->buf_ptr) {
			char guid_str[ESIF_GUID_PRINT_SIZE];
			struct esif_data_guid_event *ev_ptr = (struct esif_data_guid_event*)eventData->buf_ptr;

			UNREFERENCED_PARAMETER(guid_str);

			ms_guid_to_esif_guid(&ev_ptr->event_GUID);

			ESIF_TRACE_DEBUG(
				"GUID:          %s\n"
				"Data Length:   %d\n"
				"Data:          %08x\n",
				esif_guid_print(&ev_ptr->event_GUID, guid_str),
				ev_ptr->event_context_length,
				*(UINT32*)ev_ptr->event_context);
		}

		if (ESIF_DATA_STRING == eventData->type && NULL != eventData->buf_ptr) {
			ESIF_TRACE_DEBUG(
				"Data Length:   %d\n"
				"Data:          %s\n",
				eventData->data_len,
				(EsifString)eventData->buf_ptr);
		}
	}

	/* For Each Application Send EVENT */
	for (i = 0; i < ESIF_MAX_APPS; i++) {
		app_ptr = &g_appMgr.fEntries[i];
		if (NULL == app_ptr->fHandle) {
			continue;
		}
		ESIF_TRACE_DEBUG("Send To APPLICATION: %s\n", app_ptr->fLibNamePtr);
		EsifAppEvent(app_ptr, participantId, domainId, eventData, eventType);
	}
	return ESIF_OK;
}


/* Creates the participant in each running application */
eEsifError EsifAppMgrCreateCreateParticipantInAllApps(const EsifUpPtr upPtr)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr app_ptr = NULL;
	UInt8 i;

	if(NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Now offer this participant to each running application */
	esif_ccb_read_lock(&g_appMgr.fLock);

	for (i = 0; i < ESIF_MAX_APPS; i++) {
		app_ptr = &g_appMgr.fEntries[i];
		if (NULL != app_ptr->fHandle) {
			EsifAppCreateParticipant(app_ptr, upPtr);
		}
	}
	esif_ccb_read_unlock(&g_appMgr.fLock);
exit:
	return rc;
}


/* Removes a participant from each running application */
eEsifError EsifAppMgrDestroyParticipantInAllApps(const EsifUpPtr upPtr)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr app_ptr = NULL;
	UInt8 i;

	if(NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_read_lock(&g_appMgr.fLock);

	for (i = 0; i < ESIF_MAX_APPS; i++) {

		app_ptr = &g_appMgr.fEntries[i];

		if (NULL != app_ptr->fHandle) {
			EsifAppDestroyParticipant(app_ptr, upPtr);
		}
	}

	esif_ccb_read_unlock(&g_appMgr.fLock);
exit:
	return rc;
}



eEsifError EsifAppMgrInit ()
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_DEBUG("%s: Init Action Manager (APPMGR)", ESIF_FUNC);

	EsifAppInit();

	g_appMgr.GetAppFromName = GetAppFromName;
	g_appMgr.GetPrompt = GetPrompt;

	return rc;
}


void EsifAppMgrExit ()
{
	u8 i = 0;
	EsifAppPtr a_app_ptr = NULL;

	EsifAppExit();
	ESIF_TRACE_DEBUG("%s: Exit Action Manager (APPMGR)", ESIF_FUNC);

	esif_ccb_read_lock(&g_appMgr.fLock);
	for (i = 0; i < ESIF_MAX_APPS; i++) {
		a_app_ptr = &g_appMgr.fEntries[i];
		
		if (NULL != a_app_ptr->fLibNamePtr) {
			esif_ccb_free(a_app_ptr->fLibNamePtr);
			esif_ccb_memset(a_app_ptr, 0, sizeof(*a_app_ptr));
		}
	}
	esif_ccb_read_unlock(&g_appMgr.fLock);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
