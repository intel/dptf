/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "esif.h"
#include "esif_uf_trace.h"

#ifdef ESIF_ATTR_OS_WINDOWS
/*
 *
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified
 * against Windows SDK/DDK included headers which we have no control over.
 *
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#include "esif_uf_log.h"

#ifdef ESIF_ATTR_OS_WINDOWS
#endif
#ifdef ESIF_ATTR_OS_LINUX
# ifdef ESIF_ATTR_OS_ANDROID
#  include <android/log.h>
#  define IDENT    "DPTF"
#  define ESIF_PRIORITY_FATAL   ANDROID_LOG_FATAL
#  define ESIF_PRIORITY_ERROR   ANDROID_LOG_ERROR
#  define ESIF_PRIORITY_WARNING ANDROID_LOG_WARN
#  define ESIF_PRIORITY_INFO    ANDROID_LOG_INFO
#  define ESIF_PRIORITY_DEBUG   ANDROID_LOG_DEBUG
# else
#  include <syslog.h>
#  define IDENT    "DPTF"
#  define OPTION   LOG_PID
#  define FACILITY LOG_DAEMON
#  define ESIF_PRIORITY_FATAL   LOG_EMERG
#  define ESIF_PRIORITY_ERROR   LOG_ERR
#  define ESIF_PRIORITY_WARNING LOG_WARNING
#  define ESIF_PRIORITY_INFO    LOG_INFO
#  define ESIF_PRIORITY_DEBUG   LOG_DEBUG
# endif
#endif

int g_traceLevel = ESIF_TRACELEVEL_DEFAULT;

struct esif_tracelevel_s g_traceinfo[] = {
	{"FATAL",	ESIF_TRACELEVEL_FATAL,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_EVENTLOG},
	{"ERROR",	ESIF_TRACELEVEL_ERROR,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_EVENTLOG},
	{"WARNING",	ESIF_TRACELEVEL_WARN,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_DEBUGGER},
	{"INFO",	ESIF_TRACELEVEL_INFO,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_DEBUGGER},
	{"DEBUG",	ESIF_TRACELEVEL_DEBUG,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_DEBUGGER},
};
#define ESIF_TRACELEVEL_MAX ((sizeof(g_traceinfo) / sizeof(struct esif_tracelevel_s)) - 1)
int g_traceLevel_max = ESIF_TRACELEVEL_MAX;

#define DETAILED_TRACELEVEL	ESIF_TRACELEVEL_FATAL /* All Trace Levels */

#define TRACE_MESSAGE_PADDING	(21+12+1)	// Room for %d and %lld

const struct EsifTraceModuleList_s {
	enum esif_tracemodule	id;
	const char				*str;
} g_EsifTraceModuleList[] = {
	ENUM_TRACEMODULE(ENUMLIST)
	{(enum esif_tracemodule)0}
};

const enum esif_tracemodule EsifTraceModule_FromString(const char *name)
{
	int j;
	if (esif_ccb_stricmp(name, "IPF") == 0) {
		name = "DPTF";
	}
	for (j=0; g_EsifTraceModuleList[j].str; j++) {
		if (esif_ccb_stricmp(name, g_EsifTraceModuleList[j].str)==0 ||  // ESIF_TRACEMODULE_XXXX
			esif_ccb_stricmp(name, g_EsifTraceModuleList[j].str+17)==0) // XXXX
			return g_EsifTraceModuleList[j].id;
	}
	return (enum esif_tracemodule)0;
}

static const char *EsifTraceModule_FullName(enum esif_tracemodule val)
{
	if (val == ESIF_TRACEMODULE_DPTF) {
		return "ESIF_TRACEMODULE_IPF";
	}
	switch (val)
	{
	ENUM_TRACEMODULE(ENUMSWITCH)
	}
	return ESIF_NOT_AVAILABLE;
}


const char *EsifTraceModule_ToString(enum esif_tracemodule val)
{
	const char *str = EsifTraceModule_FullName(val);
	if (esif_ccb_strlen(str, 20) >= 17)
		str += 17; // Truncate "ESIF_TRACEMODULE_"
	return str;
}

