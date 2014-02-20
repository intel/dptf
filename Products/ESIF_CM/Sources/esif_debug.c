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

#include "esif.h"
#include "esif_debug.h"

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

#ifdef ESIF_ATTR_KERNEL

/* Debug Data */
u32 g_esif_trace_level = ESIF_TRACELEVEL_ERROR;
u32 g_esif_module_mask = 0xFFFFFFFF;
u32 g_esif_module_category_mask[ESIF_DEBUG_MOD_MAX];

/* Init Debug Modules */
void esif_debug_init_module_categories ()
{
	int i;

	for (i = 0; i < ESIF_DEBUG_MOD_MAX; i++)
		g_esif_module_category_mask[i] = ESIF_TRACE_CATEGORY_DEFAULT;
}


/* Set Debug Modules */
void esif_debug_set_modules (u32 module_mask)
{
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF,
		       ESIF_TRACE_CATEGORY_DEBUG,
		       "%s: setting debug modules: 0x%08X\n",
		       ESIF_FUNC,
		       module_mask);

	g_esif_module_mask = module_mask;
}


/* Get Debug Modules */
void esif_debug_get_modules (u32 *module_mask_ptr)
{
	*module_mask_ptr = g_esif_module_mask;
}


/* Set Module Level */
void esif_debug_set_module_category (
	u32 module,
	u32 module_level_mask
	)
{
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ELF,
		       ESIF_TRACE_CATEGORY_DEBUG,
		       "%s: %s(%d) debug level: 0x%08X\n",
		       ESIF_FUNC,
		       esif_debug_mod_str((enum esif_debug_mod)module),
		       module,
		       module_level_mask);

	if (module < ESIF_DEBUG_MOD_MAX)
		g_esif_module_category_mask[module] = module_level_mask;
	else if (module == (u32)(-1) && module_level_mask <= ESIF_TRACELEVEL_DEBUG)
		g_esif_trace_level = module_level_mask;
}


/* Get Module Level */
void esif_debug_get_module_category (
	u32 module,
	u32 *module_level_mask_ptr
	)
{
	if (module < ESIF_DEBUG_MOD_MAX)
		*module_level_mask_ptr = g_esif_module_category_mask[module];
}


#else /* USER */

#include "esif_uf_log.h"

#ifdef ESIF_ATTR_OS_WINDOWS
#endif
#ifdef ESIF_ATTR_OS_LINUX
# include <syslog.h>
# define IDENT		"DPTF"
# define OPTION		LOG_PID
# define FACILITY	LOG_DAEMON
#endif

#define TRACEON(module)		((esif_tracemask_t)1 << (module))
#define TRACEOFF(module)	((esif_tracemask_t)0)

// Default Trace Module BitMask for DEBUG Messages
// NOTE: Modules without a "#define ESIF_TRACE_ID" use DEFAULT option
#define	ESIF_TRACEMASK_DEBUG ( \
	TRACEOFF(ESIF_TRACEMODULE_DEFAULT)  | \
	TRACEON (ESIF_TRACEMODULE_DPTF)  | \
	TRACEOFF(ESIF_TRACEMODULE_CCB)  | \
	TRACEON (ESIF_TRACEMODULE_SHELL)  | \
	TRACEOFF(ESIF_TRACEMODULE_UF)  | \
	TRACEON (ESIF_TRACEMODULE_ACTION)  | \
	TRACEON (ESIF_TRACEMODULE_APP)  | \
	TRACEON (ESIF_TRACEMODULE_CONJURE)  | \
	TRACEOFF(ESIF_TRACEMODULE_DSP)  | \
	TRACEOFF(ESIF_TRACEMODULE_EVENT)  | \
	TRACEOFF(ESIF_TRACEMODULE_IPC)  | \
	TRACEOFF(ESIF_TRACEMODULE_PARTICIPANT)  | \
	TRACEOFF(ESIF_TRACEMODULE_PRIMITIVE)  | \
	TRACEON (ESIF_TRACEMODULE_SERVICE)  | \
	TRACEON (ESIF_TRACEMODULE_TEST)  | \
	TRACEOFF(ESIF_TRACEMODULE_DATAVAULT)  | \
	TRACEOFF(ESIF_TRACEMODULE_EQL)  | \
	TRACEON (ESIF_TRACEMODULE_WEBSERVER)  | \
	/* windows specific */ \
	TRACEOFF(ESIF_TRACEMODULE_WINDOWS)  | \
	TRACEON (ESIF_TRACEMODULE_ACTWIRELESS)  | \
	TRACEOFF(ESIF_TRACEMODULE_UMDF) | \
	TRACEOFF(0) )

int g_traceLevel = ESIF_TRACELEVEL_DEFAULT;

