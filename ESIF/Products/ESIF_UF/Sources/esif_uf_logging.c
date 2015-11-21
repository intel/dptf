/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_SHELL

#include "esif_uf_logging.h"
#include "esif_uf_trace.h"
#include "esif_pm.h"
#include "esif_participant.h"
#include "esif_dsp.h"
#include "esif_uf_fpc.h"
#include "esif_uf_tableobject.h"
#include "esif_uf_primitive.h"
#include "esif_uf_ccb_timedwait.h"


#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define ESIF_DATALOG_LOCK_SLEEP_MS 1

static atomic_t g_dataLogLock = ATOMIC_INIT(0);
static Bool g_dataLogActive = ESIF_FALSE;
static Bool g_dataLogQuit = ESIF_FALSE;
static esif_ccb_event_t g_dataLogQuitEvent;
static esif_thread_t g_dataLogThread;  //upper framework polling thread
static DataLogParticipant g_dataLogParticipants[MAX_PARTICIPANT_ENTRY];
static UInt32 g_dataLogInterval = DEFAULT_STATUS_LOG_INTERVAL;


static void EsifDataLogExit(esif_thread_t *dataLogThread);
static void EsifDataLogFire(int isHeader);
static eEsifError EsifDataLogValidateParticipantList(char *inParticipantList);
static eEsifError EsifDataLogEnableAllParticipants();
static eEsifError EsifDataLogEnableParticipantsFromList(char *inParticipantList);
static int EsifDataLogGetCapabilityFieldCount(EsifUpPtr upPtr);
static eEsifError EsifDataLogAddParticipantToList(EsifUpPtr upPtr, int indexPosition);
static eEsifError EsifDataLogAddParticipant(char *logString, int logColumn, int isHeader);
static void *ESIF_CALLCONV EsifDataLogWorkerThread(void *ptr);
static Bool EsifDataIsLogStarted();
static eEsifError EsifDataLogOpenLogFile();
static void EsifDataLogStart();
static void EsifDataLogSchedule(const void *ctx);
static void EsifDataLogCleanup(void);

struct dataLogContext {
	UInt32 dataLogInterval;
	esif_ccb_timer_t *dataLogScheduleTimer;
	char *dataLogParticipantList;
};
struct dataLogContext *dataLogContextPtr = NULL;

