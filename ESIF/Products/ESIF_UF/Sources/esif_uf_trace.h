/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_TRACE_H_
#define _ESIF_UF_TRACE_H_

#include "esif.h"
#include "esif_trace.h"

/* Enumerated Type Base Macros */
#define ENUMDECL(ENUM)              ENUM,
#define ENUMLIST(ENUM)              {ENUM, #ENUM},
#define ENUMSWITCH(ENUM)            case ENUM: return #ENUM;

/* Enumerated Type Macros with Explicit Values */
#define ENUMDECL_VAL(ENUM, VAL)      ENUM = VAL,
#define ENUMLIST_VAL(ENUM, VAL)      ENUMLIST(ENUM)
#define ENUMSWITCH_VAL(ENUM, VAL)    ENUMSWITCH(ENUM)

/* Trace Module and Route Masks */
typedef u32		esif_tracemask_t;
typedef u8		esif_traceroute_t;
#define esif_tracemask_fmt	"%u"	// %u or %llu
#define esif_tracemask_fmtx	"%x"	// %x or %llx
#define ESIF_TRACEMASK_MAX		32
#define ESIF_TRACEMAPPING_MAX	256

#ifdef ESIF_ATTR_OS_WINDOWS

/* Avoid Klocworks "suspicious semicolon" warnings */
#define ESIF_ALWAYSFALSE (0)
#define ESIF_TRACENULL(fmt, ...) (0)
#define ESIF_TRACEFUNC(fmt, ...) CMD_OUT(fmt, ##__VA_ARGS__)
#define ESIF_FILENAME __FILE__

/* OS Trace is only available for Windows components */
#define ESIF_FEAT_OPT_OS_TRACE

#endif /* ESIF_ATTR_OS_WINDOWS */

#ifdef ESIF_ATTR_OS_LINUX
#define ESIF_ALWAYSFALSE (0)
#define ESIF_TRACENULL(fmt, ...)
#define ESIF_TRACEFUNC(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define ESIF_FILENAME __FILE__
#endif /* LINUX */

/* 
 * ESIF_TRACE_ID must be #defined in each source file before any #includes
 * Otherwise the module will use settings for DEFAULT module
 */
#ifndef ESIF_TRACE_ID
# define ESIF_TRACE_ID		ESIF_TRACEMODULE_DEFAULT
#endif
#define ESIF_TRACEMASK(module)	((esif_tracemask_t)1 << (module))

/* Trace Level Data */
struct esif_tracelevel_s {
	const char *		label;		// label, i.e. "ERROR"
	int					level;		// level, i.e, 1
	esif_tracemask_t	modules;	// active module bitmask
	esif_traceroute_t	routes;		// active routes bitmask
};

/* Enumerated TraceModule User Mode Values
 *
 * NOTE:  These values are tied to Windows ETW manifest files.
 * Any changes here must be reflected in the manifest files
 * and the associated WIKI page.
 */
