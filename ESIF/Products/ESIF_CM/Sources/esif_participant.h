/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_PARTICIPANT_H_
#define _ESIF_PARTICIPANT_H_

#include "esif.h"
#include "esif_sdk_iface_participant.h"
#include "esif_event.h"
#include "esif_dsp.h"
#include "esif_ccb.h"

#define ESIF_PARTICIPANT_INVALID_UID ""

#define ESIF_NUM_LPAT_ENTRIES 512
#define ESIF_VARS_PER_LPAT_ENTRY 2


/* ESIF LF Participant States */
enum esif_lp_state {
	ESIF_LP_STATE_INVALID = 0,
	/* Suspended and no DSP */
	ESIF_LP_STATE_SUSPENDED_NEEDDSP,
	/* DSP Received while suspended */
	ESIF_LP_STATE_REGISTERED_SUSPENDED,
	/* In The Process Of Registering or DSP unloaded */
	ESIF_LP_STATE_NEEDDSP,
	ESIF_LP_STATE_REGISTERED,	/* DSP Ready */
	ESIF_LP_STATE_SUSPENDED,	/* Part out of D0 */
	ESIF_LP_STATE_RESUMED,		/* Returned to D0 */
};


/* ESIF LF Participant State Descriptions */
static ESIF_INLINE esif_string esif_lp_state_str(
	enum esif_lp_state state)
{
	switch (state) {
	ESIF_CASE(ESIF_LP_STATE_INVALID, "INVALID");
	ESIF_CASE(ESIF_LP_STATE_SUSPENDED_NEEDDSP, "SUSPENDED_NEEDDSP");
	ESIF_CASE(ESIF_LP_STATE_REGISTERED_SUSPENDED, "REGISTERED_SUSPENDED");
	ESIF_CASE(ESIF_LP_STATE_NEEDDSP, "NEED_DSP");
	ESIF_CASE(ESIF_LP_STATE_REGISTERED, "REGISTERED");
	ESIF_CASE(ESIF_LP_STATE_SUSPENDED, "SUSPENDED");
	ESIF_CASE(ESIF_LP_STATE_RESUMED, "RESUMED");
	}
	return ESIF_NOT_AVAILABLE;
}

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
#define ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION 6

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
	enum esif_rc(ESIF_CALLCONV *send_event)(struct _t_EsifParticipantIface *pi,
		enum esif_event_type type, void *data);
	/* EVENT Receive Event From Framework To Conjure */
	enum esif_rc(ESIF_CALLCONV *recv_event)(enum esif_event_type type, void *data);

	/*
	* Add new fields after this point to maintain compatibility with any existing Phidget
	* conjure implementaiton where build versions may not match
	*/
	
	/* Storage fields */
	int scbl;
	int port;
};


enum esif_event_data_create_participant_type
{
	ESIF_EVENT_DATA_CREATE_PARTICIPANT_TYPE_INVALID = 0,
	ESIF_EVENT_DATA_CREATE_PARTICIPANT_TYPE_LP = 1, /* Creation notification from LF to UF */
	ESIF_EVENT_DATA_CREATE_PARTICIPANT_TYPE_UP = 2, /* UF creation request */
	ESIF_EVENT_DATA_CREATE_PARTICIPANT_TYPE_UP_W_LP = 3, /* UF request for CNJ ACPI participant */
};

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


#pragma pack(pop)


#include "esif_sdk_iface_app.h"
#include "esif_uf_fpc.h"
#include "esif_uf_domain.h"

typedef struct esif_ipc_event_data_create_participant EsifLpData;
typedef struct esif_event_data_create_participant EsifEventDataCreateParticipant;
typedef struct esif_event_data_create_participant_hdr EsifEventDataCreateParticipantHdr;
typedef enum esif_event_data_create_participant_type EsifEventDataCreateParticipantType;
typedef struct _t_EsifParticipantIface EsifParticipantIface, *EsifParticipantIfacePtr;

typedef enum {
	eParticipantOriginLF,
	eParticipantOriginUF
} eEsifParticipantOrigin;