int EsifUfTraceMessageArgs(
	esif_tracemask_t module,
	int level,
	const char *func,
	const char *file,
	int line,
	const char *msg,
	va_list arglist)
{
	int rc=0;
	char *appname  = "";
	char *fmtDetail= "%s%s:[<%s>%s@%s#%d]<%llu ms>: ";
	char *fmtInfo  = "%s%s:[<%s>]<%llu ms>: ";
	const char *sep=NULL;
	size_t fmtlen=esif_ccb_strlen(msg, 0x7FFFFFFF);
	int  detailed_message = (level >= DETAILED_TRACELEVEL ? ESIF_TRUE : ESIF_FALSE);
	va_list args;
	esif_ccb_time_t msec = 0;
	enum esif_tracemodule moduleid = ESIF_TRACEMODULE_DEFAULT;
	char *module_name = NULL;
	size_t module_len = 0;

	esif_ccb_system_time(&msec);

	// Build Trace Module Name(s) List [NAME or NAME1|NAME2|...]
	while (module) {
		if (module & 0x1) {
			const char *this_module = EsifTraceModule_ToString(moduleid);
			char *old_name = module_name;
			module_len += (esif_ccb_strlen(this_module, MAX_PATH) + 2);
			module_name = esif_ccb_realloc(module_name, module_len);
			if (module_name == NULL) {
				module_name = old_name;
				goto exit;
			}
			if (old_name) {
				esif_ccb_strcat(module_name, "><", module_len);
			}
			esif_ccb_strcat(module_name, this_module, module_len);
		}
		module >>= 1;
		if (module) {
			moduleid++;
		}
	}
	if (module_name == NULL)
		goto exit;

	level = esif_ccb_min(level, ESIF_TRACELEVEL_MAX);
	level = esif_ccb_max(level, ESIF_TRACELEVEL_FATAL);
	if ((sep = strrchr(file, *ESIF_PATH_SEP)) != NULL)
		file = sep+1;

	// Do not log function/file/line information for DPTF app interface messages logged from EsifSvcWriteLog
	if (moduleid == ESIF_TRACEMODULE_DPTF) {
		detailed_message = ESIF_FALSE;
	}

	if (g_traceinfo[level].routes & ESIF_TRACEROUTE_CONSOLE) {
		if (detailed_message)
			rc =  CMD_CONSOLE(fmtDetail, appname, g_traceinfo[level].label, module_name, func, file, line, msec);
		else
			rc =  CMD_CONSOLE(fmtInfo, appname, g_traceinfo[level].label, module_name, msec);
		va_copy(args, arglist);
		rc += EsifConsole_WriteConsole(msg, args);
		va_end(args);

		if (fmtlen && msg[fmtlen-1]!='\n')
			CMD_CONSOLE("\n");
	}

	if (g_traceinfo[level].routes & ESIF_TRACEROUTE_LOGFILE && EsifLogFile_IsOpen(ESIF_LOG_TRACE)) {
		size_t  msglen = 0;
		char *buffer = 0;
		size_t  offset = 0;
		time_t now=0;
		char timestamp[MAX_CTIME_LEN]={0};

		time(&now);
		esif_ccb_ctime(timestamp, sizeof(timestamp), &now);
		timestamp[20] = 0; // truncate year

		va_copy(args, arglist);
		msglen = esif_ccb_vscprintf(msg, args) + esif_ccb_strlen(g_traceinfo[level].label, MAX_PATH) + esif_ccb_strlen(timestamp, MAX_PATH) + esif_ccb_strlen(func, MAX_PATH) + esif_ccb_strlen(file, MAX_PATH) + esif_ccb_strlen(module_name, MAX_PATH) + TRACE_MESSAGE_PADDING;
		va_end(args);
		msglen += (detailed_message ? esif_ccb_strlen(fmtDetail, MAX_PATH) : esif_ccb_strlen(fmtInfo, MAX_PATH));
		buffer = (char *)esif_ccb_malloc(msglen);

		if (NULL != buffer) {
			if (detailed_message)
				rc = esif_ccb_sprintf(msglen, buffer, fmtDetail, timestamp + 4, g_traceinfo[level].label, module_name, func, file, line, msec);
			else
				rc = esif_ccb_sprintf(msglen, buffer, fmtInfo, timestamp + 4, g_traceinfo[level].label, module_name, msec);

			offset = rc;
			va_copy(args, arglist);
			rc += esif_ccb_vsprintf(msglen - offset, buffer + offset, msg, args);
			va_end(args);
			if (rc && buffer[rc - 1] != '\n')
				esif_ccb_strcat(buffer, "\n", msglen);

			rc = EsifLogFile_Write(ESIF_LOG_TRACE, "%s", buffer);
			esif_ccb_free(buffer);
		}
	}

#ifdef ESIF_ATTR_OS_WINDOWS
	if (g_traceinfo[level].routes & (ESIF_TRACEROUTE_DEBUGGER)) {
		size_t  msglen=0;
		char *buffer=0;
		int  offset=0;

		va_copy(args, arglist);
		msglen = esif_ccb_vscprintf(msg, args) + esif_ccb_strlen(g_traceinfo[level].label, MAX_PATH) + esif_ccb_strlen(appname, MAX_PATH) + esif_ccb_strlen(func, MAX_PATH) + esif_ccb_strlen(file, MAX_PATH) + esif_ccb_strlen(module_name, MAX_PATH) + TRACE_MESSAGE_PADDING;
		va_end(args);
		msglen += (detailed_message ? esif_ccb_strlen(fmtDetail, MAX_PATH) : esif_ccb_strlen(fmtInfo, MAX_PATH));
		buffer = (char *)esif_ccb_malloc(msglen);

		if (NULL != buffer) {
			if (detailed_message)
				rc =  esif_ccb_sprintf(msglen, buffer, fmtDetail, appname, g_traceinfo[level].label, module_name, func, file, line, msec);
			else
				rc =  esif_ccb_sprintf(msglen, buffer, fmtInfo, appname, g_traceinfo[level].label, module_name, msec);

			offset = rc;
			va_copy(args, arglist);
			rc += esif_ccb_vsprintf(msglen-offset, buffer+offset, msg, args);
			va_end(args);
			if (rc && buffer[rc-1]!='\n')
				esif_ccb_strcat(buffer, "\n", msglen);

			OutputDebugStringA(buffer); 
			esif_ccb_free(buffer);
		}
	}
	if (g_traceinfo[level].routes & (ESIF_TRACEROUTE_EVENTLOG)) {
		size_t  msglen=0;
		char *buffer=0;
		char *replaced=0;
		int  offset=0;

		appname  = "";
		fmtInfo  = "%sESIF(%s) TYPE: %s MODULE: %s TIME %llu ms\n\n";
		fmtDetail= "%sESIF(%s) TYPE: %s MODULE: %s FUNC: %s FILE: %s LINE: %d TIME: %llu ms\n\n";

		va_copy(args, arglist);
		msglen = esif_ccb_vscprintf(msg,args) + esif_ccb_strlen(g_traceinfo[level].label, MAX_PATH) + esif_ccb_strlen(appname, MAX_PATH) + esif_ccb_strlen(func, MAX_PATH) + esif_ccb_strlen(file, MAX_PATH) + esif_ccb_strlen(module_name, MAX_PATH) + sizeof(ESIF_UF_VERSION) + TRACE_MESSAGE_PADDING;
		va_end(args);
		msglen += (detailed_message ? esif_ccb_strlen(fmtDetail, MAX_PATH) : esif_ccb_strlen(fmtInfo, MAX_PATH));
		buffer = (char *)esif_ccb_malloc(msglen);

		if (NULL != buffer) {
			char *alt_module_name = esif_str_replace(module_name, "><", "+");

			if (detailed_message)
				rc = esif_ccb_sprintf(msglen, buffer, fmtDetail, appname, ESIF_UF_VERSION, g_traceinfo[level].label, (alt_module_name ? alt_module_name : module_name), func, file, line, msec);
			else
				rc = esif_ccb_sprintf(msglen, buffer, fmtInfo, appname, ESIF_UF_VERSION, g_traceinfo[level].label, (alt_module_name ? alt_module_name : module_name), msec);

			offset = rc;
			va_copy(args, arglist);
			rc += esif_ccb_vsprintf(msglen-(offset), buffer+offset, msg, args);
			va_end(args);
			if (rc && buffer[rc-1]=='\n')
				buffer[--rc] = 0;

			// Escape any "%" in message before writing to EventLog
			if ((replaced = esif_str_replace(buffer, "%", "%%")) != NULL) {
				esif_ccb_free(buffer);
				buffer = replaced;
				replaced = NULL;
			}
			report_event_to_event_log(g_traceinfo[level].level, buffer);
			esif_ccb_free(buffer);
			esif_ccb_free(alt_module_name);
		}
	}
#endif
#ifdef ESIF_ATTR_OS_LINUX
	if (g_traceinfo[level].routes & (ESIF_TRACEROUTE_EVENTLOG|ESIF_TRACEROUTE_DEBUGGER)) {
		size_t  msglen=0;
		char *buffer=0;
		int  offset=0;
		int priority;

		fmtDetail= "%s:[<%s>%s@%s#%d]<%llu ms>: ";
		fmtInfo  = "%s:[<%s>]<%llu ms>: ";

		va_copy(args, arglist);
		msglen = esif_ccb_vscprintf(msg,args) + esif_ccb_strlen(g_traceinfo[level].label, MAX_PATH) + esif_ccb_strlen(func, MAX_PATH) + esif_ccb_strlen(file, MAX_PATH) + esif_ccb_strlen(module_name, MAX_PATH) + TRACE_MESSAGE_PADDING;
		va_end(args);
		msglen += (detailed_message ? esif_ccb_strlen(fmtDetail, MAX_PATH) : esif_ccb_strlen(fmtInfo, MAX_PATH));
		buffer = (char *)esif_ccb_malloc(msglen);

		if (NULL != buffer) {
			char *lf;
			if (detailed_message)
				rc =  esif_ccb_sprintf(msglen, buffer, fmtDetail, g_traceinfo[level].label, module_name, func, file, line, msec);
			else
				rc =  esif_ccb_sprintf(msglen, buffer, fmtInfo, g_traceinfo[level].label, module_name, msec);

			offset = rc;
			va_copy(args, arglist);
			rc += esif_ccb_vsprintf(msglen-offset, buffer+offset, msg, args);
			va_end(args);
			if (rc && buffer[rc-1]=='\n')
				buffer[--rc] = 0;

			while ((lf = esif_ccb_strchr(buffer, '\n')) != NULL)
				*lf = '\t';

			switch (g_traceinfo[level].level) {
			case ESIF_TRACELEVEL_FATAL:
				priority = ESIF_PRIORITY_FATAL;
				break;
			case ESIF_TRACELEVEL_ERROR:
				priority = ESIF_PRIORITY_ERROR;
				break;
			case ESIF_TRACELEVEL_WARN:
				priority = ESIF_PRIORITY_WARNING;
				break;
			case ESIF_TRACELEVEL_INFO:
				priority = ESIF_PRIORITY_INFO;
				break;
			case ESIF_TRACELEVEL_DEBUG:
			default:
				priority = ESIF_PRIORITY_DEBUG;
				break;
			}
		#ifdef ESIF_ATTR_OS_ANDROID
			__android_log_write(priority, IDENT, buffer);
		#else
			openlog(IDENT, OPTION, FACILITY);
			syslog(priority, "%s", buffer);
			closelog();
		#endif
			esif_ccb_free(buffer);
		}
	}
#endif
exit:
	esif_ccb_free(module_name);
	return rc;
}

