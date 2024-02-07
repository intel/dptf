/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "esif_ccb_string.h"
#include "esif_ccb_time.h"
#include "ipf_trace.h"

///////////////////////////////////////////////////////////////////////////////
// General Definitions
///////////////////////////////////////////////////////////////////////////////

#define TRACE_MESSAGE_PADDING	(21+12+1)	// Room for %d and %lld
#define TRACE_LEVEL_DEFAULT		IPF_TRACE_LEVEL_ERROR

typedef struct {
	char				appname[ESIF_NAME_LEN];	// App Name reported in Trace Message
	AppWriteLogFunction fWriteLogFuncPtr;		// ESIF Interface WriteLog Function, if any
	Bool				consoleOutput;			// Write Message to Console? (stderr)
} IpfTraceConfig;

///////////////////////////////////////////////////////////////////////////////
// Global Definitions
///////////////////////////////////////////////////////////////////////////////

int g_ipfTraceLevel = TRACE_LEVEL_DEFAULT;  // Current level used by module
int g_globalTraceLevel = IPF_TRACE_LEVEL_FATAL; // Global client level
int g_osTraceLevel = IPF_TRACE_LEVEL_FATAL; // Level request by OS capabilities

static IpfTraceConfig g_ipfTraceConfig = { 0 };

///////////////////////////////////////////////////////////////////////////////
// Function Prototypes
///////////////////////////////////////////////////////////////////////////////

static int IpfTrace_MessageArgs(
	int level,
	const char* func,
	const char* file,
	int line,
	const char* fmt,
	va_list arglist
);

///////////////////////////////////////////////////////////////////////////////
// Function Definitions
///////////////////////////////////////////////////////////////////////////////

esif_error_t IpfTrace_Init()
{
	return ESIF_OK;
}

void IpfTrace_Exit()
{
	g_ipfTraceLevel = TRACE_LEVEL_DEFAULT;
	esif_ccb_memset(&g_ipfTraceConfig, 0, sizeof(g_ipfTraceConfig));
}

void IpfTrace_SetConfig(
	const char *appname,
	AppWriteLogFunction func,
	Bool consoleOutput
	)
{
	if (appname) {
		esif_ccb_strcpy(g_ipfTraceConfig.appname, appname, sizeof(g_ipfTraceConfig.appname));
		esif_ccb_strupr(g_ipfTraceConfig.appname, sizeof(g_ipfTraceConfig.appname));
	}
	g_ipfTraceConfig.fWriteLogFuncPtr = func;
	g_ipfTraceConfig.consoleOutput = consoleOutput;
}

void IpfTrace_SetTraceLevel(int level)
{
	if (level >= IPF_TRACE_LEVEL_FATAL && level <= IPF_TRACE_LEVEL_DEBUG) {

		g_globalTraceLevel = level;  // Set the global client level
		g_ipfTraceLevel = esif_ccb_max(g_osTraceLevel, g_globalTraceLevel); // Set the level to use
	}
}

void IpfTrace_SetOsTraceLevel(int level)
{
	if (level >= IPF_TRACE_LEVEL_FATAL && level <= IPF_TRACE_LEVEL_DEBUG) {

		g_osTraceLevel = level;  // Set the OS trace level
		g_ipfTraceLevel = esif_ccb_max(g_osTraceLevel, g_globalTraceLevel); // Set the level to use

	}
}

int IpfTrace_GetTraceLevel()
{
	return g_ipfTraceLevel;
}

int IpfTrace_Message(
	int level,
	const char *func,
	const char *file,
	int line,
	const char *fmt,
	...
)
{
	va_list args;
	va_start(args, fmt);
	int rc = IpfTrace_MessageArgs(level, func, file, line, fmt, args);
	va_end(args);
	return rc;
}

static int IpfTrace_MessageArgs(
	int level,
	const char *func,
	const char *file,
	int line,
	const char *fmt,
	va_list arglist
)
{
	int rc = 0;
	char *buffer = NULL;
	char *fmtDetail = "[<%s>%s@%s#%d]<%llu ms>: ";
	size_t  msglen = 0;
	va_list args;
	const char *sep = NULL;
	int numChars = 0;
	size_t  offset = 0;
	char timestamp[MAX_CTIME_LEN] = { 0 };
	esif_ccb_time_t msec = 0;
	time_t now = 0;

	// Do nothing if no WriteLog function configured and no Console output
	if (g_ipfTraceConfig.fWriteLogFuncPtr == NULL && !(g_ipfTraceConfig.consoleOutput)) {
		return rc;
	}

	if ((sep = strrchr(file, *ESIF_PATH_SEP)) != NULL)
		file = sep + 1;

	esif_ccb_system_time(&msec);
	time(&now);
	esif_ccb_ctime(timestamp, sizeof(timestamp), &now);
	timestamp[19] = 0; // truncate year

	va_copy(args, arglist);
	msglen = esif_ccb_vscprintf(fmt, args) + esif_ccb_strlen(g_ipfTraceConfig.appname, sizeof(g_ipfTraceConfig.appname)) + esif_ccb_strlen(func, MAX_PATH) + esif_ccb_strlen(file, MAX_PATH) + TRACE_MESSAGE_PADDING;
	va_end(args);
	msglen += esif_ccb_strlen(fmtDetail, MAX_PATH);
	buffer = (char *)esif_ccb_malloc(msglen);

	if (NULL != buffer) {
		numChars = esif_ccb_sprintf(msglen, buffer, fmtDetail, g_ipfTraceConfig.appname, func, file, line, msec);

		offset = numChars;
		va_copy(args, arglist);
		numChars += esif_ccb_vsprintf(msglen - offset, buffer + offset, fmt, args);
		va_end(args);
		if (numChars && buffer[numChars - 1] != '\n')
			esif_ccb_strcat(buffer, "\n", msglen);

		// Write Message to Console?
		if (g_ipfTraceConfig.consoleOutput) {
			static char *logtypes[] = { "FATAL","ERROR","WARNING","INFO","DEBUG" };
			fprintf(stderr, "%s %s:%s", timestamp + 4, logtypes[level % 5], buffer);
		}
		// Write Message to ESIF?
		if (g_ipfTraceConfig.fWriteLogFuncPtr) {
			EsifData esifMessage = { ESIF_DATA_STRING };
			esifMessage.buf_ptr = buffer;
			esifMessage.buf_len = (u32)esif_ccb_strlen(buffer, msglen);
			esifMessage.data_len = esifMessage.buf_len;

			// Call into ESIF Interface Function
			g_ipfTraceConfig.fWriteLogFuncPtr(
				ESIF_INVALID_HANDLE,
				ESIF_INVALID_HANDLE,
				ESIF_INVALID_HANDLE,
				&esifMessage,
				(eLogType)level
			);
		}
		esif_ccb_free(buffer);
	}
	return rc;
}
