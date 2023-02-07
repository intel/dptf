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

#pragma once

#if !defined(ESIF_ATTR_KERNEL) && !defined(ESIF_ATTR_USER)
/* User Mode Build */
#define ESIF_ATTR_USER
#endif

#if defined(_WIN32)
#if !defined(ESIF_ATTR_OS_WINDOWS)
/* Windows OS */
#define ESIF_ATTR_OS_WINDOWS
#endif
#else
#if !defined(ESIF_ATTR_OS_LINUX) && !defined(ESIF_ATTR_OS_CHROME) && !defined(ESIF_ATTR_OS_ANDROID)
/* Linux Derived OS */
#define ESIF_ATTR_OS_LINUX
#endif
#endif

#ifdef ESIF_ATTR_OS_CHROME
/* Linux Derived OS */
#define ESIF_ATTR_OS_LINUX
#define ESIF_ATTR_OS	"Chrome"  /* OS Is Chromium */
#endif
#ifdef ESIF_ATTR_OS_ANDROID
/* Linux Derived OS */
#define ESIF_ATTR_OS_LINUX
#define ESIF_ATTR_OS	"Android" /* OS Is Android */
#endif
#if defined(ESIF_ATTR_OS_LINUX) && !defined(_GNU_SOURCE)
/* -std=gnu99 support */
#define _GNU_SOURCE
#endif
#if defined(ESIF_ATTR_OS_LINUX) && !defined(ESIF_ATTR_OS_ANDROID) && defined(ESIF_ATTR_USER) && !defined(__x86_64__)
/* Large NFS support for 32-bit stat() on Linux */
#define _FILE_OFFSET_BITS 64
#endif

/* OS Agnostic */
#ifdef ESIF_ATTR_USER
#include <stdio.h>
#include <stdlib.h>
#else
#include <stddef.h>
#endif

#ifdef ESIF_ATTR_OS_WINDOWS

/* Windows OS */
#pragma strict_gs_check(on)

#ifdef ESIF_ATTR_USER
#define _WINSOCKAPI_ /* Override for Winsock */
#include <windows.h>

/* Avoids Serialization Warnings*/
#define ESIF_SERIAL_FENCE()	_mm_lfence()

#else
#include <ntddk.h>
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)	/* Invalid ESIF Handle */
#endif

/* Add Linux Base Types for Windows */
typedef unsigned char u8;	/* A BYTE  */
typedef unsigned short u16;	/* A WORD  */
typedef unsigned int u32;	/* A DWORD */
typedef unsigned long long u64;	/* A QWORD */

typedef	char *esif_string;		/* NULL-teriminated ANSI string */
typedef HANDLE esif_os_handle_t;/* opaque OS Handle (not a pointer) */
typedef u64 esif_handle_t;	/* opaque ESIF 64-bit handle (may NOT be a pointer) */
typedef u64 esif_context_t;	/* opaque ESIF 64-bit context (may be a pointer) */

#define ESIF_ATTR_OS		"Windows"				/* OS Is Windows */
#define ESIF_INLINE			__inline				/* Inline Function Directive */
#define ESIF_FUNC			__FUNCTION__			/* Current Function Name */
#define ESIF_CALLCONV		__cdecl					/* SDK Calling Convention */
#define ESIF_PATH_SEP		"\\"					/* Path Separator String */
#define ESIF_EXPORT			__declspec(dllexport)	/* Used for Exported Symbols */
#define ESIF_INVALID_HANDLE	((esif_handle_t)(-1))	/* Invalid ESIF Handle */

#define ESIF_HANDLE_DEFAULT ((esif_handle_t)(0))        /* Reserved ESIF handle */
#define ESIF_HANDLE_PRIMARY_PARTICIPANT ((esif_handle_t)(1))   /* Reserved ESIF primary participant handle */
#define ESIF_HANDLE_MATCH_ANY_EVENT ((esif_handle_t)(-2)) /* Reserved ESIF handle */

#define	ESIF_WS_LIBRARY_NAME	"ipf_ws"	/* Legacy Library/App Name for deprecated in-process web server*/

#define esif_ccb_isfullpath(fname)	(fname[0] == '\\' || (isalpha(fname[0]) && fname[1] == ':'))

#ifdef __cplusplus
#define ESIF_ELEMENT(x)  /* C99 Designated Initializers unsupported in C++ */
#else
#define ESIF_ELEMENT(x)	x =	/* Support C99 Designated Initializers */
#endif

