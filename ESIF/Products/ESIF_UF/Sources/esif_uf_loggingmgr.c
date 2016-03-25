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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_LOGGINGMGR

#include "esif_uf_loggingmgr.h"

UInt32 g_statusCapability[] = {
	ESIF_CAPABILITY_TYPE_TEMP_STATUS,
	ESIF_CAPABILITY_TYPE_POWER_STATUS,
	ESIF_CAPABILITY_TYPE_UTIL_STATUS,
};

EsifLoggingManager g_loggingManager = { 0 };

static eEsifError EsifLogMgr_Init(EsifLoggingManagerPtr self);
static eEsifError EsifLogMgr_Uninit(EsifLoggingManagerPtr self);
static void EsifLogMgr_Suspend(EsifLoggingManagerPtr self);
static void EsifLogMgr_Resume(EsifLoggingManagerPtr self);

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
static void EsifLogMgr_ParticipantLogFire(
	EsifLoggingManagerPtr self
	);
static eEsifError EsifLogMgr_AddAllParticipants(EsifLoggingManagerPtr self);
static eEsifError EsifLogMgr_GetParticipantId(
	char *participantstr,
	UInt32 *participantId
	);
static eEsifError EsifLogMgr_GetDomainId(
	char *domainstr,
	UInt8 participantId,
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
	UInt8 participantId,
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
	UInt32 participantId,
	UInt32 domainId,
	UInt32 capabilityId
	);
static void EsifLogMgr_UpdateCapabilityData(
	EsifParticipantLogDataNodePtr capabilityEntryPtr,
	EsifCapabilityDataPtr capabilityPtr
	);
static eEsifError EsifLogMgr_OpenParticipantLogFile(char *fileName);
static eEsifError ESIF_CALLCONV EsifLogMgr_EventCallback(
	void *contextPtr,
	UInt8 participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);
static EsifLinkListNodePtr EsifLogMgr_GetCapabilityNodeWLock(
	EsifLoggingManagerPtr self,
	UInt32 participantId, 
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
	EsifCapabilityDataPtr capabilityPtr
	);
static void EsifLogMgr_DataLogWrite(
	EsifLoggingManagerPtr self,
	char *logstring,
	...
	);
static void EsifLogMgr_SendParticipantLogEvent(
	eEsifEventType eventType,
	UInt8 participantId,
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
static void EsifLogMgr_DestroyParticipantLogData(EsifLoggingManagerPtr self, Bool destroyFlag);
static void EsifLogMgr_DestroyEntry(EsifParticipantLogDataNodePtr curEntryPtr);
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
	self->listenersMask = 0;

	self->argc = 0;
	self->commandInfo = NULL;
	self->commandInfoCount = 0;
	self->logData = (char *)esif_ccb_malloc(MAX_LOG_DATA);
	if (NULL == self->logData) {
		ESIF_TRACE_DEBUG("Unable to allocate memory for log data");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_DPTF_PARTICIPANT_CONTROL_ACTION,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_MATCH_ANY,
		EsifLogMgr_EventCallback,
		self);

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_CREATE,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		self);

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND,
		0,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		self);

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME,
		0,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		self);

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		self);

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
	if ((self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) > 0) {
		EsifLogFile_Close(ESIF_LOG_PARTICIPANT);
	}
	/*
	 * Uninitialize the manager structure
	 */
	EsifLogMgr_Uninit(self);
}

void EsifLogMgr_ParticipantCreate(
	EsifLoggingManagerPtr self,
	UInt8 participantId
	)
{
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;
		
	ESIF_ASSERT(self != NULL);

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
				(curEntryPtr->participantId == participantId)) {
				EsifLogMgr_SendParticipantLogEvent(ESIF_EVENT_DPTF_PARTICIPANT_ACTIVITY_LOGGING_ENABLED,
					(UInt8)curEntryPtr->participantId,
					(UInt16)curEntryPtr->domainId,
					(1 << curEntryPtr->capabilityData.type)
					);
			}
			nodePtr = nodePtr->next_ptr;
		}
		esif_ccb_write_unlock(&self->participantLogData.listLock);

		//Update the header log to print the header info for this change
		self->isLogHeader = ESIF_TRUE;
	}
}

void EsifLogMgr_Suspend(EsifLoggingManagerPtr self)
{
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;
	
	ESIF_ASSERT(self != NULL);

	if ((self->isLogSuspended == ESIF_FALSE) &&
		(self->isInitialized == ESIF_TRUE) &&
		EsifLogMgr_IsDataAvailableForLogging(self)) {

		self->isLogSuspended = ESIF_TRUE;
		EsifLogMgr_ParticipantLogStop(self);

		esif_ccb_write_lock(&self->participantLogData.listLock);
		/*
		 * Loop through the complete list and send the disable event
		 */
		nodePtr = self->participantLogData.list->head_ptr;
		while (nodePtr != NULL) {
			curEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
			if (curEntryPtr != NULL) {
				EsifLogMgr_SendParticipantLogEvent(ESIF_EVENT_DPTF_PARTICIPANT_ACTIVITY_LOGGING_DISABLED,
					(UInt8)curEntryPtr->participantId,
					(UInt16)curEntryPtr->domainId,
					(1 << curEntryPtr->capabilityData.type)
					);
			}
			nodePtr = nodePtr->next_ptr;
		}
		esif_ccb_write_unlock(&self->participantLogData.listLock);
	}
}

