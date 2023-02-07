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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_LOGGINGMGR

#include "esif_uf_ccb_system.h"
#include "esif_uf_loggingmgr.h"
#include "esif_temp.h"
#include "esif_sdk_fan.h"
#include "esif_uf_event_cache.h"


#define ESIF_INVALID_DATA        0xFFFFFFFF

// Bounds checking
#define MAX_SCHEDULER_MS	(24 * 60 * 60 * 1000)	// 24 hours; cannot exceed 2^31-1 (~24 days)

UInt32 g_statusCapability[] = {
	ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL,
	ESIF_CAPABILITY_TYPE_TEMP_STATUS,
	ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS,
	ESIF_CAPABILITY_TYPE_POWER_STATUS,
	ESIF_CAPABILITY_TYPE_UTIL_STATUS,
	ESIF_CAPABILITY_TYPE_BATTERY_STATUS,
	ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL,
	ESIF_CAPABILITY_TYPE_MANAGER,
	ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION,
	ESIF_CAPABILITY_TYPE_DYNAMIC_EPP
};

EsifLoggingManager g_loggingManager = { 0 };

static eEsifError EsifLogMgr_Init(EsifLoggingManagerPtr self);
static eEsifError EsifLogMgr_Uninit(EsifLoggingManagerPtr self);

static eEsifError EsifLogMgr_ParseCmdParticipantLog(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	);
static eEsifError EsifLogMgr_ParseCmdStart(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	);
static eEsifError EsifLogMgr_ParseCmdStop(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	);
static eEsifError EsifLogMgr_ParseCmdRoute(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	);
static eEsifError EsifLogMgr_ParseCmdInterval(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	);
static eEsifError EsifLogMgr_ParseCmdSchedule(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	);
static void *ESIF_CALLCONV EsifLogMgr_ParticipantLogWorkerThread(void *ptr);
static void EsifLogMgr_ParticipantLogWriteHeader(
	EsifLoggingManagerPtr self
);
static void EsifLogMgr_ParticipantLogWriteData(
	EsifLoggingManagerPtr self
);
static eEsifError EsifLogMgr_AddAllParticipants(EsifLoggingManagerPtr self);
static eEsifError EsifLogMgr_GetParticipantId(
	char *participantstr,
	esif_handle_t *participantId
	);
static eEsifError EsifLogMgr_GetDomainId(
	char *domainstr,
	esif_handle_t participantId,
	UInt32 *domainIdPtr
	);
static eEsifError EsifLogMgr_GetCapabilityId(
	char *domainstr,
	UInt32 *capabilityIdPtr
	);
static eEsifError EsifLogMgr_AddParticipant(
	EsifLoggingManagerPtr self,
	EsifUpPtr upPtr
	);
static eEsifError EsifLogMgr_AddDomain(
	EsifLoggingManagerPtr self,
	esif_handle_t participantId,
	UInt8 domainId,
	UInt32 capabilityMask
	);
static eEsifError EsifLogMgr_AddCapabilityMask(
	EsifLoggingManagerPtr self,
	EsifUpPtr upPtr,
	EsifUpDomainPtr domainPtr,
	UInt32 capabilityMask
	);
static eEsifError EsifLogMgr_AddCapability(
	EsifLoggingManagerPtr self,
	EsifUpPtr upPtr,
	UInt32 domainId,
	UInt32 capabilityId
	);
static void EsifLogMgr_UpdateCapabilityData(
	EsifParticipantLogDataNodePtr capabilityEntryPtr,
	EsifCapabilityDataPtr capabilityPtr
	);
static eEsifError EsifLogMgr_OpenParticipantLogFile(char *fileName);
static eEsifError ESIF_CALLCONV EsifLogMgr_EventCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);
static EsifLinkListNodePtr EsifLogMgr_GetCapabilityNodeWLock(
	EsifLoggingManagerPtr self,
	esif_handle_t participantId, 
	UInt32 domainId, 
	UInt32 capabilityId
	);
static Bool EsifLogMgr_IsStatusCapable(UInt32 capabilityId);
static eEsifError EsifLogMgr_ParticipantLogAddDataNode(
	char *logString,
	size_t dataLength,
	EsifParticipantLogDataNodePtr dataNodePtr
	);
void EsifLogMgr_ParticipantLogStart(EsifLoggingManagerPtr self);
void EsifLogMgr_ParticipantLogStop(EsifLoggingManagerPtr self);
static void EsifLogMgr_UpdateStatusCapabilityData(EsifParticipantLogDataNodePtr dataNodePtr);
static eEsifError EsifLogMgr_ParticipantLogAddHeaderData(
	char *logString,
	size_t dataLength,
	EsifCapabilityDataPtr capabilityPtr,
	EsifString participantName,
	UInt8 domainId
	);
static eEsifError EsifLogMgr_ParticipantLogAddCapabilityData(
	char *logString,
	size_t dataLength,
	EsifParticipantLogDataNodePtr dataNodePtr
	);
static void EsifLogMgr_DataLogWrite(
	EsifLoggingManagerPtr self,
	char *logstring,
	...
	);
static void EsifLogMgr_SendParticipantLogEvent(
	eEsifEventType eventType,
	esif_handle_t participantId,
	UInt16 domainId,
	UInt32 capabilityMask
	);
static eEsifError EsifLogMgr_AddParticipantDataListEntry(
	EsifLoggingManagerPtr self,
	EsifParticipantLogDataNodePtr entryPtr
	);
static void EsifLogMgr_PrintLogStatus(
	EsifLoggingManagerPtr self,
	char *outputString,
	size_t datalength
	);
static void EsifLogMgr_PrintListenerStatus(
	EsifLoggingManagerPtr self,
	char *output,
	size_t datalength
	);
static eEsifError EsifLogMgr_GetInputParameters(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell,
	const int index
	);
static Bool EsifLogMgr_IsDataAvailableForLogging(EsifLoggingManagerPtr self);
static eEsifError EsifLogMgr_StartLoggingIfRequired(EsifLoggingManagerPtr self);
static void EsifLogMgr_CleanupLoggingContext(EsifLoggingManagerPtr self);
static eEsifError EsifLogMgr_OpenRouteTargetLogFileIfRequired(EsifLoggingManagerPtr self);
static eEsifError EsifLogMgr_ValidateInputParameters(EsifLoggingManagerPtr self);
static eEsifError EsifLogMgr_EnableLoggingFromCommandInfo(EsifLoggingManagerPtr self);
static eEsifError EsifLogMgr_IntializeScheduleTimer(EsifLoggingManagerPtr self);
static void EsifLogMgr_DestroyScheduleTimer(EsifLoggingManagerPtr self);
static void EsifLogMgr_ScheduledStartThread(const void *contextPtr);
static void EsifLogMgr_DestroyParticipantLogData(EsifLoggingManagerPtr self);
static void EsifLogMgr_DestroyEntry(EsifParticipantLogDataNodePtr curEntryPtr);
static void EsifLogMgr_DestroyArgv(EsifLoggingManagerPtr self);

//
// PUBLIC INTERFACE---------------------------------------------------------------------
//
char *EsifShellCmd_ParticipantLog(EsifShellCmdPtr shell)
{
	char *output = NULL;
	eEsifError rc = ESIF_OK;

	if ((shell == NULL) ||
		(shell->outbuf == NULL)) {
		ESIF_TRACE_ERROR("Input Parameter is NULL");
		goto exit;
	}

	output = shell->outbuf;
	
	rc = EsifLogMgr_Init(&g_loggingManager);
	if (rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error in Initialization\n");
		EsifLogMgr_Exit();
		goto exit;
	}
	
	EsifLogMgr_ParseCmdParticipantLog(&g_loggingManager, shell);

exit:
	return output;
}

static eEsifError EsifLogMgr_Init(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();
	if (self == NULL) {
		ESIF_TRACE_ERROR("self is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (self->isInitialized != ESIF_FALSE) {
		//Already Initialized
		ESIF_TRACE_INFO("Logging Manager Already initialized.");
		goto exit;
	}
	esif_ccb_lock_init(&self->participantLogData.listLock);
	self->participantLogData.list = esif_link_list_create();
	if (NULL == self->participantLogData.list) {
		ESIF_TRACE_ERROR("esif_link_list_create() failed");
		rc = ESIF_E_NO_MEMORY;
		EsifLogMgr_Uninit(self);
		goto exit;
	}

	self->isInitialized = ESIF_TRUE;
	self->pollingThread.interval = DEFAULT_LOG_INTERVAL;
	self->isLogHeader = ESIF_FALSE;
	self->isLogStarted = ESIF_FALSE;
	self->isLogStopped = ESIF_FALSE;
	self->isLogSuspended = ESIF_FALSE;
	self->isDefaultFile = ESIF_TRUE;
	self->listenersMask = ESIF_LISTENER_NONE;
	self->listenerHeadersWrittenMask = ESIF_LISTENER_NONE;


	self->argc = 0;
	self->commandInfo = NULL;
	self->commandInfoCount = 0;
	self->logData = (char *)esif_ccb_malloc(MAX_LOG_DATA);
	if (NULL == self->logData) {
		ESIF_TRACE_DEBUG("Unable to allocate memory for log data");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_DTT_PARTICIPANT_CONTROL_ACTION,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_MATCH_ANY_DOMAIN,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_CREATE_COMPLETE,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND,
		ESIF_HANDLE_PRIMARY_PARTICIPANT,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME,
		ESIF_HANDLE_PRIMARY_PARTICIPANT,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

exit:
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}

void EsifLogMgr_Exit()
{
	EsifLoggingManagerPtr self = &g_loggingManager;

	/*
	 * Stop the polling thread
	 */
	EsifLogMgr_ParticipantLogStop(self);

	/*
	 * Close the file handle if the listener is log file
	 */
	if (self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) {
		EsifLogFile_Close(ESIF_LOG_PARTICIPANT);
	}
	/*
	 * Uninitialize the manager structure
	 */
	EsifLogMgr_Uninit(self);
}

void EsifLogMgr_EnableParticipant(
	EsifLoggingManagerPtr self,
	esif_handle_t participantId
	)
{
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;
	EsifUpPtr upPtr = NULL;
		
	ESIF_ASSERT(self != NULL);

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (NULL == upPtr) {
		goto exit;
	}

	if ((self->isInitialized == ESIF_TRUE) &&
		EsifLogMgr_IsDataAvailableForLogging(self)) {

		esif_ccb_write_lock(&self->participantLogData.listLock);
		/*
		 * Loop through the complete list and send the enable event for the participantId
		 */
		nodePtr = self->participantLogData.list->head_ptr;
		while (nodePtr != NULL) {
			curEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
			if ((curEntryPtr != NULL) &&
				(0 == esif_ccb_strcmp(EsifUp_GetName(upPtr), curEntryPtr->name))) {

				// Update the ID as the participant may have been removed and restored with new ID
				curEntryPtr->participantId = participantId;

				EsifLogMgr_SendParticipantLogEvent(ESIF_EVENT_DTT_PARTICIPANT_ACTIVITY_LOGGING_ENABLED,
					curEntryPtr->participantId,
					(UInt16)curEntryPtr->domainId,
					(1 << curEntryPtr->capabilityData.type)
					);
				curEntryPtr->isPresent = ESIF_TRUE;
			}
			nodePtr = nodePtr->next_ptr;
		}
		esif_ccb_write_unlock(&self->participantLogData.listLock);
	}
exit:
	EsifUp_PutRef(upPtr);
}

void EsifLogMgr_DisableParticipant(
	EsifLoggingManagerPtr self,
	esif_handle_t participantId
	)
{
	EsifLinkListNodePtr nodePtr = NULL;
	EsifLinkListNodePtr nextNodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;

	ESIF_ASSERT(self != NULL);

	if ((self->isInitialized == ESIF_TRUE) &&
		EsifLogMgr_IsDataAvailableForLogging(self)) {
		/*
		 * Loop through the complete list and send the disable event for the participantId
		 */
		esif_ccb_write_lock(&self->participantLogData.listLock);
		nodePtr = self->participantLogData.list->head_ptr;
		while (nodePtr != NULL) {
			nextNodePtr = nodePtr->next_ptr;
			curEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
			if ((curEntryPtr != NULL) &&
				(curEntryPtr->participantId == participantId)) {

				EsifLogMgr_SendParticipantLogEvent(ESIF_EVENT_DTT_PARTICIPANT_ACTIVITY_LOGGING_DISABLED,
					participantId,
					(UInt16)curEntryPtr->domainId,
					(1 << curEntryPtr->capabilityData.type)
				);
				curEntryPtr->isAcknowledged = ESIF_FALSE;
				curEntryPtr->isPresent = ESIF_FALSE;
			}
			nodePtr = nextNodePtr;
		}
		esif_ccb_write_unlock(&self->participantLogData.listLock);
	}
}

eEsifError EsifLogMgr_ParseCmdParticipantLog(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	)
{
	eEsifError rc = ESIF_OK;
	int argc = 0;
	char **argv = NULL;
	char *output = NULL;

	ESIF_TRACE_ENTRY_INFO();

	ESIF_ASSERT(shell != NULL);
	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(shell->outbuf != NULL);

	argc = shell->argc;
	argv = shell->argv;
	output = shell->outbuf;

	if (argc < (PARTICITPANTLOG_CMD_INDEX + 1)) {
		EsifLogMgr_PrintLogStatus(self, output, OUT_BUF_LEN);
		goto exit;
	}

	if (esif_ccb_stricmp(argv[PARTICITPANTLOG_CMD_INDEX], PARTICIPANTLOG_CMD_START_STR) == 0) {
		rc = EsifLogMgr_ParseCmdStart(self, shell);
	}
	else if (esif_ccb_stricmp(argv[PARTICITPANTLOG_CMD_INDEX], PARTICIPANTLOG_CMD_STOP_STR) == 0) {
		rc = EsifLogMgr_ParseCmdStop(self, shell);
	}
	else if (esif_ccb_stricmp(argv[PARTICITPANTLOG_CMD_INDEX], PARTICIPANTLOG_CMD_ROUTE_STR) == 0) {
		rc = EsifLogMgr_ParseCmdRoute(self, shell);
	}
	else if (esif_ccb_stricmp(argv[PARTICITPANTLOG_CMD_INDEX], PARTICIPANTLOG_CMD_INTERVAL_STR) == 0) {
		rc = EsifLogMgr_ParseCmdInterval(self, shell);
	}
	else if (esif_ccb_stricmp(argv[PARTICITPANTLOG_CMD_INDEX], PARTICIPANTLOG_CMD_SCHEDULE_STR) == 0) {
		rc = EsifLogMgr_ParseCmdSchedule(self, shell);
	}
	else {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error:Invalid usage. See help for command usage.\n");
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}
	EsifLogMgr_PrintLogStatus(self, output, OUT_BUF_LEN);

exit:
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}

eEsifError EsifLogMgr_ParseCmdStart(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	)
{
	eEsifError rc = ESIF_OK;
	int argc = 0;
	char **argv = NULL;
	char *output = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(shell != NULL);
	ESIF_ASSERT(shell->outbuf != NULL);

	argc = shell->argc;
	argv = shell->argv;
	output = shell->outbuf;

	self->argc = 0;

	if (self->isLogStarted != ESIF_FALSE) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Logging session is already active\n");
		goto exit;
	}

	//Cleanup any active logging context here
	EsifLogMgr_CleanupLoggingContext(self);

	rc = EsifLogMgr_GetInputParameters(self, shell, PARTICITPANTLOG_SUB_CMD_INDEX);
	if (rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error invalid input arguments. See help for command usage\n");		
		goto exit;
	}

	rc = EsifLogMgr_ValidateInputParameters(self);
	if (rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error:Invalid input command. See help for command usage.\n");
		goto exit;
	}

	rc = EsifLogMgr_EnableLoggingFromCommandInfo(self);
	if (rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error while enabling logging.\n");
		goto exit;
	}
	
	//Start the logging thread only if there are any data to log
	rc = EsifLogMgr_StartLoggingIfRequired(self);
	if (rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error : Not able to start logging\n", OUT_BUF_LEN);
		goto exit;
	}

	//Open target file if required
	rc = EsifLogMgr_OpenRouteTargetLogFileIfRequired(self);
	if (rc != ESIF_OK) {
		goto exit;
	}
	esif_ccb_strcat(output, "Participant logging started\n", OUT_BUF_LEN);

exit:
	if (rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error code : %s(%d)\n", esif_rc_str(rc), rc);
		/*
		 * Stop the polling thread
		 */
		EsifLogMgr_ParticipantLogStop(self);
		esif_ccb_strcat(output, "Stopped participant logging\n", OUT_BUF_LEN);
	}
	return rc;
}

static eEsifError EsifLogMgr_ParseCmdStop(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	)
{
	eEsifError rc = ESIF_OK;
	char *output = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(shell != NULL);
	ESIF_ASSERT(shell->outbuf != NULL);

	output = shell->outbuf;

	if (self->isLogStopped != ESIF_FALSE) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "No active logging session to stop\n");
		goto exit;
	}	

	// Remove the timer
	EsifLogMgr_DestroyScheduleTimer(self);

	/*
	 * Stop the polling thread
	 */
	EsifLogMgr_ParticipantLogStop(self);

	/*
	 * Close the file handle if the listener is log file
	 */
	if (self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) {
		EsifLogFile_Close(ESIF_LOG_PARTICIPANT);
	}

	esif_ccb_strcat(output, "Stopped participant logging\n", OUT_BUF_LEN);

