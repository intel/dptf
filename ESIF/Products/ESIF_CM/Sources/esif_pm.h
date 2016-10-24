/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#define MAX_PARTICIPANT_ENTRY 32

/* Participant Instances */
#define ESIF_INSTANCE_LF        0	/* Reserved For ESIF Lower Framework */
#define ESIF_INSTANCE_FIRST     1	/* The First Useable Instance        */
#define ESIF_INSTANCE_UF        254	/* Reserved For ESIF Upper Framework */
#define ESIF_INSTANCE_BROADCAST 255	/* Send To All ESIF Instances        */


/* Participant Manager State Machine States */
enum esif_pm_participant_state {
	/* Slot is open and never been used */
	ESIF_PM_PARTICIPANT_STATE_AVAILABLE = 0,
	/* Removed participant */
	ESIF_PM_PARTICIPANT_STATE_REMOVED,
	/* Created Lower Participant Instance */
	ESIF_PM_PARTICIPANT_STATE_CREATED,
};

/* Participant State Machine Descriptions */
static ESIF_INLINE esif_string esif_pm_participant_state_str(
	enum esif_pm_participant_state state)
{
	switch (state) {
	ESIF_CASE(ESIF_PM_PARTICIPANT_STATE_AVAILABLE, "AVAILABLE");
	ESIF_CASE(ESIF_PM_PARTICIPANT_STATE_REMOVED, "REMOVED");
	ESIF_CASE(ESIF_PM_PARTICIPANT_STATE_CREATED, "CREATED");
	}
	return ESIF_NOT_AVAILABLE;
}


/*
 *******************************************************************************
 ** KERNEL - Lower Framework Lower Participant (esif_lf_pm.c)
 *******************************************************************************
 */
#ifdef ESIF_ATTR_KERNEL

struct lf_pm_iterator {
	u8 handle;
	u8 ref_taken;
	u32 marker;
};

#define lf_pm_iterator_t struct lf_pm_iterator

#define LF_PM_ITERATOR_MARKER 'LFPM'

/*
 * Get LP Instance By ID
 * NOTE: Code should call esif_lp_put_ref after done using LP
 */
struct esif_lp *esif_lf_pm_get_lp_by_instance_id(
	const u8 id
	);

/*
 * Get By participant type and takes a reference to the LP
 * NOTE: Code should call esif_lp_put_ref after done using LP
 */
struct esif_lp *esif_lf_pm_get_lp_by_type(
	enum esif_domain_type part_type
	);

/*
 * Get LP By PI Pointer and takes a reference to the LP
 * NOTE: Code should call esif_lp_put_ref after done using LP
 */
struct esif_lp *esif_lf_pm_get_lp_by_pi(
	/* Participant Interface */
	const struct esif_participant_iface *pi_ptr
	);

/*
 * Used to iterate through the available participants.
 * First call esif_lf_pm_init_iterator to initialize the iterator.
 * Next, call esif_lf_pm_get_next_lp using the iterator.  Repeat until
 * esif_lf_pm_get_next_lp fails. The call will release the reference of the
 * participant from the previous call.  If you stop iteration part way through
 * all participants, the caller is responsible for releasing the reference on
 * the last participant returned.  Iteration is complete when
 * ESIF_E_ITERATION_DONE is returned.
 */
enum esif_rc esif_lf_pm_init_iterator(
	lf_pm_iterator_t *iterator_ptr
	);

/* See esif_lf_pm_init_iterator for usage */
enum esif_rc esif_lf_pm_get_next_lp(
	lf_pm_iterator_t *iterator_ptr,
	struct esif_lp **lp_ptr
	);

/* Register Participant */
enum esif_rc esif_lf_pm_register_participant(
	/* Participant Interface */
	struct esif_participant_iface *pi_ptr
	);

/* Unregister Participant */
enum esif_rc esif_lf_pm_unregister_participant(
	struct esif_participant_iface *pi_ptr
	);

/* Unregister All Participants */
void esif_lf_pm_unregister_all_participants(void);

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

typedef struct UfPmIterator_s {
	u32 marker;
	UInt8 handle;
	Bool ref_taken;
	EsifUpPtr upPtr;
} UfPmIterator, *UfPmIteratorPtr;

#define UF_PM_ITERATOR_MARKER 'UFPM'

/* Participant Manager Entry */
typedef struct _t_EsifUpManagerEntry {
	enum esif_pm_participant_state  fState;
	EsifUpPtr fUpPtr;
} EsifUpManagerEntry, *EsifUpManagerEntryPtr, **EsifUpManagerEntryPtrLocation;


/* Participant Manager */
typedef struct _t_EsifUppMgr {
	UInt8 fEntryCount;
	EsifUpManagerEntry fEntries[MAX_PARTICIPANT_ENTRY];
	esif_ccb_lock_t fLock;
} EsifUppMgr, *EsifUppMgrPtr, **EsifUppMgrPtrLocation;


#ifdef __cplusplus
extern "C" {
#endif
/* The caller should call EsifUp_PutRef to release reference on participant when done with it */
EsifUpPtr EsifUpPm_GetAvailableParticipantByInstance(const UInt8 upInstance);

Bool EsifUpPm_DoesAvailableParticipantExistByName(char *participantName);
Bool EsifUpPm_DoesAvailableParticipantExistByHID(char *participantHID);

/* The caller should call EsifUp_PutRef to release reference on participant when done with it */
EsifUpPtr EsifUpPm_GetAvailableParticipantByName(char *participantName);

eEsifError EsifUpPm_RegisterParticipant(
	const eEsifParticipantOrigin origin,
	const void *metadataPtr,
	UInt8 *upInstancePtr
	);

eEsifError EsifUpPm_UnregisterParticipant(
	const eEsifParticipantOrigin origin,
	const UInt8 upInstance
	);

eEsifError EsifUpPm_ResumeParticipant(const UInt8 upInstance);

/*
 * Used to iterate through the available participants.
 * First call EsifUpPm_InitIterator to initialize the iterator.
 * Next, call EsifUpPm_GetNextUp using the iterator.  Repeat until
 * EsifUpPm_GetNextUp fails. The call will release the reference of the
 * participant from the previous call.  If you stop iteration part way through
 * all participants, the caller is responsible for releasing the reference on
 * the last participant returned.  Iteration is complete when
 * ESIF_E_ITERATOR_DONE is returned.
 */
eEsifError EsifUpPm_InitIterator(
	UfPmIteratorPtr iteratorPtr
	);

/* See EsifUpPm_InitIterator for usage */
eEsifError EsifUpPm_GetNextUp(
	UfPmIteratorPtr iteratorPtr,
	EsifUpPtr *upPtr
	);

eEsifError EsifUpPm_MapLpidToParticipantInstance(
	const UInt8 lpInstance,
	UInt8 *upInstancePtr
	);

eEsifError EsifUpPm_Init(void);
void EsifUpPm_Exit(void);

// Econo-Poll
#define ESIF_UFPOLL_PERIOD_DEFAULT 1000
#define ESIF_UFPOLL_PERIOD_MIN 500
eEsifError EsifUFPollStart(int pollInterval);
void EsifUFPollStop(void);
Bool EsifUFPollStarted(void);

#ifdef __cplusplus
}
#endif

#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_PM_H_    */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
