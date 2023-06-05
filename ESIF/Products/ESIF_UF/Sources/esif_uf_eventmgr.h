/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/
#ifndef _ESIF_UF_EVENTMGR_H_
#define _ESIF_UF_EVENTMGR_H_

#include "esif.h"
#include "esif_uf_fpc.h"
#include "esif_link_list.h"
#include "esif_queue.h"
#include "esif_ccb_thread.h"

#define EVENT_MGR_DOMAIN_D0 '0D'
//
// Use EVENT_MGR_DOMAIN_NA for sending events where the domain ignored;
// but use EVENT_MGR_MATCH_ANY_DOMAIN to register for events where the
// domain is to always be ignored.
//
#define EVENT_MGR_DOMAIN_NA 'NA'
#define EVENT_MGR_MATCH_ANY_DOMAIN 0xFF

 /*
  * Used to match any participant. (Useful for registration internally to ESIF
  * before participants are available) Only the event type will be available in
  * the callback of an event registered using this option.
  * Note:  Can only be used when registering 'by type'
  */
#define EVENT_MGR_MATCH_ANY ESIF_HANDLE_MATCH_ANY_EVENT


#define ESIF_UF_EVENT_QUEUE_SIZE 0xFFFFFFFF
#define ESIF_UF_EVENT_QUEUE_NAME "UfQueue"
#define ESIF_UF_EVENT_QUEUE_TIMEOUT ESIF_QUEUE_TIMEOUT_INFINITE /* No timeout */
#define EVENT_MGR_SYNCHRONOUS_EVENT_TIME_MAX 10000 /* ms */

#include "lin/esif_uf_sensor_manager_os_lin.h"

#define register_for_power_notification(guid_ptr) register_for_system_metric_notification_lin(guid_ptr)
#define unregister_power_notification(guid_ptr) (ESIF_E_NOT_IMPLEMENTED)

#define register_for_system_metrics_notification(guid_ptr) register_for_system_metric_notification_lin(guid_ptr)
#define unregister_system_metrics_notification(guid_ptr) (ESIF_E_NOT_IMPLEMENTED)

#define esif_enable_code_event(eventType) enable_code_event_lin(eventType)
#define esif_disable_code_event(eventType) disable_code_event_lin(eventType)

#define EsifEventMgr_SendInitialEvent(participantId, domainId, eventType) (ESIF_E_NOT_IMPLEMENTED)

typedef eEsifError (ESIF_CALLCONV * EVENT_OBSERVER_CALLBACK)(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);



#pragma pack(push, 1)

// Used for events sent via IPC
typedef struct EsifEventParams_s {
	esif_handle_t participantId;
	UInt16 domainId;
	eEsifEventType eventType;
	enum esif_data_type dataType;
	UInt32 dataLen;
	// If applicable; data follows at this point
} EsifEventParams, *EsifEventParamsPtr;

typedef struct UfEventIterator_s {
	UInt32 marker;
	eEsifEventType eventType;
	size_t index;
} UfEventIterator, *UfEventIteratorPtr;

typedef struct EventMgr_IteratorData_s {
	eEsifEventType eventType;
	esif_handle_t participantId;		/* UF Participant ID */
	UInt16 domainId;					/* Domain ID - '0D'*/
	EVENT_OBSERVER_CALLBACK callback;	/* Callback routine - Also used to uniquely identify an event observer */
	esif_context_t context;				/* For applications, this is the app handle */
} EventMgr_IteratorData, *EventMgr_IteratorDataPtr;


#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

eEsifError EsifEventMgr_Init(void);
void EsifEventMgr_Exit(void);
eEsifError EsifEventMgr_Start(void);
void EsifEventMgr_Disable(void);

eEsifError ESIF_CALLCONV EsifEventMgr_RegisterEventByType(
	eEsifEventType eventType,
	esif_handle_t participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
	);

eEsifError ESIF_CALLCONV EsifEventMgr_UnregisterEventByType(
	eEsifEventType eventType,
	esif_handle_t participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
);

eEsifError ESIF_CALLCONV EsifEventMgr_UnregisterAllForApp(
	EVENT_OBSERVER_CALLBACK eventCallback,
	esif_context_t context
);

eEsifError ESIF_CALLCONV EsifEventMgr_SignalEvent(
	esif_handle_t participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventData
);

/*
 * Used to signal an event that will be processed synchronously for up to the
 * specified wait time
 * NOTE: Maximum allowed wait time is EVENT_MGR_SYNCHRONOUS_EVENT_TIME_MAX
 */
eEsifError ESIF_CALLCONV EsifEventMgr_SignalSynchronousEvent(
	esif_handle_t participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventData,
	esif_ccb_time_t waitTime
);

/* For simulation/shell use */
eEsifError ESIF_CALLCONV EsifEventMgr_SignalUnfilteredEvent(
	esif_handle_t participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	const EsifDataPtr eventDataPtr
);

/* For simulation use */
esif_error_t EsifEventMgr_FilterEventType(eEsifEventType eventType);
esif_error_t EsifEventMgr_UnfilterEventType(eEsifEventType eventType);
esif_error_t EsifEventMgr_UnfilterAllEventTypes();


/* For shell use */
Bool EsifEventMgr_IsEventRegistered(
	eEsifEventType eventType,
	esif_context_t key,
	esif_handle_t participantId,
	UInt16 domainId
	);

eEsifError HandlePackagedEvent(
	EsifEventParamsPtr eventParamsPtr,
	size_t dataLen
	);

/* Used with EsifEventMgr_GetNextEvent to iterate through the events present
* in the Event Manager
* Note(s):
* 1) This iteration is based on the event types and the node number for the
* linked list for that event type.  This is due to the fact that items may
* be removed, so using event entry pointers for iteration is not possible
* without reference counting, which would require added complexity which
* is not required for the targeted usage (displaying current registered events.)
* 2) There is no guarantee that all events present at the start of iteration
* will be part of the iteration if events are removed during the iteration
* 3) There is no guarantee that events added after the start of iteration
* will be part of the iteration
*/
esif_error_t EsifEventMgr_InitIterator(UfEventIteratorPtr iterPtr);

/* Use EsifEventMgr_InitIterator to initialize an iterator prior to use */
esif_error_t EsifEventMgr_GetNextEvent(
	UfEventIteratorPtr iterPtr,
	EventMgr_IteratorDataPtr dataPtr
	);

#ifdef __cplusplus
}
#endif

#endif /* _ESIF_UF_EVENTMGR_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
