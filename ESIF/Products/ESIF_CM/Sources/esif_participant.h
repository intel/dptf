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

#ifndef _ESIF_PARTICIPANT_H_
#define _ESIF_PARTICIPANT_H_

#include "esif.h"
#include "esif_sdk_iface_participant.h"
#include "esif_event.h"

#define ESIF_PARTICIPANT_INVALID_TYPE 0xFFFFFFFF
#define ESIF_PARTICIPANT_INVALID_UID ""
#define ESIF_PARTICIPANT_INVALID_INSTANCE 0xFF

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

#ifdef ESIF_ATTR_KERNEL

#include "esif_dsp.h"
#include "esif_temp.h"
#include "esif_power.h"
#include "esif_time.h"
#include "esif_percent.h"
#include "esif_primitive.h"
#include "esif_lf_domain.h"

/* ESIF LF Participant State Machine Event types */
enum esif_lp_sm_event {
	ESIF_LP_SM_EVENT_DSP_LOAD = 0,
	ESIF_LP_SM_EVENT_DSP_UNLOAD,
	ESIF_LP_SM_EVENT_SUSPEND,
	ESIF_LP_SM_EVENT_RESUME,
};


/* ESIF LF Participant State Machine Event Descriptions */
static ESIF_INLINE esif_string esif_lp_sm_event_str(
	enum esif_lp_sm_event state)
{
	switch (state) {
	ESIF_CASE(ESIF_LP_SM_EVENT_DSP_LOAD, "DSP_LOAD");
	ESIF_CASE(ESIF_LP_SM_EVENT_DSP_UNLOAD, "DSP_UNLOAD");
	ESIF_CASE(ESIF_LP_SM_EVENT_SUSPEND, "LP_SUSPEND");
	ESIF_CASE(ESIF_LP_SM_EVENT_RESUME, "LP_RESUME");
	}
	return ESIF_NOT_AVAILABLE;
}


/* Lower Participant */
#ifndef ESIF_FEAT_OPT_USE_VIRT_DRVRS
struct esif_lp {

	/* State control items */
	u8  instance;			/* Lower Participant Instance */
	char pi_name[ESIF_NAME_LEN];	/* PI Name */
	enum esif_lp_state lp_state;	/* Participant state */

	struct esif_participant_iface *pi_ptr;	/* Particpant INTERFACE */

	struct esif_lp_dsp *dsp_ptr;		/* DSP */

	/* Number Of Qualifiers For A Participant */
	u8  domain_count;
	/* Domains For Participants */
	struct esif_lp_domain  domains[ESIF_DOMAIN_MAX];

	u32 ref_count;			/* Reference count */
	u8 marked_for_delete;		/* Delete pending flag */
	esif_ccb_low_priority_thread_lock_t  lp_lock;	/* LP Lock */
	esif_ccb_low_priority_thread_lock_t  lp_sm_lock;/* For State Machine */

	/* Signals waiters when the LP is no longer in use and may be destroyed */
	esif_ccb_event_t lp_destroy_event;

	/* Interface Functions*/
	/* XFORM Temperature */
	enum esif_rc  (*xform_temp)(const enum esif_temperature_type,
			    esif_temp_t *temp,
			    const enum esif_action_type action,
			    const struct esif_lp_dsp *dsp,
			    const struct esif_lp_primitive *primitive_ptr,
			    const struct esif_lp *lp_ptr);

	/* XFORM Power */
	enum esif_rc  (*xform_power)(const enum esif_power_unit_type,
				     esif_power_t *power,
				     const enum esif_action_type action,
				     const struct esif_lp_dsp *dsp,
				     const enum esif_primitive_opcode opcode);

	/* XFORM Time */
	enum esif_rc (*xform_time)(const enum esif_time_type,
		esif_time_t *time,
		const enum esif_action_type action,
		const struct esif_lp_dsp *dsp_ptr,
		const struct esif_lp_primitive *primitive_ptr,
		const struct esif_lp *lp_ptr);

	/* XFORM Percent */
	enum esif_rc (*xform_percent)(const enum esif_percent_type type,
		u32 *value_ptr,
		const enum esif_action_type action,
		const struct esif_lp_dsp *dsp_ptr,
		const struct esif_lp_primitive *primitive_ptr);

};

/* Takes an additional reference on an LP object */
enum esif_rc esif_lp_get_ref(
	struct esif_lp *self
	);

/* Release a reference on an LP */
void esif_lp_put_ref(
	struct esif_lp *self
	);

/* 
 * Locks the state of an LP so that it can be used with all state changes
 * blocked.  Must not be called by any function which will call
 * esif_lp_handle_event
 */
void esif_lp_lock_state(
	struct esif_lp *self
	);

/* 
 * Unlocks the state of an LP after using it with the state locked by
 * esif_lp_lock_state
 */
