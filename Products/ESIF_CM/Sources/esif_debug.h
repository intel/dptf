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

#ifndef _ESIF_DEBUG_H_
#define _ESIF_DEBUG_H_

#include "esif.h"

#define NO_ESIF_DEBUG(fmt, ...) ESIF_TRACENULL
#define ESIF_DEBUG(fmt, ...)    ESIF_TRACE_DEBUG(fmt, ##__VA_ARGS__)

/*
 * The following enumeration specifies the index of a given module into
 * an array of debug masks for each modue type.  Each C file should
 * contain a ESIF_DEBUG_MODULE item defined as one of the enum items.
 * Note that Kernel-Mode Debug Modules definitions are available in User Mode
 * so that the kernel-mode debug level can be set from user-mode.
 */
enum esif_debug_mod {
	ESIF_DEBUG_MOD_ACTION_VAR      = 0,	/* Action Variable        */
	ESIF_DEBUG_MOD_ACTION_CONST    = 1,	/* Action Constant        */
	ESIF_DEBUG_MOD_ACTION_MSR      = 2,	/* Action MSR             */
	ESIF_DEBUG_MOD_ACTION_MMIO     = 3,	/* Action MMIO            */
	ESIF_DEBUG_MOD_ACTION_ACPI     = 4,	/* Action ACPI            */
	ESIF_DEBUG_MOD_IPC             = 5,	/* IPC                    */
	ESIF_DEBUG_MOD_COMMAND         = 6,	/* Command Pre DSP        */
	ESIF_DEBUG_MOD_PRIMITIVE       = 7,	/* Primitive Requires DSP */
	ESIF_DEBUG_MOD_ACTION          = 8,	/* Primitive Requires DSP */
	ESIF_DEBUG_MOD_CPC             = 9,	/* Loads DSP              */
	ESIF_DEBUG_MOD_DATA            = 10,	/* Data Operations        */
	ESIF_DEBUG_MOD_DSP             = 11,	/* DSP Operations         */
	ESIF_DEBUG_MOD_EVENT           = 12,	/* Event Processing       */
	ESIF_DEBUG_MOD_ELF             = 13,	/* ESIF Lower Framework   */
	ESIF_DEBUG_MOD_PMG             = 14,	/* Package Manager        */
	ESIF_DEBUG_MOD_QUEUE           = 15,	/* Queue Manager          */
	ESIF_DEBUG_MOD_HASH            = 16,	/* Hash Tables            */
	ESIF_DEBUG_MOD_ACTION_SYSTEMIO = 17,	/* Action SYSTEM IO       */
	ESIF_DEBUG_MOD_ACTION_CODE     = 18,	/* Action Code            */
	ESIF_DEBUG_MOD_POL             = 19,	/* Polling Code           */
	ESIF_DEBUG_MOD_ACTION_MBI      = 20,	/* Action MBI (ATOM)      */
	ESIF_DEBUG_MOD_GENERAL         = 21,	/* Non-Specific tracing   */
	ESIF_DEBUG_MOD_WINDOWS         = 22,	/* Windows-Specific       */
	ESIF_DEBUG_MOD_LINUX           = 23,	/* Linux-Specific         */
	ESIF_DEBUG_MOD_MAX
};

static ESIF_INLINE char *esif_debug_mod_str(enum esif_debug_mod mod)
{
	#define ESIF_CREATE_MOD(mod, ab, str) case mod: str = (esif_string) #ab; break;

	esif_string str = (esif_string)ESIF_NOT_AVAILABLE;
	switch (mod) {
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_VAR, VAR, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_CONST, CON, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_MSR, MSR, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_MMIO, MMI, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_ACPI, ACP, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_IPC, IPC, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_COMMAND, CMD, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_PRIMITIVE, PRI, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION, ACT, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_CPC, CPC, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_DATA, DAT, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_DSP, DSP, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_EVENT, EVE, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ELF, ELF, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_PMG, PMG, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_QUEUE, QUE, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_HASH, HSH, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_SYSTEMIO, SIO, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_CODE, COD, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_POL, POL, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_MBI, MBI, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_GENERAL, GNL, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_WINDOWS, WND, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_LINUX, LNX, str)
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_MAX, MAX, str)
	}
	return str;
}