void EsifLogMgr_Resume(EsifLoggingManagerPtr self)
{
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;

	ESIF_ASSERT(self != NULL);

	if ((self->isLogSuspended == ESIF_TRUE) &&
		(self->isInitialized == ESIF_TRUE) &&
		EsifLogMgr_IsDataAvailableForLogging(self)) {
		
		self->isLogSuspended = ESIF_FALSE;

		esif_ccb_write_lock(&self->participantLogData.listLock);
		/*
		 * Loop through the complete list and send the enable event
		 */
		nodePtr = self->participantLogData.list->head_ptr;
		while (nodePtr != NULL) {
			curEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
			if (curEntryPtr != NULL) {
				EsifLogMgr_SendParticipantLogEvent(ESIF_EVENT_DPTF_PARTICIPANT_ACTIVITY_LOGGING_ENABLED,
					(UInt8)curEntryPtr->participantId,
					(UInt16)curEntryPtr->domainId,
					(1 << curEntryPtr->capabilityData.type)
					);
			}
			nodePtr = nodePtr->next_ptr;
		}
		esif_ccb_write_unlock(&self->participantLogData.listLock);
		EsifLogMgr_ParticipantLogStart(self);
	}
}

void EsifLogMgr_ParticipantDestroy(
	EsifLoggingManagerPtr self,
	UInt8 participantId
	)
{
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;

	ESIF_ASSERT(self != NULL);

	if ((self->isInitialized == ESIF_TRUE) &&
		EsifLogMgr_IsDataAvailableForLogging(self)) {
		/*
		 * Loop through the complete list and send the enable event for the participantId
		 */
		esif_ccb_write_lock(&self->participantLogData.listLock);
		nodePtr = self->participantLogData.list->head_ptr;
		while (nodePtr != NULL) {
			curEntryPtr = (EsifParticipantLogDataNodePtr)nodePtr->data_ptr;
			if ((curEntryPtr != NULL) &&
				(curEntryPtr->participantId == participantId)) {
				EsifLogMgr_DestroyEntry(curEntryPtr);
				esif_link_list_node_remove(self->participantLogData.list, nodePtr);
			}
			nodePtr = nodePtr->next_ptr;
		}
		esif_ccb_write_unlock(&self->participantLogData.listLock);

		//Update the header log to print the header info for this change
		self->isLogHeader = ESIF_TRUE;
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

	if (argc <= PARTICITPANTLOG_CMD_INDEX) {
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
	UInt32 i = PARTICITPANTLOG_SUB_CMD_INDEX;

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

	rc = EsifLogMgr_GetInputParameters(self, shell, i);
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
	if ((self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) > 0) {
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

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(shell != NULL);
	ESIF_ASSERT(shell->outbuf != NULL);

	argc = shell->argc;
	argv = shell->argv;
	output = shell->outbuf;

	if ((UInt32)argc <= i) {
		rc = EsifLogMgr_OpenRouteTargetLogFileIfRequired(self);
		if (rc != ESIF_OK) {
			goto exit;
		}
		EsifLogMgr_PrintListenerStatus(self, output, OUT_BUF_LEN);
		goto exit;
	}

	//reset the listener mask if new route target is specified
	self->listenersMask = 0;
	self->isDefaultFile = ESIF_TRUE;

	//Close the old file
	EsifLogFile_Close(ESIF_LOG_PARTICIPANT);

	if (esif_ccb_stricmp(argv[i], ESIF_LISTENER_ALL_STR) == 0) {
		self->listenersMask = ESIF_LISTENER_ALL_MASK;
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Participant log target set to all\n");
	}
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
					//Pass NULL as file name for creating new file based on time stamp
					rc = EsifLogMgr_OpenParticipantLogFile(NULL);
					if (rc != ESIF_OK) {
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error: Unable to open/create log file. Exiting\n");
						self->listenersMask = 0;
						goto exit;
					}
				}
				else {
					//File name is given as input
					rc = EsifLogMgr_OpenParticipantLogFile(argv[i]);
					if (rc != ESIF_OK) {
						esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Error: Unable to open/create log file. Exiting\n");
						self->listenersMask = 0;
						goto exit;
					}
					self->isDefaultFile = ESIF_FALSE;
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
		//Update the header flag here if log is started already
		//otherwise not required
		self->isLogHeader = ESIF_TRUE;
	}

	EsifLogMgr_PrintListenerStatus(self, output, OUT_BUF_LEN);
exit:
	return rc;
}

eEsifError EsifLogMgr_ParseCmdInterval(
	EsifLoggingManagerPtr self,
	EsifShellCmdPtr shell
	)
{
	eEsifError rc = ESIF_OK;
	UInt16 interval = 0;
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
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	interval = (UInt16)esif_atoi(argv[i]);
	if ((Int16)interval < MIN_LOG_INTERVAL) {
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "Input interval value is less than minimum supported value %d ms \n", MIN_LOG_INTERVAL);
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	self->pollingThread.interval = interval;
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
		esif_ccb_sprintf_concat(OUT_BUF_LEN, output, "No Interval specified .Setting to default polling interval : %d ms\n", DEFAULT_SCHEDULE_DELAY_INTERVAL);
		self->logScheduler.delay = DEFAULT_SCHEDULE_DELAY_INTERVAL;
	}
	else {
		//Delay is specified as input
		self->logScheduler.delay = (UInt32)esif_atoi(argv[i]);
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

	argc = shell->argc;
	argv = shell->argv;
	output = shell->outbuf;

	self->argc = 0;
	// Default if no sub command is specified, or all is specified
	// start logging for all data for all available participants
	if (((UInt32)argc <= i) ||
		(esif_ccb_stricmp(argv[i], "all") == 0)) {
		self->argv[self->argc] = esif_ccb_strdup("all");
		if (self->argv[self->argc] == NULL) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
		self->argc++;
	}
	else {
		// if subcommand is anything other than all and argument count is only 3
		if ((i == ((UInt32)argc - 1) ||
			argc >= MAX_COMMAND_ARGUMENTS)) {
			ESIF_TRACE_ERROR("Error:Invalid input command.");
			rc = ESIF_E_INVALID_ARGUMENT_COUNT;
			goto exit;
		}
		else {
			for (; i < ((UInt32)argc - 1);) {

				if ((UInt32)argc < (i + START_CMD_TRIPLET)) {
					ESIF_TRACE_ERROR("Error:Invalid input command.");
					rc = ESIF_E_INVALID_ARGUMENT_COUNT;
					goto exit;
				}

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
		}
	}
exit:
	return rc;
}

static eEsifError EsifLogMgr_ValidateInputParameters(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;
	UInt32 i = 0;
	UInt32 commandTripletsCount = 0;
	UInt32 participantId = 0;
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
		self->commandInfo[0].participantId = ESIF_INVALID_DATA;
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
			rc = EsifLogMgr_GetDomainId(self->argv[i], (UInt8)participantId, &domainId);
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
	for (i = 0; i < (UInt32)self->argc; i++) {
		if (self->argv[i] != NULL) {
			esif_ccb_free(self->argv[i]);
			self->argv[i] = NULL;
		}
	}
	self->argc = 0;
	return rc;
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
		(self->commandInfo[0].participantId == ESIF_INVALID_DATA)) {
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
					(UInt8)self->commandInfo[i].participantId,
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

				upPtr = EsifUpPm_GetAvailableParticipantByInstance((UInt8)self->commandInfo[i].participantId);
				if (upPtr == NULL) {
					ESIF_TRACE_ERROR("EsifUpPm_GetAvailableParticipantByInstance() failed");
					rc = ESIF_E_NOT_SUPPORTED;
					goto exit;
				}

				domainCount = EsifUp_GetDomainCount(upPtr);
				for (domainId = 0; domainId < domainCount; domainId++) {
					rc = EsifLogMgr_AddDomain(self,
						(UInt8)self->commandInfo[i].participantId,
						(UInt8)domainId,
						self->commandInfo[i].capabilityMask
						);
					if (rc != ESIF_OK) {
						ESIF_TRACE_ERROR("EsifLogMgr_AddDomain() failed");
						goto exit;
					}
				}
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
	void *contextPtr,
	UInt8 participantId,
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
		(NULL == contextPtr)) {
		ESIF_TRACE_ERROR("input parameter is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	self = (EsifLoggingManagerPtr)contextPtr;

	switch (fpcEventPtr->esif_event) {
	case ESIF_EVENT_DPTF_PARTICIPANT_CONTROL_ACTION:

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
			ESIF_TRACE_ERROR("nodePtr is NULL");
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
		
		//Update the header flag for any new addition of capability Data
		if (capabilityEntryPtr->state == ESIF_DATA_CREATED) {
			self->isLogHeader = ESIF_TRUE;
		}
		EsifLogMgr_UpdateCapabilityData(capabilityEntryPtr, capabilityDataPtr);

		esif_ccb_write_unlock(&self->participantLogData.listLock);
		break;
	case ESIF_EVENT_PARTICIPANT_CREATE:
		if (participantId != ESIF_INSTANCE_LF) {
			EsifLogMgr_ParticipantCreate(self, participantId);
		}
		break;
	case ESIF_EVENT_PARTICIPANT_SUSPEND:
		EsifLogMgr_Suspend(self);
		break;
	case ESIF_EVENT_PARTICIPANT_RESUME:
		EsifLogMgr_Resume(self);
		break;
	case ESIF_EVENT_PARTICIPANT_UNREGISTER:
		if (participantId != ESIF_INSTANCE_LF) {
			EsifLogMgr_ParticipantDestroy(self, participantId);
		}
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
	int i = 0;
	ESIF_ASSERT(self != NULL);

	// Remove the old timer if any
	EsifLogMgr_DestroyScheduleTimer(self);

	/*
	 * Stop the polling thread
	 */
	EsifLogMgr_ParticipantLogStop(self);

	//reset listener mask to 0 if it was set to default file mask
	if (self->listenersMask == ESIF_LISTENER_LOGFILE_MASK) {
		self->listenersMask = 0;
	}

	//Close the old file
	EsifLogFile_Close(ESIF_LOG_PARTICIPANT);

	//Free the input argv
	for (i = 0; i < self->argc; i++) {
		if (self->argv[i] != NULL) {
			esif_ccb_free(self->argv[i]);
			self->argv[i] = NULL;
		}
	}
	self->argc = 0;

	if (self->commandInfo != NULL) {
		esif_ccb_free(self->commandInfo);
		self->commandInfo = NULL;
		self->commandInfoCount = 0;
	}
	// Remove the complete participant log data list nodes
	// No need to delete the list structure here for stop
	EsifLogMgr_DestroyParticipantLogData(self, ESIF_FALSE);

	return;
}

void EsifLogMgr_ParticipantLogStart(EsifLoggingManagerPtr self)
{
	ESIF_ASSERT(self != NULL);

	if (self->isLogStarted == ESIF_FALSE) {
		self->isLogStarted = ESIF_TRUE;
		self->isLogStopped = ESIF_FALSE;
		self->isLogHeader = ESIF_TRUE;
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
	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("EsifUpPm_InitIterator() failed with status : %s (%d)", esif_rc_str(rc), rc);
		goto exit;
	}

	rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	while (ESIF_OK == rc) {
		if (EsifUp_GetInstance(upPtr) != ESIF_INSTANCE_LF) {
			//Add participant only if participant id is not 0 as 
			// DPTF/IETM participant has no data to log
			rc = EsifLogMgr_AddParticipant(self, upPtr);
			if (rc != ESIF_OK) {
				ESIF_TRACE_ERROR("EsifLogMgr_AddParticipant() failed with status : %s (%d)", esif_rc_str(rc), rc);
				goto exit;
			}
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
	UInt8 participantId = 0;
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
	UInt8 participantId,
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

	//MASK out the status capability bits
	capabilityMask = capabilityMask & CONTROL_CAPABIILTY_MASK;

	EsifLogMgr_SendParticipantLogEvent(
		ESIF_EVENT_DPTF_PARTICIPANT_ACTIVITY_LOGGING_ENABLED,
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
			rc = EsifLogMgr_AddCapability(self, EsifUp_GetInstance(upPtr), EsifUp_GetDomainId(domainPtr), capabilityId);
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
	UInt32 participantId,
	UInt32 domainId,
	UInt32 capabilityId
	)
{
	eEsifError rc = ESIF_OK;
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr newEntryPtr = NULL;

	ESIF_ASSERT(self != NULL);

	/*
	 * Check If a matching entry is already present
	 */
	esif_ccb_write_lock(&self->participantLogData.listLock);
	nodePtr = EsifLogMgr_GetCapabilityNodeWLock(self, participantId, domainId, capabilityId);
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

	newEntryPtr->participantId = participantId;
	newEntryPtr->domainId = domainId;
	esif_ccb_lock_init(&newEntryPtr->capabilityDataLock);
	newEntryPtr->state = ESIF_DATA_CREATED;

	newEntryPtr->capabilityData.type = capabilityId;
	newEntryPtr->capabilityData.size = sizeof(newEntryPtr->capabilityData);

	//If it is STATUS capability Update the data now itself and change the state to Initialized
	if (EsifLogMgr_IsStatusCapable(newEntryPtr->capabilityData.type)) {
		newEntryPtr->state = ESIF_DATA_INITIALIZED;
		EsifLogMgr_UpdateStatusCapabilityData(newEntryPtr);
	}

	rc = EsifLogMgr_AddParticipantDataListEntry(self, newEntryPtr);

exit:
	if (rc != ESIF_OK) {
		esif_ccb_lock_uninit(&newEntryPtr->capabilityDataLock);
		esif_ccb_free(newEntryPtr);
	}
	return rc;
}

static EsifLinkListNodePtr EsifLogMgr_GetCapabilityNodeWLock(
	EsifLoggingManagerPtr self,
	UInt32 participantId,
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
			ESIF_TRACE_DEBUG("Found a matching entry participant Id : %d domain Id : %d capability Id : %d : %s in the list",
				participantId,
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
	if (rc != ESIF_OK) {
		EsifLogMgr_DataLogWrite(self, "\nError code : %s(%d)", esif_rc_str(rc), rc);		
		EsifLogMgr_DataLogWrite(self, "\nStopped participant logging");
		EsifLogMgr_DataLogWrite(self, "\n");
		EsifLogMgr_ParticipantLogStop(self);
	}
}

static eEsifError EsifLogMgr_OpenRouteTargetLogFileIfRequired(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	//if route is not set at all
	// set route's default value to file
	if (self->listenersMask == 0) {
		self->listenersMask = ESIF_LISTENER_LOGFILE_MASK;
	}
	
	if ((self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) == ESIF_LISTENER_LOGFILE_MASK) {
		if (self->isDefaultFile == ESIF_FALSE) {
			//Pass input file name for creating new file
			rc = EsifLogMgr_OpenParticipantLogFile(EsifLogFile_GetFileNameFromType(ESIF_LOG_PARTICIPANT));
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
	UInt32 *participantIdPtr
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 index = 0;
	EsifUpPtr upPtr = NULL;

	ESIF_ASSERT(participantstr != NULL);
	ESIF_ASSERT(participantIdPtr != NULL);

	//Parse the participant and check if it is available
	if (((int)esif_atoi(participantstr) > 0) ||
		(esif_ccb_strcmp(participantstr, "0") == 0)) {
		index = (UInt32)esif_atoi(participantstr);
		upPtr = EsifUpPm_GetAvailableParticipantByInstance((UInt8)index);
	}
	else {
		upPtr = EsifUpPm_GetAvailableParticipantByName(participantstr);
		if (upPtr != NULL) {
			index = EsifUp_GetInstance(upPtr);
		}
	}
	//Throw error if participant not found
	if (NULL == upPtr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		ESIF_TRACE_ERROR("Error:Participant Not Found");
		goto exit;
	}

	//Throw error if participant ID is 0 as DPTF/IETM has no data to log
	if (ESIF_INSTANCE_LF == index) {
		rc = ESIF_E_INVALID_PARTICIPANT_ID;
		ESIF_TRACE_ERROR("Error: Participant Id 0 is not valid");
		goto exit;
	}

exit:
	if (rc == ESIF_OK) {
		*participantIdPtr = index;
	}

	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}

static eEsifError EsifLogMgr_GetDomainId(
	char *domainstr, 
	UInt8 participantId,
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
		if (self->listenersMask > 0) {
			esif_ccb_system_time(&msecStart);
			//Header needs to be updated
			if (self->isLogHeader) {
				EsifLogMgr_ParticipantLogFire(self);
				self->isLogHeader = ESIF_FALSE;
			}
			else {
				EsifLogMgr_ParticipantLogFire(self);
			}
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

static void EsifLogMgr_ParticipantLogFire(
	EsifLoggingManagerPtr self
	)
{
	time_t now = time(NULL);
	struct tm time = { 0 };
	esif_ccb_time_t msec = 0;
	EsifLinkListNodePtr nodePtr = NULL;
	EsifParticipantLogDataNodePtr curEntryPtr = NULL;
	UInt32 currentParticipantId = (UInt32)-1;
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
			if (curEntryPtr->state >= ESIF_DATA_INITIALIZED) {
				if (self->isLogHeader != ESIF_FALSE) {
					if (printTimeInfo != ESIF_FALSE) {
						esif_ccb_sprintf(dataLength, self->logData, " Date, Time, Server Msec,");
						printTimeInfo = ESIF_FALSE;
					}
					if (currentParticipantId != curEntryPtr->participantId) {
						esif_ccb_sprintf_concat(dataLength, self->logData, " Participant Index, Participant Name, Domain Id,");
					}
					else if ((currentParticipantId == curEntryPtr->participantId) &&
						(currentDomainId != curEntryPtr->domainId)) {
						esif_ccb_sprintf_concat(dataLength, self->logData, " Domain Id,");
					}

					partName = "UNK";
					upPtr = EsifUpPm_GetAvailableParticipantByInstance((UInt8)curEntryPtr->participantId);
					if (upPtr != NULL) {
						partName = EsifUp_GetName(upPtr);
						EsifUp_PutRef(upPtr);
					}
					EsifDomainIdToIndex((UInt16)curEntryPtr->domainId, &domainIndex);
					esif_ccb_read_lock(&curEntryPtr->capabilityDataLock);
					EsifLogMgr_ParticipantLogAddHeaderData(self->logData, dataLength, &curEntryPtr->capabilityData, partName, domainIndex);
					esif_ccb_read_unlock(&curEntryPtr->capabilityDataLock);
				}
				else
				{
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
						EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByInstance((UInt8)curEntryPtr->participantId);
						if (upPtr != NULL) {
							esif_ccb_sprintf_concat(dataLength, self->logData, " %d, %s, %d,", curEntryPtr->participantId, EsifUp_GetName(upPtr), domainIndex);
							EsifUp_PutRef(upPtr);
						}
					}
					else if ((currentParticipantId == curEntryPtr->participantId) &&
						(currentDomainId != curEntryPtr->domainId)) {
						esif_ccb_sprintf_concat(dataLength, self->logData, " %d,", domainIndex);
					}
					EsifLogMgr_ParticipantLogAddDataNode(self->logData, dataLength, curEntryPtr);
				}
				currentParticipantId = curEntryPtr->participantId;
				currentDomainId = curEntryPtr->domainId;
			}
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
		esif_ccb_sprintf_concat(dataLength, logString, " ControlID, Speed,");
		break;
	case ESIF_CAPABILITY_TYPE_CTDP_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " ControlId, Tdp Frequency, Tdp Power, Tdp Ratio,");
		break;
	case ESIF_CAPABILITY_TYPE_CORE_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " Active Cores, Lower Limit, Upper Limit,");
		break;
	case ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " Brightness Limit, Lower Limit, Upper Limit,");
		break;
	case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
		esif_ccb_sprintf_concat(dataLength, logString, " %s_D%dPriority,",participantName,domainId);
		break;
	case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %s_D%d_PState Index, %s_D%d_Lower Limit, %s_D%d_Upper Limit,", 
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
			if ((capabilityPtr->data.powerControl.powerDataSet[powerType].isEnabled != ESIF_FALSE) &&
				(capabilityPtr->data.powerControl.powerDataSet[powerType].powerType <= MAX_POWER_CONTROL_TYPE)) {
				esif_ccb_sprintf_concat(dataLength, logString, " PL%d Limit(mW), PL%d LowerLimit(mW), PL%d UpperLimit(mW), Stepsize(mW),"
					"Minimum TimeWindow(ms), Maximum TimeWindow(ms), Minimum DutyCycle, Maximum DutyCycle,",
					capabilityPtr->data.powerControl.powerDataSet[powerType].powerType,
					capabilityPtr->data.powerControl.powerDataSet[powerType].powerType,
					capabilityPtr->data.powerControl.powerDataSet[powerType].powerType
					);
			}
		}
		break;
	}
	case ESIF_CAPABILITY_TYPE_POWER_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " %s_D%d_Power(mW),",participantName,domainId);
		break;
	case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " %s_D%dTemperature(C),", participantName,domainId);
		break;
	case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " %s_D%d_Utilization,",participantName,domainId);
		break;
	case ESIF_CAPABILITY_TYPE_PIXELCLOCK_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " Pixel Clock Status,");
		break;
	case ESIF_CAPABILITY_TYPE_PIXELCLOCK_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " Pixel Clock Control,");
		break;
	case ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " Platform Power Status,");
		break;
	case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
		esif_ccb_sprintf_concat(dataLength, logString, " %s_D%d_Aux0(C), %s_D%d_Aux1(C), %s_D%d_Hysteresis(C),", 
			participantName, 
			domainId, 
			participantName, 
			domainId, 
			participantName, 
			domainId
			);
		break;
	case ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " RF Profile Frequency Status,");
		break;
	case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " RF Profile Frequency Control,");
		break;
	case ESIF_CAPABILITY_TYPE_NETWORK_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " Network Control,");
		break;
	case ESIF_CAPABILITY_TYPE_XMITPOWER_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " Transmit Power Control,");
		break;
	case ESIF_CAPABILITY_TYPE_HDC_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " Hdc Duty Cycle, Hdc Status,");
		break;
	case ESIF_CAPABILITY_TYPE_PSYS_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " Limit Type, Power Limit, Duty Cycle, Time Window,");
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
	EsifLogMgr_ParticipantLogAddCapabilityData(logString, dataLength, &dataNodePtr->capabilityData);
	esif_ccb_read_unlock(&dataNodePtr->capabilityDataLock);

	return rc;
}