/* Upper Particpant Data.  Everything we know about a participant */
typedef struct _t_EsifUpData {
	/* Common */
	esif_ver_t    fVersion;				/* Version */
	esif_guid_t   fDriverType;			/* Driver Type */
	char          fName[ESIF_NAME_LEN];		/* Friendly Name */
	char          fDesc[ESIF_DESC_LEN];		/* Description */
	char          fDriverName[ESIF_NAME_LEN];	/* Driver Name */
	char          fDeviceName[ESIF_NAME_LEN];	/* Device Name */
	char          fDevicePath[ESIF_PATH_LEN]; /* Device Path
						  * /sys/bus/platform...*/

	enum esif_participant_enum fEnumerator; /* Device Enumerator If Any */
	esif_flags_t  fFlags;				/* Flags If Any */

	/* ACPI */
	char  fAcpiDevice[ESIF_NAME_LEN]; /* Device INT340X */
	char  fAcpiScope[ESIF_SCOPE_LEN]; /* Scope/REGEX e.g. \_SB.PCI0.TPCH */
	char  fAcpiUID[ESIF_ACPI_UID_LEN];	/* Unique ID If Any */
	eDomainType  fAcpiType;			/* Participant Type If Any */

	/* PCI */
	UInt16  fPciVendor;	/* PCI Vendor e.g. 0x8086 For Intel */
	UInt16  fPciDevice;	/* Device ID Unique To Vendor */
	UInt8   fPciBus;	/* Bus This Device Resides On */
	UInt8   fPciBusDevice;	/* Device Number On Bus */
	UInt8   fPciFunction;	/* Function Of Device */
	UInt8   fPciRevision;	/* Revision Of PCI Hardware Device */
	UInt8   fPciClass;	/* Class 3 bytes: (base class,sub sclass, prog-if) */
	UInt8   fPciSubClass;	/* Sub Class */
	UInt8   fPciProgIf;	/* Program Interface */

	/* Storage fields */
	int fScbl;
	int fPort;

} EsifUpData, *EsifUpDataPtr, **EsifUpDataPtrLocation;

/* Upper Participant */
typedef struct _t_EsifUp {
	/*
	 * The following data items may be accessed through accessor functions.
	 * See below
	 */
	esif_handle_t fInstance; /* Unique Upper Participant Instance */
	UInt8  fLpInstance;	/* Lower Participant Instance */
	EsifDspPtr fDspPtr; /* Pointer To Our DSP */
	EsifUpData fMetadata; /* Participant Data */
	void* arbitrationContext; /* Opaque arbitration information */

	/*
	 * Data below this point is private to the participant and
	 * Participant Manager.
	 */
	eEsifParticipantOrigin  fOrigin;	/* Origin Of Creation */

	/* Domains For Participants */
	UInt8 domainCount;
	EsifUpDomain domains[ESIF_DOMAIN_MAX];

	/* life control */
	UInt32 refCount;		
	UInt8 markedForDelete;
	esif_ccb_event_t deleteEvent;
	esif_ccb_lock_t objLock;
} EsifUp, *EsifUpPtr, **EsifUpPtrLocation;

/*
 * The following functions are data "accessor" functions
 */
static ESIF_INLINE esif_handle_t EsifUp_GetInstance(
	EsifUpPtr self
	)
{
	return (self != NULL) ? self->fInstance : ESIF_INVALID_HANDLE;
}


static ESIF_INLINE UInt8 EsifUp_GetLpInstance(
	EsifUpPtr self
	)
{
	return (self != NULL) ? self->fLpInstance : ESIF_INSTANCE_INVALID;
}

static ESIF_INLINE EsifString EsifUp_GetName(
	EsifUpPtr self
	)
{
	return (self != NULL) ? self->fMetadata.fName : (char *)"UNK";
}


static ESIF_INLINE  EsifDspPtr EsifUp_GetDsp(
	EsifUpPtr self
	)
{
	return (self != NULL) ? self->fDspPtr : NULL;
}


static ESIF_INLINE EsifUpDataPtr EsifUp_GetMetadata(
	EsifUpPtr self
	)
{
	return (self != NULL) ? &self->fMetadata : NULL;
}