/*
** Kernel Memory Statistics
*/
struct esif_memory_stats {
	u32  allocs;		/* Total number of allocations */
	u32  frees;		/* Total number of frees       */
	u32  memPoolAllocs;	/* Total number of allocations */
	u32  memPoolFrees;	/* Total number of frees       */
	u32  memPoolObjAllocs;	/* Total number of allocations */
	u32  memPoolObjFrees;	/* Total number of frees       */
	u32  memTypeAllocs;	/* Total number of allocations */
	u32  memTypeFrees;	/* Total number of frees       */
	u32  memTypeObjAllocs;	/* Total number of allocations */
	u32  memTypeObjFrees;	/* Total number of frees       */
};

/* ESIF Trace Levels. These correspond to eLogLevel enum type */
#define ESIF_TRACELEVEL_FATAL       0
#define ESIF_TRACELEVEL_ERROR       1
#define ESIF_TRACELEVEL_WARN        2
#define ESIF_TRACELEVEL_INFO        3
#define ESIF_TRACELEVEL_DEBUG       4

#ifdef ESIF_ATTR_KERNEL

/*
 * The kernel debug facilities allow the user to specify debug output from
 * particular modules and categories specific to a given module.  The categories
 * are enabled by a bitmask associated each module.  Several categories are
 * predefined to specify "levels" for universal trace macros.  (These are in
 * the upper bits of the category mask.  Module specific macros should use the
 * lower bits.)
 * For general items, the universal macros may be used:
 *    ESIF_TRACE_ERROR
 *    ESIF_TRACE_WARN
 *    ESIF_TRACE_INFO
 *    ESIF_TRACE_DEBUG
 * For more granular control using module-specific categories, use:
 *    ESIF_TRACE_DYN
 * By default, the ERROR macros are automatically enabled; but may be disabled.
 * Notes:
 *    (1) The current implementation compiles out all but the ERROR items in
 *        the released build.
 *    (2) The "module" must be define before the use of any of the macros in
 *        each C file by defining ESIF_DEBUG_MODULE using the ESIF_DEBUG_MOD_XXX
 *        definitions. (This value is an index into the module debug mask
 *array.)
 */

/*
 * Pre-Defined Debug Categories (Bit positions for selection mask.)
 * These are translated to the TRACE LEVEL using the following:
 */
#define ESIF_TRACE_CATEGORY_ERROR           31
#define ESIF_TRACE_CATEGORY_WARN            30
#define ESIF_TRACE_CATEGORY_INFO            29
#define ESIF_TRACE_CATEGORY_DEBUG           28
#define ESIF_TRACE_CATEGORY_ENTRY_AND_EXIT  27

/* Convert Module Trace Category bit to Global Trace Level:
 * ERROR (31) = TRACELEVEL_ERROR (1)
 * WARN  (30) = TRACELEVEL_WARN  (2)
 * INFO  (29) = TRACELEVEL_INFO  (3)
 * DEBUG (28) = TRACELEVEL_DEBUG (4)
 * All others = TRACELEVEL_DEBUG (4)
 */
#define ESIF_TRACE_CATEGORY_TO_TRACELEVEL(id) \
  ((((id) & 0x1F) > ESIF_TRACE_CATEGORY_DEBUG) ? (32-(id)) : ESIF_TRACELEVEL_DEBUG)

#define ESIF_TRACE_CATEGORY_DEFAULT   ((u32)1 << ESIF_TRACE_CATEGORY_ERROR)

#ifdef ESIF_ATTR_OS_LINUX
#define ESIF_ALWAYSFALSE (0) /*used for do...while(0) macros */
#define ESIF_TRACENULL
#define ESIF_TRACEFUNC  printk
#define ESIF_KERN_ERR   KERN_ERR
#define ESIF_KERN_INFO  KERN_INFO
#define ESIF_KERN_DEBUG KERN_DEBUG
#define ESIF_FILENAME   __FILE__/* or (strrchr(__FILE__,'/') ?
					*strrchr(__FILE__,'/')+1 : __FILE__) */
