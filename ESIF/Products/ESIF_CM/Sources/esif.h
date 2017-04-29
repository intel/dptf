/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "esif_sdk.h"

/* Enable WebSocket Server */
#define ESIF_ATTR_WEBSOCKET

/* Definitions not moved to esif_ccb.h or esif_sdk.h */

/* Build option to enable simulation support */
/* #define ESIF_FEAT_OPT_SIM_SUPPORT_ENABLED */

#ifdef ESIF_ATTR_OS_LINUX	/* All Linux-Derived OS */

/*
 * ESIF_FEAT_OPT_SYSFS exports SYSFS entries. It is defined
 * in ESIF_LF/Linuxx64/Debug/makefile and
 * ESIF_LF/Linuxx64/Release/makefile. It is disabled by default.
 * To enable it, it must be uncommented out in makefile.
 */

#endif /* Linux Derived OS */

#ifdef ESIF_ATTR_OS_WINDOWS

/* Build option to enable thermal interrupt support for Windows*/
#define ESIF_FEAT_OPT_THERMAL_INTERRUPTS_ENABLED

/* Build option to enable sensor support in Windows */
#define ESIF_FEAT_OPT_SENSOR_SUPPORT_ENABLED

/* Build option to enable PERC support in Windows */
#define ESIF_FEAT_OPT_PERC_SUPPORT_ENABLED

/* Build option to enable HID support in Windows Kernel code */
/*#define ESIF_FEAT_OPT_KHID_SUPPORT_ENABLED*/

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
#define esif_device_t struct device * /* opaque: use #define not typedef */

#define ESIF_FEAT_OPT_THERMAL_INTERRUPTS_ENABLED

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

#define EXPORT_SYMBOL_GPL(fn) extern void NOOP_##fn(void) /* NOOP for Windows */

#endif /* ESIF_ATTR_KERNEL:: ESIF_ATTR_OS_WINDOWS */
#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER

/*
 * OS Agnostic
 */

/* Linux */
#ifdef ESIF_ATTR_OS_LINUX
#include <fcntl.h>
#include <sys/ioctl.h>

#endif /* USER::ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS
#pragma warning(disable : 4204)		/* Non-constant aggregate initializer */
#pragma warning(disable : 4221)		/* Can't be init'd with addr of autos */
#endif /* USER::ESIF_ATTR_OS_WINDOWS */

typedef esif_string EsifString;
#endif /* ESIF_ATTR_USER */

#define ESIF_PERCENT_CONV_FACTOR 100 /* Percentages are in 100ths */

#define ESIF_PLATFORM_MSG "Registering Platform Driver"
#define ESIF_LICENSE "Dual BSD/GPL"
#define ESIF_AUTHOR  "Intel Corporation <dptf@lists.01.org>"

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

/* Debug Options */
#define MEMPOOL_DEBUG NO_ESIF_DEBUG

/*
 * Common Code Base - OS/Platform Abstraction layer
 */
#include "esif_ccb_rc.h"	/* Return Codes     */

#ifdef ESIF_ATTR_USER

#include <stdlib.h>		/* Standard Library */
#include <string.h>		/* String Library   */
#include "esif_ccb_thread.h"
#include "esif_ccb_library.h"
#include "esif_ccb_file.h"

#endif /* ESIF_ATTR_USER */

#include "esif_ccb_lock.h"
#include "esif_ccb_mempool.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_time.h"
#include "esif_ccb_string.h"
#include "esif_ccb_sem.h"
#include "esif_ccb_timer.h"

#ifdef ESIF_ATTR_KERNEL

#include "esif_lf_ccb_mbi.h"
#include "esif_lf_ccb_mmio.h"
#include "esif_lf_ccb_msr.h"
#include "esif_lf_ccb_acpi.h"

#endif /* ESIF_ATTR_KERNEL */

/*
 * Autogenerated Data Types
 */
#ifdef ESIF_ATTR_USER

/* Autogen Types Required for Upper Framework */
#include "esif_enum_algorithm_type.h"
#include "esif_enum_acpi_device.h"
#include "esif_enum_event_group.h"
#include "esif_enum_pci_device.h"
#include "esif_enum_vendor_type.h"
#include "esif_sdk_action_type.h"
#include "esif_sdk_capability_type.h"
#include "esif_sdk_class_guid.h"
#include "esif_sdk_data.h"
#include "esif_sdk_data_misc.h"
#include "esif_sdk_domain_type.h"
#include "esif_sdk_event_type.h"
#include "esif_sdk_event_map.h"
#include "esif_sdk_iface.h"
#include "esif_sdk_iface_app.h"
#include "esif_sdk_iface_esif.h"
#include "esif_sdk_participant_enum.h"
#include "esif_sdk_primitive_type.h"
#include "esif_sdk_logging_data.h"
#include "esif_uf.h"

#endif /* ESIF_ATTR_USER */

#ifdef ESIF_ATTR_KERNEL

/* Autogen Types Required for Lower Framework */
#include "esif_enum_algorithm_type.h"
#include "esif_enum_acpi_device.h"
#include "esif_enum_event_group.h"
#include "esif_enum_pci_device.h"
#include "esif_sdk_action_type.h"
#include "esif_sdk_capability_type.h"
#include "esif_sdk_class_guid.h"
#include "esif_sdk_domain_type.h"
#include "esif_sdk_primitive_type.h"

#endif /* ESIF_ATTR_KERNEL */

/* OS Agnostic */
#include "esif_mempool.h"	/* Memory Pool      */

/* Opaque Types */
typedef u32 esif_temp_t;	/* Temperature  */
typedef u32 esif_power_t;	/* Power        */
typedef u32 esif_time_t;	/* Time         */