/* Avoids "conditional constant is an expression" warnings for do...while(0) loops */
#define ESIF_CONSTEXPR(expr)  __pragma(warning(push)) __pragma(warning(disable:4127)) (expr) __pragma(warning(pop))

/* Sleep Interface */
#define esif_ccb_sleep(sec)		Sleep(sec * 1000)
#define esif_ccb_sleep_msec(msec)	Sleep(msec)

/* Deduce Platform based on predefined compiler flags */
#ifdef _WIN64
#define ESIF_ATTR_64BIT
#endif

/* Deduce Debug Build for Windows. Non-Windows can define this in Makefile */
#ifdef _DEBUG
#define ESIF_ATTR_DEBUG
#endif

#ifdef ESIF_ATTR_DEBUG
#ifdef ESIF_ATTR_USER
#define ESIF_ASSERT(x)			\
	do {				\
		if ESIF_CONSTEXPR(!(x))	\
			DebugBreak();	\
	} while ESIF_CONSTEXPR(0)

#else /* !ESIF_ATTR_USER */
#define ESIF_ASSERT(x) ASSERT(x)
#endif /* !ESIF_ATTR_USER */
#else /* !ESIF_ATTR_DEBUG */
#define ESIF_ASSERT(x) (0)
#endif /* !ESIF_ATTR_DEBUG */


#ifdef ESIF_ATTR_USER
/* Byte Ordering Utilities (Winsock2.h) */
#define esif_ccb_htons(val)		htons(val)
#define esif_ccb_htonl(val)		htonl(val)
#define esif_ccb_htonll(val)	htonll(val)
#endif

#endif /* WINDOWS */

#ifdef ESIF_ATTR_OS_LINUX

/* All Linux Derived OS */

#ifdef ESIF_ATTR_USER
#include <unistd.h>	/* POSIX API */

/* Common Windows Symbols */
#define MAX_PATH 260
#define INVALID_HANDLE_VALUE ((esif_os_handle_t)(-1))	/* Invalid OS Handle */

#define STATUS_SUCCESS 0

/* Avoids Serialization SDL Warnings*/
#include <x86intrin.h>
#define ESIF_SERIAL_FENCE()	_mm_lfence()

#endif

/* Add Linux Base Types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#ifndef ESIF_ATTR_OS
#define ESIF_ATTR_OS		"Linux"			/* OS Is Generic Linux */
#endif
#define ESIF_INLINE			inline		/* Inline Function Directive */
#define ESIF_FUNC			__func__	/* Current Function Name */
#define ESIF_CALLCONV					/* Func Calling Convention */
#define ESIF_PATH_SEP		"/"			/* Path Separator String */
#define ESIF_EXPORT			__attribute__((visibility("default")))	/* Used for Exported Symbols */
#define ESIF_INVALID_HANDLE	((esif_handle_t)(-1))	/* Invalid ESIF Handle */

#define ESIF_HANDLE_DEFAULT ((esif_handle_t)(0))        /* Reserved ESIF handle */
#define ESIF_HANDLE_PRIMARY_PARTICIPANT ((esif_handle_t)(1))   /* Reserved ESIF handle */
#define ESIF_HANDLE_MATCH_ANY_EVENT ((esif_handle_t)(-2))	/* Reserved ESIF handle */

#define	ESIF_WS_LIBRARY_NAME	"esif_ws"	/* Legacy Library/App Name for deprecated in-process web server*/

#define esif_ccb_isfullpath(fname)	(fname[0] == '/')

typedef char *esif_string;		/* NULL-terminated ANSI string */
typedef int   esif_os_handle_t;	/* opaque OS Handle (not a pointer) */
typedef u64 esif_handle_t;	/* opaque ESIF 64-bit handle (may not be a pointer) */
typedef u64 esif_context_t;	/* opaque ESIF 64-bit context (may be a pointer) */