static ESIF_INLINE UInt8 EsifUp_GetDomainCount(
	EsifUpPtr self
	)
{
	return (self != NULL) ? self->domainCount : 0;
}

static ESIF_INLINE unsigned int EsifUp_GetDomainCapabilityMask(
	EsifUpDomainPtr domainPtr
	)
{
	return (domainPtr != NULL) ? domainPtr->capability_for_domain.capability_flags: 0;
}

static ESIF_INLINE unsigned char EsifUp_GetDomainCapabilityCount(
	EsifUpDomainPtr domainPtr
	)
{
	return (domainPtr != NULL) ? domainPtr->capability_for_domain.number_of_capability_flags : 0;
}

static ESIF_INLINE UInt16 EsifUp_GetDomainId(
	EsifUpDomainPtr domainPtr
	)
{
	return (domainPtr != NULL) ? domainPtr->domain : 0;
}

static ESIF_INLINE Bool EsifUp_IsPrimaryParticipant(
	EsifUpPtr self
	)
{
	return (self && (self->fMetadata.fFlags & ESIF_FLAG_DPTFZ));
}

static ESIF_INLINE enum esif_participant_enum EsifUp_GetEnumerator(
	EsifUpPtr self
	)
{
	return (self != NULL) ? self->fMetadata.fEnumerator : ESIF_PARTICIPANT_ENUM_INVALID;
}

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Takes an additional reference on a participant object.  (The function is
 * called for you by the Participant Manager when one of the PM functions are
 * called which returns a pointer to a participant.)  After using the
 * participant, EsifUp_PutRef must be called to release the reference.
 */
eEsifError EsifUp_GetRef(
	EsifUpPtr self
	);

/*
 * Releases a reference on a participant object.  This function should be
 * called when done using a participant pointer obtained through any of the
 * Participant Manager interfaces.
 */
void EsifUp_PutRef(
	EsifUpPtr self
	);

EsifFpcEventPtr EsifUp_GetFpcEventByType(
	EsifUpPtr self,
	eEsifEventType eventType
	);

EsifFpcEventPtr EsifUp_GetFpcEventByGuid(
	EsifUpPtr self,
	esif_guid_t *guid
	);

/*
 * Execute Primitive (Internal version)
 * NOTE: This version should only be called by functions within the
 * participant/domain while Participant Mangager or participant locks are
 * already or from within the participant when executing in a
 * known/guaranteed state.  EsifExecutePrimitive should be called when executing
 * outside the context of the participant.
 */
eEsifError EsifUp_ExecutePrimitive(
	EsifUpPtr upPtr,
	EsifPrimitiveTuplePtr tuplePtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

eEsifError EsifUp_UpdatePolling(
	EsifUpPtr self,
	UInt16 domain_index,
	esif_time_t *period_ptr
	);

eEsifError EsifUp_UpdateHysteresis(
	EsifUpPtr self,
	UInt16 domain_index,
	esif_temp_t *hysteresis_ptr
	);

EsifUpDomainPtr EsifUp_GetDomainById(
	EsifUpPtr self,
	UInt16 domainId
	);

EsifUpDomainPtr EsifUp_GetDomainByIndex(
	EsifUpPtr self,
	UInt8 domainIndex
	);

EsifString EsifUp_CreateTokenReplacedParamString(
	const EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifString paramStr
	);


Bool EsifUp_IsActionInDsp(
	EsifUpPtr self,
	enum esif_action_type actionType
	);


#if defined(ESIF_FEAT_OPT_ARBITRATOR_ENABLED)

void *EsifUp_GetArbitrationContext(EsifUpPtr self);

void EsifUp_SetArbitrationContext(
	EsifUpPtr self,
	void *arbCtx
);
#else 
static ESIF_INLINE void *EsifUp_GetArbitrationContext(EsifUpPtr self) { UNREFERENCED_PARAMETER(self);  return NULL; }
static ESIF_INLINE void EsifUp_SetArbitrationContext(EsifUpPtr self, void *arbCtx)
{
	UNREFERENCED_PARAMETER(self);
	UNREFERENCED_PARAMETER(arbCtx);
}
#endif

#ifdef __cplusplus
}
#endif
#endif /* _ESIF_PARTICIPANT_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
