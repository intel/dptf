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

#ifndef _ESIF_PM_H_
#define _ESIF_PM_H_

#include "esif.h"
#include "esif_participant.h"

/*
 *******************************************************************************
 ** AGNOSTIC KERNEL/USER
 *******************************************************************************
 */

/* Maximum Participant Entries */
#define MAX_PARTICIPANT_ENTRY 20

/* Participant Instances */
#define ESIF_INSTANCE_LF        0	/* Reserved For ESIF Lower Framework */
#define ESIF_INSTANCE_FIRST     1	/* The First Useable Instance        */
#define ESIF_INSTANCE_UF        254	/* Reserved For ESIF Upper Framework */
#define ESIF_INSTANCE_BROADCAST 255	/* Send To All ESIF Instances        */

/* Participant State Machine States */
enum esif_pm_participant_state {
	/* Waiting For Registration Event */
	ESIF_PM_PARTICIPANT_STATE_AVAILABLE = 0,
	/* Removed participant */
	ESIF_PM_PARTICIPANT_REMOVED,
	/* Created Lower Participant Instance */
	ESIF_PM_PARTICIPANT_STATE_CREATED,
	/* In The Process Of Registering */
	ESIF_PM_PARTICIPANT_STATE_REGISTERING,
	ESIF_PM_PARTICIPANT_STATE_NEEDDSP,	/* Need DSP To Continue */
	ESIF_PM_PARTICIPANT_STATE_REQUESTDSP,	/* Requested DSP */
	ESIF_PM_PARTICIPANT_STATE_REGISTERED,	/* Registered And Ready To Go */
	ESIF_PM_PARTICIPANT_STATE_OPERATIONAL	/* Upper and Lower Ready */
};

/* Participant State Machine Descriptions */
static ESIF_INLINE esif_string esif_pm_participant_state_str(
	enum esif_pm_participant_state state)
{
	#define CREATE_STATE(ps, psd, str) case ps: str = (esif_string) psd; break;

	esif_string str = (esif_string)ESIF_NOT_AVAILABLE;
	switch (state) {
		CREATE_STATE(ESIF_PM_PARTICIPANT_STATE_AVAILABLE, "AVAILABLE", str)
		CREATE_STATE(ESIF_PM_PARTICIPANT_REMOVED, "REMOVED", str)
		CREATE_STATE(ESIF_PM_PARTICIPANT_STATE_CREATED, "CREATED", str)
		CREATE_STATE(ESIF_PM_PARTICIPANT_STATE_REGISTERING, "REGISTERING", str)
		CREATE_STATE(ESIF_PM_PARTICIPANT_STATE_NEEDDSP, "NEED_DSP", str)
		CREATE_STATE(ESIF_PM_PARTICIPANT_STATE_REQUESTDSP, "REQUESTED_DSP", str)
		CREATE_STATE(ESIF_PM_PARTICIPANT_STATE_REGISTERED, "REGISTERED", str)
		CREATE_STATE(ESIF_PM_PARTICIPANT_STATE_OPERATIONAL, "OPERATIONAL", str)
	}
	return str;
}


/*
 *******************************************************************************
 ** KERNEL - Lower Framework Lower Participant (esif_lf_pm.c)
 *******************************************************************************
 */
#ifdef ESIF_ATTR_KERNEL

/* Get State */
enum esif_pm_participant_state esif_lf_pm_lp_get_state(
	/* Lower Participant Intance */
	const struct esif_lp *lp_ptr);

/* Set State */
enum esif_rc esif_lf_pm_lp_set_state(
	/* Lower Participant Instance */
	struct esif_lp *lp_ptr,
	/* Particpant State Enumeration*/
	const enum esif_pm_participant_state state);

/* Get LP Instance By ID */
struct esif_lp *esif_lf_pm_lp_get_by_instance_id(const u8 id);

/* Get Interface By ID */
struct esif_participant_iface *esif_lf_pm_pi_get_by_instance_id(const u8 id);

/* Get LP By PI Pointer */
struct esif_lp *esif_lf_pm_lp_get_by_pi(
	/* Participant Interface */
	const struct esif_participant_iface *pi_ptr);

/* Create Participant */
struct esif_lp *esif_lf_pm_lp_create(
	/* Participant Interface */
	struct esif_participant_iface *pi_ptr);

/* Remove Participant */
void esif_lf_pm_lp_remove(
	/* Lower Participant */
	struct esif_lp *lp_ptr);

/* Participant Manager Init */
enum esif_rc esif_lf_pm_init(void);

/* Participant Manager Exit */
void esif_lf_pm_exit(void);

#endif	/* ESIF_ATTR_KERNEL */

/*
 *******************************************************************************
 ** USER - Upper Framework Upper Participant (esif_uf_pm.c)
 *******************************************************************************
 */

#ifdef ESIF_ATTR_USER


/* Participant Manager Entry */
typedef struct _t_EsifUpManagerEntry {
	enum esif_pm_participant_state  fState;
	EsifUpPtr fUpPtr;
} EsifUpManagerEntry, *EsifUpManagerEntryPtr, **EsifUpManagerEntryPtrLocation;


/* Package Manager */
typedef struct _t_EsifUppMgr {
	UInt8 fEntryCount;
	EsifUpManagerEntry fEntries[MAX_PARTICIPANT_ENTRY];
	esif_ccb_lock_t fLock;
} EsifUppMgr, *EsifUppMgrPtr, **EsifUppMgrPtrLocation;


EsifUpPtr EsifUpManagerGetAvailableParticipantByInstance(const UInt8 instance);
Bool EsifUpManagerDoesAvailableParticipantExistByName(char *participantName);
EsifUpPtr EsifUpManagerGetAvailableParticipantByName(char *participantName);

EsifUpPtr EsifUpManagerCreateParticipant(const eEsifParticipantOrigin origin,
					 const void *handle,
					 const void *metadataPtr);

eEsifError EsifUpManagerUnregisterParticipant(
					const eEsifParticipantOrigin origin,
					const void *handle);


typedef struct _t_EsifApp *EsifAppPtr;
eEsifError EsifUpManagerRegisterParticipantsWithApp(EsifAppPtr aAppPtr);
eEsifError EsifUpManagerDestroyParticipantsInApp(EsifAppPtr aAppPtr);

eEsifError EsifUppMgrInit(void);

void EsifUppMgrExit(void);

#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_PM_H_    */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