#ifdef ESIF_ATTR_DEBUG
# ifdef ESIF_ATTR_USER
#  include <assert.h>
#  define ESIF_ASSERT(x)   assert(x)
# else
#  define ESIF_ASSERT(x)	\
	   do {			\
			  if (x)		\
					 break;	\
			  printk(KERN_EMERG "!ESIF_ASSERT! [%s@%s#%d]: %s\n", ESIF_FUNC, __FILE__, __LINE__, #x); \
			  BUG();		\
	   } while (0)
# endif
#else
# define ESIF_ASSERT(x)
#endif

#define UNREFERENCED_PARAMETER(x) (void)(x) /* Avoid Unused Variable Klocwork errors */

/* Sleep Interface */
#define esif_ccb_sleep(sec)		sleep(sec)
#define esif_ccb_sleep_msec(msec)	usleep(msec * 1000)

/* Deduce Platform based on predefined compiler flags */
#ifdef __x86_64__
#define ESIF_ATTR_64BIT
#endif

#ifdef __cplusplus
#define ESIF_ELEMENT(x)		/* C99 Designated Initializers unsupported in C++ */
#else
#define ESIF_ELEMENT(x)	x =	/* Support C99 Designated Initializers */
#endif

/* Used for constant expressions, such as do...while(0) loops */
#define ESIF_CONSTEXPR(expr)  (expr)

#ifdef ESIF_ATTR_USER
/* Byte Ordering Utilities */
#include <arpa/inet.h> /* htonl */
#define esif_ccb_htons(val)		htons(val)
#define esif_ccb_htonl(val)		htonl(val)

static ESIF_INLINE u64 esif_ccb_htonll(u64 value)
{
	u32 hi = htonl((u32)(value >> 32));
	u32 lo = htonl((u32)value);
	return (((u64)lo) << 32) | hi;
}
#endif

#endif /* LINUX */

/*
 * OS Agnostic
 */
#if defined(ESIF_ATTR_USER) && !defined(ESIF_ATTR_NO_TYPES)
typedef u8   		UInt8;	/* A CCB BYTE  */
typedef char 		Int8;
typedef u16  		UInt16;	/* A CCB WORD  */
typedef short 		Int16;
typedef u32 		UInt32;	/* A CCB DWORD */
typedef int     	Int32;
typedef u64 		UInt64;	/* A CCB QWORD */
typedef long long 	Int64;
typedef unsigned int 	UIntN;	/* A CCB UINT */
typedef int 		IntN;
#ifdef __cplusplus
typedef bool  		Bool;	/* C++ BOOLEAN */
#else
typedef u8		Bool;	/* C BOOLEAN */
#endif
#endif

/*
 * OS and Kernel/User Agnostic
 */
#define esif_ccb_max(a, b)	((a) >= (b) ? (a) : (b))
#define esif_ccb_min(a, b)	((a) <= (b) ? (a) : (b))
#define esif_ccb_handle2llu(h)	((unsigned long long)(h))	/* for use with "%llu" formats !!! for ESIF handles only !!! */
#define esif_ccb_os_handle2llu(h)	((unsigned long long)(size_t)(h))	/* for use with "%llu" formats !!! for OS handles only !!! */
#define esif_ccb_value2ptr(v)	((void *)(size_t)(v))	/* for use with "%p" formats or casting int types to (void *) */

/* Helper macros for ptr/context conversion */
#define esif_ccb_ptr2context(p)	((esif_context_t)(size_t)(p))
#define esif_ccb_context2ptr(u)	((void *)(size_t)(u))

#define ESIF_HANDLE_FMT "0x%016llX"
#define OS_HANDLE_FMT "%llu"

/*
 * Macros required for esif_rc and esif_sdk headers
 */

/* ID Lookup Failed To Find String */
#define ESIF_NOT_AVAILABLE (esif_string) "NA"

/* Used for Value-To-String case statements */
#define ESIF_CASE_ENUM(e)	case e: return (esif_string) #e
#define ESIF_CASE(e, val)	case e: return (esif_string) val
#define ESIF_CASE_VAL(e, val)	case e: return val

/* Static array length */
#define ESIF_ARRAY_LEN(a)	(sizeof(a) / sizeof(a[0]))

/* Used for enum-to-string map tables */
#define ESIF_MAP_ENUM(val)	{ val, (esif_string) #val }
#define ESIF_MAP(id, val)	{ (esif_string) id, (esif_string) val }

/* True and False */
#define ESIF_TRUE	1
#define ESIF_FALSE	0

/* Invalid or Undefined enum type value */
#define ESIF_INVALID_ENUM_VALUE  (-1)

/* Platform Architecture Type */
#ifdef ESIF_ATTR_64BIT
#define ESIF_PLATFORM_TYPE "x64"
#else
#define ESIF_PLATFORM_TYPE "x86"
#endif

/* Build Type */
#ifdef ESIF_ATTR_DEBUG
#define ESIF_BUILD_TYPE	"Debug"
#else
#define ESIF_BUILD_TYPE "Release"
#endif