#define ENUM_TRACEMODULE(ENUM)	\
	ENUM(ESIF_TRACEMODULE_DEFAULT)		/* Generic Module - Use Default Trace Settings*/ \
	ENUM(ESIF_TRACEMODULE_DPTF)			/* DPTF Loadable App */ \
	\
	ENUM(ESIF_TRACEMODULE_SHELL)		/* ESIF Shell */ \
	ENUM(ESIF_TRACEMODULE_UF)			/* UF Main */ \
	\
	ENUM(ESIF_TRACEMODULE_ACTION)		/* Actions */ \
	ENUM(ESIF_TRACEMODULE_APP)			/* Apps */ \
	ENUM(ESIF_TRACEMODULE_CONJURE)		/* Conjure */ \
	ENUM(ESIF_TRACEMODULE_DOMAIN)		/* Domains */ \
	ENUM(ESIF_TRACEMODULE_DSP)			/* DSPs */ \
	ENUM(ESIF_TRACEMODULE_EVENT)		/* Events */ \
	ENUM(ESIF_TRACEMODULE_IPC)			/* IPC */ \
	ENUM(ESIF_TRACEMODULE_PARTICIPANT)	/* Upper Participants */ \
	ENUM(ESIF_TRACEMODULE_PRIMITIVE)	/* Primitives */ \
	ENUM(ESIF_TRACEMODULE_SERVICE)		/* Services */ \
	\
	ENUM(ESIF_TRACEMODULE_DATAVAULT)	/* DataVaults/DataCache/DataBank */ \
	ENUM(ESIF_TRACEMODULE_DEPRECATED1)	/* Formerly Web Socket Server */ \
	\
	ENUM(ESIF_TRACEMODULE_WINDOWS)		/* Windows General */ \
	ENUM(ESIF_TRACEMODULE_ACTWIRELESS)	/* Windows ACT Wireless */ \
	ENUM(ESIF_TRACEMODULE_UMDF)			/* Windows UMDF */ \
	ENUM(ESIF_TRACEMODULE_THERMALAPI)	/* Windows Thermal Framework API */ \
	\
	ENUM(ESIF_TRACEMODULE_LINUX)		/* Linux General */ \
	\
	ENUM(ESIF_TRACEMODULE_LOGGINGMGR)   /* Logging Manager */ \
	\
	ENUM(ESIF_TRACEMODULE_APITRACE)     /* Entry/Exit for  */ \
	ENUM(ESIF_TRACEMODULE_POWER)        /* Device power state information  */ \
	ENUM(ESIF_TRACEMODULE_PNP)          /* PnP information  */ \
	ENUM(ESIF_TRACEMODULE_TABLEOBJECT)  /* TableObject  */ \
	ENUM(ESIF_TRACEMODULE_ARBITRATION)  /* Arbitration  */ \

enum esif_tracemodule {
	ENUM_TRACEMODULE(ENUMDECL)
};

#define	ESIF_TRACEMASK_ALL	((esif_tracemask_t)-1)	/* all bits */

#pragma pack(push,1)
struct esif_tracemodule_s {
	const char 				*module;
	enum esif_tracemodule	module_id;
};
#pragma pack(pop)

/* ESIF_UF Trace Routes */
#define ESIF_TRACEROUTE_CONSOLE		1	/* Shell Console */
#define ESIF_TRACEROUTE_EVENTLOG	2	/* Windows=EventLog Linux=syslog */
#define ESIF_TRACEROUTE_DEBUGGER	4	/* Windows=DebugView Linux=syslog */
#define ESIF_TRACEROUTE_LOGFILE		8	/* Trace Log File (create with "trace log open <file>") */

/* Do not access these functions and variables directly in any code, just the macros at the bottom */

#ifdef __cplusplus
extern "C" {
#endif

extern int g_traceLevel;
extern const char *g_tracelabel[];
extern int g_traceLevel_max;
extern struct esif_tracelevel_s g_traceinfo[];

extern const enum esif_tracemodule EsifTraceModule_FromString(const char *name);
extern const char *EsifTraceModule_ToString(enum esif_tracemodule val);

#ifdef __cplusplus
}
#endif

/* Alias for Current Module's Trace Module Mask that can be passed to DOTRACE and TRACEACTIVE/ENABLED macros */
#define ESIF_TRACEMASK_CURRENT	ESIF_TRACEMASK(ESIF_TRACE_ID)

/* Test whether Tracing is currently active for the given module and level based on the currrent trace level*/
#define ESIF_TRACEACTIVE(module, level) ((g_traceLevel >= (level)) && !!(g_traceinfo[level].modules & (module)))

/* Test whether Tracing is currently enabled for the given module and level (regardless of current trace level) */
#define ESIF_TRACEENABLED(module, level) (!!(g_traceinfo[level].modules & (module)))

/* OS Trace Support. Note different parameters for EsifUfTraceMessage for non-OS-Trace builds to conserve code size */
#ifdef ESIF_FEAT_OPT_OS_TRACE
#ifdef __cplusplus
extern "C" {
#endif
	extern int EsifUfTraceMessage(
		int isEsifTrace,
		int isOsTrace,
		esif_tracemask_t module,
		int level,
		const char *func,
		const char *file,
		int line,
		const char *msg, 
		...);
	extern int IsEsifUfEtwEnabled(
		esif_tracemask_t moduleMask,
		int level
		);
	extern int EtwPrintArgs(
		esif_tracemask_t moduleMask,
		int level,
		const char *funcPtr,
		const char *filePtr,
		int line,
		const char *fmtPtr,
		va_list argList
		);
#ifdef __cplusplus
}
#endif
#define ESIF_OS_TRACEACTIVE(module, level)		IsEsifUfEtwEnabled(module, level)