static eEsifError EsifLogMgr_ParticipantLogAddCapabilityData(
	char *logString,
	size_t dataLength,
	EsifCapabilityDataPtr capabilityPtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(logString != NULL);
	ESIF_ASSERT(capabilityPtr != NULL);

	switch (capabilityPtr->type) {
	case ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u, %u,",
			capabilityPtr->data.activeControl.controlId,
			capabilityPtr->data.activeControl.speed
			);
		break;
	case ESIF_CAPABILITY_TYPE_CTDP_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, "%llu, %llu, %llu, %llu,",
			capabilityPtr->data.configTdpControl.controlId,
			capabilityPtr->data.configTdpControl.tdpFrequency,
			capabilityPtr->data.configTdpControl.tdpPower,
			capabilityPtr->data.configTdpControl.tdpRatio
			);
		break;
	case ESIF_CAPABILITY_TYPE_CORE_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u, %u, %u,",
			capabilityPtr->data.coreControl.activeLogicalProcessors,
			capabilityPtr->data.coreControl.maximumActiveCores,
			capabilityPtr->data.coreControl.minimumActiveCores
			);
		break;
	case ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u, %u, %u,",
			capabilityPtr->data.displayControl.currentDPTFLimit,
			capabilityPtr->data.displayControl.lowerLimit,
			capabilityPtr->data.displayControl.upperLimit
			);
		break;
	case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.domainPriority.priority
			);
		break;
	case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u, %u, %u,",
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
			if ((capabilityPtr->data.powerControl.powerDataSet[powerType].isEnabled != ESIF_FALSE) &&
				(capabilityPtr->data.powerControl.powerDataSet[powerType].powerType <= MAX_POWER_CONTROL_TYPE)) {
				esif_ccb_sprintf_concat(dataLength, logString, " %u, %u, %u, %u, %u, %u, %u, %u,",
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
		}
		break;
	}
	case ESIF_CAPABILITY_TYPE_POWER_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.powerStatus.power
			);
		break;
	case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.temperatureStatus.temperature
			);
		break;
	case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.utilizationStatus.utilization
			);
		break;
	case ESIF_CAPABILITY_TYPE_PIXELCLOCK_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.pixelClockStatus.pixelClockStatus
			);
		break;
	case ESIF_CAPABILITY_TYPE_PIXELCLOCK_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.pixelClockControl.pixelClockControl
			);
		break;
	case ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.platformPowerStatus.platformPowerStatus
			);
		break;
	case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
		esif_ccb_sprintf_concat(dataLength, logString, " %u, %u, %u,",
			capabilityPtr->data.temperatureControl.aux0,
			capabilityPtr->data.temperatureControl.aux1,
			capabilityPtr->data.temperatureControl.hysteresis
			);
		break;
	case ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.rfProfileStatus.rfProfileFrequency
			);
		break;
	case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.rfProfileControl.rfProfileFrequency
			);
		break;
	case ESIF_CAPABILITY_TYPE_NETWORK_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.networkControl.networkControl
			);
		break;
	case ESIF_CAPABILITY_TYPE_XMITPOWER_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u,",
			capabilityPtr->data.xmitPowerControl.xmitPowerControl
			);
		break;
	case ESIF_CAPABILITY_TYPE_HDC_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u, %u,",
			capabilityPtr->data.HdcControl.hdcDutyCycle,
			capabilityPtr->data.HdcControl.hdcStatus
			);
		break;
	case ESIF_CAPABILITY_TYPE_PSYS_CONTROL:
		esif_ccb_sprintf_concat(dataLength, logString, " %u, %u, %u, %u,",
			capabilityPtr->data.psysControl.powerLimitType,
			capabilityPtr->data.psysControl.powerLimit,
			capabilityPtr->data.psysControl.PowerLimitDutyCycle,
			capabilityPtr->data.psysControl.PowerLimitTimeWindow
			);
		break;
	}

	return rc;
}