exit:
	return rc;
}

eEsifError EsifLogMgr_ParseCmdRoute(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	)
{
	eEsifError rc = ESIF_OK;
	int argc = 0;
	char **argv = NULL;
	char *output = NULL;
	UInt32 i = PARTICITPANTLOG_SUB_CMD_INDEX;
	UInt32 orgListenersMask = 0;
	UInt32 tempMask = 0;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(shell != NULL);
	ESIF_ASSERT(shell->outbuf != NULL);

	argc = shell->argc;
	argv = shell->argv;
	output = shell->outbuf;

	/*
	* Close the file handle if the listener is log file
	*/
	if (self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) {
		EsifLogFile_Close(ESIF_LOG_PARTICIPANT);
	}

	orgListenersMask = self->listenersMask;
	self->listenersMask = ESIF_LISTENER_NONE;

	// If no args, use default file logging
	if ((UInt32)argc <= i) {
		self->isDefaultFile = ESIF_TRUE;
		self->listenersMask = ESIF_LISTENER_LOGFILE_MASK;
	}
	// If "all" is specified, route to all and use whatever previous setting for file is already there
	else if (esif_ccb_stricmp(argv[i], ESIF_LISTENER_ALL_STR) == 0) {
		self->listenersMask = ESIF_LISTENER_ALL_MASK;
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Participant log target set to all\n");
	}
	// Loop through remaining arguments and set specific options as required
	else {
		for (; i <= ((UInt32)argc - 1); i++) {
			if (esif_ccb_stricmp(argv[i], ESIF_LISTENER_EVENTLOG_STR) == 0) {
				self->listenersMask = self->listenersMask | ESIF_LISTENER_EVENTLOG_MASK;
			}
			else if (esif_ccb_stricmp(argv[i], ESIF_LISTENER_CONSOLE_STR) == 0) {
				self->listenersMask = self->listenersMask | ESIF_LISTENER_CONSOLE_MASK;
			}
			else if (esif_ccb_stricmp(argv[i], ESIF_LISTENER_DEBUGGER_STR) == 0) {
				self->listenersMask = self->listenersMask | ESIF_LISTENER_DEBUGGER_MASK;
			}
			else if (esif_ccb_stricmp(argv[i], ESIF_LISTENER_LOGFILE_STR) == 0) {
				self->listenersMask = self->listenersMask | ESIF_LISTENER_LOGFILE_MASK;
				i++;

				// Check if file name is available as argument
				if ((UInt32)argc <= i) {
					self->isDefaultFile = ESIF_TRUE;
				}
				else {
					char *fileExtn = esif_ccb_strchr(argv[i], '.');

					//File name is given as input
					self->isDefaultFile = ESIF_FALSE;

					if (fileExtn == NULL) {
						esif_ccb_sprintf(sizeof(self->filename), self->filename, "%s.csv", argv[i]);
					}
					else {
						esif_ccb_sprintf(sizeof(self->filename), self->filename, "%s", argv[i]);
					}
				}
			}
			else {
				esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Invalid participant log target specified. See help for command line usage\n");
				rc = ESIF_E_NOT_SUPPORTED;
				goto exit;
			}
		}
	}

	if (self->isLogStarted != ESIF_FALSE) {
		// Clear the flags indicating the header has been written to the routes which have changed so that the header gets written to the new routing
		tempMask = self->listenersMask & ~orgListenersMask;
		self->listenerHeadersWrittenMask &= ~tempMask;

		//Update the header flag here if log is started already
		//otherwise not required
		self->isLogHeader = ESIF_TRUE;
	}
exit:
	return rc;
}

eEsifError EsifLogMgr_ParseCmdInterval(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	)
{
	eEsifError rc = ESIF_OK;
	int interval = 0;
	int argc = 0;
	char **argv = NULL;
	char *output = NULL;
	UInt32 i = PARTICITPANTLOG_SUB_CMD_INDEX;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(shell != NULL);
	ESIF_ASSERT(shell->outbuf != NULL);

	argc = shell->argc;
	argv = shell->argv;
	output = shell->outbuf;

	if ((UInt32)argc <= i) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "No Interval specified .Setting to default polling interval : %d ms\n", DEFAULT_LOG_INTERVAL);
		interval = DEFAULT_LOG_INTERVAL;
	} else {
		interval = esif_atoi(argv[i]);
		if ((interval < MIN_LOG_INTERVAL) || (interval > MAX_LOG_INTERVAL)) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Input interval is outside allowed range of %d to %d ms \n", MIN_LOG_INTERVAL, MAX_LOG_INTERVAL);
			rc = ESIF_E_NOT_SUPPORTED;
			goto exit;
		}
	}

	self->pollingThread.interval = (UInt16)interval;
	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Polling interval set to : %d ms \n", self->pollingThread.interval);

exit:
	return rc;
}

static eEsifError EsifLogMgr_ParseCmdSchedule(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	)
{
	eEsifError rc = ESIF_OK;
	int argc = 0;
	char **argv = NULL;
	char *output = NULL;
	UInt32 i = PARTICITPANTLOG_SUB_CMD_INDEX;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(shell != NULL);
	ESIF_ASSERT(shell->outbuf != NULL);

	argc = shell->argc;
	argv = shell->argv;
	output = shell->outbuf;

	if (self->isLogStarted != ESIF_FALSE) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Logging session is already active\n");
		goto exit;
	}

	if ((UInt32)argc <= i) {
		// No Delay specified
		// Set by default to 5000ms
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "No Interval specified .Setting to default delay interval : %d ms\n", DEFAULT_SCHEDULE_DELAY_INTERVAL);
		self->logScheduler.delay = DEFAULT_SCHEDULE_DELAY_INTERVAL;
	}
	else {
		// Delay is specified as input
		int delay = esif_atoi(argv[i]);
		self->logScheduler.delay = (delay < 0) ? 0 : (delay > MAX_SCHEDULER_MS) ? MAX_SCHEDULER_MS : (UInt32) delay;
		if (self->logScheduler.delay < MIN_LOG_INTERVAL) {
			esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Input interval value is less than minimum supported value %d ms \n", MIN_LOG_INTERVAL);
			rc = ESIF_E_NOT_SUPPORTED;
			goto exit;
		}
		i++;
	}

	//Cleanup any active logging context here
	EsifLogMgr_CleanupLoggingContext(self);

	rc = EsifLogMgr_GetInputParameters(self, shell, i);
	if (rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error invalid input arguments. See help for command usage\n");
		goto exit;
	}

	rc = EsifLogMgr_IntializeScheduleTimer(self);
	if (rc != ESIF_OK) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error initializing timer\n");
		goto exit;
	}

	esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Participant logging scheduled for start in %d ms\n", self->logScheduler.delay);

exit:
	return rc;
}

