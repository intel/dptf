/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
#ifndef _ESIF_EVENT_H_
#define _ESIF_EVENT_H_

#include "esif.h"

#define ESIF_EVENT_VERSION 0x1

/* Event Priority */
enum esif_event_priority {
	ESIF_EVENT_PRIORITY_NORMAL = 0,
	ESIF_EVENT_PRIORITY_LOW,
	ESIF_EVENT_PRIORITY_HIGH
};

/* Event Priority String */
static ESIF_INLINE char *
esif_event_priority_str(enum esif_event_priority priority)
{

	#define CREATE_EVENT_PRIORITY(ep, str) case ep: str = (esif_string) #ep; break;

	esif_string str = (esif_string)ESIF_NOT_AVAILABLE;
	switch (priority) {
		CREATE_EVENT_PRIORITY(ESIF_EVENT_PRIORITY_NORMAL, str)
		CREATE_EVENT_PRIORITY(ESIF_EVENT_PRIORITY_LOW, str)
		CREATE_EVENT_PRIORITY(ESIF_EVENT_PRIORITY_HIGH, str)
	}
	return str;
}


/* Event Type */
enum esif_event_type {
	/* App Events */
	ESIF_EVENT_APP_CONNECTED_STANDBY_ENTRY      = 0, /* Enter conn standby */
	ESIF_EVENT_APP_CONNECTED_STANDBY_EXIT       = 1, /* Exit conn standby */
	ESIF_EVENT_APP_ACTIVE_RELATIONSHIP_CHANGED  = 2,
	ESIF_EVENT_APP_THERMAL_RELATIONSHIP_CHANGED = 3,
	ESIF_EVENT_APP_FOREGROUND_CHANGED    = 4, /* Foreground app changed */
	ESIF_EVENT_PARTICIPANT_SUSPEND       = 5, /* Suspend Upper Framework */
	ESIF_EVENT_PARTICIPANT_RESUME        = 6, /* Resume Upper Framework */
	ESIF_EVENT_OS_LPM_MODE_CHANGED       = 23,	/* LPM Mode Changed */
	ESIF_EVENT_APP_PASSIVE_TABLE_CHANGED = 24,	/* Passive Tbl Changed */

	/* Domain Events */
	/* Config TDP Capability changed (Configurable TDP) */
	ESIF_EVENT_DOMAIN_CTDP_CAPABILITY_CHANGED    = 7,
	ESIF_EVENT_DOMAIN_CORE_CAPABILITY_CHANGED    = 8, /* For future use */
	/* Display control upper/lower limits changed. */
	ESIF_EVENT_DOMAIN_DISPLAY_CAPABILITY_CHANGED = 9,
	/* Current Display brightness status changed due to usr or other override */
	ESIF_EVENT_DOMAIN_DISPLAY_STATUS_CHANGED     = 10,
	/* Performance Control Upper/Lower Limits Changed */
	ESIF_EVENT_DOMAIN_PERF_CAPABILITY_CHANGED    = 11,
	/* For future use */
	ESIF_EVENT_DOMAIN_PERF_CONTROL_CHANGED       = 12,
	/* Power Control Capability Changed (Participant)*/
	ESIF_EVENT_DOMAIN_POWER_CAPABILITY_CHANGED   = 13,
	/* Programmable Threshold Power Event */
	ESIF_EVENT_DOMAIN_POWER_THRESHOLD_CROSSED    = 14,
	ESIF_EVENT_DOMAIN_PRIORITY_CHANGED = 15, /* Domain priority changed. */
	/* Temperature Threshold Changed (Participant)*/
	ESIF_EVENT_DOMAIN_TEMP_THRESHOLD_CROSSED     = 16,

	/* Participant Events */
	/* Participant Specific Information Changed. */
	ESIF_EVENT_PARTICIPANT_SPEC_INFO_CHANGED = 17,

	/*
	** ESIF EVENTS
	*/
	/* Create Upper Framework (Participant) */
	ESIF_EVENT_PARTICIPANT_CREATE   = 18,
	/* Destroy Upper Framework (Participant) */
	ESIF_EVENT_PARTICIPANT_UNREGISTER  = 19, /* Unregister UF Participant */
	/* Shutdown Upper Framework (Participant) */
	ESIF_EVENT_PARTICIPANT_SHUTDOWN = 20,

	/*
	** Driver Events (To Be Mapped To ESIF Event)
	*/
	ESIF_EVENT_ACPI = 21,	/* ACPI events, ex. ART changed */