static void EsifLogMgr_UpdateStatusCapabilityData(EsifParticipantLogDataNodePtr dataNodePtr)
{
	eEsifError rc = ESIF_OK;
	char qualifierStr[MAX_NAME_STRING_LENGTH] = "D0";

	ESIF_ASSERT(dataNodePtr != NULL);

	switch (dataNodePtr->capabilityData.type) {
	case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
	{
		UInt32 temp = 0;
		struct esif_data temp_response = { ESIF_DATA_TEMPERATURE, &temp, sizeof(temp), sizeof(temp) };

		rc = EsifExecutePrimitive((u8)dataNodePtr->participantId, GET_TEMPERATURE, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &temp_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_TEMPERATURE primitive for participant %d domain : %d", dataNodePtr->participantId, dataNodePtr->domainId);
			temp = 0;
		}
		dataNodePtr->capabilityData.data.temperatureStatus.temperature = temp;
		break;
	}
	case ESIF_CAPABILITY_TYPE_POWER_STATUS:
	{
		UInt32 power = 0;
		struct esif_data power_response = { ESIF_DATA_POWER, &power, sizeof(power), sizeof(power) };

		rc = EsifExecutePrimitive((u8)dataNodePtr->participantId, GET_RAPL_POWER, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &power_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_RAPL_POWER primitive for participant %d domain : %d", dataNodePtr->participantId, dataNodePtr->domainId);
			power = 0;
		}
		dataNodePtr->capabilityData.data.powerStatus.power = power;
		break;
	}
	case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
	{
		UInt32 utilization = 0;
		struct esif_data util_response = { ESIF_DATA_PERCENT, &utilization, sizeof(utilization), sizeof(utilization) };

		rc = EsifExecutePrimitive((u8)dataNodePtr->participantId, GET_PARTICIPANT_UTILIZATION, esif_primitive_domain_str((u16)dataNodePtr->domainId, qualifierStr, MAX_NAME_STRING_LENGTH), ESIF_INSTANCE_INVALID, NULL, &util_response);
		if (ESIF_OK != rc) {
			ESIF_TRACE_INFO("Error while executing GET_PARTICIPANT_UTILIZATION primitive for participant %d domain : %d", dataNodePtr->participantId, dataNodePtr->domainId);
			utilization = 0;
		}
		dataNodePtr->capabilityData.data.utilizationStatus.utilization = utilization / 100;
		break;
	}
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
	eEsifError rc = ESIF_OK;
	va_list args;

	ESIF_ASSERT(self != NULL);

	if (logstring == NULL) {
		ESIF_TRACE_ERROR("logstring is NULL");
		goto exit;
	}

	if (self->listenersMask == 0) {
		goto exit;
	}

	if ((self->listenersMask & ESIF_LISTENER_CONSOLE_MASK) > 0) {
		va_start(args, logstring);
		rc += EsifConsole_WriteConsole(logstring, args);
		va_end(args);
	}
	if ((self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) > 0) {
		va_start(args, logstring);
		rc += EsifLogFile_WriteArgsAppend(ESIF_LOG_PARTICIPANT, " ", logstring, args);
		va_end(args);
	}
	if ((self->listenersMask & ESIF_LISTENER_DEBUGGER_MASK) > 0) {
		size_t  msglen = 0;
		char *buffer = 0;

		va_start(args, logstring);
		msglen = esif_ccb_vscprintf(logstring, args) + 1;
		va_end(args);

		buffer = (char *)esif_ccb_malloc(msglen);

		if (NULL != buffer) {
			va_start(args, logstring);
			rc += esif_ccb_vsprintf(msglen, buffer, logstring, args);
			va_end(args);

			EsifLogMgr_LogToDebugger(buffer);
			esif_ccb_free(buffer);
		}
	}
	if ((self->listenersMask & ESIF_LISTENER_EVENTLOG_MASK) > 0) {
		size_t  msglen = 0;
		char *buffer = 0;

		va_start(args, logstring);
		msglen = esif_ccb_vscprintf(logstring, args) + 1;
		va_end(args);

		buffer = (char *)esif_ccb_malloc(msglen);

		if (NULL != buffer) {
			va_start(args, logstring);
			rc += esif_ccb_vsprintf(msglen, buffer, logstring, args);
			va_end(args);

			EsifLogMgr_LogToEvent(buffer);
			esif_ccb_free(buffer);
		}
	}

exit:
	return;
}

static eEsifError EsifLogMgr_Uninit(EsifLoggingManagerPtr self)
{
	eEsifError rc = ESIF_OK;
	int i = 0;

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
	for (i = 0; i < self->argc; i++) {
		if (self->argv[i] != NULL) {
			esif_ccb_free(self->argv[i]);
			self->argv[i] = NULL;
		}
	}
	self->argc = 0;

	if (self->commandInfo != NULL) {
		esif_ccb_free(self->commandInfo);
		self->commandInfo = NULL;
		self->commandInfoCount = 0;
	}
	
	EsifLogMgr_DestroyScheduleTimer(self);
	
	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_DPTF_PARTICIPANT_CONTROL_ACTION,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_MATCH_ANY,
		EsifLogMgr_EventCallback,
		self);

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_CREATE,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		self);

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_SUSPEND,
		0,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		self);

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_RESUME,
		0,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		self);

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifLogMgr_EventCallback,
		self);

	EsifLogMgr_DestroyParticipantLogData(self, ESIF_TRUE);

