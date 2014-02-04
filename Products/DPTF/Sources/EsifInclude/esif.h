/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_H_
#define _ESIF_H_

#include "esif_basic_types.h"

/*******************************************************************************
** Linux, Kernel/User Agnostic
*******************************************************************************/

#if defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_ANDROID)
    #define ESIF_ATTR_OS "linux"            /* OS Is Linux             */
    #define ESIF_INLINE  inline             /* Normalize Compiler      */
    #define ESIF_FUNC __func__              /* Normalize Compiler      */
    #define ESIF_PATH_SEP "/"               /* Path Separator          */
#ifdef __cpluscplus
    #define ESIF_ELEMENT(x) x =             /* Support C99 Init        */
#else /* __cpluscplus */
    #define ESIF_ELEMENT(x)                 /* Emulate/Ignore C99 Init */
#endif
#endif

#ifdef ESIF_ATTR_OS_ANDROID
#undef ESIF_ATTR_OS
#define ESIF_ATTR_OS "android"
#define ESIF_ATTR_OS_LINUX
#endif

/*******************************************************************************
** Windows, Kernel/User Agnostic
*******************************************************************************/

#ifdef ESIF_ATTR_OS_WINDOWS
    #define ESIF_ATTR_OS "windows"          /* OS Is Windows             */
    #define ESIF_INLINE __inline            /* Normalize Compiler        */
    #define ESIF_FUNC __FUNCTION__          /* Normalize Compiler        */
    #define ESIF_PATH_SEP "\\"              /* Path Separator            */
    #define ESIF_ELEMENT(x)                 /* Emulate/Ignore C99 Init   */
                                            /* Note Static Init Must Be  */
                                            /* In Structure Order        */
#endif

/*******************************************************************************
** Kernel
*******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

    /* Linux */
    #ifdef ESIF_ATTR_OS_LINUX
        #include <linux/module.h>           /* Linux Module           */
        #include <linux/platform_device.h>  /* Linux Platform Device  */
        #include <linux/acpi.h>             /* Linux ACPI Device      */
        #include <linux/pci.h>              /* Linux PCI Device       */
        #include <linux/thermal.h>          /* Linux Thermals         */
        #include <linux/kobject.h>          /* Linux Kernel Objects   */
        #include <linux/version.h>          /* Linux Kernel Version   */

        /*
         * Devices Are Defined To Be Opaque to allow for OS
         * implementation of Pointer VS Handle Etc. Linux uses a
         * a device pointer.
         */
        typedef struct device* esif_device_t;

        /*
         * The Windows Compiler Requires This It Does Not Hurt To Annotate In
         * Linux so we include it here for the abstraction.
         */
         #define UNREFERENCED_PARAMETER(x)
    #endif /* ESIF_ATTR_KERNEL::ESIF_ATTR_OS_LINUX  */

    /* Windows */
    #ifdef ESIF_ATTR_OS_WINDOWS
        #include "ntddk.h"                  /* The Windows DDK          */
        #include "wdf.h"                    /* Windows Driver Framework */
        #include "INITGUID.H"               /* Init GUID                */
        #include "wdmguid.h"                /* Windows GUIDS            */

        /*
         * Devices Are Defined To Be Opaque to allow for OS
         * implementation of Pointer VS Handle Etc. Windows uses a
         * a device handle for WDF.
         */
         typedef WDFDEVICE esif_device_t;
         extern WDFDEVICE g_wdf_device_handle;
         extern WDFQUEUE g_wdf_ipc_queue_handle;

         /* Map Linux To Windows */
         #define __iomem                    /* NOOP For Windows          */
         #define __le16 short int           /* Add To Windows Dictionary */
    #endif /* ESIF_ATTR_KERNEL:: ESIF_ATTR_OS_WINDOWS */

#endif /* ESIF_ATTR_KERNEL */

/*******************************************************************************
** User
*******************************************************************************/

#ifdef ESIF_ATTR_USER

    /* OS Agnostic */
    #include <stdio.h>
    #include <fcntl.h>
    #include <sys/stat.h>

    /* Linux */
    #ifdef ESIF_ATTR_OS_LINUX
        #include <sys/ioctl.h>              /* IOCTL Interface */
        #include <unistd.h>                 /* Unix Standard Library */
        #include <sys/time.h>               /* Time */
        #define ESIF_ASSERT(x)              /* NOOP For Now */
        #define UNREFERENCED_PARAMETER(x)   /* NOOP For Now */
    #endif /* USER::ESIF_ATTR_OS_LINUX */

    /* Windows */
    #ifdef ESIF_ATTR_OS_WINDOWS
        #define _WINSOCKAPI_                /* Override for Winsock */
        #include <windows.h>                /* Windows Includes almost everything */
        #include <winioctl.h>               /* Except IOCTL Interface */
        #define ESIF_ASSERT                 /* NOOP For Now */
    #endif /* USER::ESIF_ATTR_OS_WINDOWS */

    typedef esif_string EsifString;

#endif /* ESIF_ATTR_USER */

/*******************************************************************************
** OS Agnostic, Kernel/User Agnostic
*******************************************************************************/

/* ID Lookup Failed To Find String */
#define ESIF_NOT_AVAILABLE "NA"

/* Data Lengths */
#define ESIF_NAME_LEN    32                 /* Maximum Name Length        */
#define ESIF_DESC_LEN    32                 /* Maximum Description Length */
#define ESIF_SCOPE_LEN   64                 /* Maximum ACPI Scope Length  */
#define ESIF_OBJ_LEN     64                 /* Maximum Object Name Length */
#define ESIF_GUID_LEN    16                 /* Length of a GUID In Bytes  */
#define ESIF_PATH_LEN    128                /* Maximum Path Length        */
#define ESIF_LIBPATH_LEN 128                /* Maximum Lib Path Length    */

#define ESIF_TRUE 1                         /* C True */
#define ESIF_FALSE 0                        /* C False */

#ifdef ESIF_ATTR_OS_WINDOWS
typedef HANDLE esif_handle_t;
#define ESIF_INVALID_HANDLE INVALID_HANDLE_VALUE
#else
typedef int esif_handle_t;
#define ESIF_INVALID_HANDLE -1
#endif

typedef u32 esif_flags_t;                   /* FLAGS        */
typedef u8  esif_guid_t[ESIF_GUID_LEN];     /* GUID         */
typedef u8  esif_ver_t;                     /* Version      */
typedef u32 esif_temp_t;                    /* Temperature  */
typedef u32 esif_power_t;                   /* Power        */

#define ESIF_PLATFORM_MSG "Platform Driver Intended For Internal VM Use Only"
#define ESIF_LICENSE "GPL"
#define ESIF_AUTHOR  "doug.hegge@intel.com"

/*******************************************************************************
** Kernel
*******************************************************************************/

#ifdef ESIF_ATTR_KERNEL
    extern esif_ccb_lock_t g_mempool_lock;
    extern esif_ccb_lock_t g_memtype_lock;
    extern esif_ccb_lock_t g_memstat_lock;
#endif

// Enumerated Type Base Macros
#define ENUMDECL(ENUM)              ENUM,
#define ENUMSWITCH(ENUM)            case ENUM: return #ENUM; break;
#define ENUMLIST(ENUM)              { ENUM, #ENUM },
#define ENUMDECL_VAL(ENUM,VAL)      ENUM=VAL,
#define ENUMSWITCH_VAL(ENUM,VAL)    ENUMSWITCH(ENUM)
#define ENUMLIST_VAL(ENUM,VAL)      ENUMLIST(ENUM)

#endif /* _ESIF_H_ */