/* Note different parameters for builds without OS Trace support to conserve code size */

#ifdef ESIF_FEAT_OPT_OS_TRACE
/* Conditionally Log a Trace Message to ESIF Route(s) and/or OS-Specific Interface (ETW) 
 * This prevents double-epxansion of variable argument list in ESIF_TRACE macros
 */
int EsifUfTraceMessage(
	int isEsifTrace,
	int isOsTrace,
	esif_tracemask_t module,
	int level,
	const char *func,
	const char *file,
	int line,
	const char *msg,
	...)
{
	int rc = 0;
	if (isEsifTrace) {
		va_list args;
		va_start(args, msg);
		rc = EsifUfTraceMessageArgs(module, level, func, file, line, msg, args);
		va_end(args);
	}
	if (isOsTrace) {
		va_list args;
		va_start(args, msg);
		rc = EtwPrintArgs(module, level, func, file, line, msg, args);
		va_end(args);
	}
	return rc;
}
#else
/* Always Log a Trace Message to ESIF Route(s) */
int EsifUfTraceMessage(
	esif_tracemask_t module,
	int level,
	const char *func,
	const char *file,
	int line,
	const char *msg,
	...)
{
	int rc = 0;
	va_list args;
	va_start(args, msg);
	rc = EsifUfTraceMessageArgs(module, level, func, file, line, msg, args);
	va_end(args);
	return rc;
}
#endif