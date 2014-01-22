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
		ESIF_CREATE_MOD(ESIF_DEBUG_MOD_DSP, DSPi, str)
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
 *    ESIF_TRACE_WARNING
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
 */
#define ESIF_TRACE_CATEGORY_ERROR           31
#define ESIF_TRACE_CATEGORY_WARN            30
#define ESIF_TRACE_CATEGORY_INFO            29
#define ESIF_TRACE_CATEGORY_DEBUG           28
#define ESIF_TRACE_CATEGORY_ENTRY_AND_EXIT  27

#define ESIF_TRACE_CATEGORY_DEFAULT   ((u32)1 << ESIF_TRACE_CATEGORY_ERROR)

/* Linux */
#ifdef ESIF_ATTR_OS_LINUX
#define ESIF_WHILEFALSE (0) /*used for do...while(0) macros */
#define ESIF_TRACENULL
#define ESIF_TRACEFUNC  printk
#define ESIF_KERN_ERR   KERN_ERR
#define ESIF_KERN_INFO  KERN_INFO
#define ESIF_KERN_DEBUG KERN_DEBUG
#define ESIF_FILENAME   __FILE__/* or (strrchr(__FILE__,'/') ?
					*strrchr(__FILE__,'/')+1 : __FILE__) */
#endif /* ESIF_ATTR_OS_LINUX */


/* Windows */
#ifdef ESIF_ATTR_OS_WINDOWS
/* avoids "conditional expression is constant" warnings for do...while(0) macros in Windows */
static char g_whilefalse;
#define ESIF_WHILEFALSE (!&g_whilefalse)

#define ESIF_TRACENULL  (0)
#define ESIF_TRACEFUNC  DbgPrint
#define ESIF_KERN_ERR
#define ESIF_KERN_INFO
#define ESIF_KERN_DEBUG
#define ESIF_FILENAME   __FILE__/* or (strrchr(__FILE__,'\\') ?
					*strrchr(__FILE__,'\\')+1 : __FILE__) */
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

#define ESIF_TRACE_CATEGORY_ON(module, module_level) \
	(((1 << (module)) & g_esif_module_mask) != 0 && \
	 ((1 << (module_level)) & g_esif_module_category_mask[(module)]) != 0)

#define ESIF_TRACE_ERROR(format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_ERROR)) \
			ESIF_TRACE_FMT_ERROR(format, ##__VA_ARGS__); \
	} while (ESIF_WHILEFALSE)

#ifdef ESIF_ATTR_DEBUG

#define ESIF_TRACE_WARN(format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_WARN)) \
			ESIF_TRACE_FMT_WARN(format, ##__VA_ARGS__); \
	} while (ESIF_WHILEFALSE)

#define ESIF_TRACE_INFO(format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_INFO)) \
			ESIF_TRACE_FMT_INFO(format, ##__VA_ARGS__); \
	} while (ESIF_WHILEFALSE)

#define ESIF_TRACE_DEBUG(format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_DEBUG)) \
			ESIF_TRACE_FMT_DEBUG(format, ##__VA_ARGS__); \
	} while (ESIF_WHILEFALSE)

#define ESIF_TRACE_ENTRY() \
	do { \
	    if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_ENTRY_AND_EXIT)) \
			ESIF_TRACE_FMT_ENTRY(); \
	} while (ESIF_WHILEFALSE)

#define ESIF_TRACE_EXIT() \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_ENTRY_AND_EXIT)) \
			ESIF_TRACE_FMT_EXIT(); \
	} while (ESIF_WHILEFALSE)

#define ESIF_TRACE_EXIT_W_STATUS(status) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(ESIF_DEBUG_MODULE, ESIF_TRACE_CATEGORY_ENTRY_AND_EXIT)) \
			ESIF_TRACE_FMT_EXIT_W_STATUS(status); \
	} while (ESIF_WHILEFALSE)

#define ESIF_TRACE_DYN(module, module_level, format, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(module, module_level)) \
			ESIF_TRACE_FMT_DEBUG(format, ##__VA_ARGS__); \
	} while (ESIF_WHILEFALSE)

#else /* DBG */
#define ESIF_TRACE_WARN(format, ...)		ESIF_TRACENULL
#define ESIF_TRACE_INFO(format, ...)		ESIF_TRACENULL
#define ESIF_TRACE_DEBUG(format, ...)		ESIF_TRACENULL
#define ESIF_TRACE_ENTRY()					ESIF_TRACENULL
#define ESIF_TRACE_EXIT()					ESIF_TRACENULL
#define ESIF_TRACE_EXIT_W_STATUS(status)	ESIF_TRACENULL
#define ESIF_TRACE_DYN(module, module_level, format, ...)       ESIF_TRACENULL
#endif /* NOT ESIF_ATTR_DEBUG */

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
extern u32 g_esif_module_mask;
extern u32 g_esif_module_category_mask[ESIF_DEBUG_MOD_MAX];

