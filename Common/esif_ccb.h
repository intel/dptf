/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

/* OS Agnostic */
#ifdef ESIF_ATTR_USER
#include <stdio.h>
#else
#include <stddef.h>
#endif

#ifdef ESIF_ATTR_OS_WINDOWS

/* Windows OS */

#ifdef ESIF_ATTR_USER
#define _WINSOCKAPI_ /* Override for Winsock */
#include <windows.h>
#else
#include <ntddk.h>
#endif

#define ESIF_ATTR_OS	"Windows"		/* OS Is Windows */
#define ESIF_INLINE	__inline		/* Inline Function Directive */
#define ESIF_FUNC	__FUNCTION__		/* Current Function Name */
#define ESIF_CALLCONV	__cdecl			/* SDK Calling Convention */
#define ESIF_PATH_SEP	"\\"			/* Path Separator String */
#define ESIF_EXPORT	__declspec(dllexport)	/* Used for Exported Symbols */

typedef	char *esif_string;

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


/* Add Linux Base Types for Windows */
typedef unsigned char u8;	/* A BYTE  */
typedef unsigned short u16;	/* A WORD  */
typedef unsigned int u32;	/* A DWORD */
typedef unsigned long long u64;	/* A QWORD */

#endif /* WINDOWS */

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

#ifdef ESIF_ATTR_OS_LINUX

/* All Linux Derived OS */

#ifdef ESIF_ATTR_USER
#include <unistd.h>	/* POSIX API */
#endif

#ifndef ESIF_ATTR_OS
#define ESIF_ATTR_OS	"Linux"			/* OS Is Generic Linux */
#endif
#define ESIF_INLINE	inline			/* Inline Function Directive */
#define ESIF_FUNC	__func__		/* Current Function Name */
#define ESIF_CALLCONV				/* Func Calling Convention */
#define ESIF_PATH_SEP	"/"			/* Path Separator String */
#define ESIF_EXPORT				/* Used for Exported Symbols */

#ifdef ESIF_ATTR_KERNEL
#define esif_string char *	/* opaque: use #define instead of typedef */
#else
typedef char *esif_string;
#endif

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

#define UNREFERENCED_PARAMETER(x) /* Avoid Unused Variable warnings */

/* Sleep Interface */
#define esif_ccb_sleep(sec)		sleep(sec)
#define esif_ccb_sleep_msec(msec)	usleep(msec * 1000)

/* Deduce Platform based on predefined compiler flags */
#ifdef __x86_64__
#define ESIF_ATTR_64BIT
#endif

/* Add Linux Base Types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#ifdef __cplusplus
#define ESIF_ELEMENT(x)		/* C99 Designated Initializers unsupported in C++ */
#else
#define ESIF_ELEMENT(x)	x =	/* Support C99 Designated Initializers */
#endif

/* Used for constant expressions, such as do...while(0) loops */
#define ESIF_CONSTEXPR(expr)  (expr)

#endif /* LINUX */

/*
 * OS Agnostic
 */
#if defined(ESIF_ATTR_USER)
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