#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS
/* avoids "conditional expression is constant" warnings for do...while(0) macros in Windows */
static char g_alwaysfalse;
#define ESIF_ALWAYSFALSE (!&g_alwaysfalse)

#define ESIF_TRACENULL  (0)
#define ESIF_TRACEFUNC  DbgPrint
#define ESIF_KERN_ERR
#define ESIF_KERN_INFO
#define ESIF_KERN_DEBUG
#define ESIF_FILENAME   __FILE__
#endif /* ESIF_ATTR_OS_WINDOWS */

#define ESIF_TRACE_FMT_ERROR(fmt, ...)  \
	ESIF_TRACEFUNC(ESIF_KERN_ERR   "!ERR! %s:%d: %s: " fmt, \
		ESIF_FILENAME, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ESIF_TRACE_FMT_WARN(fmt, ...) \
	ESIF_TRACEFUNC(ESIF_KERN_DEBUG "!WRN! %s:%d: %s: " fmt, \
		ESIF_FILENAME, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ESIF_TRACE_FMT_INFO(fmt, ...) \
	ESIF_TRACEFUNC(ESIF_KERN_INFO  "=INF= %s:%d: %s: " fmt, \
		ESIF_FILENAME, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ESIF_TRACE_FMT_DEBUG(fmt, ...) \
	ESIF_TRACEFUNC(ESIF_KERN_DEBUG "~DBG~ %s:%d: %s: " fmt, \
		ESIF_FILENAME, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ESIF_TRACE_FMT_ENTRY() \
	ESIF_TRACEFUNC(ESIF_KERN_DEBUG "--> %s:%d: %s:...", \
		ESIF_FILENAME, __LINE__, __FUNCTION__)
#define ESIF_TRACE_FMT_EXIT() \
	ESIF_TRACEFUNC(ESIF_KERN_DEBUG "<-- %s:%d: %s:...", \
		ESIF_FILENAME, __LINE__, __FUNCTION__)
#define ESIF_TRACE_FMT_EXIT_W_STATUS(status) \
	ESIF_TRACEFUNC(ESIF_KERN_DEBUG "<-- %s:%d: %s: Exit status = 0x%08X", \
		ESIF_FILENAME, __LINE__, __FUNCTION__, status)

#define ESIF_TRACE_CATEGORY_ON(module, category) \
	((ESIF_TRACE_CATEGORY_TO_TRACELEVEL(category) <= g_esif_trace_level) && \
	 ((1 << (module)) & g_esif_module_mask) != 0 && \
	 ((1 << (category)) & g_esif_module_category_mask[(module)]) != 0)

#define ESIF_TRACE_ERROR(format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_ERROR)) \
			ESIF_TRACE_FMT_ERROR(format, ##__VA_ARGS__); \
	} while (ESIF_ALWAYSFALSE)

/* Compile out all Trace messages except ERROR for Release Candidates 
 * Include messages in regular RELEASE and DEBUG builds
 */
#ifdef ESIF_ATTR_RELEASE_CAND

#define ESIF_TRACE_WARN(format, ...)		ESIF_TRACENULL
#define ESIF_TRACE_INFO(format, ...)		ESIF_TRACENULL
#define ESIF_TRACE_DEBUG(format, ...)		ESIF_TRACENULL
#define ESIF_TRACE_ENTRY()			ESIF_TRACENULL
#define ESIF_TRACE_EXIT()			ESIF_TRACENULL
#define ESIF_TRACE_EXIT_W_STATUS(status)	ESIF_TRACENULL
#define ESIF_TRACE_DYN(module, module_level, format, ...)       ESIF_TRACENULL

#else

#define ESIF_TRACE_WARN(format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_WARN)) \
			ESIF_TRACE_FMT_WARN(format, ##__VA_ARGS__); \
	} while (ESIF_ALWAYSFALSE)

#define ESIF_TRACE_INFO(format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_INFO)) \
			ESIF_TRACE_FMT_INFO(format, ##__VA_ARGS__); \
	} while (ESIF_ALWAYSFALSE)

#define ESIF_TRACE_DEBUG(format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_DEBUG)) \
			ESIF_TRACE_FMT_DEBUG(format, ##__VA_ARGS__); \
	} while (ESIF_ALWAYSFALSE)

#define ESIF_TRACE_ENTRY() \
	do { \
	    if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_ENTRY_AND_EXIT)) \
			ESIF_TRACE_FMT_ENTRY(); \
	} while (ESIF_ALWAYSFALSE)

#define ESIF_TRACE_EXIT() \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_ENTRY_AND_EXIT)) \
			ESIF_TRACE_FMT_EXIT(); \
	} while (ESIF_ALWAYSFALSE)

#define ESIF_TRACE_EXIT_W_STATUS(status) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_ENTRY_AND_EXIT)) \
			ESIF_TRACE_FMT_EXIT_W_STATUS(status); \
	} while (ESIF_ALWAYSFALSE)