struct esif_tracelevel_s g_traceinfo[] = {
	{"FATAL",	ESIF_TRACELEVEL_FATAL,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_EVENTLOG},
	{"ERROR",	ESIF_TRACELEVEL_ERROR,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_EVENTLOG},
	{"WARNING",	ESIF_TRACELEVEL_WARN,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_EVENTLOG},
	{"INFO",	ESIF_TRACELEVEL_INFO,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_EVENTLOG},
	{"DEBUG",	ESIF_TRACELEVEL_DEBUG,	ESIF_TRACEMASK_ALL,		ESIF_TRACEROUTE_DEBUGGER},
};
#define ESIF_TRACELEVEL_MAX ((sizeof(g_traceinfo) / sizeof(struct esif_tracelevel_s)) - 1)
int g_traceLevel_max = ESIF_TRACELEVEL_MAX;

#define DETAILED_TRACELEVEL	ESIF_TRACELEVEL_DEBUG

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
	for (j=0; g_EsifTraceModuleList[j].str; j++) {
		if (esif_ccb_stricmp(name, g_EsifTraceModuleList[j].str)==0 ||  // ESIF_TRACEMODULE_XXXX
			esif_ccb_stricmp(name, g_EsifTraceModuleList[j].str+17)==0) // XXXX
			return g_EsifTraceModuleList[j].id;
	}
	return (enum esif_tracemodule)0;
}

#define ENUMSWITCHSTR(ENUM)		case ENUM: str = #ENUM; break;
#define ENUMSWITCHSTR_VAL(ENUM)	ENUMSWITCHSTR(ENUM)

const char *EsifTraceModule_ToString(enum esif_tracemodule val)
{
	const char *str = NULL;
	switch (val) {
	ENUM_TRACEMODULE(ENUMSWITCHSTR)
	default:
		break;
	}
	if (str)
		str += 17; // Truncate "ESIF_TRACEMODULE_"
	return str;
}