//
// PUBLIC INTERFACE---------------------------------------------------------------------
//
char *EsifShellCmdDataLog(EsifShellCmdPtr shell)
{
	eEsifError rc = ESIF_OK;
	int argc = shell->argc;
	char **argv = shell->argv;
	char *output = shell->outbuf;
	char *participantListPtr = NULL;


	/*
	 * Serialize access to the datalog state.
	 */
	while (atomic_set(&g_dataLogLock, ESIF_TRUE)) {
		esif_ccb_sleep_msec(ESIF_DATALOG_LOCK_SLEEP_MS);
	}

	if (argc < 2) {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Data logging is: %s\n", (EsifDataIsLogStarted() ? "started" : "stopped"));
	}
	else if (esif_ccb_stricmp(argv[1], "start") == 0) {

		if (EsifDataIsLogStarted()){
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Data logging is already started.\n");
			goto exit;
		}

		g_dataLogInterval = DEFAULT_STATUS_LOG_INTERVAL;
		if (argc > 2) {
			if ((int)esif_atoi(argv[2]) >= MIN_STATUS_LOG_INTERVAL) {
				g_dataLogInterval = (esif_ccb_time_t)esif_atoi(argv[2]);
			}
			else {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid sampling period specified (minimum is %d ms).\n", MIN_STATUS_LOG_INTERVAL);
				goto exit;
			}
		}
		if (argc > 3) {
			participantListPtr = (char *)esif_ccb_malloc(MAX_LOG_LINE);
			if (NULL == participantListPtr) {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "Error:  Unable to allocate memory for participant list.\n");
				goto exit;
			}

			esif_ccb_sprintf(MAX_LOG_LINE, participantListPtr, "%s", argv[3]);
		}

		rc = EsifDataLogValidateParticipantList(participantListPtr);
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "There was a problem with your participant list. You may have selected invalid participants, or too many participants. \n");
			goto exit;
		}
		rc = EsifDataLogOpenLogFile();
		if (rc != ESIF_OK) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error opening log file... \n");
			goto exit;
		}
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Data logging starting... \n");

		EsifDataLogStart();
	}
	else if (esif_ccb_stricmp(argv[1], "schedule") == 0) {
		UInt32 startTime = 1000;

		/* initialize */
		if (dataLogContextPtr == NULL) {
			dataLogContextPtr = (struct dataLogContext *)esif_ccb_malloc(sizeof(struct dataLogContext));
			if (dataLogContextPtr == NULL) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
			dataLogContextPtr->dataLogScheduleTimer = (esif_ccb_timer_t *) esif_ccb_malloc(sizeof(esif_ccb_timer_t));
			if (dataLogContextPtr->dataLogScheduleTimer == NULL) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
			dataLogContextPtr->dataLogParticipantList = NULL;
		}

		dataLogContextPtr->dataLogInterval = DEFAULT_STATUS_LOG_INTERVAL;
		/* start time (in ms from now) */
		if (argc > 2) {
			if ((int) esif_atoi(argv[2]) >= MIN_STATUS_LOG_SCHEDULE) {
				startTime = (esif_ccb_time_t) esif_atoi(argv[2]);
			}
			else {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid schedule time specified (minimum is %d ms).\n", MIN_STATUS_LOG_INTERVAL);
				goto exit;
			}
		}
		/* log interval */
		if (argc > 3) {
			if ((int) esif_atoi(argv[3]) >= MIN_STATUS_LOG_INTERVAL) {
				dataLogContextPtr->dataLogInterval = (esif_ccb_time_t) esif_atoi(argv[3]);
			}
			else {
				esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid sampling period specified (minimum is %d ms).\n", MIN_STATUS_LOG_INTERVAL);
				goto exit;
			}
		}
		if (argc > 4) {
			dataLogContextPtr->dataLogParticipantList = esif_ccb_strdup(argv[4]);
			if (dataLogContextPtr->dataLogParticipantList == NULL) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
		}

		rc = esif_ccb_timer_init(dataLogContextPtr->dataLogScheduleTimer, (esif_ccb_timer_cb) EsifDataLogSchedule, NULL);
		if (ESIF_OK != rc) {
			esif_ccb_sprintf(OUT_BUF_LEN, output, "Error starting timer... \n");
			goto exit;
		}
		rc = esif_ccb_timer_set_msec(dataLogContextPtr->dataLogScheduleTimer, startTime);
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Data logging scheduled for start in %d ms at an interval of %d... \n", startTime, dataLogContextPtr->dataLogInterval);
		
	}
	else if (esif_ccb_stricmp(argv[1], "stop") == 0) {
		EsifDataLogStop();
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Data logging stopped...\n");
		g_dataLogActive = ESIF_FALSE;
	}
	else {
		esif_ccb_sprintf(OUT_BUF_LEN, output, "Invalid parameter specified\n");
	}
exit:
	esif_ccb_free(participantListPtr);
	atomic_set(&g_dataLogLock, ESIF_FALSE);
	return output;
}


void EsifDataLogStop()
{
	if (EsifDataIsLogStarted()) {
		EsifDataLogExit(&g_dataLogThread);
	}

	if (EsifLogFile_IsOpen(ESIF_LOG_UI)) {
		EsifLogFile_Close(ESIF_LOG_UI);
	}

	EsifDataLogCleanup();
}

static void EsifDataLogSchedule(const void *ctx)
{
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(ctx);

	if (dataLogContextPtr == NULL) {
		ESIF_TRACE_DEBUG("Data logging schedule fired had no context.\n");
		goto exit;
	}

	if (dataLogContextPtr->dataLogScheduleTimer == NULL) {
		ESIF_TRACE_DEBUG("Data logging schedule fired but had no reference to the members.\n");
		goto exit;
	}

	/*
	* Serialize access to the datalog state.
	*/
	while (atomic_set(&g_dataLogLock, ESIF_TRUE)) {
		esif_ccb_sleep_msec(ESIF_DATALOG_LOCK_SLEEP_MS);
	}

	if (EsifDataIsLogStarted()){
		ESIF_TRACE_DEBUG("Data logging schedule fired but was already started.\n");
		goto exit;
	}

	rc = EsifDataLogValidateParticipantList(dataLogContextPtr->dataLogParticipantList);
	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("There was a problem with your participant list. You may have selected invalid participants, or too many participants. \n");
		goto exit;
	}
	rc = EsifDataLogOpenLogFile();
	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("Error opening scheduled log file... \n");
		goto exit;
	}
	ESIF_TRACE_DEBUG("Data logging starting... \n");
	g_dataLogInterval = dataLogContextPtr->dataLogInterval;
	EsifDataLogStart();

