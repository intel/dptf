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

#include "esif_sdk_iface.h"
#include "esif_sdk_data.h"
#include "esif_sdk_participant_enum.h"
#include "esif_sdk_event_type.h"

/* Participant Interface Versions:
 * 2 - Added "whitelist" pointers (used by LF only)
 * 3 - Changed to provide IO target vs device for ACPI accesses in Windows
 * 4 - Added a void* context field to allow the dev ext for a given device to be specified for debugging
 */
#define ESIF_PARTICIPANT_VERSION 5
#define ESIF_PARTICIPANT_INVALID_TYPE ((u32)ESIF_DOMAIN_TYPE_INVALID)

/* Flags */
#define ESIF_FLAG_DPTFZ 0x1	/* Participant Is Actually A DPTF Zone */
#define ESIF_FLAG_EXTERN_DPTFZ 0x2	/* Participant Is Actually A DPTF Zone exposed by an external driver */
#define ESIF_FLAG_ACCESS_ACPI_VIA_HAL 0x4	/* Participant is using the ACPI HAL target with absolute paths */

#ifdef ESIF_ATTR_KERNEL

#ifdef ESIF_ATTR_OS_WINDOWS
#include <acpiioct.h>	/* ACPI  */
typedef WDFIOTARGET acpi_handle;
#endif

#define MSR_WHITELIST_TERMINATOR 0
#define MMIO_WHITELIST_TERMINATOR 0
#define OCMB_ALLOWLIST_TERMINATOR 0

/* The order is such that access rights are increased with value */
enum whitelist_access {
	WHITELIST_ACCESS_NONE = 0,
	WHITELIST_ACCESS_READ = 1,
	WHITELIST_ACCESS_WRITE = 2,
	WHITELIST_ACCESS_BOTH = 3
};

static ESIF_INLINE esif_string whitelist_access_str(enum whitelist_access type)
{
	switch (type) {
	ESIF_CASE_ENUM(WHITELIST_ACCESS_NONE);
	ESIF_CASE_ENUM(WHITELIST_ACCESS_READ);
	ESIF_CASE_ENUM(WHITELIST_ACCESS_WRITE);
	ESIF_CASE_ENUM(WHITELIST_ACCESS_BOTH);
	}
	return ESIF_NOT_AVAILABLE;
}

struct msr_whitelist_entry {
	u32 msr;
	enum whitelist_access access;
};

struct mmio_whitelist_entry {
	u32 start;
	u32 end;
	enum whitelist_access access;
};

struct ocmb_allowlist_entry {
	u32 cmd;
	enum whitelist_access access;
};

/* Participant INTERFACE */
struct esif_participant_iface {
	esif_ver_t    version;				/* Version */
	esif_guid_t   class_guid;			/* Class GUID */
	enum esif_participant_enum enumerator; /* Device Enumerator If Any */
	esif_flags_t  flags;				/* Flags If Any
							 *                    */
	char          name[ESIF_NAME_LEN];		/* Friendly Name */
	char          desc[ESIF_DESC_LEN];		/* Description */
	char          driver_name[ESIF_NAME_LEN];	/* Driver Name */
	char          device_name[ESIF_NAME_LEN];	/* Device Name */
	char          device_path[ESIF_PATH_LEN];

	/* EVENT Send Event From Driver To Framework */
	enum esif_rc  (*send_event)(struct esif_participant_iface *pi,
				    enum esif_event_type type, u16 domain,
				    struct esif_data *data);
	/* EVENT Receive Event From Framework To Driver */
	enum esif_rc  (*recv_event)(enum esif_event_type type, u16 domain,
				    struct esif_data *data);

	/* ACPI */
	char  acpi_device[ESIF_SCOPE_LEN];/* Device INT340X */
	char  acpi_scope[ESIF_SCOPE_LEN]; /* Scope/REGEX e.g. \_SB.PCI0.TPCH  */
	char  acpi_uid[ESIF_ACPI_UID_LEN];   /* Unique ID If Any        */
	u32   acpi_type;                  /* Participant Type If Any */

	/* PCI */
	u32  pci_vendor;	/* PCI Vendor e.g. 0x8086 For Intel */
	u32  pci_device;	/* Device ID Unique To Vendor       */
	u8   pci_bus;		/* Bus This Device Resides On       */
	u8   pci_bus_device;	/* Device Number On Bus         */
	u8   pci_function;	/* Function Of Device               */
	u8   pci_revision;	/* Revision Of PCI Hardware Device  */
	u8   pci_class; /* Class 3 bytes: (base class,sub sclass, prog-if) */
	u8   pci_sub_class;	/* Sub Class */
	u8   pci_prog_if;	/* Program Interface */

	esif_device_t  device;		/* Os Agnostic Driver Context */
	void __iomem   *mem_base;	/* MMIO/MCHBAR Address        */
	u32          mem_size;		/* MMIO/MCHBAR Size           */
	acpi_handle  acpi_handle;	/* ACPI Handle                */

	struct msr_whitelist_entry *msr_wlist_ptr;
	struct mmio_whitelist_entry *mmio_wlist_ptr;
	struct ocmb_allowlist_entry *ocmb_alist_ptr;

	void *context; /* Reserved */
};

#endif /* ESIF ATTR_KERNEL */
#ifdef ESIF_ATTR_USER

#pragma pack(push,1)

/* Upper Framework Initiated Participant INTERFACE */
typedef struct _t_EsifParticipantIface {
	esif_ver_t			version;				/* Version */
	esif_guid_t			class_guid;			/* Class GUID */
	enum esif_participant_enum enumerator; /* Device Enumerator If Any */
	esif_flags_t		flags;				/* Flags If Any */
	char				name[ESIF_NAME_LEN];		/* Friendly Name */
	char				desc[ESIF_DESC_LEN];		/* Description */
	char				driver_name[ESIF_NAME_LEN];	/* Driver Name */
	char				device_name[ESIF_NAME_LEN];	/* Device Name */
	char				device_path[ESIF_PATH_LEN];	/* Device Path /sys/bus/platform...*/
	char				object_id[ESIF_SCOPE_LEN];	/* Scope/REGEX e.g.\_UF.CNJR.WIDI  */
	u32					acpi_type;
	/* EVENT Send Event From Conjure To Framework */
	enum esif_rc (ESIF_CALLCONV *send_event)(struct _t_EsifParticipantIface *pi,
				   enum esif_event_type type, void *data);
	/* EVENT Receive Event From Framework To Conjure */
	enum esif_rc (ESIF_CALLCONV *recv_event)(enum esif_event_type type, void *data);
} EsifParticipantIface, *EsifParticipantIfacePtr,
**EsifParticipantIfacePtrLocation;

#pragma pack(pop)

#endif /* ESIF_ATTR_USER */