static eEsifError EsifLogMgr_GetInputParameters(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell,
	const int index
	)
{
	eEsifError rc = ESIF_OK;
	int argc = 0;
	char **argv = NULL;
	char *output = NULL;
	UInt32 i = index;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(shell != NULL);
	ESIF_ASSERT(shell->outbuf != NULL);

	/* We should not get here unless we have at least command */
	ESIF_ASSERT(shell->argc >= (PARTICITPANTLOG_CMD_INDEX + 1));

	/* We should should have at least as many arguments as the specified index requires */
	ESIF_ASSERT((UInt32)shell->argc >= i);

	argc = shell->argc;
	argv = shell->argv;
	output = shell->outbuf;

	self->argc = 0;

	/*
	 * If there is no subcommand or subcommand is "all", set up for all
	 */
	if (((UInt32)argc < (i + 1)) || (((UInt32)argc == (i + 1)) && ((esif_ccb_stricmp(argv[i], "all") == 0)))) {

		self->argv = esif_ccb_malloc(sizeof(*self->argv));
		if (NULL == self->argv) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		self->argv[self->argc] = esif_ccb_strdup("all");
		if (self->argv[self->argc] == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		self->argc++;
		goto exit;
	}

	/*
	 * Fail if the argument set doesn't contain full triplets
	 */
	if (((argc - i) % START_CMD_TRIPLET ) != 0) {
		ESIF_TRACE_ERROR("Error:Invalid input command.");
		rc = ESIF_E_INVALID_ARGUMENT_COUNT;
		goto exit;
	}

	self->argv = esif_ccb_malloc(sizeof(*self->argv) * (argc - i));
	if (NULL == self->argv) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	for (; i < ((UInt32)argc - 1);) {
		//Participant Id
		self->argv[self->argc] = esif_ccb_strdup(argv[i]);
		if (self->argv[self->argc] == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		self->argc++;
		i++;

		//Domain Id
		self->argv[self->argc] = esif_ccb_strdup(argv[i]);
		if (self->argv[self->argc] == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		self->argc++;
		i++;

		//Capability Mask
		self->argv[self->argc] = esif_ccb_strdup(argv[i]);
		if (self->argv[self->argc] == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		self->argc++;
		i++;
	}
exit:
	if (rc != ESIF_OK) {
		EsifLogMgr_DestroyArgv(self);
	}
	return rc;
}

static eEsifError EsifLogMgr_ValidateInputParameters(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;
	UInt32 i = 0;
	UInt32 commandTripletsCount = 0;
	esif_handle_t participantId = 0;
	UInt32 domainId = 0;
	UInt32 capabilityMask = 0;

	ESIF_ASSERT(self != NULL);

	if ((self->argc != 1) &&
		((self->argc % START_CMD_TRIPLET) != 0)) {
		ESIF_TRACE_ERROR("Invalid number of input arguments");
		rc = ESIF_E_INVALID_ARGUMENT_COUNT;
		goto exit;
	}

	if (self->argc == 1) {
		//input command is all
		self->commandInfo = esif_ccb_malloc(sizeof(*self->commandInfo));
		if (NULL == self->commandInfo) {
			ESIF_TRACE_ERROR("Unable to allocate memory");
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		//set the participant id to 0xFFFFFFFF for all
		self->commandInfo[0].participantId = ESIF_INVALID_HANDLE;
		self->commandInfoCount = 1;
	}
	else {
		commandTripletsCount = self->argc / START_CMD_TRIPLET;
		self->commandInfo = esif_ccb_malloc(sizeof(*self->commandInfo) * commandTripletsCount);
		if (NULL == self->commandInfo) {
			ESIF_TRACE_ERROR("Unable to allocate memory");
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		for (i = 0; i < ((UInt32)self->argc);) {

			rc = EsifLogMgr_GetParticipantId(self->argv[i], &participantId);
			if (rc != ESIF_OK) {
				ESIF_TRACE_ERROR("Error: Participant Id is not valid");
				goto exit;
			}
			i++;

			//Get the Domain ID from the input
			rc = EsifLogMgr_GetDomainId(self->argv[i], participantId, &domainId);
			if (rc != ESIF_OK) {
				ESIF_TRACE_ERROR("Error: Domain Id is not valid");
				goto exit;
			}
			i++;

			//Get the Capability ID from the input
			rc = EsifLogMgr_GetCapabilityId(self->argv[i], &capabilityMask);
			if (rc != ESIF_OK) {
				ESIF_TRACE_ERROR("Error: Capability Id is not valid");
				goto exit;
			}
			i++;

			self->commandInfo[self->commandInfoCount].participantId = participantId;
			self->commandInfo[self->commandInfoCount].domainId = domainId;
			self->commandInfo[self->commandInfoCount].capabilityMask = capabilityMask;
			self->commandInfoCount++;
		}
	}
exit:
	if ((rc != ESIF_OK) &&
		((self->commandInfo) != NULL)) {
		esif_ccb_free(self->commandInfo);
		self->commandInfo = NULL;
		self->commandInfoCount = 0;
	}
	EsifLogMgr_DestroyArgv(self);
	return rc;
}


static void EsifLogMgr_DestroyArgv(
	EsifLoggingManagerPtr self
	)
{
	UInt32 i = 0;

	ESIF_ASSERT(self != NULL);

	if (NULL == self->argv) {
		goto exit;
	}

	for (i = 0; i < self->argc; i++) {
		if (self->argv[i] != NULL) {
			esif_ccb_free(self->argv[i]);
			self->argv[i] = NULL;
		}
	}
exit:
	esif_ccb_free(self->argv);
	self->argv = NULL;
	self->argc = 0;
	return;
}


static eEsifError EsifLogMgr_EnableLoggingFromCommandInfo(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;
	UInt32 i = 0;
	EsifUpPtr upPtr = NULL;

	ESIF_ASSERT(self != NULL);

	if ((self->commandInfo == NULL) ||
		(self->commandInfoCount == 0)) {
		ESIF_TRACE_ERROR("Error Invalid input parameters");
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	if ((self->commandInfoCount == 1) &&
		(self->commandInfo[0].participantId == ESIF_INVALID_HANDLE)) {
		rc = EsifLogMgr_AddAllParticipants(self);
		if (rc != ESIF_OK) {
			goto exit;
		}
	}
	else {
		for (i = 0; i < ((UInt32)self->commandInfoCount); i++) {
			//Domain ID is mentioned
			if (self->commandInfo[i].domainId != ESIF_INVALID_DATA) {
				rc = EsifLogMgr_AddDomain(self,
					self->commandInfo[i].participantId,
					(UInt8)self->commandInfo[i].domainId,
					self->commandInfo[i].capabilityMask
					);
				if (rc != ESIF_OK) {
					goto exit;
				}
			}
			//Enable for all the domains
			else {
				UInt32 domainId = 0;
				UInt32 domainCount = 0;

				upPtr = EsifUpPm_GetAvailableParticipantByInstance(self->commandInfo[i].participantId);
				if (upPtr == NULL) {
					ESIF_TRACE_ERROR("EsifUpPm_GetAvailableParticipantByInstance() failed");
					rc = ESIF_E_NOT_SUPPORTED;
					goto exit;
				}

				domainCount = EsifUp_GetDomainCount(upPtr);
				for (domainId = 0; domainId < domainCount; domainId++) {
					rc = EsifLogMgr_AddDomain(self,
						self->commandInfo[i].participantId,
						(UInt8)domainId,
						self->commandInfo[i].capabilityMask
						);
					if (rc != ESIF_OK) {
						ESIF_TRACE_ERROR("EsifLogMgr_AddDomain() failed");
						goto exit;
					}
				}
				EsifUp_PutRef(upPtr);
				upPtr = NULL;
			}
		}
	}
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	//Done with the commandInfo data
	//free the commandinfo pointer here
	if (self->commandInfo != NULL) {
		esif_ccb_free(self->commandInfo);
		self->commandInfo = NULL;
		self->commandInfoCount = 0;
	}
	return rc;
}

static eEsifError ESIF_CALLCONV EsifLogMgr_EventCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifCapabilityDataPtr capabilityDataPtr = NULL;
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr capabilityEntryPtr = NULL;
	EsifLoggingManagerPtr self = NULL;

	UNREFERENCED_PARAMETER(domainId);

	if ((NULL == fpcEventPtr) ||
		(0 == context)) {
		ESIF_TRACE_ERROR("input parameter is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	self = (EsifLoggingManagerPtr)esif_ccb_context2ptr(context);

	switch (fpcEventPtr->esif_event) {
	case ESIF_EVENT_DTT_PARTICIPANT_CONTROL_ACTION:

		if (NULL == eventDataPtr) {
			ESIF_TRACE_ERROR("eventDataPtr is NULL");
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		if (eventDataPtr->data_len < sizeof(*capabilityDataPtr)) {
			ESIF_TRACE_ERROR("Invalid Event Data Received");
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}

		capabilityDataPtr = (EsifCapabilityDataPtr)eventDataPtr->buf_ptr;
		if (NULL == capabilityDataPtr) {
			ESIF_TRACE_ERROR("capabilityDataPtr is NULL");
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}

		esif_ccb_write_lock(&self->participantLogData.listLock);
		nodePtr = EsifLogMgr_GetCapabilityNodeWLock(self, participantId, domainId, capabilityDataPtr->type);
		if (nodePtr == NULL) {
			ESIF_TRACE_DEBUG("Control action for untracked entry (Participant " ESIF_HANDLE_FMT " Capability %d)", esif_ccb_handle2llu(participantId), capabilityDataPtr->type);
			rc = ESIF_E_NOT_SUPPORTED;
			esif_ccb_write_unlock(&self->participantLogData.listLock);
			goto exit;
		}

		capabilityEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
		if (NULL == capabilityEntryPtr) {
			ESIF_TRACE_ERROR("capabilityEntryPtr is NULL");
			rc = ESIF_E_PARAMETER_IS_NULL;
			esif_ccb_write_unlock(&self->participantLogData.listLock);
			goto exit;
		}

		if (!capabilityEntryPtr->isAcknowledged) {
			capabilityEntryPtr->isAcknowledged = ESIF_TRUE;
		}
		
		EsifLogMgr_UpdateCapabilityData(capabilityEntryPtr, capabilityDataPtr);

		esif_ccb_write_unlock(&self->participantLogData.listLock);
		break;
	case ESIF_EVENT_PARTICIPANT_CREATE_COMPLETE:
	case ESIF_EVENT_PARTICIPANT_RESUME:
		EsifLogMgr_EnableParticipant(self, participantId);
		break;
	case ESIF_EVENT_PARTICIPANT_UNREGISTER:
	case ESIF_EVENT_PARTICIPANT_SUSPEND:
		EsifLogMgr_DisableParticipant(self, participantId);
		break;
	default:
		break;
	}

exit:
	return rc;
}

static eEsifError EsifLogMgr_StartLoggingIfRequired(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	//Start logging only if any data is available for logging
	if (EsifLogMgr_IsDataAvailableForLogging(self) != ESIF_FALSE) {
		EsifLogMgr_ParticipantLogStart(self);
	}
	else {
		ESIF_TRACE_ERROR("Error : No Data to Log");
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	ESIF_TRACE_INFO("Participant logging started");
exit:
	return rc;
}

static void EsifLogMgr_CleanupLoggingContext(EsifLoggingManagerPtr self)
{
	ESIF_ASSERT(self != NULL);

	// Remove the old timer if any
	EsifLogMgr_DestroyScheduleTimer(self);

	/*
	 * Stop the polling thread
	 */
	EsifLogMgr_ParticipantLogStop(self);

	//reset listener mask to ESIF_LISTENER_NONE if it was set to default file mask
	if (self->listenersMask == ESIF_LISTENER_LOGFILE_MASK) {
		self->listenersMask = ESIF_LISTENER_NONE;
	}
	self->listenerHeadersWrittenMask = ESIF_LISTENER_NONE;

	//Close the old file
	EsifLogFile_Close(ESIF_LOG_PARTICIPANT);

	//Free the input argv
	EsifLogMgr_DestroyArgv(self);

	if (self->commandInfo != NULL) {
		esif_ccb_free(self->commandInfo);
		self->commandInfo = NULL;
		self->commandInfoCount = 0;
	}
	EsifLogMgr_DestroyParticipantLogData(self);

	return;
}

void EsifLogMgr_ParticipantLogStart(EsifLoggingManagerPtr self)
{
	ESIF_ASSERT(self != NULL);

	if (self->isLogStarted == ESIF_FALSE) {
		self->isLogStarted = ESIF_TRUE;
		self->isLogStopped = ESIF_FALSE;
		if (self->isLogSuspended == ESIF_FALSE) {
			self->listenerHeadersWrittenMask = ESIF_LISTENER_NONE;
			self->isLogHeader = ESIF_TRUE;
		}
		esif_ccb_event_init(&self->pollingThread.pollStopEvent);
		esif_ccb_thread_create(&self->pollingThread.thread, EsifLogMgr_ParticipantLogWorkerThread, self);
	}

	return;
}

void EsifLogMgr_ParticipantLogStop(EsifLoggingManagerPtr self)
{
	ESIF_ASSERT(self != NULL);

	if (self->isLogStarted != ESIF_FALSE) {
		self->isLogStarted = ESIF_FALSE;
		self->isLogStopped = ESIF_TRUE;
		self->isLogHeader = ESIF_FALSE;
		esif_ccb_event_set(&self->pollingThread.pollStopEvent);
		esif_ccb_thread_join(&self->pollingThread.thread);
		esif_ccb_event_uninit(&self->pollingThread.pollStopEvent);
	}

	return;
}

static eEsifError EsifLogMgr_AddAllParticipants(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;
	UfPmIterator upIter = { 0 };
	EsifUpPtr upPtr = NULL;

	ESIF_ASSERT(self != NULL);

	rc = EsifUpPm_InitIterator(&upIter);
	if (rc == ESIF_OK) {
		rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
	
	while (ESIF_OK == rc) {
		rc = EsifLogMgr_AddParticipant(self, upPtr);
		if (rc != ESIF_OK) {
			ESIF_TRACE_ERROR("EsifLogMgr_AddParticipant() failed with status : %s (%d)", esif_rc_str(rc), rc);
			goto exit;
		}
		rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
	if (rc == ESIF_E_ITERATION_DONE) {
		rc = ESIF_OK;
	}
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}

static eEsifError EsifLogMgr_AddParticipant(
	EsifLoggingManagerPtr self,
	EsifUpPtr upPtr
	)
{
	eEsifError rc = ESIF_OK;
	esif_handle_t participantId = ESIF_INVALID_HANDLE;
	UInt16 domainCount = 0;
	UInt16 domainId = 0;

	ESIF_ASSERT(self != NULL);

	if (upPtr == NULL) {
		ESIF_TRACE_ERROR("parameter is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	participantId = EsifUp_GetInstance(upPtr);
	domainCount = EsifUp_GetDomainCount(upPtr);
	if (domainCount > ESIF_DOMAIN_MAX) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	for (domainId = 0; domainId < domainCount; domainId++) {
		//To enable for all the capability under the domain
		UInt32 capabilityMask = ESIF_INVALID_DATA;
		rc = EsifLogMgr_AddDomain(self, participantId, (UInt8)domainId, capabilityMask);
		if (rc != ESIF_OK) {
			ESIF_TRACE_ERROR("EsifLogMgr_AddDomain() failed with status : %s (%d)", esif_rc_str(rc), rc);
			goto exit;
		}
	}

exit:
	return rc;
}

static eEsifError EsifLogMgr_AddDomain(
	EsifLoggingManagerPtr self,
	esif_handle_t participantId,
	UInt8 domainId,
	UInt32 capabilityMask
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr upPtr = NULL;
	EsifUpDomainPtr domainPtr = NULL;

	ESIF_ASSERT(self != NULL);

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (upPtr == NULL) {
		ESIF_TRACE_ERROR("EsifUpPm_GetAvailableParticipantByInstance failed");
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	domainPtr = EsifUp_GetDomainByIndex(upPtr, domainId);
	if (domainPtr == NULL) {
		ESIF_TRACE_ERROR("EsifUp_GetDomainByIndex failed");
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	rc = EsifLogMgr_AddCapabilityMask(self, upPtr, domainPtr, capabilityMask);
	if (rc != ESIF_OK) {
		goto exit;
	}

	EsifLogMgr_SendParticipantLogEvent(
		ESIF_EVENT_DTT_PARTICIPANT_ACTIVITY_LOGGING_ENABLED,
		EsifUp_GetInstance(upPtr),
		EsifUp_GetDomainId(domainPtr),
		capabilityMask
		);
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}

static eEsifError EsifLogMgr_AddCapabilityMask(
	EsifLoggingManagerPtr self,
	EsifUpPtr upPtr,
	EsifUpDomainPtr domainPtr,
	UInt32 capabilityMask
	)
{
	eEsifError rc = ESIF_OK;
	UInt8 capabilityCount = 0;
	UInt32 startIndex = 0;
	UInt32 capIndex = 0;
	UInt32 domainCapabilityMask = 0;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(upPtr != NULL);
	ESIF_ASSERT(domainPtr != NULL);

	domainCapabilityMask = EsifUp_GetDomainCapabilityMask(domainPtr);
	capabilityMask = capabilityMask & domainCapabilityMask;
	if ((capabilityMask == 0) &&
		(domainCapabilityMask != 0)) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}
	capabilityCount = EsifUp_GetDomainCapabilityCount(domainPtr);

	for (capIndex = 0; capIndex < capabilityCount; capIndex++)
	{
		UInt32 capabilityId = 0;
		rc = EsifUf_GetNextCapability(domainCapabilityMask, startIndex, &capabilityId);
		if (rc != ESIF_OK) {
			goto exit;
		}

		if (((1 << capabilityId) & capabilityMask) != 0) {
			rc = EsifLogMgr_AddCapability(self, upPtr, EsifUp_GetDomainId(domainPtr), capabilityId);
			if (rc != ESIF_OK) {
				goto exit;
			}
		}
		startIndex = capabilityId + 1;
	}

exit:
	return rc;
}

static eEsifError EsifLogMgr_AddCapability(
	EsifLoggingManagerPtr self,
	EsifUpPtr upPtr,
	UInt32 domainId,
	UInt32 capabilityId
	)
{
	eEsifError rc = ESIF_OK;
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr newEntryPtr = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(upPtr != NULL);

	/*
	 * Check If a matching entry is already present
	 */
	esif_ccb_write_lock(&self->participantLogData.listLock);
	nodePtr = EsifLogMgr_GetCapabilityNodeWLock(self, EsifUp_GetInstance(upPtr), domainId, capabilityId);
	if (nodePtr != NULL) {
		/*
		 * Logging has been enabled already
		 */
		esif_ccb_write_unlock(&self->participantLogData.listLock);
		goto exit;
	}
	esif_ccb_write_unlock(&self->participantLogData.listLock);

	/*
	 * If a matching entry was not present; create a new entry
	 */
	newEntryPtr = esif_ccb_malloc(sizeof(*newEntryPtr));
	if (NULL == newEntryPtr) {
		ESIF_TRACE_ERROR("esif_ccb_malloc() failed ");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	newEntryPtr->participantId = EsifUp_GetInstance(upPtr);
	newEntryPtr->domainId = domainId;
	esif_ccb_lock_init(&newEntryPtr->capabilityDataLock);
	newEntryPtr->state = ESIF_DATA_CREATED;
	newEntryPtr->isPresent = ESIF_TRUE;
	esif_ccb_strcpy(newEntryPtr->name, EsifUp_GetName(upPtr), sizeof(newEntryPtr->name));

	newEntryPtr->capabilityData.type = capabilityId;
	newEntryPtr->capabilityData.size = sizeof(newEntryPtr->capabilityData);

	//If it is STATUS capability Update the data now itself and change the state to Initialized
	if (EsifLogMgr_IsStatusCapable(newEntryPtr->capabilityData.type)) {
		newEntryPtr->state = ESIF_DATA_INITIALIZED;
		EsifLogMgr_UpdateStatusCapabilityData(newEntryPtr);
	}

	rc = EsifLogMgr_AddParticipantDataListEntry(self, newEntryPtr);

exit:
	if ((rc != ESIF_OK) && (newEntryPtr != NULL)) {
		esif_ccb_lock_uninit(&newEntryPtr->capabilityDataLock);
		esif_ccb_free(newEntryPtr);
	}
	return rc;
}

static EsifLinkListNodePtr EsifLogMgr_GetCapabilityNodeWLock(
	EsifLoggingManagerPtr self,
	esif_handle_t participantId,
	UInt32 domainId,
	UInt32 capabilityId
	)
{
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(self->participantLogData.list != NULL);

	/*
	 * Loop through the complete list to find the if it is existing already.
	 */
	nodePtr = self->participantLogData.list->head_ptr;
	while (nodePtr != NULL) {
		curEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
		if ((curEntryPtr != NULL) &&
			(curEntryPtr->participantId == participantId) &&
			(curEntryPtr->domainId == domainId) &&
			(curEntryPtr->capabilityData.type == capabilityId)) {
			ESIF_TRACE_DEBUG("Found a matching entry participant Id : " ESIF_HANDLE_FMT " domain Id : %d capability Id : %d in the list",
				esif_ccb_handle2llu(participantId),
				domainId,
				capabilityId);
			break;
		}
		nodePtr = nodePtr->next_ptr;
	}

	return nodePtr;
}

static eEsifError EsifLogMgr_IntializeScheduleTimer(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	self->logScheduler.scheduleTimer = (esif_ccb_timer_t *)esif_ccb_malloc(sizeof(esif_ccb_timer_t));
	if (self->logScheduler.scheduleTimer == NULL) {
		ESIF_TRACE_ERROR("Error esif_ccb_malloc() failed");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	rc = esif_ccb_timer_init(self->logScheduler.scheduleTimer, (esif_ccb_timer_cb)EsifLogMgr_ScheduledStartThread, self);
	if (ESIF_OK != rc) {
		ESIF_TRACE_ERROR("Error starting timer");
		esif_ccb_free(self->logScheduler.scheduleTimer);
		self->logScheduler.scheduleTimer = NULL;
		goto exit;
	}

	rc = esif_ccb_timer_set_msec(self->logScheduler.scheduleTimer, self->logScheduler.delay);
exit:
	return rc;
}

static void EsifLogMgr_DestroyScheduleTimer(EsifLoggingManagerPtr self)
{
	ESIF_ASSERT(self != NULL);

	if (self->logScheduler.scheduleTimer != NULL) {
		esif_ccb_timer_kill(self->logScheduler.scheduleTimer);
		esif_ccb_free(self->logScheduler.scheduleTimer);
		self->logScheduler.scheduleTimer = NULL;
	}

	return;
}

static void EsifLogMgr_ScheduledStartThread(const void *contextPtr)
{
	eEsifError rc = ESIF_OK;
	EsifLoggingManagerPtr self = NULL;
	
	if (contextPtr == NULL) {
		goto exit;
	}

	self = (EsifLoggingManagerPtr)contextPtr;

	//To log errors to the target route for the schedule command
	//Open target file if required
	rc = EsifLogMgr_OpenRouteTargetLogFileIfRequired(self);
	if (rc != ESIF_OK) {
		goto exit;
	}

	rc = EsifLogMgr_ValidateInputParameters(self);
	if (rc != ESIF_OK) {
		goto exit;
	}

	rc = EsifLogMgr_EnableLoggingFromCommandInfo(self);
	if (rc != ESIF_OK) {
		goto exit;
	}

	//Start the logging thread only if there are any data to log
	rc = EsifLogMgr_StartLoggingIfRequired(self);
	if (rc != ESIF_OK) {		
		goto exit;
	}

exit:
	if (rc != ESIF_OK && self) {
		EsifLogMgr_DataLogWrite(self, "\nError code : %s(%d)", esif_rc_str(rc), rc);		
		EsifLogMgr_DataLogWrite(self, "\nStopped participant logging");
		EsifLogMgr_DataLogWrite(self, "\n");
		EsifLogMgr_ParticipantLogStop(self);

		/*
		* Close the file handle if the listener is log file
		*/
		if (self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) {
			EsifLogFile_Close(ESIF_LOG_PARTICIPANT);
		}
	}
}

static eEsifError EsifLogMgr_OpenRouteTargetLogFileIfRequired(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	//if route is not set at all
	// set route's default value to file
	if (self->listenersMask == ESIF_LISTENER_NONE) {
		self->listenersMask = ESIF_LISTENER_LOGFILE_MASK;
	}
	
	if (self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) {
		if ((self->isDefaultFile == ESIF_FALSE) && (*self->filename != '\0')) {
			//Pass input file name for creating new file
			rc = EsifLogMgr_OpenParticipantLogFile(self->filename);
			if (rc != ESIF_OK) {
				goto exit;
			}
		}
		else {
			//if it is default file then create new file every time
			//Pass NULL as file name for creating new file based on time stamp
			rc = EsifLogMgr_OpenParticipantLogFile(NULL);
			if (rc != ESIF_OK) {
				goto exit;
			}
		}
	}
exit:
	return rc;
}

static Bool EsifLogMgr_IsDataAvailableForLogging(EsifLoggingManagerPtr self)
{
	Bool isAvailable = ESIF_FALSE;

	ESIF_ASSERT(self != NULL);

	esif_ccb_write_lock(&self->participantLogData.listLock);
	if (esif_link_list_get_node_count(self->participantLogData.list) > 0) {
		isAvailable = ESIF_TRUE;
	}
	else {
		isAvailable = ESIF_FALSE;
	}
	esif_ccb_write_unlock(&self->participantLogData.listLock);

	return isAvailable;
}

static eEsifError EsifLogMgr_GetParticipantId(
	char *participantstr,
	esif_handle_t *participantIdPtr
	)
{
	eEsifError rc = ESIF_OK;
	esif_handle_t participantId = ESIF_INVALID_HANDLE;
	EsifUpPtr upPtr = NULL;

	ESIF_ASSERT(participantstr != NULL);
	ESIF_ASSERT(participantIdPtr != NULL);

	//Parse the participant and check if it is available
	if ((esif_atoi64(participantstr) > 0) ||
		(esif_ccb_strcmp(participantstr, "0") == 0)) {
		participantId = (esif_handle_t)esif_atoi64(participantstr);
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	}
	else {
		upPtr = EsifUpPm_GetAvailableParticipantByName(participantstr);
		if (upPtr != NULL) {
			participantId = EsifUp_GetInstance(upPtr);
		}
	}
	//Throw error if participant not found
	if (NULL == upPtr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		ESIF_TRACE_ERROR("Error:Participant Not Found");
		goto exit;
	}

exit:
	if (rc == ESIF_OK) {
		*participantIdPtr = participantId;
	}

	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}

static eEsifError EsifLogMgr_GetDomainId(
	char *domainstr, 
	esif_handle_t participantId,
	UInt32 *domainIdPtr
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 index = 0;
	EsifUpPtr upPtr = NULL;

	ESIF_ASSERT(domainstr != NULL);
	ESIF_ASSERT(domainIdPtr != NULL);

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (upPtr == NULL) {
		ESIF_TRACE_ERROR("EsifUpPm_GetAvailableParticipantByInstance failed");
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}
	
	//Parse the Domain Id and check if it is available
	if (esif_ccb_stricmp(domainstr, "all") == 0) {
		index = ESIF_INVALID_DATA;
	}
	else if (esif_ccb_strlen(domainstr, MAX_DOMAIN_ID_LENGTH) < MAX_DOMAIN_ID_LENGTH) {
		index = (UInt32)esif_atoi(domainstr);
		if (index >= EsifUp_GetDomainCount(upPtr)) {
			rc = ESIF_E_INVALID_DOMAIN_ID;
			ESIF_TRACE_ERROR("Error: Domain not available.");
			goto exit;
		}
	}
	else {
		if (esif_ccb_strlen(domainstr, MAX_DOMAIN_ID_LENGTH) > MAX_DOMAIN_ID_LENGTH) {
			rc = ESIF_E_INVALID_DOMAIN_ID;
			ESIF_TRACE_ERROR("Error:Invalid Domain Id Input.");
			goto exit;
		}
		else if ((domainstr[0] != ESIF_DOMAIN_IDENT_CHAR_D) &&
				 (domainstr[0] != ESIF_DOMAIN_IDENT_CHAR_d)) {
			rc = ESIF_E_INVALID_DOMAIN_ID;
			ESIF_TRACE_ERROR("Error:Invalid Domain Id Input.");
			goto exit;
		}
		
		if (esif_ccb_strlen(domainstr, MAX_DOMAIN_ID_LENGTH) != MAX_DOMAIN_ID_LENGTH) {
			index = ESIF_INVALID_DATA;
			goto exit;
		}
		index = domainstr[1] - ESIF_NUMERIC_IDENT_CHAR_0;
		if (index >= ESIF_DOMAIN_MAX) {
			index = ESIF_INVALID_DATA;
		}

		if (index >= EsifUp_GetDomainCount(upPtr)) {
			rc = ESIF_E_INVALID_DOMAIN_ID;
			ESIF_TRACE_ERROR("Error: Domain not available.");
			goto exit;
		}
	}
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}

	if (rc == ESIF_OK) {
		*domainIdPtr = index;
	}
	return rc;
}

static eEsifError EsifLogMgr_GetCapabilityId(
	char *domainstr,
	UInt32 *capabilityIdPtr
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 capabilityMask = 0;
	char *endptr = NULL;

	ESIF_ASSERT(domainstr != NULL);
	ESIF_ASSERT(capabilityIdPtr != NULL);

	if (esif_ccb_stricmp(domainstr, "all") == 0) {
		capabilityMask = ESIF_INVALID_DATA;
	}
	else if ((domainstr[0] == ESIF_NUMERIC_IDENT_CHAR_0) &&
		((domainstr[1] == ESIF_HEX_IDENT_CHAR_x) ||
		(domainstr[1] == ESIF_HEX_IDENT_CHAR_X))) {
		capabilityMask = (UInt32)strtoul(domainstr, &endptr, 0);
	}
	else {
		capabilityMask = (UInt32)strtoul(domainstr, &endptr, BASE_HEX);
	}

	if (((endptr != NULL) && (*endptr != '\0')) ||
		(capabilityMask == 0)) {
		ESIF_TRACE_ERROR("Error:Invalid input for capability Id");
		rc = ESIF_E_INVALID_CAPABILITY_MASK;
		goto exit;
	}

exit:
	if (rc == ESIF_OK) {
		*capabilityIdPtr = capabilityMask;
	}
	return rc;
}

static eEsifError EsifLogMgr_AddParticipantDataListEntry(
	EsifLoggingManagerPtr self,
	EsifParticipantLogDataNodePtr entryPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifLinkListNodePtr nodePtr = NULL;
	EsifLinkListNodePtr newNodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(entryPtr != NULL);

	/*
	 * Create a list node
	 */
	newNodePtr = esif_link_list_create_node(entryPtr);
	if (newNodePtr == NULL) {
		ESIF_TRACE_ERROR("esif_link_list_create_node() failed");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	esif_ccb_write_lock(&self->participantLogData.listLock);
	nodePtr = self->participantLogData.list->head_ptr;
	if (nodePtr == NULL) {
		// There are no nodes in the list
		// this is the First node to be inserted into the list
		esif_link_list_add_node_at_front(self->participantLogData.list, newNodePtr);
		esif_ccb_write_unlock(&self->participantLogData.listLock);
		goto exit;
	}

	/*
	 * Loop through the list to find a location to insert new node
	 */
	while (nodePtr != NULL) {
		curEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
		if (curEntryPtr == NULL) {
			rc = ESIF_E_NOT_SUPPORTED;
			esif_ccb_write_unlock(&self->participantLogData.listLock);
			goto exit;
		}
		if ((entryPtr->participantId > curEntryPtr->participantId) ||
			((entryPtr->participantId == curEntryPtr->participantId) &&
			(entryPtr->domainId > curEntryPtr->domainId))) {
			/*
			 * Search the list based on the Participant Id and Domain ID
			 */
			nodePtr = nodePtr->next_ptr;
		}
		else {
			/*
			* Found a location to insert. Break out of the loop
			*/
			break;
		}
	}

	if (nodePtr != NULL) {
		//Found a location to insert the new node in the list
		if (nodePtr->prev_ptr != NULL) {
			//not in the first node of the list
			nodePtr->prev_ptr->next_ptr = newNodePtr;
			newNodePtr->prev_ptr = nodePtr->prev_ptr;

			newNodePtr->next_ptr = nodePtr;
			nodePtr->prev_ptr = newNodePtr;
			self->participantLogData.list->nodes++;
		}
		else {
			//this is the first node in the list
			//need to update the head
			esif_link_list_add_node_at_front(self->participantLogData.list, newNodePtr);
		}
	}
	else {
		//this is last entry in the list to be added
		esif_link_list_add_node_at_back(self->participantLogData.list, newNodePtr);
	}

	esif_ccb_write_unlock(&self->participantLogData.listLock);

exit:
	if (rc != ESIF_OK) {
		esif_link_list_destroy_node(newNodePtr);
	}
	return rc;
}

static void *ESIF_CALLCONV EsifLogMgr_ParticipantLogWorkerThread(void *ptr)
{
	eEsifError rc = ESIF_OK;
	EsifLoggingManagerPtr self = NULL;

	ESIF_TRACE_ENTRY_INFO();

	if (ptr == NULL) {
		ESIF_TRACE_ERROR("input parameter is NULL");
		goto exit;
	}

	self = (EsifLoggingManagerPtr)ptr;

	//Wait for sometime initially before starting the thread for data to be initialized from DPTF
	esif_ccb_sleep_msec(MIN_LOG_INTERVAL);
	while (!self->isLogStopped) {
		esif_ccb_time_t msecStart = 0;
		esif_ccb_time_t msecStop = 0;
		esif_ccb_time_t msecDif = 0;
		UInt32 sleepMs = 0;

		/*
		 * Log to listeners only if there are any listeners available
		 */
		if (self->listenersMask != ESIF_LISTENER_NONE) {
			esif_ccb_system_time(&msecStart);
			//Header needs to be updated
			if (self->isLogHeader) {
				EsifLogMgr_ParticipantLogWriteHeader(self);
				self->listenerHeadersWrittenMask |= ESIF_LISTENER_ALL_MASK;
				self->isLogHeader = ESIF_FALSE;
			}
			EsifLogMgr_ParticipantLogWriteData(self);

			esif_ccb_system_time(&msecStop);
		}

		/* Determine how long processing actually took */
		msecDif = msecStop - msecStart;

		/* Determine next sleep time based on previous processing time */
		sleepMs = self->pollingThread.interval - ((int)msecDif);
		if ((sleepMs < MIN_LOG_INTERVAL_ADJUSTED) ||
			(msecDif > self->pollingThread.interval)) {
			sleepMs = MIN_LOG_INTERVAL_ADJUSTED;
		}

		rc = EsifTimedEventWait(&self->pollingThread.pollStopEvent, sleepMs);
		if (rc != ESIF_OK) {
			ESIF_TRACE_ERROR("Error waiting on data log event");
			goto exit;
		}
	}
exit:
	ESIF_TRACE_EXIT_INFO();
	return 0;
}

static void EsifLogMgr_ParticipantLogWriteHeader(
	EsifLoggingManagerPtr self
	)
{
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;
	esif_handle_t currentParticipantId = ESIF_INVALID_HANDLE;
	UInt32 currentDomainId = (UInt32)-1;
	UInt8 domainIndex = 0;
	size_t dataLength = MAX_LOG_DATA;
	EsifString partName = "UNK";
	EsifUpPtr upPtr = NULL;
	Bool printTimeInfo = ESIF_TRUE;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(self->logData != NULL);

	if (self->participantLogData.list == NULL) {
		ESIF_TRACE_INFO("\n list is NULL. No Data to Log");
		goto exit;
	}

	/*
	 * Loop through the complete list
	 */
	esif_ccb_read_lock(&self->participantLogData.listLock);
	nodePtr = self->participantLogData.list->head_ptr;
	while (nodePtr != NULL) {
		curEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
		if (curEntryPtr != NULL) {
			if (printTimeInfo != ESIF_FALSE) {
				esif_ccb_sprintf(dataLength, self->logData, "Date,Time,Server Msec,");
				printTimeInfo = ESIF_FALSE;
			}
			if (currentParticipantId != curEntryPtr->participantId) {
				esif_ccb_sprintf_concat(dataLength, self->logData, "Participant ID,Participant Name,Domain Id,");
			}
			else if ((currentParticipantId == curEntryPtr->participantId) &&
				(currentDomainId != curEntryPtr->domainId)) {
				esif_ccb_sprintf_concat(dataLength, self->logData, "Domain Id,");
			}

			partName = "UNK";
			upPtr = EsifUpPm_GetAvailableParticipantByInstance(curEntryPtr->participantId);
			if (upPtr != NULL) {
				partName = EsifUp_GetName(upPtr);
				EsifUp_PutRef(upPtr);
			}
			EsifDomainIdToIndex((UInt16)curEntryPtr->domainId, &domainIndex);
			esif_ccb_read_lock(&curEntryPtr->capabilityDataLock);
			EsifLogMgr_ParticipantLogAddHeaderData(self->logData, dataLength, &curEntryPtr->capabilityData, partName, domainIndex);
			esif_ccb_read_unlock(&curEntryPtr->capabilityDataLock);

			currentParticipantId = curEntryPtr->participantId;
			currentDomainId = curEntryPtr->domainId;
		}
		nodePtr = nodePtr->next_ptr;
	}
	esif_ccb_read_unlock(&self->participantLogData.listLock);

	//Print the output only if PrintTimeInfo Flag has been switched off
	if (printTimeInfo == ESIF_FALSE) {
		EsifLogMgr_DataLogWrite(self, "%s \n", self->logData);
	}
exit:
	return;
}


static void EsifLogMgr_ParticipantLogWriteData(
	EsifLoggingManagerPtr self
	)
{
	time_t now = time(NULL);
	struct tm time = { 0 };
	esif_ccb_time_t msec = 0;
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;
	esif_handle_t currentParticipantId = ESIF_INVALID_HANDLE;
	UInt32 currentDomainId = (UInt32)-1;
	UInt8 domainIndex = 0;
	size_t dataLength = MAX_LOG_DATA;
	EsifUpPtr upPtr = NULL;
	Bool printTimeInfo = ESIF_TRUE;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(self->logData != NULL);

	if (self->participantLogData.list == NULL) {
		ESIF_TRACE_INFO("\n list is NULL. No Data to Log");
		goto exit;
	}

	/*
	 * Loop through the complete list
	 */
	esif_ccb_read_lock(&self->participantLogData.listLock);
	nodePtr = self->participantLogData.list->head_ptr;
	while (nodePtr != NULL) {
		curEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
		if (curEntryPtr != NULL) {
			if (printTimeInfo != ESIF_FALSE) {
				esif_ccb_system_time(&msec);
				if (esif_ccb_localtime(&time, &now) == 0) {
					esif_ccb_sprintf(dataLength, self->logData, "%04d-%02d-%02d,%02d:%02d:%02d,%llu,",
						time.tm_year + TIME_BASE_YEAR, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, msec);
				}
				printTimeInfo = ESIF_FALSE;
			}
			/*
				* Print the participant Name and Index for every new participant Id
				*/
			EsifDomainIdToIndex((UInt16)curEntryPtr->domainId, &domainIndex);
			if (currentParticipantId != curEntryPtr->participantId) {
				upPtr = EsifUpPm_GetAvailableParticipantByInstance(curEntryPtr->participantId);
				if (upPtr != NULL) {
					if (!curEntryPtr->isAcknowledged) {
						/* Covers a race condition where the app may not know about participant at the time we tell it to enable logging */
						EsifLogMgr_SendParticipantLogEvent(ESIF_EVENT_DTT_PARTICIPANT_ACTIVITY_LOGGING_ENABLED,
							curEntryPtr->participantId,
							(UInt16)curEntryPtr->domainId,
							(1 << curEntryPtr->capabilityData.type)
						);
					}
					esif_ccb_sprintf_concat(dataLength, self->logData, "%llu,%s,%d,", esif_ccb_handle2llu(curEntryPtr->participantId), EsifUp_GetName(upPtr), domainIndex);
					EsifUp_PutRef(upPtr);
				}
				else {
					esif_ccb_sprintf_concat(dataLength, self->logData, "%llu,UNAVAIL,%d,", esif_ccb_handle2llu(curEntryPtr->participantId), domainIndex);
				}
			}
			else if ((currentParticipantId == curEntryPtr->participantId) &&
				(currentDomainId != curEntryPtr->domainId)) {
				esif_ccb_sprintf_concat(dataLength, self->logData, "%d,", domainIndex);
			}
			EsifLogMgr_ParticipantLogAddDataNode(self->logData, dataLength, curEntryPtr);

			currentParticipantId = curEntryPtr->participantId;
			currentDomainId = curEntryPtr->domainId;
		}
		nodePtr = nodePtr->next_ptr;
	}
	esif_ccb_read_unlock(&self->participantLogData.listLock);

	//Print the output only if PrintTimeInfo Flag has been switched off
	if (printTimeInfo == ESIF_FALSE) {
		EsifLogMgr_DataLogWrite(self, "%s \n", self->logData);
	}
exit:
	return;
}

static eEsifError EsifLogMgr_ParticipantLogAddHeaderData(
	char *logString,
	size_t dataLength,
	EsifCapabilityDataPtr capabilityPtr,
	EsifString participantName,
	UInt8 domainId
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(logString != NULL);
	ESIF_ASSERT(capabilityPtr != NULL);
	ESIF_ASSERT(participantName != NULL);
	ESIF_ASSERT(participantName != NULL);

	switch (capabilityPtr->type) {
	case ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, "ControlID,Speed,Min Fan Speed %%,Max Fan Speed %%,");
		break;
	case ESIF_CAPABILITY_TYPE_CORE_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, "Active Cores,Lower Limit,Upper Limit,");
		break;
	case ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, "Brightness Limit,Lower Limit,Upper Limit,");
		break;
	case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%dPriority,",participantName,domainId);
		break;
	case ESIF_CAPABILITY_TYPE_ENERGY_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_Energy Counter,%s_Instantaneous Power (mW),", participantName, participantName);
		break;
	case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%d_PState Index,%s_D%d_Lower Limit,%s_D%d_Upper Limit,", 
			participantName, 
			domainId,
			participantName, 
			domainId,
			participantName, 
			domainId
			);
		break;
	case ESIF_CAPABILITY_TYPE_POWER_CONTROL:
	{
		UInt32 powerType = 0;
		for (powerType = 0; powerType < MAX_POWER_CONTROL_TYPE; powerType++)
		{
			esif_ccb_sprintf_concat(dataLength, logString, "%s_PL%d Limit(mW),%s_PL%d Min Power Limit(mW),%s_PL%d Max Power Limit(mW),Stepsize(mW),"
				"Minimum TimeWindow(ms),Maximum TimeWindow(ms),Minimum DutyCycle,Maximum DutyCycle,",
				participantName,
				powerType + 1,
				participantName,
				powerType + 1,
				participantName,
				powerType + 1
				);
		}
		esif_ccb_sprintf_concat(dataLength, logString, "SoC Power Floor State,");
		break;
	}
	case ESIF_CAPABILITY_TYPE_POWER_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%d_Current Power(mW),%s_D%d_Current Power Sent To Filter(mW),"
			"%s_D%d_Power Calculated By Filter(mW),",
			participantName, domainId,
			participantName,domainId,
			participantName, domainId);
		break;
	case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%d_Temperature(C),", participantName,domainId);
		break;
	case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%d_Utilization,",participantName,domainId);
		break;
	case ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%d_PROP(mW),%s_D%d_ARTG(mW),"
			"%s_D%d_PSRC,%s_D%d_AVOL(mV),%s_D%d_ACUR(mA),%s_D%d_AP01(%%),%s_D%d_AP02(%%),%s_D%d_AP10(%%),",
			participantName,domainId,participantName,domainId,
			participantName,domainId,participantName,domainId,
			participantName,domainId,participantName,domainId,
			participantName,domainId,participantName,domainId
			);
		break;
	case ESIF_CAPABILITY_TYPE_BATTERY_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%d_PMAX(mW),%s_D%d_PBSS(mW),"
			"%s_D%d_CTYP,%s_D%d_RBHF(mOhm),%s_D%d_CMPP(mA),%s_D%d_VBNL(mV),%s_D%d_batteryPercentage(%%),",
			participantName, domainId, participantName, domainId,
			participantName, domainId, participantName, domainId,
			participantName, domainId, participantName, domainId,
			participantName, domainId
		);
		break;
	case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%d_Aux0(C),%s_D%d_Aux1(C),%s_D%d_Hysteresis(C),", 
			participantName, 
			domainId, 
			participantName, 
			domainId, 
			participantName, 
			domainId
			);
		break;
	case ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS:
	{
		for (UInt32 channelNumber = 0; channelNumber < MAX_FREQUENCY_CHANNEL_NUM; channelNumber++)
		{
			esif_ccb_sprintf_concat(dataLength, logString, "Channel Number,%s_D%d_Center Frequency(Hz),%s_D%d_Left Frequency Spread(Hz),%s_D%d_Right Frequency Spread(Hz),", 
				participantName, domainId, participantName, domainId, participantName, domainId);
		}
		break;
	}
	case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%d_Min Frequency(Hz),%s_D%d_Center Frequency(Hz),%s_D%d_Max Frequency(Hz),%s_D%d_SSC,",
			participantName, domainId, participantName, domainId, participantName, domainId,
			participantName, domainId);
		break;
	case ESIF_CAPABILITY_TYPE_PSYS_CONTROL:
	{
		UInt32 psysType = 0;
		for (psysType = 0; psysType < MAX_PSYS_CONTROL_TYPE; psysType++) {
			esif_ccb_sprintf_concat(dataLength, logString, "Psys PL%d Power Limit (mW),Psys PL%d Duty Cycle,Psys PL%d Time Window (ms),",
				psysType + 1,
				psysType + 1,
				psysType + 1
				);
		}
	}
		break;
	case ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, "AC Peak Power,DC Peak Power,");
		break;
	case ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, "%s_D%d_TCC Offset(C),%s_D%d_Under Voltage Threshold (mV),", participantName, domainId, participantName, domainId);
		break;
	case ESIF_CAPABILITY_TYPE_MANAGER:
		esif_ccb_sprintf_concat(dataLength, logString, "OS Power Source,OS Battery Percent,OS Dock Mode,OS Game Mode,OS Lid State,OS Power Slider,OS User Interaction,"
			"OS User Presence,OS Screen State,Device Orientation,In Motion,System Cooling Mode,OS Platform Type,"
			"Display Orientation,OS Power Scheme Personality,OS Mixed Reality Mode,Platform User Presence,Foreground Background Ratio,PPM Package,Collaboration State,");
		break;
	case ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION:
		esif_ccb_sprintf_concat(dataLength, logString, "SOC Workload,");
		break;
	case ESIF_CAPABILITY_TYPE_DYNAMIC_EPP:
		esif_ccb_sprintf_concat(dataLength, logString, "MBT Hint,EPP,");
		break;
	default:
		break;
	}

	return rc;
}

static eEsifError EsifLogMgr_ParticipantLogAddDataNode(
	char *logString,
	size_t dataLength,
	EsifParticipantLogDataNodePtr dataNodePtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(logString != NULL);
	ESIF_ASSERT(dataNodePtr != NULL);

	if (EsifLogMgr_IsStatusCapable(dataNodePtr->capabilityData.type)) {
		esif_ccb_write_lock(&dataNodePtr->capabilityDataLock);
		EsifLogMgr_UpdateStatusCapabilityData(dataNodePtr);
		esif_ccb_write_unlock(&dataNodePtr->capabilityDataLock);
	}

	esif_ccb_read_lock(&dataNodePtr->capabilityDataLock);
	EsifLogMgr_ParticipantLogAddCapabilityData(logString, dataLength, dataNodePtr);
	esif_ccb_read_unlock(&dataNodePtr->capabilityDataLock);

	return rc;
}

static eEsifError EsifLogMgr_ParticipantLogAddCapabilityData(
	char *logString,
	size_t dataLength,
	EsifParticipantLogDataNodePtr dataNodePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifCapabilityDataPtr capabilityPtr = NULL;
	char guidStr[ESIF_GUID_PRINT_SIZE] = { 0 };

	ESIF_ASSERT(logString != NULL);
	ESIF_ASSERT(dataNodePtr != NULL);

	capabilityPtr = &dataNodePtr->capabilityData;

	if ((dataNodePtr->state >= ESIF_DATA_INITIALIZED) && (dataNodePtr->isPresent != ESIF_FALSE)) {
	
		switch (capabilityPtr->type) {
		case ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,%u,",
				capabilityPtr->data.activeControl.controlId,
				capabilityPtr->data.activeControl.speed,
				capabilityPtr->data.activeControl.lowerLimit,
				capabilityPtr->data.activeControl.upperLimit
				);
			break;
		case ESIF_CAPABILITY_TYPE_CORE_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,",
				capabilityPtr->data.coreControl.activeLogicalProcessors,
				capabilityPtr->data.coreControl.maximumActiveCores,
				capabilityPtr->data.coreControl.minimumActiveCores
				);
			break;
		case ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,",
				capabilityPtr->data.displayControl.currentDPTFLimit,
				capabilityPtr->data.displayControl.lowerLimit,
				capabilityPtr->data.displayControl.upperLimit
				);
			break;
		case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,",
				capabilityPtr->data.domainPriority.priority
				);
			break;
		case ESIF_CAPABILITY_TYPE_ENERGY_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,",
				capabilityPtr->data.energyControl.energyCounter,
				capabilityPtr->data.energyControl.instantaneousPower);
			break;
		case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,",
				capabilityPtr->data.performanceControl.pStateLimit,
				capabilityPtr->data.performanceControl.lowerLimit,
				capabilityPtr->data.performanceControl.upperLimit
				);
			break;
		case ESIF_CAPABILITY_TYPE_POWER_CONTROL:
		{
			UInt32 powerType = 0;
			for (powerType = 0; powerType < MAX_POWER_CONTROL_TYPE; powerType++)
			{
				if ((capabilityPtr->data.powerControl.powerDataSet[powerType].powerType <= MAX_POWER_CONTROL_TYPE) &&
					(capabilityPtr->data.powerControl.powerDataSet[powerType].isEnabled != ESIF_FALSE)) {
					esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,%u,%u,%u,%u,%u,",
						capabilityPtr->data.powerControl.powerDataSet[powerType].powerLimit,
						capabilityPtr->data.powerControl.powerDataSet[powerType].lowerLimit,
						capabilityPtr->data.powerControl.powerDataSet[powerType].upperLimit,
						capabilityPtr->data.powerControl.powerDataSet[powerType].stepsize,
						capabilityPtr->data.powerControl.powerDataSet[powerType].minTimeWindow,
						capabilityPtr->data.powerControl.powerDataSet[powerType].maxTimeWindow,
						capabilityPtr->data.powerControl.powerDataSet[powerType].minDutyCycle,
						capabilityPtr->data.powerControl.powerDataSet[powerType].maxDutyCycle
					);
				}
				else {
					esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,X,X,X,X,");
				}
			}
			if (capabilityPtr->data.powerControl.socPowerFloorData.isSupported != ESIF_FALSE) {
				esif_ccb_sprintf_concat(dataLength, logString, "%u,",
					capabilityPtr->data.powerControl.socPowerFloorData.socPowerFloorState);
			}
			else {
				esif_ccb_sprintf_concat(dataLength, logString, "X,");
			}
			break;
		}
		case ESIF_CAPABILITY_TYPE_POWER_STATUS:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,",
				capabilityPtr->data.powerStatus.currentPower,
				capabilityPtr->data.powerStatus.powerFilterData.currentPowerSentToFilter,
				capabilityPtr->data.powerStatus.powerFilterData.powerCalculatedByFilter
				);
			break;
		case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
		{
			int temp = (int)capabilityPtr->data.temperatureStatus.temperature;
			esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_DECIC, (esif_temp_t *)&temp);

			esif_ccb_sprintf_concat(dataLength, logString, "%.1f,", temp / 10.0);
			break;
		}
		case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,",
				capabilityPtr->data.utilizationStatus.utilization
				);
			break;
		case ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,%u,%u,%u,%u,%u,",
				capabilityPtr->data.platformPowerStatus.platformRestOfPower,
				capabilityPtr->data.platformPowerStatus.adapterPowerRating,
				capabilityPtr->data.platformPowerStatus.platformPowerSource,
				capabilityPtr->data.platformPowerStatus.acNominalVoltage,
				capabilityPtr->data.platformPowerStatus.acOperationalCurrent,
				capabilityPtr->data.platformPowerStatus.ac1msOverload,
				capabilityPtr->data.platformPowerStatus.ac2msOverload,
				capabilityPtr->data.platformPowerStatus.ac10msOverload
				);
			break;
		case ESIF_CAPABILITY_TYPE_BATTERY_STATUS:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,%u,%u,%u,%u,",
				capabilityPtr->data.batteryStatus.maxBatteryPower,
				capabilityPtr->data.batteryStatus.steadyStateBatteryPower,
				capabilityPtr->data.batteryStatus.chargerType,
				capabilityPtr->data.batteryStatus.highFrequencyImpedance,
				capabilityPtr->data.batteryStatus.maxPeakCurrent,
				capabilityPtr->data.batteryStatus.noLoadVoltage,
				capabilityPtr->data.batteryStatus.batteryPercentage
				);
			break;
		case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
		{
			int tempAux0 = (int)capabilityPtr->data.temperatureThresholdControl.aux0;
			int tempAux1 = (int)capabilityPtr->data.temperatureThresholdControl.aux1;
			int tempHyst = (int)capabilityPtr->data.temperatureThresholdControl.hysteresis;

			esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_DECIC, (esif_temp_t *)&tempAux0);
			esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_DECIC, (esif_temp_t *)&tempAux1);
			esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_DECIC, (esif_temp_t *)&tempHyst);

			esif_ccb_sprintf_concat(dataLength, logString, "%.1f,%.1f,%.1f,",
				tempAux0 / 10.0,
				tempAux1 / 10.0,
				tempHyst / 10.0
				);
			break;
		}
		case ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS:
		{
			for (UInt32 channelNumber = 0; channelNumber < MAX_FREQUENCY_CHANNEL_NUM; channelNumber++)
			{
				UInt32 centerFrequency = capabilityPtr->data.rfProfileStatus.rfProfileFrequencyData[channelNumber].centerFrequency;
				UInt32 leftFrequencySpread = capabilityPtr->data.rfProfileStatus.rfProfileFrequencyData[channelNumber].leftFrequencySpread;
				UInt32 rightFrequencySpread = capabilityPtr->data.rfProfileStatus.rfProfileFrequencyData[channelNumber].rightFrequencySpread;
				if (centerFrequency != ESIF_INVALID_DATA && leftFrequencySpread != ESIF_INVALID_DATA && rightFrequencySpread != ESIF_INVALID_DATA) {
					esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,%u,", channelNumber, centerFrequency,
						leftFrequencySpread, rightFrequencySpread);
				}
				else {
					esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,");
				}
			}
			break;
		}
		case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,%u,",
				capabilityPtr->data.rfProfileControl.rfProfileMinFrequency,
				capabilityPtr->data.rfProfileControl.rfProfileCenterFrequency,
				capabilityPtr->data.rfProfileControl.rfProfileMaxFrequency,
				capabilityPtr->data.rfProfileControl.rfProfileSSC
				);
			break;
		case ESIF_CAPABILITY_TYPE_PSYS_CONTROL:
		{
			UInt32 psysType = 0;
			for (psysType = 0; psysType < MAX_PSYS_CONTROL_TYPE; psysType++)
			{
				if (capabilityPtr->data.psysControl.powerDataSet[psysType].powerLimitType <= MAX_PSYS_CONTROL_TYPE) {
					if (capabilityPtr->data.psysControl.powerDataSet[psysType].powerLimit != ESIF_INVALID_DATA) {
						esif_ccb_sprintf_concat(dataLength, logString, "%u,",
							capabilityPtr->data.psysControl.powerDataSet[psysType].powerLimit
						);
					}
					else {
						esif_ccb_sprintf_concat(dataLength, logString, "X,"
						);
					}

					if (capabilityPtr->data.psysControl.powerDataSet[psysType].PowerLimitDutyCycle != ESIF_INVALID_DATA) {
						esif_ccb_sprintf_concat(dataLength, logString, "%u,",
							capabilityPtr->data.psysControl.powerDataSet[psysType].PowerLimitDutyCycle
						);
					}
					else {
						esif_ccb_sprintf_concat(dataLength, logString, "X,"
						);
					}

					if (capabilityPtr->data.psysControl.powerDataSet[psysType].PowerLimitTimeWindow != ESIF_INVALID_DATA) {
						esif_ccb_sprintf_concat(dataLength, logString, "%u,",
							capabilityPtr->data.psysControl.powerDataSet[psysType].PowerLimitTimeWindow
						);
					}
					else {
						esif_ccb_sprintf_concat(dataLength, logString, "X,"
						);
					}
				}
			}
			break;
		}
		case ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,",
				capabilityPtr->data.peakPowerControl.acPeakPower,
				capabilityPtr->data.peakPowerControl.dcPeakPower
			);
			break;
		case ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL:
		{
			int temp = (int)capabilityPtr->data.processorControlStatus.tccOffset;
			esif_convert_temp(NORMALIZE_TEMP_TYPE, ESIF_TEMP_DECIC, (esif_temp_t *)&temp);

			esif_ccb_sprintf_concat(dataLength, logString, "%.1f,%u,", temp / 10.0, capabilityPtr->data.processorControlStatus.uvth);
			break;
		}
		case ESIF_CAPABILITY_TYPE_MANAGER:
		{
			if (capabilityPtr->data.managerStatus.osPowerSource == ESIF_INVALID_DATA) {
				esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,");
			}
			else {
				esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%s,%u,%u,%u,%u,%u,",
					capabilityPtr->data.managerStatus.osPowerSource,
					capabilityPtr->data.managerStatus.batteryPercent,
					capabilityPtr->data.managerStatus.dockMode,
					capabilityPtr->data.managerStatus.gameMode,
					capabilityPtr->data.managerStatus.lidState,
					capabilityPtr->data.managerStatus.powerSlider,
					capabilityPtr->data.managerStatus.userInteraction,
					capabilityPtr->data.managerStatus.userPresence,
					capabilityPtr->data.managerStatus.screenState,
					capabilityPtr->data.managerStatus.deviceOrientation,
					capabilityPtr->data.managerStatus.inMotion,
					capabilityPtr->data.managerStatus.systemCoolingMode,
					capabilityPtr->data.managerStatus.platformType,
					capabilityPtr->data.managerStatus.displayOrientation,
					esif_guid_print(&capabilityPtr->data.managerStatus.powerSchemePersonality, guidStr),
					capabilityPtr->data.managerStatus.mixedRealityMode,
					capabilityPtr->data.managerStatus.platformUserPresence,
					capabilityPtr->data.managerStatus.foregroundBackgroundRatio,
					capabilityPtr->data.managerStatus.ppmPackage,
					capabilityPtr->data.managerStatus.collaboration
				);
			}
			break;
		}
		case ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,", capabilityPtr->data.workloadClassification.socWorkload);
			break;
		case ESIF_CAPABILITY_TYPE_DYNAMIC_EPP:
			esif_ccb_sprintf_concat(dataLength, logString, "%u,%u,", capabilityPtr->data.dynamicEppControl.eppHint, capabilityPtr->data.dynamicEppControl.eppValue);
			break;
		default:
			rc = ESIF_E_UNSPECIFIED;
			break;
		}
	}
	else {
		switch (capabilityPtr->type) {
		case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
		case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
		case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
		case ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION:
			esif_ccb_sprintf_concat(dataLength, logString, "X,");
			break;
		case ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,");
			break;
		case ESIF_CAPABILITY_TYPE_CORE_CONTROL:
		case ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL:
		case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
		case ESIF_CAPABILITY_TYPE_POWER_STATUS:
		case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
			esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,");
			break;
		case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,");
			break;
		case ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL:
		case ESIF_CAPABILITY_TYPE_ENERGY_CONTROL:
		case ESIF_CAPABILITY_TYPE_DYNAMIC_EPP:
			esif_ccb_sprintf_concat(dataLength, logString, "X,X,");
			break;
		case ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS:
			esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,X,X,X,X,");
			break;
		case ESIF_CAPABILITY_TYPE_BATTERY_STATUS:
			esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,X,X,X,");
			break;
		case ESIF_CAPABILITY_TYPE_POWER_CONTROL:
		{
			UInt32 powerType = 0;
			for (powerType = 0; powerType < MAX_POWER_CONTROL_TYPE; powerType++)
			{
				esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,X,X,X,X,");
			}
			esif_ccb_sprintf_concat(dataLength, logString, "X,");
			break;
		}
		case ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS:
		{
			for (UInt32 channelNumber = 0; channelNumber < MAX_FREQUENCY_CHANNEL_NUM; channelNumber++)
			{
				esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,");
			}
			break;
		}
		case ESIF_CAPABILITY_TYPE_PSYS_CONTROL:
		{
			UInt32 psysType = 0;
			for (psysType = 0; psysType < MAX_PSYS_CONTROL_TYPE; psysType++)
			{
				esif_ccb_sprintf_concat(dataLength, logString, "X,X,X, ");
			}
			break;
		}
		case ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL:
			esif_ccb_sprintf_concat(dataLength, logString, "X,X,");
			break;
		case ESIF_CAPABILITY_TYPE_MANAGER:
			esif_ccb_sprintf_concat(dataLength, logString, "X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,");
			break;
		default:
			rc = ESIF_E_UNSPECIFIED;
			break;
		}
	}

	return rc;
}