exit:
	EsifDataLogCleanup();
	atomic_set(&g_dataLogLock, ESIF_FALSE);
}

static void EsifDataLogCleanup()
{
	if (dataLogContextPtr != NULL) {
		esif_ccb_timer_kill(dataLogContextPtr->dataLogScheduleTimer);
		esif_ccb_free(dataLogContextPtr->dataLogScheduleTimer);
		esif_ccb_free(dataLogContextPtr->dataLogParticipantList);
		esif_ccb_free(dataLogContextPtr);
		dataLogContextPtr = NULL;
	}
}

void EsifDataLogStart()
{
	/* write header */
	EsifDataLogFire(ESIF_TRUE);

	g_dataLogQuit = ESIF_FALSE;
	esif_ccb_event_init(&g_dataLogQuitEvent);
	esif_ccb_thread_create(&g_dataLogThread, EsifDataLogWorkerThread, NULL);
	g_dataLogActive = ESIF_TRUE;
}


//
// PRIVATE FUNCTIONS---------------------------------------------------------------------
//

static eEsifError EsifDataLogOpenLogFile()
{
	eEsifError rc = ESIF_OK;
	EsifLogType logtype = ESIF_LOG_UI;
	char logname[35] = { 0 };
	int append = ESIF_FALSE;
	char fullpath[MAX_PATH] = { 0 };
	time_t now = time(NULL);
	struct tm time = { 0 };

	if (esif_ccb_localtime(&time, &now) == 0) {
		esif_ccb_sprintf(sizeof(logname), logname, "data_log_%04d-%02d-%02d-%02d%02d%02d.csv",
			time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
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


static Bool EsifDataIsLogStarted()
{
	return g_dataLogActive;
}


static void *ESIF_CALLCONV EsifDataLogWorkerThread(void *ptr)
{
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(ptr);

	ESIF_TRACE_ENTRY_INFO();

	while (!g_dataLogQuit) {
		esif_ccb_time_t msecStart = 0;
		esif_ccb_time_t msecStop = 0;
		esif_ccb_time_t msecDif = 0;
		UInt32 sleepMs = 0;

		esif_ccb_system_time(&msecStart);
		EsifDataLogFire(0);
		esif_ccb_system_time(&msecStop);

		/* Determine how long processing actually took */
		msecDif = msecStop - msecStart;

		/* Determine next sleep time based on previous processing time*/
		sleepMs = g_dataLogInterval - ((int) msecDif);
		if ((sleepMs < MIN_STATUS_LOG_INTERVAL_ADJUSTED) || (msecDif > g_dataLogInterval)){
			sleepMs = MIN_STATUS_LOG_INTERVAL_ADJUSTED;
		}

		rc = EsifTimedEventWait(&g_dataLogQuitEvent, sleepMs);
		if (rc != ESIF_OK) {
			ESIF_TRACE_ERROR("Error waiting on data log event\n");
			goto exit;
		}
	}
exit:
	ESIF_TRACE_EXIT_INFO();
	return 0;
}


static void EsifDataLogFire(int isHeader)
{
	int i = 0;
	char *outLinePtr = NULL;
	EsifLogType logtype = ESIF_LOG_UI;
	time_t now = time(NULL);
	struct tm time = { 0 };
	esif_ccb_time_t msec = 0;

	esif_ccb_system_time(&msec);

	outLinePtr = (char *)esif_ccb_malloc(MAX_LOG_LINE);
	if (NULL == outLinePtr) {
		ESIF_TRACE_ERROR("Unable to allocate memory for logging data.");
		goto exit;
	}

	if (isHeader > 0) {
		esif_ccb_sprintf(MAX_LOG_LINE, outLinePtr, "Time,Server Msec,");
	}
	else {
		if (esif_ccb_localtime(&time, &now) == 0) {
				esif_ccb_sprintf(MAX_LOG_LINE, outLinePtr, "%04d-%02d-%02d %02d:%02d:%02d,%llu,",
					time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, msec);
		}
	}
	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		eEsifError rc = ESIF_OK;
		DataLogParticipant currentParticipant = g_dataLogParticipants[i];

		if (currentParticipant.participantNumFields > 0) {
			rc = EsifDataLogAddParticipant(outLinePtr, i, isHeader);
		}
	}
	EsifLogFile_Write(logtype, "%s\n", outLinePtr);
exit:
	esif_ccb_free(outLinePtr);
}


static void EsifDataLogExit(esif_thread_t *dataLogThread)
{
	g_dataLogQuit = ESIF_TRUE;
	esif_ccb_event_set(&g_dataLogQuitEvent);

	esif_ccb_thread_join(dataLogThread);
	esif_ccb_event_uninit(&g_dataLogQuitEvent);
}

static int EsifDataLogGetCapabilityFieldCount(EsifUpPtr upPtr)
{
	EsifDspPtr dspPtr = NULL;
	EsifFpcDomainPtr domainPtr = NULL;
	UInt32 domainCount = 0;
	UInt32 domainIndex = 0;
	int fieldCounter = 0;

	dspPtr = EsifUp_GetDsp(upPtr);
	if (dspPtr != NULL) {
		domainCount = dspPtr->get_domain_count(dspPtr);

		for (domainIndex = 0; domainIndex < domainCount; domainIndex++) {
			domainPtr = dspPtr->get_domain(dspPtr, domainIndex);
			if (NULL == domainPtr) {
				continue;
			}
			if (domainPtr->capability_for_domain.capability_flags & ESIF_CAPABILITY_TEMP_STATUS) {
				fieldCounter += 4;
			}
			if (domainPtr->capability_for_domain.capability_flags & ESIF_CAPABILITY_POWER_CONTROL) {
				fieldCounter += 2;
			}
			if (domainPtr->capability_for_domain.capability_flags & ESIF_CAPABILITY_ACTIVE_CONTROL) {
				fieldCounter += 1;
			}
		}
	}

	return fieldCounter;
}

static eEsifError EsifDataLogEnableParticipantsFromList(char *inParticipantList)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr upPtr = NULL;
	char *participantList = NULL;
	char *participantSelect = NULL;
	char colDelims [] = ",";
	char *partTok = NULL;
	UInt8 participantId = 0;
	int participantCounter = 0;
	
	participantList = esif_ccb_strdup(inParticipantList);
	participantSelect = esif_ccb_strtok(participantList, colDelims, &partTok);

	while (participantSelect != NULL) {
		if (participantCounter >= (sizeof(g_dataLogParticipants) / sizeof(*g_dataLogParticipants))) {
			rc = ESIF_E_NOT_SUPPORTED;
			goto exit;
		}

		if ((int) esif_atoi(participantSelect) > 0 || esif_ccb_strcmp(participantSelect, "0") == 0) {
			participantId = (UInt8) esif_atoi(participantSelect);
			upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
		}
		else {
			upPtr = EsifUpPm_GetAvailableParticipantByName(participantSelect);
			participantId = EsifUp_GetInstance(upPtr);
		}
		if (NULL == upPtr) {
			rc = ESIF_E_PARTICIPANT_NOT_FOUND;
			goto exit;
		}

		EsifDataLogAddParticipantToList(upPtr, participantCounter);
		EsifUp_PutRef(upPtr);
		participantCounter++;
		participantSelect = esif_ccb_strtok(NULL, colDelims, &partTok);
	}
exit:
	esif_ccb_free(participantList);

	return rc;
}

static eEsifError EsifDataLogEnableAllParticipants()
{
	eEsifError rc = ESIF_OK;
	eEsifError iterRc = ESIF_OK;
	UfPmIterator upIter = { 0 };
	EsifUpPtr upPtr = NULL;
	int participantCounter = 0;

	iterRc = EsifUpPm_InitIterator(&upIter);
	if (iterRc != ESIF_OK) {
		rc = iterRc;
		goto exit;
	}

	iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	while (ESIF_OK == iterRc) {
		EsifDataLogAddParticipantToList(upPtr, participantCounter);
		participantCounter++;
		iterRc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
	
exit:
	EsifUp_PutRef(upPtr);
	return rc;
}

static eEsifError EsifDataLogAddParticipantToList(EsifUpPtr upPtr, int indexPosition)
{
	eEsifError rc = ESIF_OK;
	UInt8 participantId = 0;
	int fieldCounter = 0;
	DataLogParticipant nextParticipant = { 0 };

	fieldCounter = EsifDataLogGetCapabilityFieldCount(upPtr);

	participantId = EsifUp_GetInstance(upPtr);
	nextParticipant.participantId = participantId;
	nextParticipant.participantNumFields = fieldCounter;
	g_dataLogParticipants[indexPosition] = nextParticipant;

	return rc;
}
static eEsifError EsifDataLogValidateParticipantList(char *inParticipantList)
{
	eEsifError rc = ESIF_OK;
	int totalFields = 0;
	int i = 0;

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++){
		g_dataLogParticipants[i].participantId = 0;
		g_dataLogParticipants[i].participantNumFields = 0;
	}

	if (inParticipantList == NULL) {
		rc = EsifDataLogEnableAllParticipants();
		goto exit;
	}

	if (esif_ccb_strlen(inParticipantList, MAX_LOG_LINE) >= 1) {
		rc = EsifDataLogEnableParticipantsFromList(inParticipantList);
	}
	else {
		rc = EsifDataLogEnableAllParticipants();
	}

exit:
	//make sure there is at least one valid field to log
	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		totalFields += g_dataLogParticipants[i].participantNumFields;
	}
	if (totalFields == 0) {
		rc = ESIF_E_NOT_SUPPORTED;
	}
	return rc;
}


