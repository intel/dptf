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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_EVENT

#include "esif_uf.h"	/* Upper Framework */
#include "esif_dsp.h"	/* Device Support Package */
#include "esif_pm.h"	/* Upper Participant Manager */
#include "esif_ipc.h"	/* IPC Abstraction */
#include "esif_uf_appmgr.h"
#include "esif_uf_ipc.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

void esif_dispatch_event();
int esif_send_dsp(char *filename, u8 dst_id);

extern int g_quit;
extern int g_quit2;
extern int g_disconnectClient;
extern int g_autocpc;
extern esif_handle_t g_ipc_handle;

#define PATH_STR_LEN 128

static char *esif_primitive_domain_str(
	u16 domain,
	char *str,
	u8 str_len
	)
{
	u8 *ptr = (u8 *)&domain;
	esif_ccb_sprintf(str_len, str, "%c%c", *(ptr + 1), *ptr);
	return str;
}


// Dispatch An Event
void esif_dispatch_event()
{
	int r_bytes     = 0;
	int data_len    = 1024;	/* TODO: Change from "magic number" */
	enum esif_rc rc = ESIF_OK;


	struct esif_ipc *ipc_ptr = esif_ipc_alloc(ESIF_IPC_TYPE_EVENT, data_len);
	if (NULL == ipc_ptr) {
		ESIF_TRACE_ERROR("Fail to allocate esif_ipc\n");
		return;
	}

//
// TODO:  This needs to be in an OS abstraction layer
//
#ifdef ESIF_ATTR_OS_LINUX
	r_bytes = read(g_ipc_handle, ipc_ptr, data_len);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS

	rc = ipc_execute(ipc_ptr);
	//
	// TODO:  The return size check is a W/A until a better solution is developed to determine if an event was returned.
	// (The current LF code does not change the original data_len or fail the request if there are not events.)
	//
	r_bytes = 0;
	if ((ESIF_OK == rc) && (ipc_ptr->data_len != (u32)data_len)) {
		r_bytes = ipc_ptr->data_len;
	}
#endif

	// Have Event?
	if (r_bytes > 0) {
		struct esif_ipc_event_header *event_hdr_ptr = NULL;
		ESIF_TRACE_DEBUG("%s_%s: ipc version=%d, type=%d, len = %d, data_len=%d\n",
						 ESIF_ATTR_OS, ESIF_FUNC,
						 ipc_ptr->version,
						 ipc_ptr->type,
						 r_bytes,
						 ipc_ptr->data_len);

		event_hdr_ptr = (struct esif_ipc_event_header *)(ipc_ptr + 1);
		EsifEventProcess(event_hdr_ptr);
	}
	esif_ipc_free(ipc_ptr);
}


