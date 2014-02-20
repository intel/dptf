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

#ifndef _ESIF_H_
#define _ESIF_H_

#define ESIF_ATTR_WEBSOCKET
typedef char *esif_string;		/* Opaque ESIF String ASCIIZ Today */

/* Linux Derrived OS */
#if defined(ESIF_ATTR_OS_LINUX) || \
    	defined(ESIF_ATTR_OS_ANDROID) || \
	defined(ESIF_ATTR_OS_CHROME)

#define ESIF_ATTR_OS "linux"	/* OS Is Linux             */
#define ESIF_INLINE  inline	/* Normalize Compiler      */
#define ESIF_FUNC __func__	/* Normalize Compiler      */
#define ESIF_PATH_SEP "/"	/* Path Seperator          */
#ifdef __cpluscplus
#define ESIF_ELEMENT(x) x =	/* Support C99 Init        */
#else /* __cpluscplus */
#define ESIF_ELEMENT(x)		/* Emulate/Ignore C99 Init */
#endif /* __cplusplus */

/* Deduce Platform based on predefined gcc compiler flags */
#ifdef __x86_64__
#define ESIF_ATTR_64BIT
#endif /* __x86_64__ */
#endif /* Linux Derrived OS */

/* Android Override For Linux */
#ifdef ESIF_ATTR_OS_ANDROID
#undef ESIF_ATTR_OS
#define ESIF_ATTR_OS "android"
#define ESIF_ATTR_OS_LINUX
#endif /* ESIF_ATTR_OS_ANDROID */

/* Chrome Override For Linux */
#ifdef ESIF_ATTR_OS_CHROME
#undef ESIF_ATTR_OS
#define ESIF_ATTR_OS "chrome"
#define ESIF_ATTR_OS_LINUX
#endif /* ESIF_ATTR_OS_CHROME */

#ifdef ESIF_ATTR_OS_WINDOWS
#define ESIF_ATTR_OS "windows"	/* OS Is Windows             */
#define ESIF_INLINE __inline	/* Normalize Compiler        */
#define ESIF_FUNC __FUNCTION__	/* Normalize Compiler        */
#define ESIF_PATH_SEP "\\"	/* Path Seperator            */
#define ESIF_ELEMENT(x)		/* Emulate/Ignore C99 Init   */
				/* Note Static Init Must Be  */
				/* In Structure Order        */

/* Build option to enable coalescable timers in Windows */
/* #define ESIF_ATTR_USE_COALESCABLE_TIMERS */

/* We Use The Linux Dictionary So Add For Windows */
typedef unsigned char u8;	/* A BYTE  */
typedef unsigned short u16;	/* A WORD  */
typedef unsigned int u32;	/* A DWORD */
typedef unsigned long long u64;	/* A QWORD */

/* Deduce Platform based on predefined compiler flags */
#ifdef _WIN64
#define ESIF_ATTR_64BIT
#endif /* _WIN64 */

/* Override windows DEBUG To ESIF or Project can include ESIF_ATTR_DEBUG */
#ifdef _DEBUG
#define ESIF_ATTR_DEBUG
#endif /* _DEBUG */
#endif /* ESIF_ATTR_OS_WINDOWS */

#ifdef ESIF_ATTR_KERNEL
#include "esif_version.h"

#ifdef ESIF_ATTR_OS_LINUX
#include <linux/module.h>		/* Linux Module           */
#include <linux/platform_device.h>	/* Linux Platform Device  */
#include <linux/acpi.h>		/* Linux ACPI Device      */
#include <linux/pci.h>		/* Linux PCI Device       */
#include <linux/thermal.h>		/* Linux Thermals         */
#include <linux/kobject.h>		/* Linux Kernel Objects   */
#include <linux/version.h>		/* Linux Kernel Version   */

/*
 * Devices Are Defined To Be Opaque to allow for OS
 * implementation of Pointer VS Handle Etc. Linux uses a
 * a device pointer.
 */

typedef struct device *esif_device_t;

#define UNREFERENCED_PARAMETER(x)
#endif /* ESIF_ATTR_KERNEL::ESIF_ATTR_OS_LINUX  */

#ifdef ESIF_ATTR_OS_WINDOWS
    #include "ntddk.h"		/* The Windows DDK          */
    #include "wdf.h"		/* Windows Driver Framework */
    #include "INITGUID.H"	/* Init GUID                */
    #include "wdmguid.h"	/* Windows GUIDS            */

/*
** Suppress Windows Warnings for warn level 4.  Need to find out if we can
** challenge the Windows Blue /W4 /WX with no pragramas assumption?
*/
    #pragma warning(disable : 4127) /* Conditional expression is constant */
    #pragma warning(disable : 4204) /* Non-constant aggregate initializer */
    #pragma warning(disable : 4221) /* Can't be initialized using address of
				       automatic variable */
/*
 * Devices Are Defined To Be Opaque to allow for OS
 * implementation of Pointer VS Handle Etc. Windows uses a
 * a device handle for WDF.
 */