/* Kernel Memory Statistics */
extern struct esif_memory_stats g_memstat;

#else /* NOT ESIF_ATTR_KERNEL */

/* USER Mode Debug Interface */

#ifdef ESIF_ATTR_OS_WINDOWS
/* Avoid "conditional expression is constant" warnings in macros */
static char g_whilefalse;
#define ESIF_WHILEFALSE (!&g_whilefalse)

/* Avoid Klocworks "suspicious semicolon" warnings */
#define ESIF_TRACENULL (0)
#define ESIF_TRACEFUNC printf
#define ESIF_FILENAME __FILE__

#else
#define ESIF_WHILEFALSE (0)
#define ESIF_TRACENULL
#define ESIF_TRACEFUNC printf
#define ESIF_FILENAME __FILE__
#endif

/* These trace levels correspond to Linux printk KERN_* log levels,
 * although macros are provided only for ERROR, WARN, INFO, DEBUG
 */
#define ESIF_TRACELEVEL_FATAL           0
#define ESIF_TRACELEVEL_ALERT           1
#define ESIF_TRACELEVEL_CRITICAL        2
#define ESIF_TRACELEVEL_ERROR           3
#define ESIF_TRACELEVEL_WARN            4
#define ESIF_TRACELEVEL_NOTICE          5
#define ESIF_TRACELEVEL_INFO            6
#define ESIF_TRACELEVEL_DEBUG           7
#define ESIF_TRACELEVEL_VERBOSE		8
#define ESIF_TRACELEVEL_VERBOSE_EX	9

#define	ESIF_TRACEMODULE			1	/* Single for now */

/* Only Basic Application-wide Trace Level for now */
extern int g_traceLevel;
#define ESIF_TRACE_CATEGORY_ON(module, module_level) \
	(g_traceLevel >= module_level)

#define ESIF_TRACE_MESSAGE(module, module_level, fmt, ...) \
	do { \
		if (ESIF_TRACE_CATEGORY_ON(module, module_level)) \
			ESIF_TRACEFUNC(fmt, ##__VA_ARGS__); \
	} while (ESIF_WHILEFALSE)

/* DEBUG Traces are disabled in DEBUG Builds.
 * All other modes always available in both Release/Debug Builds*/
#ifdef ESIF_ATTR_DEBUG
# define ESIF_TRACELEVEL_DEFAULT		ESIF_TRACELEVEL_ERROR
#else
# define ESIF_TRACELEVEL_DEFAULT		ESIF_TRACELEVEL_ERROR
# define ESIF_TRACE_DEBUG_DISABLED
#endif

#ifdef ESIF_TRACE_DEBUG_DISABLED
# define ESIF_TRACE_DEBUG(fmt, ...)		ESIF_TRACENULL
# define ESIF_TRACE_VERBOSE(fmt, ...)		ESIF_TRACENULL
# define ESIF_TRACE_VERBOSE_EX(fmt, ...)	ESIF_TRACENULL
#else
#define ESIF_TRACE_DEBUG(fmt, ...) \
	ESIF_TRACE_MESSAGE(ESIF_TRACEMODULE, \
			   ESIF_TRACELEVEL_DEBUG, \
			   fmt, \
			   ##__VA_ARGS__)
#define ESIF_TRACE_VERBOSE(fmt, ...) \
	ESIF_TRACE_MESSAGE(ESIF_TRACEMODULE, \
			   ESIF_TRACELEVEL_VERBOSE, \
			   fmt, \
			   ##__VA_ARGS__)
#define ESIF_TRACE_VERBOSE_EX(fmt, ...) \
	ESIF_TRACE_MESSAGE(ESIF_TRACEMODULE, \
			   ESIF_TRACELEVEL_VERBOSE_EX, \
			   fmt, \
			   ##__VA_ARGS__)
#endif

#define ESIF_TRACE_INFO(fmt, ...) \
	ESIF_TRACE_MESSAGE(ESIF_TRACEMODULE, \
			   ESIF_TRACELEVEL_INFO, \
			   fmt, \
			   ##__VA_ARGS__)
#define ESIF_TRACE_WARN(fmt, ...) \
	ESIF_TRACE_MESSAGE(ESIF_TRACEMODULE, \
			   ESIF_TRACELEVEL_WARN, \
			   fmt, \
			   ##__VA_ARGS__)
#define ESIF_TRACE_ERROR(fmt, ...) \
	ESIF_TRACE_MESSAGE(ESIF_TRACEMODULE, \
			   ESIF_TRACELEVEL_ERROR, \
			   fmt, \
			   ##__VA_ARGS__)
# define ESIF_TRACE_DYN(module, module_level, fmt, ...) \
	ESIF_TRACE_MESSAGE(module, module_level, fmt, ##__VA_ARGS__)

#endif /* ESIF_ATTR_USER */

#endif	/* _ESIF_DEBUG_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