#define ESIF_TRACE_DYN(module, module_level, format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(module, module_level)) \
			ESIF_TRACE_FMT_DEBUG(format, ##__VA_ARGS__); \
	} while (ESIF_ALWAYSFALSE)

#endif

/* Initialize the enabled module debug categories */
void esif_debug_init_module_categories(void);

/* Sets the mask of modules with debugging on */
void esif_debug_set_modules(u32 module_mask);

/* Gets the mask of modules with debugging on */
void esif_debug_get_modules(u32 *module_mask_ptr);

/* Sets the debugging category mask of a particular module */
void esif_debug_set_module_category(u32 module, u32 module_level_mask);

/* Gets the debugging category mask of a particular module */
void esif_debug_get_module_category(u32 module, u32 *module_level_mask_ptr);

/* Kernel-mode Debug Module and Category Masks */
extern u32 g_esif_trace_level;
extern u32 g_esif_module_mask;
extern u32 g_esif_module_category_mask[ESIF_DEBUG_MOD_MAX];

/* Kernel Memory Statistics */
extern struct esif_memory_stats g_memstat;

#else /* NOT ESIF_ATTR_KERNEL */

/* USER Mode Debug Interface */

#ifdef ESIF_ATTR_OS_WINDOWS
/* Avoid "conditional expression is constant" warnings in macros */
static char g_alwaysfalse;
#define ESIF_ALWAYSFALSE (!&g_alwaysfalse)

/* Avoid Klocworks "suspicious semicolon" warnings */
#define ESIF_TRACENULL (0)
#define ESIF_TRACEFUNC CMD_OUT
#define ESIF_FILENAME __FILE__

#endif
#ifdef ESIF_ATTR_OS_LINUX
#define ESIF_ALWAYSFALSE (0)
#define ESIF_TRACENULL
#define ESIF_TRACEFUNC printf
#define ESIF_FILENAME __FILE__
#endif

// Enumerated Type Base Macros
#define ENUMDECL(ENUM)              ENUM,
#define ENUMLIST(ENUM)              {ENUM, #ENUM},
#define ENUMSWITCH(ENUM)            case ENUM: return #ENUM;break;

// Enumerated Type Macros with Explicit Values
#define ENUMDECL_VAL(ENUM, VAL)      ENUM = VAL,
#define ENUMLIST_VAL(ENUM, VAL)      ENUMLIST(ENUM)
#define ENUMSWITCH_VAL(ENUM, VAL)    ENUMSWITCH(ENUM)

// Trace Module and Route Masks
typedef u32		esif_tracemask_t;
typedef u8		esif_traceroute_t;
#define esif_tracemask_fmt	"%u"	// %u or %llu
#define esif_tracemask_fmtx	"%x"	// %x or %llx
#define ESIF_TRACEMASK_MAX		32
#define ESIF_TRACEMAPPING_MAX	256

// ESIF_TRACE_ID must be #defined in each source file before any #includes
// Otherwise the module will use settings for DEFAULT module
#ifndef ESIF_TRACE_ID
# define ESIF_TRACE_ID		ESIF_TRACEMODULE_DEFAULT
#endif
#define ESIF_TRACEMASK(module)	((esif_tracemask_t)1 << (module))

// Trace Level Data
struct esif_tracelevel_s {
	const char *		label;		// label, i.e. "ERROR"
	int					level;		// level, i.e, 1
	esif_tracemask_t	modules;	// active module bitmask
	esif_traceroute_t	routes;		// active routes bitmask
};

