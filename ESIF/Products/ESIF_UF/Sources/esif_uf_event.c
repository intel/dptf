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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_EVENT

#include "esif_uf.h"	/* Upper Framework */
#include "esif_uf_ipc.h"
#include "esif_uf_primitive.h"
#include "esif_uf_eventmgr.h"
#include "esif_ccb_socket.h"

#ifdef ESIF_ATTR_OS_WINDOWS
#include <synchapi.h>
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

void EsifEvent_GetAndSignalIpcEvent();
static eEsifError EsifEvent_InitEventWorkerThread();
static void EsifEvent_CleanupEventWorkerThread();

/*
* Friend function:  Target is determined only once removed from queue and all
* others before it have been processed
*/
eEsifError ESIF_CALLCONV EsifEventMgr_SignalLfEvent(
	esif_handle_t targetId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventData
);


extern int g_quit;
extern esif_os_handle_t g_ipc_handle;
int g_eventThreadExit = 0;

#ifdef ESIF_ATTR_OS_WINDOWS

BOOL SendIpcIoctl(
	DWORD ioControlCode,
	LPVOID inBufferPtr,
	DWORD inBufferSize,
	LPVOID outBufferPtr,
	DWORD outBufferSize,
	LPDWORD bytesReturnedPtr
	);

static eEsifError EsifEvent_InitEventObjects();
static eEsifError EsifEvent_StartEventIpc();
static eEsifError EsifEvent_HandleLfExit();

HANDLE g_hEventDoorbell = NULL;
HANDLE g_hEventLfExit = NULL;

#endif


// Dispatch An Event
void EsifEvent_GetAndSignalIpcEvent()
{
#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	// NOOP
#else
	int r_bytes     = 0;
	u32 data_len    = 1024;	/* TODO: Change from "magic number" */
	u32 event_len   = 0;
	enum esif_rc rc = ESIF_OK;
	struct esif_ipc *ipc_ptr = NULL;
	struct esif_ipc_event *event_ptr = NULL;

	ipc_ptr = esif_ipc_alloc_event(&event_ptr,
		ESIF_DATA_VOID, /* The type is changed to the actual event data type.*/
		data_len);
	if (NULL == ipc_ptr) {
		ESIF_TRACE_ERROR("Fail to allocate esif_ipc\n");
		return;
	}
	event_len = ipc_ptr->data_len;

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
	if ((ESIF_OK == rc) && (ipc_ptr->data_len < event_len)) {
		r_bytes = ipc_ptr->data_len;
	}
#endif

	// Have Event?
	if (r_bytes > 0) {
		ESIF_TRACE_DEBUG("IPC version=%d, type=%d, len = %d, data_len=%d\n",
			ipc_ptr->version,
			ipc_ptr->type,
			r_bytes,
			ipc_ptr->data_len);

		if (event_ptr->data_len <= data_len) {
			EsifEvent_SignalIpcEvent(event_ptr);
		}
	}
	esif_ipc_free(ipc_ptr);
#endif
}


void EsifEvent_SignalIpcEvent(struct esif_ipc_event *eventHdrPtr)
{
#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
#else
	EsifData eventData = {ESIF_DATA_VOID, NULL, 0, 0};
	char domainStr[8] = "";
	esif_ccb_time_t now;

	UNREFERENCED_PARAMETER(domainStr);

	if (eventHdrPtr->version != ESIF_EVENT_VERSION) {
		ESIF_TRACE_ERROR("Unsupported event version received\n");
		goto exit;
	}

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
		"Data Type:   %s\n"
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
		esif_primitive_domain_str(eventHdrPtr->dst_domain_id, domainStr, 8),
		eventHdrPtr->dst_domain_id,
		esif_data_type_str(eventHdrPtr->data_type),
		eventHdrPtr->data_len);

	if (eventHdrPtr->data_len > 0) {
		eventData.buf_ptr = eventHdrPtr + 1;
		eventData.type = eventHdrPtr->data_type;
		eventData.buf_len = eventHdrPtr->data_len;
		eventData.data_len = eventHdrPtr->data_len;
	}

	/*
	* At this time, the UF if the only valid destination
	*/
	if (eventHdrPtr->dst_id != ESIF_INSTANCE_UF) {
		goto exit;
	}

	// Best Effort Delivery
	EsifEventMgr_SignalLfEvent(eventHdrPtr->src_id, eventHdrPtr->dst_domain_id, eventHdrPtr->type, &eventData);
exit:
	return;
#endif
}