void esif_lp_unlock_state(
	struct esif_lp *self
	);

/* Get State */
enum esif_lp_state esif_lp_get_state(
	/* Lower Participant Intance */
	const struct esif_lp *self
	);

/* Set State */
enum esif_rc esif_lp_handle_event(
	/* Lower Participant Instance */
	struct esif_lp *self,
	/* Particpant State Machine Event Enumeration*/
	const enum esif_lp_sm_event state,
	/* Event-dependent context data */
	void *ctx_ptr
	);

esif_string esif_lp_get_name(
	const struct esif_lp *self
	);

enum esif_rc esif_lp_get_dmn_by_index(
	const struct esif_lp *self,
	const u8 index,
	struct esif_lp_domain **dmn_ptr
	);

enum esif_rc esif_lp_get_dmn_by_id(
	const struct esif_lp *self,
	const u16 id,
	struct esif_lp_domain **dmn_ptr
	);

enum esif_rc esif_lp_check_msr_whitelist(
	const struct esif_lp *self,
	const u32 msr,
	const enum whitelist_access req_access
	);

enum esif_rc esif_lp_check_mmio_whitelist(
	const struct esif_lp *self,
	const u32 offset,
	const enum whitelist_access req_access
	);

enum esif_rc esif_lp_load_dsp(
	struct esif_lp *self,
	const struct esif_data *cpc_ptr
	);

void esif_lp_unload_dsp(
	struct esif_lp *self
	);

enum esif_rc esif_lp_suspend(
	struct esif_lp *self
	);

enum esif_rc esif_lp_resume(
	struct esif_lp *self
	);

#endif /* ESIF_FEAT_OPT_USE_VIRT_DRVRS */
#endif /* ESIF ATTR_KERNEL          */
#ifdef ESIF_ATTR_USER

#include "esif_sdk_iface_app.h"
#include "esif_uf_fpc.h"
#include "esif_uf_domain.h"

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
} EsifUpData, *EsifUpDataPtr, **EsifUpDataPtrLocation;

/* Upper Participant */
typedef struct _t_EsifUp {
	UInt8  fInstance;	/* Unique Upper Participant Instance */
	UInt8  fLpInstance;	/* Lower Participant Instance */
	eEsifParticipantOrigin  fOrigin;	/* Origin Of Creation */
	struct esif_up_dsp      *fDspPtr;	/* Pointer To Our DSP */
	EsifUpData fMetadata;	/* Participant Data */

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
 * Execute Primitive (Internal version)
 * NOTE: This version should only be called by functions within the
 * participant /domain while Participant Mangager or participant locks are
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

eEsifError EsifUp_DspReadyInit(
	EsifUpPtr self
);

eEsifError EsifUp_UpdatePolling(
	EsifUpPtr self,
	UInt16 domain_index,
	UInt32 period
);

eEsifError EsifUp_UpdateHysteresis(
	EsifUpPtr self,
	UInt16 domain_index,
	esif_temp_t hysteresis_val
);

EsifFpcEventPtr EsifUp_GetFpcEventByType(
	EsifUpPtr self,
	eEsifEventType eventType
);

EsifFpcEventPtr EsifUp_GetFpcEventByGuid(
	EsifUpPtr self,
	esif_guid_t *guid
);

EsifString EsifUp_GetName(
	EsifUpPtr self
);

eEsifError EsifUp_CreateParticipant(
	const eEsifParticipantOrigin origin,
	UInt8 upInstance,
	const void *metadataPtr,
	EsifUpPtr *upPtr
);

eEsifError EsifUp_ReInitializeParticipant(
	EsifUpPtr self,
	const eEsifParticipantOrigin origin,
	const void *metadataPtr
);

void EsifUp_DestroyParticipant(
	EsifUpPtr self
);


eEsifError EsifUp_SuspendParticipant(
	EsifUpPtr self
	);

eEsifError EsifUp_ResumeParticipant(
	EsifUpPtr self
	); 

UInt8 EsifUp_GetInstance(
	EsifUpPtr self
);

void EsifUp_PollParticipant(
	EsifUpPtr self
	);

void EsifUp_RegisterParticipantForPolling(
	EsifUpPtr self
	);

void EsifUp_UnRegisterParticipantForPolling(
	EsifUpPtr self
	);

static ESIF_INLINE EsifString EsifUp_IntToShellDomainStr(
	UInt32 domain_index,
	EsifString str,
	UInt8 str_len
	)
{
	esif_ccb_sprintf(str_len, str, "D%X", domain_index);
	return str;
}

EsifUpDomainPtr EsifUp_GetDomainById(
	EsifUpPtr self,
	UInt16 domainId
);

eEsifError EsifUp_GetRef(
	EsifUpPtr self
);

void EsifUp_PutRef(
	EsifUpPtr self
);

#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_PARTICIPANT_H_ */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
