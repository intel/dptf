/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#define MAX_EVENT_TYPES 64
#define EVENT_MGR_DOMAIN_D0 '0D'
#define EVENT_MGR_DOMAIN_NA 'NA'
 /*
  * Used to match any participant. (Useful for registration internally to ESIF
  * before participants are available) Only the event type will be available in
  * the callback of an event registered using this option.
  * Note:  Can only be used when registering 'by type'
  */
#define EVENT_MGR_MATCH_ANY 0xFF

#define ESIF_UF_EVENT_QUEUE_SIZE 0xFFFFFFFF
#define ESIF_UF_EVENT_QUEUE_NAME "UfQueue"
#define ESIF_UF_EVENT_QUEUE_TIMEOUT 0 /* No timeout */

#ifdef ESIF_ATTR_OS_WINDOWS
#include "win\dppe.h"
#include "win\cem_csensormanager.h"

#define register_for_power_notification(guid_ptr) register_for_power_notification_win(guid_ptr)
#define unregister_power_notification(guid_ptr) unregister_power_notification_win(guid_ptr)

#define register_for_system_metrics_notification(guid_ptr) register_for_system_metrics_notification_win(guid_ptr)
#define esif_register_sensors(eventType) esif_register_sensors_win(eventType)
#define esif_unregister_sensors(eventType) esif_unregister_sensors_win(eventType)

#else /* NOT ESIF_ATTR_OS_WINDOWS */

#define register_for_power_notification(guid_ptr) (ESIF_E_ACTION_NOT_IMPLEMENTED)
#define unregister_power_notification(guid_ptr) (ESIF_E_ACTION_NOT_IMPLEMENTED)

#define register_for_system_metrics_notification(guid_ptr) (ESIF_E_ACTION_NOT_IMPLEMENTED)
#define esif_register_sensors(eventType) (ESIF_E_ACTION_NOT_IMPLEMENTED)
#define esif_unregister_sensors(eventType) (ESIF_E_ACTION_NOT_IMPLEMENTED)

#endif

typedef eEsifError (ESIF_CALLCONV * EVENT_OBSERVER_CALLBACK)(
	void *contextPtr,
	UInt8 participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);

typedef struct EventMgrEntry_s {
	UInt8 participantId;				/* UF Participant ID */
	UInt16 domainId;					/* Domain ID - '0D'*/
	EsifFpcEvent fpcEvent;				/* Event definition from the DSP */
	EVENT_OBSERVER_CALLBACK callback;	/* Callback routine - Also used to uniquely identify an event observer when unregistering */
	void *contextPtr;					/* This is normally expected to be a pointer to the observing event object.
										 * Expected to act as a context for the callback, an event observer identifier,
										 * and to help uniquely identify an event observer while unregistering.  
										 */
	atomic_t refCount;					/* Reference count */
} EventMgrEntry, *EventMgrEntryPtr;

typedef struct EsifEventMgr_s {
	EsifLinkListPtr observerLists[MAX_EVENT_TYPES];
	esif_ccb_lock_t listLock;

	EsifLinkListPtr garbageList;

	EsifQueuePtr eventQueuePtr;
	esif_ccb_sem_t eventQueueDoorbell;
	Bool eventQueueExitFlag;

	esif_thread_t eventQueueThread;

}EsifEventMgr, *EsifEventMgrPtr;

typedef struct EsifEventQueueItem_s {
	UInt8 participantId;
	UInt16 domainId;
	eEsifEventType eventType;
	EsifData eventData;
}EsifEventQueueItem, *EsifEventQueueItemPtr;


#ifdef __cplusplus
extern "C" {
#endif

eEsifError EsifEventMgr_Init();
eEsifError EsifEventMgr_Exit();

eEsifError ESIF_CALLCONV EsifEventMgr_RegisterEventByGuid(
	esif_guid_t *guidPtr,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
	);

eEsifError ESIF_CALLCONV EsifEventMgr_UnregisterEventByGuid(
	esif_guid_t *guidPtr,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
	);

eEsifError ESIF_CALLCONV EsifEventMgr_RegisterEventByType(
	eEsifEventType eventType,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
	);

eEsifError ESIF_CALLCONV EsifEventMgr_UnregisterEventByType(
	eEsifEventType eventType,
	UInt8 participantId,
	UInt16 domainId,
	EVENT_OBSERVER_CALLBACK eventCallback,
	void *contextPtr
	);

eEsifError ESIF_CALLCONV EsifEventMgr_SignalEvent(
	UInt8 participantId,
	UInt16 domainId,
	eEsifEventType eventType,
	EsifDataPtr eventData
	);

UInt64 EsifEventMgr_GetEventMask(
	void *key,
	UInt8 participantId,
	UInt16 domainId
	);

#ifdef __cplusplus
}
#endif

#endif /* _ESIF_UF_EVENTMGR_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