//
// WARNING:  Any new cases must be added to g_statusCapability
//
static void EsifLogMgr_UpdateStatusCapabilityData(EsifParticipantLogDataNodePtr dataNodePtr)
{
	eEsifError rc = ESIF_OK;
	char qualifierStr[MAX_NAME_STRING_LENGTH] = "D0";

	ESIF_ASSERT(dataNodePtr != NULL);

	switch (dataNodePtr->capabilityData.type) {
	case ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL:
	{
		UInt32 controlId = 0;
		UInt32 speed = 0;
		struct EsifDataBinaryFstPackage fst = { 0 };
		struct esif_data fstData = { ESIF_DATA_BINARY, &fst, sizeof(fst), 0 };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_FAN_STATUS, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &fstData);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_FAN_STATUS primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
		}
		else {
			controlId = (UInt32)fst.control.integer.value;
			speed = (UInt32)fst.speed.integer.value;
		}

		dataNodePtr->capabilityData.data.activeControl.controlId = controlId;
		dataNodePtr->capabilityData.data.activeControl.speed = speed;
		break;
	}
	case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
	{
		UInt32 temp = 0;
		struct esif_data temp_response = { ESIF_DATA_TEMPERATURE, &temp, sizeof(temp), sizeof(temp) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_TEMPERATURE, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &temp_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_TEMPERATURE primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			temp = 0;
		}
		dataNodePtr->capabilityData.data.temperatureStatus.temperature = temp;
		break;
	}
	case ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS:
	{
		UInt32 powerValue = 0;
		struct esif_data power_response = { ESIF_DATA_POWER, &powerValue, sizeof(powerValue), sizeof(powerValue) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_PLATFORM_REST_OF_POWER, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &power_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_PLATFORM_REST_OF_POWER primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			powerValue = 0;
		}
		dataNodePtr->capabilityData.data.platformPowerStatus.platformRestOfPower = powerValue;

		powerValue = 0;
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_ADAPTER_POWER_RATING, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &power_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_ADAPTER_POWER_RATING primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			powerValue = 0;
		}
		dataNodePtr->capabilityData.data.platformPowerStatus.adapterPowerRating = powerValue;

		UInt32 u32Value = 0;
		struct esif_data u32_response = { ESIF_DATA_UINT32, &u32Value, sizeof(u32Value), sizeof(u32Value) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_PLATFORM_POWER_SOURCE, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_PLATFORM_POWER_SOURCE primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.platformPowerStatus.platformPowerSource = u32Value;

		u32Value = 0;
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_AVOL, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_AVOL primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.platformPowerStatus.acNominalVoltage = u32Value;

		u32Value = 0;
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_ACUR, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_ACUR primitive for participant " ESIF_HANDLE_FMT "d domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.platformPowerStatus.acOperationalCurrent = u32Value;

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_AP01, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_AP01 primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.platformPowerStatus.ac1msOverload = u32Value;

		u32Value = 0;
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_AP02, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_AP02 primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.platformPowerStatus.ac2msOverload = u32Value;

		u32Value = 0;
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_AP10, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_AP10 primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.platformPowerStatus.ac10msOverload = u32Value;
		break;
	}
	case ESIF_CAPABILITY_TYPE_BATTERY_STATUS:
	{
		UInt32 powerValue = 0;
		struct esif_data power_response = { ESIF_DATA_POWER, &powerValue, sizeof(powerValue), sizeof(powerValue) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_PLATFORM_MAX_BATTERY_POWER, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &power_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_PLATFORM_MAX_BATTERY_POWER primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			powerValue = 0;
		}
		dataNodePtr->capabilityData.data.batteryStatus.maxBatteryPower = powerValue;

		powerValue = 0;
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_PLATFORM_BATTERY_STEADY_STATE, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &power_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_PLATFORM_BATTERY_STEADY_STATE primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			powerValue = 0;
		}
		dataNodePtr->capabilityData.data.batteryStatus.steadyStateBatteryPower = powerValue;

		UInt32 u32Value = 0;
		struct esif_data u32_response = { ESIF_DATA_UINT32, &u32Value, sizeof(u32Value), sizeof(u32Value) };
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_CHARGER_TYPE, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_CHARGER_TYPE primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.batteryStatus.chargerType = u32Value;

		u32Value = 0;
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_BATTERY_HIGH_FREQUENCY_IMPEDANCE, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_BATTERY_HIGH_FREQUENCY_IMPEDANCE primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.batteryStatus.highFrequencyImpedance = u32Value;

		u32Value = 0;
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_BATTERY_MAX_PEAK_CURRENT, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_BATTERY_MAX_PEAK_CURRENT primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.batteryStatus.maxPeakCurrent = u32Value;

		u32Value = 0;
		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_BATTERY_NO_LOAD_VOLTAGE, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &u32_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_BATTERY_NO_LOAD_VOLTAGE primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			u32Value = 0;
		}
		dataNodePtr->capabilityData.data.batteryStatus.noLoadVoltage = u32Value;

		UInt32 battPercent = 0;
		struct esif_data batt_percent_response = { ESIF_DATA_PERCENT, &battPercent, sizeof(battPercent), sizeof(battPercent) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_BATTERY_PERCENTAGE, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &batt_percent_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_BATTERY_PERCENTAGE primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			battPercent = 0;
		}
		dataNodePtr->capabilityData.data.batteryStatus.batteryPercentage = battPercent / 100;
		break;
	}
	case ESIF_CAPABILITY_TYPE_POWER_STATUS:
	{
		UInt32 power = 0;
		struct esif_data power_response = { ESIF_DATA_POWER, &power, sizeof(power), sizeof(power) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_RAPL_POWER, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &power_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_RAPL_POWER primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			power = 0;
		}
		dataNodePtr->capabilityData.data.powerStatus.currentPower = power;
		break;
	}
	case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
	{
		UInt32 utilization = 0;
		struct esif_data util_response = { ESIF_DATA_PERCENT, &utilization, sizeof(utilization), sizeof(utilization) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_PARTICIPANT_UTILIZATION, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &util_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_PARTICIPANT_UTILIZATION primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			utilization = 0;
		}
		dataNodePtr->capabilityData.data.utilizationStatus.utilization = utilization / 100;
		break;
	}
	case ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL:
	{
		UInt32 temp = 0;
		struct esif_data temp_response = { ESIF_DATA_TEMPERATURE, &temp, sizeof(temp), sizeof(temp) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_TCC_OFFSET, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &temp_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_TCC_OFFSET primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			temp = 0;
		}
		dataNodePtr->capabilityData.data.processorControlStatus.tccOffset = temp;
		break;
	}
	case ESIF_CAPABILITY_TYPE_MANAGER:
	{
		UInt32 eventData = ESIF_INVALID_DATA;
		esif_guid_t  eventGuidData = { 0 };
		struct esif_data capabilityEventData = { ESIF_DATA_UINT32, &eventData, sizeof(eventData), sizeof(eventData) };
		struct esif_data capabilityEventGuidData = { ESIF_DATA_GUID, &eventGuidData, sizeof(eventGuidData), sizeof(eventGuidData) };
		

		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_POWER_SOURCE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os power source event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.osPowerSource = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_BATTERY_PERCENT_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os battery percent event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.batteryPercent = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_DOCK_MODE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os dock mode event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.dockMode = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_GAME_MODE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os game mode event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.gameMode = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_LID_STATE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os lid state event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.lidState = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_POWER_SLIDER_VALUE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os power slider event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.powerSlider = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_USER_INTERACTION_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os user interaction event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.userInteraction = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_USER_PRESENCE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os user presence event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.userPresence = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_SCREEN_STATE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os screen state event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.screenState = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_DEVICE_ORIENTATION_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve device orientation event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.deviceOrientation = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_MOTION_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve in motion event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.inMotion = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_DTT_SYSTEM_COOLING_POLICY_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve system cooling mode event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.systemCoolingMode = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve platform type event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.platformType = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve display orientation event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.displayOrientation = eventData;

		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_POWERSCHEME_PERSONALITY_CHANGED, &capabilityEventGuidData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os power scheme personality event cache data \n");
		}
		esif_ccb_memcpy(&dataNodePtr->capabilityData.data.managerStatus.powerSchemePersonality, &eventGuidData, SIZE_OF(EsifManagerStatusData, powerSchemePersonality));
		esif_guid_mangle(&dataNodePtr->capabilityData.data.managerStatus.powerSchemePersonality);

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_OS_MIXED_REALITY_MODE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve os mixed reality mode event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.mixedRealityMode = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_PLATFORM_USER_PRESENCE_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve platform user presence event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.platformUserPresence = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_FOREGROUND_BACKGROUND_RATIO_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve foreground background ratio event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.foregroundBackgroundRatio = eventData;

		eventData = ESIF_INVALID_DATA;
		rc = EsifEventCache_GetValue(ESIF_EVENT_COLLABORATION_CHANGED, &capabilityEventData);
		if (rc != ESIF_OK) {
			ESIF_TRACE_INFO("Failed to retrieve collaboration event cache data \n");
		}
		dataNodePtr->capabilityData.data.managerStatus.collaboration = eventData;

		break;
	}
	case ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION:
	{
		UInt32 socwc = 0;
		struct esif_data socwc_response = { ESIF_DATA_UINT32, &socwc, sizeof(socwc), sizeof(socwc) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_SOC_WORKLOAD, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &socwc_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_SOC_WORKLOAD primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			socwc = 0;
		}
		dataNodePtr->capabilityData.data.workloadClassification.socWorkload = socwc;
		break;
	}
	case ESIF_CAPABILITY_TYPE_DYNAMIC_EPP:
	{
		UInt32 eppHint = 0;
		struct esif_data epphint_response = { ESIF_DATA_UINT32, &eppHint, sizeof(eppHint), sizeof(eppHint) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_EPP_SENSITIVITY_HINT_MODEL, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &epphint_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_EPP_SENSITIVITY_HINT_MODEL primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			eppHint = 0;
		}
		dataNodePtr->capabilityData.data.dynamicEppControl.eppHint = eppHint;

		UInt32 eppValue = 0;
		struct esif_data eppvalue_response = { ESIF_DATA_UINT32, &eppValue, sizeof(eppValue), sizeof(eppValue) };

		rc = EsifExecutePrimitive(dataNodePtr->participantId, GET_HWP_EPP_VALUE, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &eppvalue_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_HWP_EPP_VALUE primitive for participant " ESIF_HANDLE_FMT " domain : %d", esif_ccb_handle2llu(dataNodePtr->participantId), dataNodePtr->domainId);
			eppValue = 0;
		}
		dataNodePtr->capabilityData.data.dynamicEppControl.eppValue = eppValue;
		break;
	}
	// WARNING:  Any new cases must be added to g_statusCapability
	default:
		break;
	}

	return;
}