// Enumerated TraceModule Values
#define ENUM_TRACEMODULE(ENUM)	\
	ENUM(ESIF_TRACEMODULE_DEFAULT)		/* Generic Module - Use Default Trace Settings*/ \
	ENUM(ESIF_TRACEMODULE_DPTF)			/* DPTF Loadable App */ \
	\
	ENUM(ESIF_TRACEMODULE_CCB)			/* CCB Modules mixed with Kernel Tracing */ \
	ENUM(ESIF_TRACEMODULE_SHELL)		/* ESIF Shell */ \
	ENUM(ESIF_TRACEMODULE_UF)			/* UF Main */ \
	\
	ENUM(ESIF_TRACEMODULE_ACTION)		/* Actions */ \
	ENUM(ESIF_TRACEMODULE_APP)			/* Apps */ \
	ENUM(ESIF_TRACEMODULE_CONJURE)		/* Conjure */ \
	ENUM(ESIF_TRACEMODULE_DSP)			/* DSPs */ \
	ENUM(ESIF_TRACEMODULE_EVENT)		/* Events */ \
	ENUM(ESIF_TRACEMODULE_IPC)			/* IPC */ \
	ENUM(ESIF_TRACEMODULE_PARTICIPANT)	/* Upper Participants */ \
	ENUM(ESIF_TRACEMODULE_PRIMITIVE)	/* Primitives */ \
	ENUM(ESIF_TRACEMODULE_SERVICE)		/* Services */ \
	ENUM(ESIF_TRACEMODULE_TEST)			/* Tests */ \
	\
	ENUM(ESIF_TRACEMODULE_DATAVAULT)	/* DataVaults/DataCache/DataBank */ \
	ENUM(ESIF_TRACEMODULE_EQL)			/* EQL Parser/IString/EsifData */ \
	ENUM(ESIF_TRACEMODULE_WEBSERVER)	/* Web Socket Server */ \
	\
	ENUM(ESIF_TRACEMODULE_WINDOWS)		/* Windows General */ \
	ENUM(ESIF_TRACEMODULE_ACTWIRELESS)	/* Windows ACT Wireless */ \
	ENUM(ESIF_TRACEMODULE_UMDF)			/* Windows UMDF */ \

enum esif_tracemodule {
	ENUM_TRACEMODULE(ENUMDECL)
};

#define	ESIF_TRACEMASK_ALL	((esif_tracemask_t)-1)	/* all bits */

struct esif_tracemodule_s {
	const char 				*module;
	enum esif_tracemodule	module_id;
};

/* ESIF_UF Trace Routes */
#define ESIF_TRACEROUTE_CONSOLE		1	/* Shell Console */
#define ESIF_TRACEROUTE_EVENTLOG	2	/* Windows=EventLog Linux=syslog */
#define ESIF_TRACEROUTE_DEBUGGER	4	/* Windows=DebugView Linux=syslog */
#define ESIF_TRACEROUTE_LOGFILE		8	/* Trace Log File (create with "trace log open <file>") */

/* Do not access these functions and variables directly in any code, just the macros at the bottom */
extern int g_traceLevel;
extern const char *g_tracelabel[];
extern int g_traceLevel_max;
extern struct esif_tracelevel_s g_traceinfo[];

extern const enum esif_tracemodule EsifTraceModule_FromString(const char *name);
extern const char *EsifTraceModule_ToString(enum esif_tracemodule val);
extern int EsifTraceMessage(esif_tracemask_t module, int level, const char *func, const char *file, int line, const char *msg, ...);

/* Test whether Tracing is currently active for the given module and level based on the currrent trace level*/
#define ESIF_TRACEACTIVE(module, level) ((g_traceLevel >= (level)) && !!(g_traceinfo[level].modules & (module)))

/* Test whether Tracing is currently enabled for the given module and level (regardless of current trace level) */
#define ESIF_TRACEENABLED(module, level) (!!(g_traceinfo[level].modules & (module)))

/* Conditionally route trace message if given module/level are currently active, based on the current trace level.
 * Optional arguments are not evaluated and message string not created if message not routed
 */