/* Conditionally route trace message if given module/level are currently active, based on the current trace level.
 * Optional arguments are not evaluated and message string not created if message not routed
 */
#define ESIF_DOTRACE_IFACTIVE(module, level, msg, ...) \
	do { \
		int isEsifTrace = ESIF_TRACEACTIVE(module, level), isOsTrace = ESIF_OS_TRACEACTIVE(module, level); \
		if (isEsifTrace || isOsTrace) { \
			EsifUfTraceMessage(isEsifTrace, isOsTrace, module, level, ESIF_FUNC, ESIF_FILENAME, __LINE__, msg, ##__VA_ARGS__); \
		} \
	} while ESIF_CONSTEXPR(ESIF_ALWAYSFALSE)

/*
 * Conditionally route trace message if given module/level are currently enabled, regardless of the current trace level.
 * Optional arguments are not evaluated and message string not created if message not routed
 */
#define ESIF_DOTRACE_IFENABLED(module, level, msg, ...) \
	do { \
		int isEsifTrace = ESIF_TRACEENABLED(module, level), isOsTrace = ESIF_OS_TRACEACTIVE(module, level); \
		if (isEsifTrace || isOsTrace) { \
			EsifUfTraceMessage(isEsifTrace, isOsTrace, module, level, ESIF_FUNC, ESIF_FILENAME, __LINE__, msg, ##__VA_ARGS__); \
		} \
	} while ESIF_CONSTEXPR(ESIF_ALWAYSFALSE)

/* Always route trace message regardless of current module masks and trace level */
#define ESIF_DOTRACE_ALWAYS(module, level, msg, ...) \
		EsifUfTraceMessage(1, 1, module, level,	ESIF_FUNC, ESIF_FILENAME, __LINE__, msg, ##__VA_ARGS__); \

#else /* not ESIF_FEAT_OPT_OS_TRACE */

#ifdef __cplusplus
extern "C" {
#endif
	/* Note different parameter list for non-OS Trace build to conserve code size */
	extern int EsifUfTraceMessage(
		esif_tracemask_t module,
		int level,
		const char *func,
		const char *file,
		int line,
		const char *msg,
		...);
#ifdef __cplusplus
}
#endif

#define ESIF_OS_TRACEACTIVE(module, level)		(0)

/* Conditionally route trace message if given module/level are currently active, based on the current trace level.
 * Optional arguments are not evaluated and message string not created if message not routed
 */
#define ESIF_DOTRACE_IFACTIVE(module, level, msg, ...) \
	do { \
		if (ESIF_TRACEACTIVE(module, level)) { \
			EsifUfTraceMessage(module, level, ESIF_FUNC, ESIF_FILENAME, __LINE__, msg, ##__VA_ARGS__); \
		} \
	} while ESIF_CONSTEXPR(ESIF_ALWAYSFALSE)

/*
 * Conditionally route trace message if given module/level are currently enabled, regardless of the current trace level.
 * Optional arguments are not evaluated and message string not created if message not routed
 */
#define ESIF_DOTRACE_IFENABLED(module, level, msg, ...) \
	do { \
		if (ESIF_TRACEENABLED(module, level)) { \
			EsifUfTraceMessage(module, level,	ESIF_FUNC, ESIF_FILENAME, __LINE__, msg, ##__VA_ARGS__); \
		} \
	} while ESIF_CONSTEXPR(ESIF_ALWAYSFALSE)

/* Always route trace message regardless of current module masks and trace level */
#define ESIF_DOTRACE_ALWAYS(module, level, msg, ...) \
		EsifUfTraceMessage(module, level,	ESIF_FUNC, ESIF_FILENAME, __LINE__, msg, ##__VA_ARGS__); \

#endif

#define ESIF_TRACE_DYN(id, level, format, ...) \
	ESIF_DOTRACE_IFACTIVE(\
		ESIF_TRACEMASK(id), \
		level, \
		format, \
		##__VA_ARGS__ \
		)

/* Never route a trace message and compile it out of the binary */
#define ESIF_DOTRACE_NEVER(mod, lev, msg, ...)	(0)

/* Default User-Mode Trace Level at startup */
#define ESIF_TRACELEVEL_DEFAULT	ESIF_TRACELEVEL_ERROR

/*****************************************************************************
 * App Interface for Trace messages. Only use these macros in source modules.
 ****************************************************************************/

/*
* ESIF_TRACE_XXXX messages are conditionally routed depending on current trace level and module masks.
* Change ESIF_DOTRACE_IFACTIVE to ESIF_DOTRACE_NEVER in macros below to compile them out of the binaries.
*/
#define ESIF_TRACE_FATAL(msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK_CURRENT, \
			ESIF_TRACELEVEL_FATAL, \
			msg, \
			##__VA_ARGS__ \
		)
#define ESIF_TRACE_ERROR(msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK_CURRENT, \
			ESIF_TRACELEVEL_ERROR, \
			msg, \
			##__VA_ARGS__ \
		)
#define ESIF_TRACE_WARN(msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK_CURRENT, \
			ESIF_TRACELEVEL_WARN, \
			msg, \
			##__VA_ARGS__ \
		)
#define ESIF_TRACE_INFO(msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK_CURRENT, \
			ESIF_TRACELEVEL_INFO, \
			msg, \
			##__VA_ARGS__ \
		)
#define ESIF_TRACE_DEBUG(msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK_CURRENT, \
			ESIF_TRACELEVEL_DEBUG, \
			msg, \
			##__VA_ARGS__ \
		)

/* ESIF_TRACE_IFACTIVE messages are always compiled into binary and routed if module/level active, based on current trace level  */
#define ESIF_TRACE_IFACTIVE(module, level, msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(module), \
			level, \
			msg, \
			##__VA_ARGS__ \
		)