exit:
	if ((self != NULL) &&
		(self->isInitialized != ESIF_FALSE)) {
		esif_ccb_lock_uninit(&self->participantLogData.listLock);
		self->isInitialized = ESIF_FALSE;
	}
	return rc;
}

static void EsifLogMgr_DestroyParticipantLogData(
	EsifLoggingManagerPtr self,
	Bool destroyFlag
	)
{
	ESIF_ASSERT(self != NULL);

	if ((NULL == self->participantLogData.list) ||
		(NULL == self->participantLogData.list->head_ptr)) {
		ESIF_TRACE_INFO("List is NULL");
		goto exit;
	}
	
	esif_ccb_write_lock(&self->participantLogData.listLock);
	//Delete and Free the nodes in the list
	esif_link_list_free_data(self->participantLogData.list, (link_list_data_destroy_func)EsifLogMgr_DestroyEntry);		

	if (destroyFlag != ESIF_FALSE) {
		//Destroy the list structure if this flag is set to true
		esif_ccb_free(self->participantLogData.list);
		self->participantLogData.list = NULL;
	}
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
		ESIF_EVENT_DPTF_PARTICIPANT_ACTIVITY_LOGGING_DISABLED,
		(UInt8)curEntryPtr->participantId,
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
	char fullpath[MAX_PATH] = { 0 };	

	if (fileName == NULL) {
		time_t now = time(NULL);
		struct tm time = { 0 };
		if (esif_ccb_localtime(&time, &now) == 0) {
			esif_ccb_sprintf(sizeof(logname), logname, "participant_log_%04d-%02d-%02d-%02d%02d%02d.csv",
				time.tm_year + TIME_BASE_YEAR, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
		}
	}
	else {
		char *fileExtn = esif_ccb_strchr(fileName, '.');
		if (fileExtn == NULL) {
			esif_ccb_sprintf(sizeof(logname), logname, "%s.csv", fileName);
		}
		else {
			esif_ccb_sprintf(sizeof(logname), logname, "%s", fileName);
		}
	}
	EsifLogFile_Open(logtype, logname, append);
	EsifLogFile_GetFullPath(fullpath, sizeof(fullpath), logname);
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
	UInt8 participantId, 
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
			"Log state     : Started\n"\
			"Log interval  : %d ms\n",
			self->pollingThread.interval
			);
		EsifLogMgr_PrintListenerStatus(self, output, datalength);
	}
	else {
		esif_ccb_sprintf_concat(datalength, output,
			"Log state     : Stopped\n");
	}
}