#define ESIF_DOTRACE_IFACTIVE(module, level, msg, ...) \
	do { \
		if (ESIF_TRACEACTIVE(module, level)) { \
			EsifTraceMessage(module, level,	__FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__); \
		} \
	} while (ESIF_ALWAYSFALSE)

/* Conditionally route trace message if given module/level are currently enabled, regardless of the current trace level.
 * Optional arguments are not evaluated and message string not created if message not routed
 */
#define ESIF_DOTRACE_IFENABLED(module, level, msg, ...) \
	do { \
		if (ESIF_TRACEENABLED(module, level)) { \
			EsifTraceMessage(module, level,	__FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__); \
		} \
	} while (ESIF_ALWAYSFALSE)

/* Always route trace message regardless of current module masks and trace level */
#define ESIF_DOTRACE_ALWAYS(module, level, msg, ...) \
	EsifTraceMessage(module, level,	__FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__)

/* Never route a trace message and compile it out of the binary */
#define ESIF_DOTRACE_NEVER(mod, lev, msg, ...)	((void)0)

/* Compile out DEBUG-level messages for Release Candidate Builds */
#ifdef ESIF_ATTR_RELEASE_CAND
# define ESIF_DOTRACE_IFCOMPILED(mod, lev, msg, ...) ESIF_DOTRACE_NEVER(mod, lev, msg)
# define ESIF_TRACELEVEL_DEFAULT	ESIF_TRACELEVEL_ERROR
#else
# define ESIF_DOTRACE_IFCOMPILED(mod, lev, msg, ...) ESIF_DOTRACE_IFACTIVE(mod, lev, msg, ##__VA_ARGS__)
# define ESIF_TRACELEVEL_DEFAULT	ESIF_TRACELEVEL_ERROR
#endif

/*****************************************************************************
 * App Interface for Trace messages. Only use these macros in source modules.
 ****************************************************************************/

/* ESIF_TRACE_XXXX messages are conditionally routed depending on current trace level and module masks.
 * ESIF_DOTRACE_IFCOMPILED is used for trace levels that may be conditionally compiled out of the binary
 */
#define ESIF_TRACE_FATAL(msg, ...) ESIF_DOTRACE_IFACTIVE(ESIF_TRACEMASK(ESIF_TRACE_ID), ESIF_TRACELEVEL_FATAL, msg, ##__VA_ARGS__)
#define ESIF_TRACE_ERROR(msg, ...) ESIF_DOTRACE_IFACTIVE(ESIF_TRACEMASK(ESIF_TRACE_ID), ESIF_TRACELEVEL_ERROR, msg, ##__VA_ARGS__)
#define ESIF_TRACE_WARN(msg, ...)  ESIF_DOTRACE_IFACTIVE(ESIF_TRACEMASK(ESIF_TRACE_ID), ESIF_TRACELEVEL_WARN, msg, ##__VA_ARGS__)
#define ESIF_TRACE_INFO(msg, ...)  ESIF_DOTRACE_IFACTIVE(ESIF_TRACEMASK(ESIF_TRACE_ID), ESIF_TRACELEVEL_INFO, msg, ##__VA_ARGS__)
#define ESIF_TRACE_DEBUG(msg, ...) ESIF_DOTRACE_IFCOMPILED(ESIF_TRACEMASK(ESIF_TRACE_ID), ESIF_TRACELEVEL_DEBUG, msg, ##__VA_ARGS__)

/* ESIF_TRACE_IFACTIVE messages are always compiled into binary and routed if module/level active, based on current trace level  */
#define ESIF_TRACE_IFACTIVE(module, level, msg, ...)  ESIF_DOTRACE_IFACTIVE(ESIF_TRACEMASK(module), level, msg, ##__VA_ARGS__)

/* ESIF_TRACE_IFENABLED messages are always compiled into binary and routed if module/level enabled, regardles of current trace level */
#define ESIF_TRACE_IFENABLED(module, level, msg, ...) ESIF_DOTRACE_IFENABLED(ESIF_TRACEMASK(module), level, msg, ##__VA_ARGS__)

#endif /* ESIF_ATTR_USER */

#endif	/* _ESIF_DEBUG_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