int EsifTraceMessage(
	esif_tracemask_t module, 
	int level, 
	const char *func, 
	const char *file, 
	int line, 
	const char *msg, 
	...)
{
	int rc=0;
	char *appname  = "";
	char *fmtDetail= "%s%s:[%s@%s#%d]: ";
	char *fmtInfo  = "%s%s: ";
	const char *sep=NULL;
	size_t fmtlen=esif_ccb_strlen(msg, 0x7FFFFFFF);
	va_list args;
		
	UNREFERENCED_PARAMETER(module);
	level = esif_ccb_min(level, ESIF_TRACELEVEL_MAX);
	if ((sep = strrchr(file, *ESIF_PATH_SEP)) != NULL)
		file = sep+1;

	if (g_traceinfo[level].routes & ESIF_TRACEROUTE_CONSOLE) {
		if (level >= DETAILED_TRACELEVEL)
			rc =  CMD_CONSOLE(fmtDetail, appname, g_traceinfo[level].label, func, file, line);
		else
			rc =  CMD_CONSOLE(fmtInfo, appname, g_traceinfo[level].label);
		va_start(args, msg);
		rc += EsifConsole_WriteConsole(msg, args);
		va_end(args);

		if (fmtlen && msg[fmtlen-1]!='\n')
			CMD_CONSOLE("\n");
	}

	if (g_traceinfo[level].routes & ESIF_TRACEROUTE_LOGFILE && EsifLogFile_IsOpen(ESIF_LOG_TRACE)) {
		time_t now=0;
		char timestamp[MAX_CTIME_LEN]={0};

		time(&now);
		esif_ccb_ctime(timestamp, sizeof(timestamp), &now);
		timestamp[20] = 0; // truncate year

		if (level >= DETAILED_TRACELEVEL)
			rc =  EsifLogFile_Write(ESIF_LOG_TRACE, fmtDetail, timestamp+4, g_traceinfo[level].label, func, file, line);
		else
			rc =  EsifLogFile_Write(ESIF_LOG_TRACE, fmtInfo, timestamp+4, g_traceinfo[level].label);
		va_start(args, msg);
		rc += EsifLogFile_WriteArgs(ESIF_LOG_TRACE, msg, args);
		va_end(args);

		if (fmtlen && msg[fmtlen-1]!='\n')
			EsifLogFile_Write(ESIF_LOG_TRACE, "\n");
	}

#ifdef ESIF_ATTR_OS_WINDOWS
	if (g_traceinfo[level].routes & (ESIF_TRACEROUTE_DEBUGGER)) {
		size_t  msglen=0;
		char *buffer=0;
		int  offset=0;

		va_start(args, msg);
		msglen = esif_ccb_vscprintf(msg, args) + esif_ccb_strlen(g_traceinfo[level].label, MAX_PATH) + esif_ccb_strlen(appname, MAX_PATH) + esif_ccb_strlen(func, MAX_PATH) + esif_ccb_strlen(file, MAX_PATH) + 10;
		va_end(args);
		msglen += (level >= DETAILED_TRACELEVEL ? esif_ccb_strlen(fmtDetail, MAX_PATH) : esif_ccb_strlen(fmtInfo, MAX_PATH));
		buffer = (char *)esif_ccb_malloc(msglen);

		if (NULL != buffer) {
			if (level >= DETAILED_TRACELEVEL)
				rc =  esif_ccb_sprintf(msglen, buffer, fmtDetail, appname, g_traceinfo[level].label, func, file, line);
			else
				rc =  esif_ccb_sprintf(msglen, buffer, fmtInfo, appname, g_traceinfo[level].label);

			offset = rc;
			va_start(args, msg);
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
		int  offset=0;
		int  backset=0;
		WORD eventType;

		appname  = "";
		fmtInfo= "%sESIF(%s) TYPE: %s\n\n";
		backset  = 0;

		va_start(args, msg);
		msglen = esif_ccb_vscprintf(msg,args) + esif_ccb_strlen(g_traceinfo[level].label, MAX_PATH) + esif_ccb_strlen(appname, MAX_PATH) + esif_ccb_strlen(func, MAX_PATH) + esif_ccb_strlen(file, MAX_PATH) + 20;
		va_end(args);
		msglen += esif_ccb_strlen(fmtInfo, MAX_PATH);
		buffer = (char *)esif_ccb_malloc(msglen);
		if (NULL != buffer) {

			rc =  esif_ccb_sprintf(msglen, buffer, fmtInfo, appname, ESIF_UF_VERSION, g_traceinfo[level].label);

			if (backset && backset < rc)
				buffer[rc-backset-1] = 0;
			offset = rc-backset;
			va_start(args, msg);
			rc += esif_ccb_vsprintf(msglen-(offset+backset), buffer+offset+backset, msg, args);
			va_end(args);
			if (rc && buffer[rc-1]=='\n')
				buffer[--rc] = 0;

			switch (g_traceinfo[level].level) {
			case ESIF_TRACELEVEL_FATAL:
			case ESIF_TRACELEVEL_ERROR:
				eventType = EVENTLOG_ERROR_TYPE;
				break;
			case ESIF_TRACELEVEL_WARN:
				eventType = EVENTLOG_WARNING_TYPE;
				break;
			case ESIF_TRACELEVEL_INFO:
			case ESIF_TRACELEVEL_DEBUG:
			default:
				eventType = EVENTLOG_INFORMATION_TYPE;
				break;
			}
			report_event_to_event_log(CATEGORY_GENERAL, eventType, buffer);
			esif_ccb_free(buffer);
		}
	}
#endif
#ifdef ESIF_ATTR_OS_LINUX
	if (g_traceinfo[level].routes & (ESIF_TRACEROUTE_EVENTLOG|ESIF_TRACEROUTE_DEBUGGER)) {
		size_t  msglen=0;
		char *buffer=0;
		int  offset=0;
		int priority;

		fmtDetail= "%s:[%s@%s#%d]: ";
		fmtInfo  = "%s: ";

		va_start(args, msg);
		msglen = esif_ccb_vscprintf(msg,args) + esif_ccb_strlen(g_traceinfo[level].label, MAX_PATH) + esif_ccb_strlen(func, MAX_PATH) + esif_ccb_strlen(file, MAX_PATH) + 10;
		va_end(args);
		msglen += (level >= DETAILED_TRACELEVEL ? esif_ccb_strlen(fmtDetail, MAX_PATH) : esif_ccb_strlen(fmtInfo, MAX_PATH));
		buffer = (char *)esif_ccb_malloc(msglen);

		if (NULL != buffer) {
			if (level >= DETAILED_TRACELEVEL)
				rc =  esif_ccb_sprintf(msglen, buffer, fmtDetail, g_traceinfo[level].label, func, file, line);
			else
				rc =  esif_ccb_sprintf(msglen, buffer, fmtInfo, g_traceinfo[level].label);

			offset = rc;
			va_start(args, msg);
			rc += esif_ccb_vsprintf(msglen-offset, buffer+offset, msg, args);
			va_end(args);
			if (rc && buffer[rc-1]=='\n')
				buffer[--rc] = 0;

			switch (g_traceinfo[level].level) {
			case ESIF_TRACELEVEL_FATAL:
				priority = LOG_EMERG;
				break;
			case ESIF_TRACELEVEL_ERROR:
				priority = LOG_ERR;
				break;
			case ESIF_TRACELEVEL_WARN:
				priority = LOG_WARNING;
				break;
			case ESIF_TRACELEVEL_INFO:
				priority = LOG_INFO;
				break;
			case ESIF_TRACELEVEL_DEBUG:
			default:
				priority = LOG_DEBUG;
				break;
			}
			openlog(IDENT, OPTION, FACILITY);
			syslog(priority, "%s", buffer);
			closelog();
			esif_ccb_free(buffer);
		}
	}
#endif
	return rc;
}

#endif /* NOT ESIF_ATTR_KERNEL */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