#define LOG_APPEND_VALUE_OR_DEFAULT(rc, val) \
	if (ESIF_OK == rc) {\
		esif_ccb_sprintf_concat(MAX_LOG_LINE, logString, "%d,", val); \
	} else { \
		esif_ccb_sprintf_concat(MAX_LOG_LINE, logString, "%s,", participantNullValue); \
	}

static eEsifError EsifDataLogAddParticipant(char *logString, int logColumn, int isHeader)
{
	eEsifError rc = ESIF_OK;
	char qualifierStr[32] = "D0";
	char *participantNullValue = "";
	UInt32 domainCount = 0;
	UInt32 domainIndex = 0;
	DataLogParticipant currentParticipant = g_dataLogParticipants[logColumn];
	UInt8 participantId = currentParticipant.participantId;
	EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	EsifDspPtr dspPtr = NULL;

	if (NULL == upPtr) {
		int colCounter = 0;
		/* 
		 * If we get here, the participant went offline (because the participantId was pre-validated on log start)
		 * so we must output placeholders
		 */
		for (colCounter = 0; colCounter < currentParticipant.participantNumFields; colCounter++){
			esif_ccb_sprintf_concat(MAX_LOG_LINE, logString, "%s,", participantNullValue);
		}
		goto exit;
	}

	dspPtr = EsifUp_GetDsp(upPtr);
	if (NULL == dspPtr) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}

	domainCount = dspPtr->get_domain_count(dspPtr);
	for (domainIndex = 0; domainIndex < domainCount; domainIndex++) {
		UInt32 temp = 255;
		UInt32 aux0 = 255;
		UInt32 aux1 = 255;
		UInt32 hyst = 0;
		UInt32 power = 0;
		UInt32 powerLimit = 0;
		UInt32 fanspeed = 0;
	
		EsifFpcDomainPtr domain_ptr = NULL;
		struct esif_data request = { ESIF_DATA_VOID, NULL, 0, 0 };
		struct esif_data temp_response = { ESIF_DATA_TEMPERATURE, &temp, sizeof(temp), 4 };
		struct esif_data aux0_response = { ESIF_DATA_TEMPERATURE, &aux0, sizeof(aux0), 4 };
		struct esif_data aux1_response = { ESIF_DATA_TEMPERATURE, &aux1, sizeof(aux1), 4 };
		struct esif_data hyst_response = { ESIF_DATA_TEMPERATURE, &hyst, sizeof(hyst), 4 };
		struct esif_data power_response = { ESIF_DATA_POWER, &power, sizeof(power), 4 };
		struct esif_data power_limit_response = { ESIF_DATA_POWER, &powerLimit, sizeof(powerLimit), 4 };
		struct esif_data_binary_fst_package fanpkg = { 0 };
		struct esif_data fan_response = { ESIF_DATA_BINARY, &fanpkg, sizeof(fanpkg), 4 };

		domain_ptr = dspPtr->get_domain(dspPtr, domainIndex);
		if (NULL == domain_ptr) {
			continue;
		}

		if (domain_ptr->capability_for_domain.capability_flags & ESIF_CAPABILITY_TEMP_STATUS) {
			if (isHeader) {
				EsifString partName = EsifUp_GetName(upPtr);
				esif_ccb_sprintf_concat(MAX_LOG_LINE, logString, "%s_D%d_temp,%s_D%d_aux0,%s_D%d_aux1,%s_D%d_hysteresis,", partName, domainIndex, partName, domainIndex, partName, domainIndex, partName, domainIndex);
			}
			else {
				rc = EsifExecutePrimitive((u8) participantId, GET_TEMPERATURE, esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifierStr, 32), 255, &request, &temp_response);
				LOG_APPEND_VALUE_OR_DEFAULT(rc, temp);

				rc = EsifExecutePrimitive((u8) participantId, GET_TEMPERATURE_THRESHOLDS, esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifierStr, 32), 0, &request, &aux0_response);
				LOG_APPEND_VALUE_OR_DEFAULT(rc, aux0);

				rc = EsifExecutePrimitive((u8) participantId, GET_TEMPERATURE_THRESHOLDS, esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifierStr, 32), 1, &request, &aux1_response);
				LOG_APPEND_VALUE_OR_DEFAULT(rc, aux1);

				rc = EsifExecutePrimitive((u8) participantId, GET_TEMPERATURE_THRESHOLD_HYSTERESIS, esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifierStr, 32), 255, &request, &hyst_response);
				LOG_APPEND_VALUE_OR_DEFAULT(rc, hyst);
			}
		}
		if (domain_ptr->capability_for_domain.capability_flags & ESIF_CAPABILITY_POWER_CONTROL) {
			if (isHeader) {
				EsifString partName = EsifUp_GetName(upPtr);
				esif_ccb_sprintf_concat(MAX_LOG_LINE, logString, "%s_D%d_power,%s_D%d_powerlimit,", partName, domainIndex, partName, domainIndex);
			}
			else {
				rc = EsifExecutePrimitive((u8) participantId, GET_RAPL_POWER, esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifierStr, 32), 255, &request, &power_response);
				LOG_APPEND_VALUE_OR_DEFAULT(rc, power);

				rc = EsifExecutePrimitive((u8) participantId, GET_RAPL_POWER_LIMIT, esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifierStr, 32), 0, &request, &power_limit_response);
				LOG_APPEND_VALUE_OR_DEFAULT(rc, powerLimit);
			}
		}
		if (domain_ptr->capability_for_domain.capability_flags & ESIF_CAPABILITY_ACTIVE_CONTROL) {
			if (isHeader) {
				esif_ccb_sprintf_concat(MAX_LOG_LINE, logString, "%s_D%d_fanspeed,", EsifUp_GetName(upPtr), domainIndex);
			}
			else {
				rc = EsifExecutePrimitive((u8) participantId, GET_FAN_STATUS, esif_primitive_domain_str(domain_ptr->descriptor.domain, qualifierStr, 32), 255, &request, &fan_response);
				if (rc == ESIF_OK) {
					struct esif_data_binary_fst_package *fanPtr = (struct esif_data_binary_fst_package *)fan_response.buf_ptr;
					fanspeed = (u32) fanPtr->control.integer.value;
				}
				LOG_APPEND_VALUE_OR_DEFAULT(rc, fanspeed);
			}
		}
	}
exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}

	return rc;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