static void EsifLogMgr_DataLogWrite(
	EsifLoggingManagerPtr self,
	char *logstring,
	...
	)
{
	va_list args;

	ESIF_ASSERT(self != NULL);

	if (logstring == NULL) {
		ESIF_TRACE_ERROR("logstring is NULL");
		goto exit;
	}

	if (self->listenersMask == ESIF_LISTENER_NONE) {
		goto exit;
	}

	if ((self->listenersMask & ESIF_LISTENER_CONSOLE_MASK) > 0) {
		if (!self->isLogHeader || (!(self->listenerHeadersWrittenMask & ESIF_LISTENER_CONSOLE_MASK))) {
			va_start(args, logstring);
			EsifConsole_WriteConsole(logstring, args);
			va_end(args);
		}
	}

	if ((self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) > 0) {
		if (!self->isLogHeader || (!(self->listenerHeadersWrittenMask & ESIF_LISTENER_LOGFILE_MASK))) {
			va_start(args, logstring);
			EsifLogFile_WriteArgsAppend(ESIF_LOG_PARTICIPANT, " ", logstring, args);
			va_end(args);
		}
	}

	if ((self->listenersMask & ESIF_LISTENER_DEBUGGER_MASK) > 0) {
		size_t  msglen = 0;
		char *buffer = 0;

		if (!self->isLogHeader || (!(self->listenerHeadersWrittenMask & ESIF_LISTENER_DEBUGGER_MASK))) {
			va_start(args, logstring);
			msglen = esif_ccb_vscprintf(logstring, args) + 1;
			va_end(args);

			buffer = (char *)esif_ccb_malloc(msglen);

			if (NULL != buffer) {
				va_start(args, logstring);
				esif_ccb_vsprintf(msglen, buffer, logstring, args);
				va_end(args);

				EsifLogMgr_LogToDebugger(buffer);
				esif_ccb_free(buffer);
			}
		}
	}

	if ((self->listenersMask & ESIF_LISTENER_EVENTLOG_MASK) > 0) {
		size_t  msglen = 0;
		char *buffer = 0;

		if (!self->isLogHeader || (!(self->listenerHeadersWrittenMask & ESIF_LISTENER_EVENTLOG_MASK))) {
			va_start(args, logstring);
			msglen = esif_ccb_vscprintf(logstring, args) + 1;
			va_end(args);

			buffer = (char *)esif_ccb_malloc(msglen);

			if (NULL != buffer) {
				va_start(args, logstring);
				esif_ccb_vsprintf(msglen, buffer, logstring, args);
				va_end(args);

				EsifLogMgr_LogToEvent(buffer);
				esif_ccb_free(buffer);
			}
		}
	}
exit:
	return;
}

