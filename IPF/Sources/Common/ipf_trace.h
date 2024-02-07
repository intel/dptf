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
#pragma once

#include "esif_ccb.h"
#include "esif_ccb_rc.h"
#include "esif_sdk_iface_esif.h"

/*******************************************************************************
** SUMMARY:  This header provides support for basic debug trace messaging
** support for an IPF client application.  
**
** DESCRIPTION:  IPF provides basic trace support through a set of macros
** designed to limit evaluation of variables if not processed and through an
** interface function which takes a NULL-terminated string.  The macros are
** evaluated based on the current trace level.  (The default trace level is
** IPF_TRACE_LEVEL_ERROR.)  All messages sent through the interface function
** are processed; regardless of the current trace level.
** 
** Basic macro support is provided by the following macros:
**     IPF_TRACE_FATAL(fmt, ...)
**     IPF_TRACE_ERROR(fmt, ...)
**     IPF_TRACE_WARN(fmt, ...)
**     IPF_TRACE_INFO(fmt, ...)
**     IPF_TRACE_DEBUG(fmt, ...)
**
** USAGE:  The macros may be used with standard "printf" style formatting
** strings and associated variables.
**
** Example:  IPF_TRACE_DEBUG("Value received = %d", value);
**
*******************************************************************************/

#if !defined(ESIF_ATTR_MEMTRACE)
/* Memory Leak Tracing Support Functions for Release Mode Builds */
static ESIF_INLINE esif_error_t esif_memtrace_init(void)
{
	return ESIF_OK;
}
static ESIF_INLINE void  esif_memtrace_exit(void)
{
}
#endif


// Should be called once during module init/exit after initalizing tracing
#define IpfRegisterEtwProvider() (ESIF_E_NOT_IMPLEMENTED)
#define IpfUnregisterEtwProvider()	(ESIF_E_NOT_IMPLEMENTED)


/* Used to Avoid SDL Warnings about Constant Expressions*/
#define IPF_ALWAYSFALSE (0)

/* IPF Trace Levels. */
#define IPF_TRACE_LEVEL_FATAL       0
#define IPF_TRACE_LEVEL_ERROR       1
#define IPF_TRACE_LEVEL_WARN        2
#define IPF_TRACE_LEVEL_INFO        3
#define IPF_TRACE_LEVEL_DEBUG       4

/*
* Use to determine if a trace message should be evaluated based on current level.
*/
#define IPF_TRACE_IS_ACTIVE(level)	(g_ipfTraceLevel >= (level))

/*
* Conditionally route trace message if the message is enabled and above the
* allowed level.
* NOTE:  Optional arguments are not evaluated and message string not created
* if message not active
*/
#define IPF_TRACE_DO_IF_ACTIVE(level, fmt, ...) \
	do { \
		if (IPF_TRACE_IS_ACTIVE(level)) { \
			IpfTrace_Message(level, ESIF_FUNC, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
		} \
	} while ESIF_CONSTEXPR(IPF_ALWAYSFALSE)

/*
* Unconditionally route trace message
*/
#define IPF_TRACE_DO_ALWAYS(level, fmt, ...) \
	do { \
		IpfTrace_Message(level, ESIF_FUNC, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
	} while ESIF_CONSTEXPR(IPF_ALWAYSFALSE)


/*
* Primary set of debug macros for each trace level.  The messages will only be
* evaluated if at or above the current level.
*/
#define IPF_TRACE_FATAL(fmt, ...)	IPF_TRACE_DO_IF_ACTIVE(IPF_TRACE_LEVEL_FATAL, fmt, ##__VA_ARGS__);
#define IPF_TRACE_ERROR(fmt, ...)	IPF_TRACE_DO_IF_ACTIVE(IPF_TRACE_LEVEL_ERROR, fmt, ##__VA_ARGS__);
#define IPF_TRACE_WARN(fmt, ...)	IPF_TRACE_DO_IF_ACTIVE(IPF_TRACE_LEVEL_WARN, fmt, ##__VA_ARGS__);
#define IPF_TRACE_INFO(fmt, ...)	IPF_TRACE_DO_IF_ACTIVE(IPF_TRACE_LEVEL_INFO, fmt, ##__VA_ARGS__);
#define IPF_TRACE_DEBUG(fmt, ...)	IPF_TRACE_DO_IF_ACTIVE(IPF_TRACE_LEVEL_DEBUG, fmt, ##__VA_ARGS__);

#define IPF_TRACE_ENTRY_DEBUG() \
	IPF_TRACE_DO_IF_ACTIVE(IPF_TRACE_LEVEL_DEBUG, "Entering function...");

#define IPF_TRACE_EXIT_DEBUG() \
	IPF_TRACE_DO_IF_ACTIVE(IPF_TRACE_LEVEL_DEBUG, "Exiting function...");

#define IPF_TRACE_EXIT_DEBUG_W_STATUS(status) \
	IPF_TRACE_DO_IF_ACTIVE(IPF_TRACE_LEVEL_DEBUG, "Exiting function: Exit code = 0x%08X...", status);


#ifdef __cplusplus
extern "C" {
#endif

extern int g_ipfTraceLevel;

esif_error_t IpfTrace_Init();
void IpfTrace_Exit();

void IpfTrace_SetConfig(const char *appname, AppWriteLogFunction func, Bool consoleOutput);
void IpfTrace_SetTraceLevel(int level); // Set the client level (actual level is max of client/OS levels)
void IpfTrace_SetOsTraceLevel(int level); // Set the OS requested level (actual level is max of client/OS levels)
int  IpfTrace_GetTraceLevel();

int IpfTrace_Message(
	int level,
	const char *func,
	const char *file,
	int line,
	const char *fmt,
	...
);

void UnregisterEtwProvider();
void RegisterEtwProvider();

#ifdef __cplusplus
}
#endif