typedef WDFDEVICE esif_device_t;
extern WDFDEVICE g_wdf_device_handle;
extern WDFQUEUE g_wdf_ipc_queue_handle;

/* Map Linux To Windows */
#define __iomem			/* NOOP For Windows          */
#define __le16 short int	/* Add To Windows Dictionary */

#endif /* ESIF_ATTR_KERNEL:: ESIF_ATTR_OS_WINDOWS */
#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER

/*
 * OS Agnostic
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

/* Linux */
#ifdef ESIF_ATTR_OS_LINUX
#include <sys/ioctl.h>			/* IOCTL Interface */
#include <unistd.h>			/* Unix Standard Library */
#include <sys/time.h>			/* Time */
#define ESIF_ASSERT(x)	(0)		/* NOOP For Now */
#define UNREFERENCED_PARAMETER(x) (0)	/* NOOP For Now */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
#endif /* USER::ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS
#define _WINSOCKAPI_			/* Override for Winsock */
#include <windows.h>			/* Windows Includes Almost Everything */
#include <winioctl.h>			/* Except IOCTL Interface */
#define ESIF_ASSERT(x)	(0)		/* NOOP For Now */
#pragma warning(disable : 4204)		/* Non-constant aggregate initializer */
#pragma warning(disable : 4221)		/* Can't be init'd with addr of autos */
#endif /* USER::ESIF_ATTR_OS_WINDOWS */

typedef u8   		UInt8;	/* A CCB BYTE  */
typedef char 		Int8; 
typedef u16  		UInt16;	/* A CCB WORD  */
typedef short 		Int16;
typedef u32 		UInt32;	/* A CCB DWORD */
typedef int     	Int32;
typedef u64 		UInt64;	/* A CCB QWORD */
typedef long long 	Int64;
typedef u8  		Bool;	/* BOOLEAN     */

typedef esif_string EsifString;
#endif /* ESIF_ATTR_USER */

/* ID Lookup Failed To Find String */
#define ESIF_NOT_AVAILABLE "NA"

/* Data Lengths */
#define ESIF_NAME_LEN    64	/* Maximum Name Length        */
#define ESIF_DESC_LEN    64	/* Maximum Description Length */
#define ESIF_SCOPE_LEN   64	/* Maximum ACPI Scope Length  */
#define ESIF_OBJ_LEN     64	/* Maximum Object Name Length */
#define ESIF_GUID_LEN    16	/* Length of a GUID In Bytes  */
#define ESIF_PATH_LEN    128	/* Maximum Path Length        */
#define ESIF_LIBPATH_LEN 128	/* Maximum Lib Path Length    */

#define ESIF_TRUE 1		/* C True */
#define ESIF_FALSE 0		/* C False */

#define ESIF_PERCENT_CONV_FACTOR 100 /* Percentages are in 100ths */

#ifdef ESIF_ATTR_OS_WINDOWS
typedef HANDLE esif_handle_t;
#define ESIF_INVALID_HANDLE INVALID_HANDLE_VALUE
#endif /* ESIF_ATTR_OS_WINDOWS */

#ifdef ESIF_ATTR_OS_LINUX
typedef int esif_handle_t;
#define ESIF_INVALID_HANDLE (-1)
#endif /* ESIF_ATTR_OS_LINUX */

typedef u32	esif_flags_t;			/* FLAGS        */
typedef u8 	esif_guid_t[ESIF_GUID_LEN];	/* GUID         */
typedef u8 	esif_ver_t;			/* Version      */
typedef u32 	esif_temp_t;			/* Temperature  */
typedef u32 	esif_power_t;			/* Power        */

#define ESIF_PLATFORM_MSG "Platform Driver Intended For Internal VM Use Only"
#define ESIF_LICENSE "GPL"
#define ESIF_AUTHOR  "doug.hegge@intel.com"

#ifdef ESIF_ATTR_64BIT
#define ESIF_PLATFORM_TYPE	"x64"
#else
#define ESIF_PLATFORM_TYPE	"x86"
#endif /* ESIF_ATTR_64BIT */

#ifdef ESIF_ATTR_DEBUG
#define ESIF_BUILD_TYPE	"Debug"
#else
#define ESIF_BUILD_TYPE	"Release"
#endif /* ESIF_ATTR_DEBUG */

#include "esif_debug.h"		/* Debug Helpers    */
#include "esif_rc.h"		/* Return Codes     */
#include "esif_mempool.h"	/* Memory Pool      */
#include "esif_ccb.h"		/* Common Code Base */
#include "esif_memtype.h"	/* Memory Types     */

#ifdef ESIF_ATTR_KERNEL
extern esif_ccb_lock_t g_mempool_lock;
#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER
	#include "esif_data.h"
	#include "esif_uf_app_event_type.h"
	/* Include All Headers Here */
	#include "esif_uf.h"
	#include "esif_uf_iface.h"
	#include "esif_uf_app_iface.h"
	#include "esif_uf_esif_iface.h"
#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