static eEsifError EsifLogMgr_Uninit(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;

	if (self == NULL || self->isInitialized == ESIF_FALSE) {
		//Manager is not initialized. just exit
		goto exit;
	}

	//Free the log data 
	if (self->logData != NULL) {
		esif_ccb_free(self->logData);
		self->logData = NULL;
	}

	//Free the input argv
	EsifLogMgr_DestroyArgv(self);

	if (self->commandInfo != NULL) {
		esif_ccb_free(self->commandInfo);
		self->commandInfo = NULL;
		self->commandInfoCount = 0;
	}
	
	EsifLogMgr_DestroyScheduleTimer(self);
	
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_DTT_PARTICIPANT_CONTROL_ACTION,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_MATCH_ANY_DOMAIN,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_CREATE_COMPLETE,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND,
		ESIF_HANDLE_PRIMARY_PARTICIPANT,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME,
		ESIF_HANDLE_PRIMARY_PARTICIPANT,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		esif_ccb_ptr2context(self));

	EsifLogMgr_DestroyParticipantLogData(self);

	esif_link_list_destroy(self->participantLogData.list);
	self->participantLogData.list = NULL;

	esif_ccb_lock_uninit(&self->participantLogData.listLock);
	self->isInitialized = ESIF_FALSE;
