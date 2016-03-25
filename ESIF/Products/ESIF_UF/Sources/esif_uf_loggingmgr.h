/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#pragma once
#include "esif_pm.h"
#include "esif_uf_primitive.h"
#include "esif_uf_ccb_timedwait.h"
#include "esif_uf_eventmgr.h"
#include "esif_uf_ccb_thermalapi.h"
#include "esif_uf_log.h"
#include "esif_uf_trace.h"
#include "esif_uf_ccb_logging_listener.h"

#define MAX_LOG_DATA	(24 * 1024)

#define DEFAULT_LOG_INTERVAL            2000      /* in ms*/
#define MIN_LOG_INTERVAL                1500      /* in ms*/
#define MIN_LOG_INTERVAL_ADJUSTED       1         /* minimum granularity for Log interval*/
#define PARTICITPANTLOG_CMD_INDEX       1         /* command index in input parameters*/
#define PARTICITPANTLOG_SUB_CMD_INDEX   2         /* sub command index in input parameters*/
#define START_CMD_TRIPLET               3         /* pid/did/cap mask triplet*/
#define DEFAULT_SCHEDULE_DELAY_INTERVAL 5000      /* in ms*/
#define MAX_COMMAND_ARGUMENTS           32        /* maximum number of arguments supported*/

#define PARTICIPANTLOG_CMD_START_STR        "start"
#define PARTICIPANTLOG_CMD_STOP_STR         "stop"
#define PARTICIPANTLOG_CMD_ROUTE_STR        "route"
#define PARTICIPANTLOG_CMD_INTERVAL_STR     "interval"
#define PARTICIPANTLOG_CMD_SCHEDULE_STR     "schedule"

#define ESIF_INVALID_DATA        0xFFFFFFFF
#define CONTROL_CAPABIILTY_MASK  0xFFFFFC7F
#define STATUS_CAPABIILTY_MASK   0x00000380

#define INVALID_TEMP_VALUE        255
#define MAX_DOMAIN_ID_LENGTH      2
#define ESIF_DOMAIN_IDENT_CHAR_D  'D'
#define ESIF_DOMAIN_IDENT_CHAR_d  'd'
#define ESIF_NUMERIC_IDENT_CHAR_0 '0'
#define ESIF_HEX_IDENT_CHAR_X     'X'
#define ESIF_HEX_IDENT_CHAR_x     'x'
#define BASE_HEX				  16
#define TIME_BASE_YEAR            1900

typedef struct EsifParticipantLogData_s {
	EsifLinkListPtr list;		/*List to maintain the complete list of Capability that we are tracking for logging and OS Notification*/
	esif_ccb_lock_t listLock;	/* Lock to protect the participant data list*/
}EsifParticipantLogData, *EsifParticipantLogDataPtr;

typedef struct EsifPollingThread_s{
	UInt16 interval;	            /* to store the polling interval time*/
	esif_thread_t thread;           /* to store the handle to polling thread*/
	esif_ccb_event_t pollStopEvent; /* event to stop the polling thread*/
} EsifPollingThread, *EsifPollingThreadPtr;

typedef struct EsifCommandInfo_s{
	UInt32 participantId;
	UInt32 domainId;
	UInt32 capabilityMask;
} EsifCommandInfo, *EsifCommandInfoPtr;

typedef struct EsifParticipantLogScheduler_s{
	esif_ccb_timer_t *scheduleTimer;        /* timer to schedule logging*/
	UInt32 delay;                           /* delay in ms*/
} EsifParticipantLogScheduler, *EsifParticipantLogSchedulerPtr;

typedef struct EsifLoggingManager_s {
	Bool isInitialized;
	EsifParticipantLogData participantLogData; /*Pointer to the Data structure which maintains the list of participant Data*/
	EsifPollingThread pollingThread;
	EsifParticipantLogScheduler logScheduler;
	Bool isLogHeader;
	Bool isLogStarted;
	Bool isLogStopped;
	Bool isLogSuspended;
	Bool isDefaultFile;
	UInt32 listenersMask;
	char *argv[MAX_COMMAND_ARGUMENTS];
	int argc;
	EsifCommandInfoPtr commandInfo;
	int commandInfoCount;
	char *logData;
} EsifLoggingManager, *EsifLoggingManagerPtr;

typedef enum EsifDataListenerType_e {
	ESIF_LISTENER_EVENTLOG = 0,	// Overrides System Event Logger (Windows=EventLog, Linux=syslog)
	ESIF_LISTENER_DEBUGGER = 1,	// Overrides System Debug Logger (Windows=OutputDebugString, Linux=syslog)
	ESIF_LISTENER_LOGFILE = 2, // Logs the data in the output file
	ESIF_LISTENER_CONSOLE = 3, // Console Output
	ESIF_LISTENER_MAX = 4,
} EsifDataListenerType;

#define ESIF_LISTENER_EVENTLOG_MASK  0x00000001
#define ESIF_LISTENER_DEBUGGER_MASK  0x00000002
#define ESIF_LISTENER_LOGFILE_MASK   0x00000004
#define ESIF_LISTENER_CONSOLE_MASK   0x00000008
#define ESIF_LISTENER_ALL_MASK       (ESIF_LISTENER_EVENTLOG_MASK | \
									  ESIF_LISTENER_DEBUGGER_MASK | \
									  ESIF_LISTENER_LOGFILE_MASK  | \
									  ESIF_LISTENER_CONSOLE_MASK)

#define ESIF_LISTENER_EVENTLOG_STR   "eventviewer"
#define ESIF_LISTENER_DEBUGGER_STR   "debugger"
#define ESIF_LISTENER_LOGFILE_STR    "file"
#define ESIF_LISTENER_CONSOLE_STR    "console"
#define ESIF_LISTENER_ALL_STR        "all"

typedef UInt32 esif_listenermask_t;

typedef enum EsifParticipantLogDataState_e {
	ESIF_DATA_CREATED = 0,	   /* Data structure Node is created , but not yet initialized with the data*/
	ESIF_DATA_INITIALIZED = 1, /* Data structure initialize with the initial Data from DPTF */
} EsifParticipantLogDataState;

typedef struct EsifParticipantLogDataNode_s {
	EsifParticipantLogDataState state;  /*Node State */
	UInt32 participantId;	            /*Unique Participant Id*/
	UInt32 domainId;			        /*Unique Domain Id*/
	EsifCapabilityData capabilityData;  /* structure to store the capability data*/
	esif_ccb_lock_t capabilityDataLock; /* semaphore to protect the capability structure for synchronization*/	
}EsifParticipantLogDataNode, *EsifParticipantLogDataNodePtr;

typedef eEsifError(ESIF_CALLCONV *EsifRecvEventFunction)(
	const void *esifHandle,         /* ESIF provided context handle */
	const void *appHandle,		    /* Allocated handle for application */
	const void *participantHandle,	/* Optional Participant ESIF_NO_HANDLE indicates App Level Event */
	const void *domainHandle,	    /* Optional Domain ESIF_NO_HANDLE indicates non-domain Level Event */
	const EsifDataPtr eventData,	/* Data included with the event if any MAY Be NULL */
	const EsifDataPtr eventGuid	    /* Event GUID */
	);

#ifdef __cplusplus
extern "C" {
#endif

char *EsifShellCmd_ParticipantLog(EsifShellCmdPtr shell);
void EsifLogMgr_Exit();

eEsifError EsifUf_GetNextCapability(
	esif_flags_t capabilitymask,
	UInt32 startIndex,
	UInt32 *capabilityId
	);
#ifdef __cplusplus
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

