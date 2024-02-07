/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "esif_sdk_event_type.h"

typedef struct esif_event_data_create_participant EsifEventDataCreateParticipant;
typedef struct esif_ipc_event_data_create_participant EsifLpData;
typedef struct esif_event_data_create_participant_hdr EsifEventDataCreateParticipantHdr;
typedef struct _t_EsifParticipantIface EsifParticipantIface;

/*
** Create Participant Data.  Will be tacked on the bottom
** of an IPC Event for participant creation.  Note the IPC event and
** This data must be contiguous in memory space.
*/
/* USE Native Data Types With Packed Structures */
#pragma pack(push, 1)
struct esif_ipc_event_data_create_participant {
	u8    id;				/* Participant ID */
	u8    version;				/* Version */
	u8    class_guid[ESIF_GUID_LEN];	/* Class GUID */
	enum esif_participant_enum enumerator; /* Device Enumerator If Any */
	u32   flags;				/* Flags If Any */
	char  name[ESIF_NAME_LEN];		/* Friendly Name */
	char  desc[ESIF_DESC_LEN];		/* Description */
	char  driver_name[ESIF_NAME_LEN];	/* Driver Name */
	char  device_name[ESIF_NAME_LEN];	/* Device Name */
	char  device_path[ESIF_PATH_LEN];	/* Device Path */
	char  acpi_device[ESIF_SCOPE_LEN];
	char  acpi_scope[ESIF_SCOPE_LEN];
	char  acpi_uid[ESIF_ACPI_UID_LEN];	/* Unique ID If Any */
	u32   acpi_type;			/* Participant Type If Any */
	u32   pci_vendor;			/* PCI Vendor For PCI Devices */
	u32   pci_device;			/* PCE Device For PCI Devices */
	u8    pci_bus;				/* Bus Device Enumerated On */
	u8    pci_bus_device;			/* Device Number On Bus */
	u8    pci_function;			/* PCI Function Of Device */
	u8    pci_revision;			/* PCI Hardware Revision */
	u8    pci_class;			/* PCI Hardware Class */
	u8    pci_sub_class;			/* PCI Hardware Sub Class */
	u8    pci_prog_if;			/* PCI Hardware Iface */
};


/*
* This version is a diversion from ESIF_PARTICIPANT_VERSION which is now only
* used for the version of data reported to apps and client, and not the
* structure used to create the participant internally
*/
#define ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION 7

/* Structure used to create a UF participant in event data */
struct _t_EsifParticipantIface {
	esif_ver_t			version;			/* Should be ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION */
	esif_guid_t			class_guid;			/* Class GUID */
	enum esif_participant_enum enumerator;  /* Device Enumerator If Any */
	esif_flags_t		flags;				/* Flags If Any */
	char				name[ESIF_NAME_LEN];		/* Friendly Name */
	char				desc[ESIF_DESC_LEN];		/* Description */
	char				driver_name[ESIF_NAME_LEN];	/* Driver Name */
	char				device_name[ESIF_NAME_LEN];	/* Device Name */
	char				device_path[ESIF_PATH_LEN];	/* Device Path /sys/bus/platform...*/
	char				object_id[ESIF_SCOPE_LEN];	/* Scope/REGEX e.g.\_UF.CNJR.WIDI  */
	u32					acpi_type;
	/* EVENT Send Event From Conjure To Framework */
	enum esif_rc(ESIF_CALLCONV* send_event)(struct _t_EsifParticipantIface* pi,
		enum esif_event_type type, void* data);
	/* EVENT Receive Event From Framework To Conjure */
	enum esif_rc(ESIF_CALLCONV* recv_event)(enum esif_event_type type, void* data);

	/*
	* Add new fields after this point to maintain compatibility with any existing Phidget
	* conjure implementation where build versions may not match
	*/

	/* Storage fields */
	int scbl;
	int port;

	/* PCI fields for Sysfs upper participants */
	u16 pciDeviceId;
	u16 pciVendorId;
};


enum esif_event_data_create_participant_type
{
	ESIF_EVENT_DATA_CREATE_PARTICIPANT_TYPE_INVALID = 0,
	ESIF_EVENT_DATA_CREATE_PARTICIPANT_TYPE_LP = 1, /* Creation notification from LF to UF */
	ESIF_EVENT_DATA_CREATE_PARTICIPANT_TYPE_UP = 2, /* UF creation request */
	ESIF_EVENT_DATA_CREATE_PARTICIPANT_TYPE_UP_W_LP = 3, /* UF request for CNJ ACPI participant */
};

typedef enum esif_event_data_create_participant_type EsifEventDataCreateParticipantType;

#define ESIF_EVENT_DATA_PARTICIPANT_CREATE_HDR_VERSION 1

struct esif_event_data_create_participant_hdr
{
	esif_ver_t	version; /* Header version - Must be ESIF_EVENT_DATA_PARTICIPANT_CREATE_HDR_VERSION*/
	enum esif_event_data_create_participant_type dataType;
};

struct esif_event_data_create_participant {

	struct esif_event_data_create_participant_hdr hdr;
	union {
		struct esif_ipc_event_data_create_participant lfData; /* For LP creation */
		struct _t_EsifParticipantIface ufData; /* For UP/UP_W_LP creation */
	} data;
};


/* Participant METADATA */
typedef struct _t_AppParticipantData {
	/* Common */
	UInt8		fVersion;	/* ESIF Participant version */
	UInt8		fReserved[3];	/* Pad / Align */
	EsifData	fDriverType;	/* Guid is obtained from Driver */
	EsifData	fDeviceType;	/* Guid is obtained from DSP */
	EsifData	fName;		/* Name May Come From Driver/DSP */
	EsifData	fDesc;		/* Description From Driver/DSP */
	EsifData	fDriverName;	/* Driver Name */
	EsifData	fDeviceName;	/* Device Name */
	EsifData	fDevicePath;	/* Device Path */
	UInt8		fDomainCount;	/* Domain Count */
	UInt8		fReserved2[3];	/* Pad/Align */
	eParticipantBus fBusEnumerator;	/* Enumeration Type pci, acpi, platform, conjure etc.*/

	/* ACPI */
	EsifData    fAcpiDevice;	/* ACPI Device */
	EsifData    fAcpiScope;		/* ACPI Scope/Object ID \_SB_.IETM */
	EsifData    fAcpiUID;		/* ACPI Unique ID */
	eDomainType fAcpiType;		/* ACPI Domain/Participant Type e.g. THermalSensor, Power, Etc. */

	/* PCI */
	UInt16  fPciVendor;		/* Vendor */
	UInt16  fPciDevice;		/* Device */
	UInt8   fPciBus;		/* Bus */
	UInt8   fPciBusDevice;		/* Bus Device */
	UInt8   fPciFunction;		/* Function */
	UInt8   fPciRevision;		/* Revision */
	UInt8   fPciClass;		/* Class */
	UInt8   fPciSubClass;		/* Sub Class */
	UInt8   fPciProgIf;		/* Programming Interface */
	UInt8   fReserved3[1];		/* Pad/Align */
} AppParticipantData, * AppParticipantDataPtr, ** AppParticipantDataPtrLocation;


#pragma pack(pop)