void EsifEventProcess(struct esif_ipc_event_header *eventHdrPtr)
{
	eEsifError rc = ESIF_OK;
	char domain_str[8] = "";
	esif_ccb_time_t now;
	struct esif_data void_data = {ESIF_DATA_VOID, NULL, 0};
	UInt8 participantId;

	UNREFERENCED_PARAMETER(domain_str);

	esif_ccb_system_time(&now);

	ESIF_TRACE_DEBUG(
		"\n"
		"===================================================\n"
		"ESIF IPC EVENT HEADER: Timestamp: %llu\n"
		"                         Latency: %u msec\n"
		"===================================================\n"
		"Version:     %d\n"
		"Type:        %s(%d)\n"
		"ID:          %llu\n"
		"Timestamp:   %llu\n"
		"Priority:    %s(%d)\n"
		"Source:      %d\n"
		"Dest:        %d\n"
		"Dest Domain: %s(%04X)\n"
		"Data Size:   %u\n\n",
		(u64)now,
		(int)(now - eventHdrPtr->timestamp),
		eventHdrPtr->version,
		esif_event_type_str(eventHdrPtr->type),
		eventHdrPtr->type,
		eventHdrPtr->id,
		eventHdrPtr->timestamp,
		esif_event_priority_str(eventHdrPtr->priority),
		eventHdrPtr->priority,
		eventHdrPtr->src_id,
		eventHdrPtr->dst_id,
		esif_primitive_domain_str(eventHdrPtr->dst_domain_id, domain_str, 8),
		eventHdrPtr->dst_domain_id,
		eventHdrPtr->data_len);

	if (ESIF_EVENT_PARTICIPANT_CREATE == eventHdrPtr->type) {
		struct esif_ipc_event_data_create_participant *data_ptr = NULL;
		EsifString edp_filename_ptr = NULL;

		if(eventHdrPtr->data_len < sizeof(*data_ptr)) {
			ESIF_TRACE_ERROR("The event data is not enough for ESIF_EVENT_PARTICIPANT_CREATE\n");
			goto exit;
		}

		// Event Data
		data_ptr = (struct esif_ipc_event_data_create_participant *)(eventHdrPtr + 1);

		edp_filename_ptr = esif_uf_dm_select_dsp_for_participant(data_ptr);

		if (EsifUpManagerDoesAvailableParticipantExistByName(data_ptr->name)) {
			ESIF_TRACE_WARN("Participant %s has already existed in UF\n", data_ptr->name);
			return;
		}

		if (g_autocpc && NULL != edp_filename_ptr) {
			char edp_full_path[PATH_STR_LEN]={0};
			esif_build_path(edp_full_path, sizeof(edp_full_path), ESIF_PATHTYPE_DSP, edp_filename_ptr, ".edp");
			ESIF_TRACE_DEBUG("AutoCPC Selected DSP Delivery Method EDP(CPC): %s\n", edp_full_path);

			if (0 == esif_send_dsp(edp_full_path, eventHdrPtr->src_id)) {
				EsifUpPtr up_ptr = NULL;
				ESIF_TRACE_DEBUG("\nCreate Upper Participant: %s\n", data_ptr->name);

				up_ptr = EsifUpManagerCreateParticipant(eParticipantOriginLF, &eventHdrPtr->src_id, data_ptr);
				if (up_ptr) {
					/* Associate a DSP with the participant */
					up_ptr->fDspPtr = esif_uf_dm_select_dsp_by_code(edp_filename_ptr);
					if (up_ptr->fDspPtr) {
						ESIF_TRACE_DEBUG("Hit DSP Lookup %s\n", up_ptr->fDspPtr->code_ptr);

						/* Now offer this participant to each running application */
						EsifAppMgrCreateCreateParticipantInAllApps(up_ptr);
					} else {
						ESIF_TRACE_ERROR("Missed DSP Lookup %s\n", edp_filename_ptr);
						EsifUpManagerUnregisterParticipant(eParticipantOriginLF, up_ptr);
						up_ptr = NULL;
					}
				}
			}
		} else {
			ESIF_TRACE_DEBUG("Please manually load a CPC for this Participant\n");
			ESIF_TRACE_DEBUG("Example: dst %d then loadcpc\n\n", eventHdrPtr->src_id);
		}
	} else {
		/*
		* Can't move this above everything because if a participant is not yet created, the
		* mapping function will fail.
		*/
		participantId = eventHdrPtr->src_id;
		if (eventHdrPtr->dst_id == ESIF_INSTANCE_UF) {
			rc = EsifUpManagerMapLpidToPartHandle(eventHdrPtr->src_id, &participantId);
			if (rc != ESIF_OK) {
				ESIF_TRACE_ERROR("Invalid event source received");
				goto exit;
			}
		}

		if (ESIF_EVENT_PARTICIPANT_UNREGISTER == eventHdrPtr->type) {

			ESIF_TRACE_INFO("Destroy Lower Participant: %d\n", participantId);
			EsifUpManagerUnregisterParticipant(eParticipantOriginLF, &participantId);

		} else {
			// Best Effort Delivery
			EsifAppsEvent(participantId, eventHdrPtr->dst_domain_id, eventHdrPtr->type, &void_data);
		}
	}
exit:
	(0);
}


// Event Worker Thread
// Processes ESIF Events
void *esif_event_worker_thread(void *ptr)
{
	int rc = 0;
	fd_set rfds = {0};
	struct timeval tv = {0};

	UNREFERENCED_PARAMETER(ptr);
	ESIF_TRACE_ENTRY_INFO();
	CMD_OUT("Start ESIF Upper Framework Shell\n");

	// Connect To Kernel IPC with infinite timeout
	ipc_autoconnect(0);

	// Run Until Told To Quit
	while (!g_quit) {
		if (g_ipc_handle == ESIF_INVALID_HANDLE) {
			break;
		}
		FD_ZERO(&rfds);
#ifdef ESIF_ATTR_OS_WINDOWS
		FD_SET((SOCKET)g_ipc_handle, &rfds);
#else
		FD_SET((u32)g_ipc_handle, &rfds);
#endif
		tv.tv_sec  = 0;
		tv.tv_usec = 50000;	/* 50 msec */

#ifdef ESIF_ATTR_OS_LINUX
		rc = select(g_ipc_handle + 1, &rfds, NULL, NULL, &tv);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
		// Windows does not support select/poll so we simulate here
		Sleep(50);
		rc = 1;
#endif
		if (rc > 0) {
			esif_dispatch_event();
		}
	}

	if (g_ipc_handle != ESIF_INVALID_HANDLE) {
		ipc_disconnect();
	}
	g_quit2 = 1;
	ESIF_TRACE_EXIT_INFO();
	return 0;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