// Event Worker Thread
// Processes ESIF Events
void *esif_event_worker_thread(void *ptr)
{
	int rc = 0;
	eEsifError esifStatus = ESIF_OK;
	fd_set rfds = {0};
	struct timeval tv = {0};
#ifdef ESIF_ATTR_OS_WINDOWS
	DWORD waitStatus = 0;
	HANDLE waitHandles[2] = { 0 };
#endif

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
	UNREFERENCED_PARAMETER(rc);
	UNREFERENCED_PARAMETER(rfds);
	UNREFERENCED_PARAMETER(tv);
#endif
	UNREFERENCED_PARAMETER(ptr);
	ESIF_TRACE_ENTRY_INFO();
	CMD_OUT("Start IPF Event Thread\n");

	g_eventThreadExit = 0;

	esifStatus = EsifEvent_InitEventWorkerThread();
	if (ESIF_OK == esifStatus) {

		// Run Until Told To Quit
		while (!g_quit && !g_eventThreadExit) {
	#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
			esif_ccb_sleep_msec(250);
	#else /* !ESIF_FEAT_OPT_ACTION_SYSFS */

			if (g_ipc_handle == INVALID_HANDLE_VALUE) {
				break;
			}
			if (rc > 0) {
				EsifEvent_GetAndSignalIpcEvent();
			}
			FD_ZERO(&rfds);
			FD_SET((esif_ccb_socket_t)g_ipc_handle, &rfds);
			tv.tv_sec  = 0;
			tv.tv_usec = 50000;	/* 50 msec */

	#ifdef ESIF_ATTR_OS_LINUX
			rc = select(g_ipc_handle + 1, &rfds, NULL, NULL, &tv);
	#endif
		
	#ifdef ESIF_ATTR_OS_WINDOWS

			waitHandles[0] = g_hEventDoorbell;
			waitHandles[1] = g_hEventLfExit;
				
			waitStatus = WaitForMultipleObjects(sizeof(waitHandles) / sizeof(*waitHandles), waitHandles, FALSE, INFINITE);
			if (waitStatus == (WAIT_OBJECT_0 + 1)) {
				esifStatus = EsifEvent_HandleLfExit();
				if (esifStatus != ESIF_OK) {
					break;
				}
			} else {
				ResetEvent(g_hEventDoorbell);				
			}
			rc = 1;
	#endif
		
	#endif /* !ESIF_FEAT_OPT_ACTION_SYSFS */
		}
	}

	EsifEvent_CleanupEventWorkerThread();

	ESIF_TRACE_EXIT_INFO();
	return 0;
}


void EsifEvent_StopEventWorkerThread()
{
	g_eventThreadExit = 1;
#ifdef ESIF_ATTR_OS_WINDOWS
	if (g_hEventDoorbell != NULL) {
		SetEvent(g_hEventDoorbell);
	}
#endif
}


#ifdef ESIF_ATTR_OS_WINDOWS

static eEsifError EsifEvent_InitEventWorkerThread()
{
	eEsifError rc = ESIF_OK;

	rc = EsifEvent_InitEventObjects();
	if (rc != ESIF_OK) {
		goto exit;
	}

	rc = EsifEvent_StartEventIpc();
	if (rc != ESIF_OK) {
		goto exit;
	}
exit:
	return rc;
}


static eEsifError EsifEvent_InitEventObjects()
{
	eEsifError rc = ESIF_OK;

	g_hEventDoorbell = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (NULL == g_hEventDoorbell) {
		ESIF_TRACE_ERROR("Unable to create event doorbell\n");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}
	ResetEvent(g_hEventDoorbell);

	g_hEventLfExit = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (NULL == g_hEventLfExit) {
		ESIF_TRACE_ERROR("Unable to create LF exit event\n");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}
	ResetEvent(g_hEventLfExit);
exit:
	if (rc != ESIF_OK) {
		if (g_hEventDoorbell != NULL) {
			CloseHandle(g_hEventDoorbell);
			g_hEventDoorbell = NULL;
		}
		if (g_hEventLfExit != NULL) {
			CloseHandle(g_hEventLfExit);
			g_hEventLfExit = NULL;
		}
	}
	return rc;
}


static eEsifError EsifEvent_StartEventIpc()
{
	eEsifError rc = ESIF_OK;
	EsifIpcEventEnable eventEnableStruct = {g_hEventDoorbell, g_hEventLfExit};

	// Connect To Kernel IPC with infinite timeout
	ipc_autoconnect(0);

	//
	// If we can't notify of events
	//
	if (!SendIpcIoctl(ESIF_IOCTL_ENABLE_EVENT_DOORBELL,
		&eventEnableStruct, sizeof(eventEnableStruct),
		NULL, 0,
		NULL)) {
		ESIF_TRACE_ERROR("Unable to enable event doorbell\n");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}
exit:
	return rc;
}


static void EsifEvent_CleanupEventWorkerThread()
{
	if (g_hEventDoorbell != NULL) {
		CloseHandle(g_hEventDoorbell);
		g_hEventDoorbell = NULL;
	}

	if (g_hEventLfExit != NULL) {
		CloseHandle(g_hEventLfExit);
		g_hEventLfExit = NULL;
	}

	if (g_ipc_handle != INVALID_HANDLE_VALUE) {
		ipc_disconnect();
	}
}


static eEsifError EsifEvent_HandleLfExit()
{
	eEsifError rc = ESIF_OK;

	EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_LF_UNLOADED, NULL);

	ResetEvent(g_hEventLfExit);
	ipc_disconnect();
	rc = EsifEvent_StartEventIpc();

	return rc;
}


#else /* !ESIF_ATTR_OS_WINDOWS */

static eEsifError EsifEvent_InitEventWorkerThread()
{
	// Connect To Kernel IPC with infinite timeout
	ipc_autoconnect(0);

	return ESIF_OK;
}


static void EsifEvent_CleanupEventWorkerThread()
{
	if (g_ipc_handle != INVALID_HANDLE_VALUE) {
		ipc_disconnect();
	}
}

#endif /* !ESIF_ATTR_OS_WINDOWS */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