exit:
	return rc;
}

static void EsifLogMgr_DestroyParticipantLogData(
	EsifLoggingManagerPtr self
	)
{
	ESIF_ASSERT(self != NULL);

	if (NULL == self->participantLogData.list) {
		goto exit;
	}
	
	esif_ccb_write_lock(&self->participantLogData.listLock);
	//Delete and Free the nodes in the list
	esif_link_list_free_data(self->participantLogData.list, (link_list_data_destroy_func)EsifLogMgr_DestroyEntry);		

	esif_ccb_write_unlock(&self->participantLogData.listLock);
exit:
	return;
}

static void EsifLogMgr_DestroyEntry(EsifParticipantLogDataNodePtr curEntryPtr)
{
	UInt32 capabilityMask = 0;

	if (curEntryPtr == NULL) {
		goto exit;
	}

	capabilityMask = (UInt32)(1 << curEntryPtr->capabilityData.type);
	EsifLogMgr_SendParticipantLogEvent(
		ESIF_EVENT_DTT_PARTICIPANT_ACTIVITY_LOGGING_DISABLED,
		curEntryPtr->participantId,
		(UInt16)curEntryPtr->domainId,
		capabilityMask
		);

	esif_ccb_lock_uninit(&curEntryPtr->capabilityDataLock);
	esif_ccb_free(curEntryPtr);

exit:
	return;
}