static void EsifLogMgr_PrintListenerStatus(
	EsifLoggingManagerPtr self,
	char *output,
	size_t datalength
	)
{
	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(output != NULL);

	if (self->listenersMask > 0) {
		esif_ccb_strcat(output,
			"Log route     : ",
			OUT_BUF_LEN
			);

		if ((self->listenersMask & ESIF_LISTENER_EVENTLOG_MASK) == ESIF_LISTENER_EVENTLOG_MASK) {
			esif_ccb_strcat(output, ESIF_LISTENER_EVENTLOG_STR, datalength);
		}
		esif_ccb_strcat(output, " ", datalength);
		if ((self->listenersMask & ESIF_LISTENER_DEBUGGER_MASK) == ESIF_LISTENER_DEBUGGER_MASK) {
			esif_ccb_strcat(output, ESIF_LISTENER_DEBUGGER_STR, datalength);
		}
		esif_ccb_strcat(output, " ", datalength);
		if ((self->listenersMask & ESIF_LISTENER_CONSOLE_MASK) == ESIF_LISTENER_CONSOLE_MASK) {
			esif_ccb_strcat(output, ESIF_LISTENER_CONSOLE_STR, datalength);
		}
		esif_ccb_strcat(output, " ", datalength);
		if ((self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) == ESIF_LISTENER_LOGFILE_MASK) {
			esif_ccb_strcat(output, ESIF_LISTENER_LOGFILE_STR, datalength);
		}
		esif_ccb_strcat(output, "\n", datalength);
		if ((self->listenersMask & ESIF_LISTENER_LOGFILE_MASK) == ESIF_LISTENER_LOGFILE_MASK) {
			esif_ccb_strcat(
				output,
				"Log File Name : ",
				datalength
				);

			if (EsifLogFile_GetFileNameFromType(ESIF_LOG_PARTICIPANT)) {             
					esif_ccb_strcat(
					output,
					EsifLogFile_GetFileNameFromType(ESIF_LOG_PARTICIPANT),
					datalength
					);
			}
			else {
				esif_ccb_strcat(
					output,
					"NA",
					datalength
					);
			}
			esif_ccb_strcat(output, "\n", datalength);
		}
	}
}