	/*
	** UUID Changed Events
	*/
	ESIF_EVENT_COOLING_MODE_POWER_LIMIT_CHANGED    = 22,
	ESIF_EVENT_PASSIVE_TABLE_CHANGED = 24,
	ESIF_EVENT_COOLING_MODE_ACOUSTIC_LIMIT_CHANGED = 25,
	ESIF_EVENT_SENSOR_ORIENTATION_CHANGED = 26,
	ESIF_EVENT_SENSOR_SPATIAL_ORIENTATION_CHANGED  = 27,
	ESIF_EVENT_SENSOR_PROXIMITY_CHANGED = 28,
	ESIF_EVENT_SYSTEM_COOLING_POLICY_CHANGED       = 29,
	ESIF_EVENT_LPM_MODE_CHANGED = 30,
	ESIF_EVENT_OS_CTDP_CAPABILITY_CHANGED = 31,
	ESIF_EVENT_RF_PROFILE_CHANGED = 32,
	ESIF_EVENT_RF_CONNECTION_STATUS_CHANGED = 33,
	ESIF_EVENT_LOG_VERBOSITY_CHANGED = 34
};

#ifdef ESIF_ATTR_USER
typedef enum esif_event_type eEsifEventType;
#endif

/* Event Type String */
static ESIF_INLINE char *esif_event_type_str(enum esif_event_type type)
{
	#define CREATE_EVENT_TYPE(et, str) case et: str = (esif_string) #et; break;

	esif_string str = (esif_string)ESIF_NOT_AVAILABLE;
	switch (type) {
		CREATE_EVENT_TYPE(ESIF_EVENT_APP_CONNECTED_STANDBY_ENTRY, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_APP_CONNECTED_STANDBY_EXIT, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_APP_THERMAL_RELATIONSHIP_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_APP_ACTIVE_RELATIONSHIP_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_APP_FOREGROUND_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_OS_LPM_MODE_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_APP_PASSIVE_TABLE_CHANGED, str)

		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_CTDP_CAPABILITY_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_CORE_CAPABILITY_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_DISPLAY_CAPABILITY_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_DISPLAY_STATUS_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_PERF_CAPABILITY_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_PERF_CONTROL_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_POWER_CAPABILITY_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_POWER_THRESHOLD_CROSSED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_PRIORITY_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_DOMAIN_TEMP_THRESHOLD_CROSSED, str)

		CREATE_EVENT_TYPE(ESIF_EVENT_PARTICIPANT_SPEC_INFO_CHANGED, str)

		CREATE_EVENT_TYPE(ESIF_EVENT_PARTICIPANT_CREATE, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_PARTICIPANT_UNREGISTER, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_PARTICIPANT_RESUME, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_PARTICIPANT_SHUTDOWN, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_PARTICIPANT_SUSPEND, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_ACPI, str)

		CREATE_EVENT_TYPE(ESIF_EVENT_COOLING_MODE_POWER_LIMIT_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_COOLING_MODE_ACOUSTIC_LIMIT_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_SENSOR_ORIENTATION_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_SENSOR_SPATIAL_ORIENTATION_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_SENSOR_PROXIMITY_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_SYSTEM_COOLING_POLICY_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_LPM_MODE_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_OS_CTDP_CAPABILITY_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_RF_PROFILE_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_RF_CONNECTION_STATUS_CHANGED, str)
		CREATE_EVENT_TYPE(ESIF_EVENT_LOG_VERBOSITY_CHANGED, str)

	}
	return str;
}


/* Event */
struct esif_event {
	u16  size;		/* Event Size Including Data */
	u8   version;		/* Event Version             */
	u64  id;		/* Event Transaction ID      */
	esif_ccb_time_t  timestamp;	/* Event Timestamp           */
	enum esif_event_type      type;		/* Event Type                */
	enum esif_event_priority  priority;	/* Event Priority            */
	u8   src;		/* Event Source              */
	u8   dst;		/* Event Destination         */
	u16  dst_domain_id;	/* Event Destination Domain ID */
	u16  data_size;		/* Event Data Size           */
};

/*
 * Allocate Event
 * parameters:
 * type     - Event Type
 * size     - Event Size Not Including Header
 * priority - Event Priority For Queuing
 * src      - Source Of Event
 * dst      - Destination Of Event
 * data     - Data To Copy To Event If Any.
 *
 * returns:
 * esif_event or NULL;
 */
struct esif_event *esif_event_allocate(const enum esif_event_type type,
				       const u16 size,
				       const enum esif_event_priority priority,
				       const u8 src,
				       const u8 dst,
				       const u16 dst_domain_id,
				       const void *data_ptr);


void esif_event_free(const struct esif_event *event_ptr);

struct esif_ipc;
struct esif_ipc *esif_event_queue_pull(void);
enum esif_rc esif_event_queue_push(struct esif_ipc *ipc_ptr);
enum esif_rc esif_event_queue_requeue(struct esif_ipc *ipc_ptr);

u32 esif_event_queue_size(void);

enum esif_rc esif_event_init(void);
void esif_event_exit(void);

struct esif_ipc_event_header;
void EsifEventProcess(struct esif_ipc_event_header *eventPtr);

#endif /* _ESIF_EVENT_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