static eEsifError EsifLogMgr_OpenParticipantLogFile(char *fileName)
{
	eEsifError rc = ESIF_OK;
	EsifLogType logtype = ESIF_LOG_PARTICIPANT;
	char logname[MAX_PATH] = { 0 };
	int append = ESIF_FALSE;

	if (fileName == NULL) {
		time_t now = time(NULL);
		struct tm time = { 0 };
		if (esif_ccb_localtime(&time, &now) == 0) {
			esif_ccb_sprintf(sizeof(logname), logname, "participant_log_%04d-%02d-%02d-%02d%02d%02d.csv",
				time.tm_year + TIME_BASE_YEAR, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
		}
		EsifLogFile_Open(logtype, logname, append);
	}
	else {
		EsifLogFile_Open(logtype, fileName, append);
	}
	if (!EsifLogFile_IsOpen(logtype)) {
		rc = ESIF_E_IO_ERROR;
		goto exit;
	}
exit:
	return rc;
}


static void EsifLogMgr_UpdateCapabilityData(
	EsifParticipantLogDataNodePtr capabilityEntryPtr, 
	EsifCapabilityDataPtr capabilityPtr
	)
{
	ESIF_ASSERT(capabilityEntryPtr != NULL);
	ESIF_ASSERT(capabilityPtr != NULL);

	capabilityEntryPtr->capabilityData.size = capabilityPtr->size;
	capabilityEntryPtr->state = ESIF_DATA_INITIALIZED;
	
	esif_ccb_write_lock(&capabilityEntryPtr->capabilityDataLock);
	esif_ccb_memcpy(&capabilityEntryPtr->capabilityData, capabilityPtr, sizeof(*capabilityPtr));	
	esif_ccb_write_unlock(&capabilityEntryPtr->capabilityDataLock);

	return;
}

static void EsifLogMgr_SendParticipantLogEvent(
	eEsifEventType eventType, 
	esif_handle_t participantId,
	UInt16 domainId, 
	UInt32 capabilityMask
	)
{
	//Enable/Disable Participant logging only if any one of capability mask bit is set
	if (capabilityMask > 0) {
		EsifData capabilityMask_data = { ESIF_DATA_UINT32,
			&capabilityMask,
			sizeof(capabilityMask),
			sizeof(capabilityMask) 
		};

		EsifEventMgr_SignalEvent(participantId, domainId, eventType, &capabilityMask_data);
	}
}

eEsifError EsifUf_GetNextCapability(esif_flags_t capabilitymask, UInt32 startIndex, UInt32 *capabilityIdPtr)
{
	eEsifError rc = ESIF_OK;
	UInt32 index = startIndex;
	capabilitymask = capabilitymask >> index;

	if (capabilityIdPtr == NULL) {
		ESIF_TRACE_ERROR("capabilityIdPtr is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	while (capabilitymask > 0) {
		if ((capabilitymask & 0x1) == 1) {
			break;
		}
		capabilitymask = capabilitymask >> 1;
		index++;
		if (index >= MAX_CAPABILITY_MASK) {
			index = 0;
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			break;
		}
	}

exit:
	if (rc != ESIF_OK) {
		index = (UInt32)-1;
	}

	if ((rc == ESIF_OK) && 
		(capabilityIdPtr != NULL)) {
		*capabilityIdPtr = index;
	}
	return rc;
}

static Bool EsifLogMgr_IsStatusCapable(UInt32 capabilityId)
{
	Bool result = ESIF_FALSE;
	UInt32 i = 0;
	for (i = 0; i < sizeof(g_statusCapability) / sizeof(g_statusCapability[0]); i++) {
		if (capabilityId == g_statusCapability[i]) {
			result = ESIF_TRUE;
			break;
		}
	}
	return result;
}

static void EsifLogMgr_PrintLogStatus(
	EsifLoggingManagerPtr self,
	char *output,
	size_t datalength
	)
{
	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(output != NULL);

	if (self->isLogStarted != ESIF_FALSE) {

		esif_ccb_sprintf_concat(datalength, output,
			"Log State     : Started\n"\
			"Log Interval  : %d ms\n",
			self->pollingThread.interval
			);
	}
	else {
		esif_ccb_sprintf_concat(datalength, output,
			"Log State     : Stopped\n");
	}
	EsifLogMgr_PrintListenerStatus(self, output, datalength);
}

static void EsifLogMgr_PrintListenerStatus(
	EsifLoggingManagerPtr self,
	char *output,
	size_t datalength
	)
{
	char filepath[MAX_PATH] = { 0 };

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(output != NULL);

	if (self->listenersMask != ESIF_LISTENER_NONE) {
		esif_ccb_strcat(output,
			"Log Route     : ",
			OUT_BUF_LEN
			);

		if ((self->listenersMask & ESIF_LISTENER_EVENTLOG_MASK) == ESIF_LISTENER_EVENTLOG_MASK) {
			esif_ccb_strcat(output, ESIF_LISTENER_EVENTLOG_STR, datalength);
			esif_ccb_strcat(output, " ", datalength);
		}
		if ((self->listenersMask & ESIF_LISTENER_DEBUGGER_MASK) == ESIF_LISTENER_DEBUGGER_MASK) {
			esif_ccb_strcat(output, ESIF_LISTENER_DEBUGGER_STR, datalength);
			esif_ccb_strcat(output, " ", datalength);
		}
		if ((self->listenersMask & ESIF_LISTENER_CONSOLE_MASK) == ESIF_LISTENER_CONSOLE_MASK) {
			esif_ccb_strcat(output, ESIF_LISTENER_CONSOLE_STR, datalength);
			esif_ccb_strcat(output, " ", datalength);
		}
		if ((self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) == ESIF_LISTENER_LOGFILE_MASK) {
			esif_ccb_strcat(output, ESIF_LISTENER_LOGFILE_STR, datalength);
			esif_ccb_strcat(output, " ", datalength);
		}
		esif_ccb_strcat(output, "\n", datalength);

		if ((self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) == ESIF_LISTENER_LOGFILE_MASK) {
			esif_ccb_strcat(
				output,
				"Log File Name : ",
				datalength
				);

			if (!self->isDefaultFile) {
				EsifLogFile_GetFullPath(filepath, sizeof(filepath), self->filename);

				esif_ccb_strcat(
					output,
					filepath,
					datalength
				);
			} 
			else {
				if (EsifLogFile_GetFileNameFromType(ESIF_LOG_PARTICIPANT)) {
						esif_ccb_strcat(
						output,
						EsifLogFile_GetFileNameFromType(ESIF_LOG_PARTICIPANT),
						datalength
						);
				}
			}
			esif_ccb_strcat(output, "\n", datalength);
		}
	}
}