/* ESIF_TRACE_IFENABLED messages are always compiled into binary and routed if module/level enabled, regardless of current trace level */
#define ESIF_TRACE_IFENABLED(module, level, msg, ...) \
		ESIF_DOTRACE_IFENABLED( \
			ESIF_TRACEMASK(module), \
			level, \
			msg, \
			##__VA_ARGS__ \
		)

/* Use these for tracing Entry/Exit of functions */
#define ESIF_TRACE_ENTRY() \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(ESIF_TRACEMODULE_APITRACE), \
			ESIF_TRACELEVEL_DEBUG, \
			"Entering Function..." \
		)
#define ESIF_TRACE_EXIT() \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(ESIF_TRACEMODULE_APITRACE), \
			ESIF_TRACELEVEL_DEBUG, \
			"Exiting Function..." \
		)

/* Use these for tracing Entry/Exit of functions that you need logged at INFO level (rare) */
#define ESIF_TRACE_ENTRY_INFO() \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(ESIF_TRACEMODULE_APITRACE), \
			ESIF_TRACELEVEL_INFO, \
			"Entering Function..." \
		)
#define ESIF_TRACE_EXIT_INFO() \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(ESIF_TRACEMODULE_APITRACE), \
			ESIF_TRACELEVEL_INFO, \
			"Exiting Function..." \
		)
#define ESIF_TRACE_EXIT_INFO_W_STATUS(status) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(ESIF_TRACEMODULE_APITRACE), \
			ESIF_TRACELEVEL_INFO, \
			"Exiting function: Exit code = 0x%08X...", \
			status \
		)

#define ESIF_TRACE_API_INFO(msg, ...) \
		ESIF_DOTRACE_IFACTIVE( \
			ESIF_TRACEMASK(ESIF_TRACEMODULE_APITRACE), \
			ESIF_TRACELEVEL_INFO, \
			msg, \
			##__VA_ARGS__ \
		)

#endif	/* _ESIF_DEBUG_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
