/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#pragma once

#include "esif.h"
#include "esif_link_list.h"
#include "esif_uf_action.h"
#include "esif_participant.h"
#include "esif_uf_action_iface.h"

#define ACT_MGR_ITERATOR_MARKER 'AMGR'

typedef struct ActMgrIterator_s {
	u32 marker;
	enum esif_action_type type;
	Bool ref_taken;
	EsifActPtr actPtr;
} ActMgrIterator, *ActMgrIteratorPtr;

typedef struct EsifActMgrEntry_s {
	enum esif_action_type type; /* Quick access to the type */
	esif_context_t actCtx; /* Action handle for the action associated with a specific participant */

	Bool loadDelayed; /* Indicates that the action will not be loaded until used */
	EsifString libName; /* The Name Of The Library To Load */
	esif_lib_t lib; /* Library object */

	UInt32 numWaiters;
	Bool shouldDelete; /* Indicates a failure during creation and the last waiter should delete */
	esif_ccb_event_t creationCompleteEvent; /* Event to synchronize threads waiting for creation of same delayed-load action type */

	EsifActPtr actPtr;
} EsifActMgrEntry, *EsifActMgrEntryPtr;


typedef struct EsifActMgr_s {
	esif_ccb_lock_t mgrLock;

	/* List of Actions */
	UInt8 numActions;
	EsifLinkListPtr actions;	/* EsifActMgrEntry items */

	EsifLinkListPtr possibleActions;/* EsifActMgrEntry items for delayed load */

	/* The following are used to create dependent participants based on CPU arrival */
	Bool cpuArrived;
	EsifLinkListPtr loadedActions;

} EsifActMgr, *EsifActMgrPtr;

#ifdef __cplusplus
extern "C" {
#endif

/* Init / Exit */
eEsifError EsifActMgrInit(void);
void EsifActMgrExit(void);
void EsifActMgr_LoadAutomaticActions(void);


/*
 * Note:  This function takes an additional reference on the returned action
 * object.  When done using the action, EsifAct_PutRef must be called to
 * release the reference.
 */
EsifActPtr EsifActMgr_GetAction(
	enum esif_action_type type
	);

/*
 * Used to iterate through the static and currently loaded actions.
 * First call EsifActMgr_InitIterator to initialize the iterator.
 * Next, call EsifActMgr_GetNextAction using the iterator.  Repeat until
 * EsifActMgr_GetNextAction fails. (The call will release the reference of the
 * participant from the previous call.)  If you stop iteration part way through
 * all actions, the caller is responsible for releasing the reference on
 * the last action returned.  Iteration is complete when ESIF_E_ITERATOR_DONE
 * is returned.
 */
eEsifError EsifActMgr_InitIterator(
	ActMgrIteratorPtr iteratorPtr
	);

/* See EsifActMgr_InitIterator for usage */
eEsifError EsifActMgr_GetNextAction(
	ActMgrIteratorPtr iteratorPtr,
	EsifActPtr *upPtr
	);

/* For static actions */
eEsifError EsifActMgr_RegisterAction(EsifActIfacePtr actIfacePtr);
eEsifError EsifActMgr_UnregisterAction(EsifActIfacePtr actIfacePtr);
	
/* For plugin library UPE actions */
eEsifError EsifActMgr_RegisterDelayedLoadAction(enum esif_action_type type);

eEsifError EsifActMgr_StartUpe(EsifString upeName);
eEsifError EsifActMgr_StopUpe(EsifString upeName);

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
