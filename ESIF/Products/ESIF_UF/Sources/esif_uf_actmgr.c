/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ACTION

#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_actmgr.h"	/* Action Manager */
#include "esif_uf_service.h"
#include "esif_uf_ccb_imp_spec.h"
#include "esif_uf_eventmgr.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif


typedef eEsifError (ESIF_CALLCONV *GetIfaceFuncPtr)(EsifActIfacePtr);

/*
 * PRIVATE OBJECTS
 */
static EsifActMgr g_actMgr = {0};


/*
 * FRIEND ACTION FUNCTION PROTOTYPES
 */
eEsifError EsifAct_CreateAction(
	EsifActIfacePtr actIfacePtr,
	EsifActPtr *actPtr
	);

void EsifAct_DestroyAction(EsifActPtr actPtr);

eEsifError EsifActIface_GetType(
	EsifActIfacePtr self,
	enum esif_action_type *typePtr
	);
	
void EsifAct_MarkAsPlugin(EsifActPtr self);
UInt16 EsifActIface_Sizeof(EsifActIfaceVer fIfaceVersion);
Bool EsifActIface_IsSupported(EsifActIfacePtr self);


/*
 * PRIVATE FUNCTION PROTOTYPES
 */
static eEsifError EsifActMgr_CreateEntry(EsifActMgrEntryPtr* entryPtr);
static void EsifActMgr_DestroyEntry(EsifActMgrEntryPtr entryPtr);
static eEsifError EsifActMgr_AddEntry(EsifActMgrEntryPtr entryPtr);

static eEsifError EsifActMgr_InitActions();
static void EsifActMgr_UninitActions();

static eEsifError EsifActMgr_LoadAction(
	EsifActMgrEntryPtr entryPtr,
	GetIfaceFuncPtr *getIfacePtr
	);
static eEsifError EsifActMgr_UnloadAction(EsifActMgrEntryPtr entryPtr);

static eEsifError EsifActMgr_CreateAction(
	EsifActMgrEntryPtr entryPtr,
	EsifActIfacePtr actIfacePtr
	);

static EsifActMgrEntryPtr EsifActMgr_GetActionEntry_Locked(enum esif_action_type type);
static EsifActMgrEntryPtr EsifActMgr_GetActEntryByLibname_Locked(EsifString libName);
static struct esif_link_list_node *EsifActMgr_GetNodeFromEntry_Locked(EsifActMgrEntryPtr entryPtr);
static eEsifError EsifActMgr_CreatePossActList_Locked();

static eEsifError EsifActMgr_LoadDelayLoadAction(
	enum esif_action_type type
	);

static eEsifError EsifActMgr_GetTypeFromPossAct_Locked(
	EsifActMgrEntryPtr entryPtr,
	enum esif_action_type *typePtr
	);

static void EsifActMgr_LLEntryDestroyCallback(
	void *dataPtr
	);

static eEsifError ESIF_CALLCONV EsifActMgr_EventCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);

/*
 * FUNCTION DEFINITIONS
 */

EsifActPtr EsifActMgr_GetAction(
	enum esif_action_type type
	)
{
	eEsifError rc = ESIF_OK;
	EsifActPtr actPtr = NULL;
	EsifActMgrEntryPtr entryPtr = NULL;
	struct esif_link_list_node *nodePtr = NULL;

	esif_ccb_write_lock(&g_actMgr.mgrLock);

	/* First see if the action is available, else see if it can be loaded */
	entryPtr = EsifActMgr_GetActionEntry_Locked(type);
	if (NULL == entryPtr) {
		esif_ccb_write_unlock(&g_actMgr.mgrLock);
		goto exit;
	}
	if (entryPtr->loadDelayed == ESIF_FALSE) {
		actPtr = entryPtr->actPtr;
		rc = EsifAct_GetRef(actPtr);
		if (rc != ESIF_OK) {
			actPtr = NULL;
		}
		esif_ccb_write_unlock(&g_actMgr.mgrLock);
	} else {
		nodePtr = EsifActMgr_GetNodeFromEntry_Locked(entryPtr);
		esif_link_list_node_remove(g_actMgr.actions, nodePtr);
		g_actMgr.numActions--;
		esif_ccb_write_unlock(&g_actMgr.mgrLock);

		EsifActMgr_DestroyEntry(entryPtr);
		EsifActMgr_LoadDelayLoadAction(type);
		actPtr = EsifActMgr_GetAction(type);
	}
exit:
	return actPtr;
}

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
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == iteratorPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_memset(iteratorPtr, 0, sizeof(*iteratorPtr));
	iteratorPtr->marker = ACT_MGR_ITERATOR_MARKER;
exit:
	return rc;
}


/* See EsifActMgr_InitIterator for usage */
eEsifError EsifActMgr_GetNextAction(
	ActMgrIteratorPtr iteratorPtr,
	EsifActPtr *actPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifActPtr nextActPtr = NULL;
	struct esif_link_list_node *curNodePtr = NULL;
	struct esif_link_list_node *nextNodePtr = NULL;
	EsifActMgrEntryPtr entryPtr = NULL;

	if ((NULL == actPtr) || (NULL == iteratorPtr)) {
		ESIF_TRACE_WARN("Parameter is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Verify the iterator is initialized */
	if (iteratorPtr->marker != ACT_MGR_ITERATOR_MARKER) {
		ESIF_TRACE_WARN("Iterator invalid\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	if (iteratorPtr->ref_taken) {
		EsifAct_PutRef(iteratorPtr->actPtr);
		iteratorPtr->actPtr = NULL;
		iteratorPtr->ref_taken = ESIF_FALSE;
	}

	esif_ccb_write_lock(&g_actMgr.mgrLock);

	if (g_actMgr.actions == NULL) {
		esif_ccb_write_unlock(&g_actMgr.mgrLock);
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	curNodePtr = g_actMgr.actions->head_ptr;

	/*
	 * If this is the first action of the iteraction, use the head; else, find
	 * the next node after the current action type, but skip delay load actions
	 */
	if (iteratorPtr->type == 0) {
		nextNodePtr = curNodePtr;
	} else {
		while (curNodePtr) {
			entryPtr = (EsifActMgrEntryPtr)curNodePtr->data_ptr;
			if (entryPtr != NULL) {
				if ((entryPtr->type == iteratorPtr->type) &&
					(entryPtr->loadDelayed == ESIF_FALSE)) {
					nextNodePtr = curNodePtr->next_ptr;
					break;
				}
			}
			curNodePtr = curNodePtr->next_ptr;
		}
	}

	/* Skip any delay loaded actions */
	iteratorPtr->type = 0;
	while (nextNodePtr != NULL) {
		entryPtr = (EsifActMgrEntryPtr)nextNodePtr->data_ptr;
		if ((entryPtr != NULL) &&
			(entryPtr->loadDelayed == ESIF_FALSE)) {
			iteratorPtr->type = entryPtr->type;
			break;
		}
		nextNodePtr = nextNodePtr->next_ptr;
	}

	esif_ccb_write_unlock(&g_actMgr.mgrLock);

	if (iteratorPtr->type != 0) {
		nextActPtr = EsifActMgr_GetAction(iteratorPtr->type);
	}

	*actPtr = nextActPtr;

	if (nextActPtr != NULL) {
		iteratorPtr->actPtr = nextActPtr;
		iteratorPtr->ref_taken = ESIF_TRUE;
	} else {
		rc = ESIF_E_ITERATION_DONE;
	}
exit:
	return rc;
}


/* Insert Action Into List */
eEsifError EsifActMgr_RegisterAction(
	EsifActIfacePtr actIfacePtr
	)
{
	eEsifError rc = ESIF_OK;
	enum esif_action_type actType = 0;
	EsifActMgrEntryPtr existingEntryPtr = NULL;
	EsifActMgrEntryPtr entryPtr = NULL;

	/* Check EsifActIface */
	if (!EsifActIface_IsSupported(actIfacePtr)) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;	
	}

	/* Check if this type is already available; we only allow one instance */
	rc = EsifActIface_GetType(actIfacePtr, &actType);
	if (rc != ESIF_OK) {
		goto exit;
	}

	esif_ccb_write_lock(&g_actMgr.mgrLock);
	existingEntryPtr = EsifActMgr_GetActionEntry_Locked(actType);
	esif_ccb_write_unlock(&g_actMgr.mgrLock);

	if (existingEntryPtr != NULL) {
		rc = ESIF_E_ACTION_ALREADY_STARTED;
		goto exit;
	}

	rc = EsifActMgr_CreateEntry(&entryPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	rc = EsifActMgr_CreateAction(entryPtr, actIfacePtr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = EsifActMgr_AddEntry(entryPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}
exit:
	if (rc != ESIF_OK) {
		EsifActMgr_DestroyEntry(entryPtr);
	}
	return rc;
}


eEsifError EsifActMgr_UnregisterAction(
	EsifActIfacePtr actIfacePtr
	)
{
	eEsifError rc = ESIF_OK;
	enum esif_action_type actType = 0;
	struct esif_link_list_node *nodePtr = NULL;
	EsifActMgrEntryPtr entryPtr = NULL;

	if (NULL == actIfacePtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	rc = EsifActIface_GetType(actIfacePtr, &actType);
	if (rc != ESIF_OK) {
		goto exit;
	}
	
	esif_ccb_write_lock(&g_actMgr.mgrLock);

	entryPtr = EsifActMgr_GetActionEntry_Locked(actType);
	if (NULL == entryPtr) {
		esif_ccb_write_unlock(&g_actMgr.mgrLock);
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	nodePtr = EsifActMgr_GetNodeFromEntry_Locked(entryPtr);
	esif_link_list_node_remove(g_actMgr.actions, nodePtr);
	g_actMgr.numActions--;
	esif_ccb_write_unlock(&g_actMgr.mgrLock);

	EsifActMgr_DestroyEntry(entryPtr);
exit:
	return rc;
}


eEsifError EsifActMgr_RegisterDelayedLoadAction(
	enum esif_action_type type
	)
{
	eEsifError rc = ESIF_OK;
	EsifActMgrEntryPtr entryPtr = NULL;

	rc = EsifActMgr_CreateEntry(&entryPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	entryPtr->type = type;
	entryPtr->loadDelayed = ESIF_TRUE;

	rc = EsifActMgr_AddEntry(entryPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}
exit:
	if (rc != ESIF_OK) {
		EsifActMgr_DestroyEntry(entryPtr);
	}
	return rc;
}


static eEsifError EsifActMgr_LoadDelayLoadAction(
	enum esif_action_type type
	)
{
	eEsifError rc = ESIF_OK;
	struct esif_link_list_node *curNodePtr = NULL;
	struct esif_link_list_node *nextNodePtr = NULL;
	EsifActMgrEntryPtr entryPtr = NULL;
	enum esif_action_type possType = 0;
	EsifString libName = NULL;

	UNREFERENCED_PARAMETER(type);

	esif_ccb_write_lock(&g_actMgr.mgrLock);

	/* If the "Possible Actions List" has not been created; create it */
	if (NULL == g_actMgr.possibleActions) {
		rc = EsifActMgr_CreatePossActList_Locked();
		if (rc != ESIF_OK) {
			goto lockExit;
		}
		if (NULL == g_actMgr.possibleActions) {
			rc = ESIF_E_NO_MEMORY;
			goto lockExit;
		}
	}

	/* Search through the listed actions until we find one that is the type */
	nextNodePtr = g_actMgr.possibleActions->head_ptr;
	while (nextNodePtr) {
		curNodePtr = nextNodePtr;
		nextNodePtr = curNodePtr->next_ptr;

		entryPtr = (EsifActMgrEntryPtr)curNodePtr->data_ptr;

		ESIF_ASSERT(entryPtr != NULL);

		/*
		 * Get the type of the possible action, if it fails; remove the entry as there
		 * is no point in ever looking at it again
		 */
		rc = EsifActMgr_GetTypeFromPossAct_Locked(entryPtr, &possType);
		if (rc != ESIF_OK) {
			esif_link_list_node_remove(g_actMgr.possibleActions, curNodePtr);
			EsifActMgr_DestroyEntry(entryPtr);
			continue;	
		}

		/*
		 * If the library supports the action, load the action
		 */
		if (entryPtr->type == type) {
			libName = esif_ccb_strdup(entryPtr->libName);
			esif_ccb_write_unlock(&g_actMgr.mgrLock);

			rc = EsifActMgr_StartUpe(libName);

			goto exit;
		}
	}
lockExit:
	esif_ccb_write_unlock(&g_actMgr.mgrLock);
exit:
	esif_ccb_free(libName);
	return rc;
}


static eEsifError EsifActMgr_CreatePossActList_Locked()
{
	eEsifError rc = ESIF_OK;
	struct esif_ccb_file curFile = {0};
	esif_ccb_file_enum_t fileIter = {0};
	char libPath[ESIF_LIBPATH_LEN];
	char *dotPtr = NULL;
	EsifActMgrEntryPtr newPossPtr = NULL;
	EsifString filePattern = ESIF_UPE_FILE_PREFIX "*" ESIF_LIB_EXT;

	g_actMgr.possibleActions = esif_link_list_create();
	if (NULL == g_actMgr.possibleActions) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	/* Get the loadable action directory path */
	esif_build_path(libPath, sizeof(libPath), ESIF_PATHTYPE_DLL, NULL, NULL);

	fileIter = esif_ccb_file_enum_first(libPath, filePattern, &curFile);
	if (ESIF_INVALID_FILE_ENUM_HANDLE == fileIter) {
		goto exit;
	}

	do {
		newPossPtr = esif_ccb_malloc(sizeof(*newPossPtr));
		if(NULL == newPossPtr) {
			break;
		}
		dotPtr = esif_ccb_strchr(curFile.filename, '.');
		if (dotPtr != NULL) {
			*dotPtr = '\0';
			newPossPtr->libName = (esif_string)esif_ccb_strdup(curFile.filename);

			esif_link_list_add_at_back(g_actMgr.possibleActions, (void *)newPossPtr);
		}
		else {
			esif_ccb_free(newPossPtr);
		}
	} while (esif_ccb_file_enum_next(fileIter, filePattern, &curFile));
	esif_ccb_file_enum_close(fileIter);
exit:
	return rc;
}


static eEsifError EsifActMgr_GetTypeFromPossAct_Locked(
	EsifActMgrEntryPtr entryPtr,
	enum esif_action_type *typePtr
	)
{
	eEsifError rc = ESIF_OK;
	GetIfaceFuncPtr getIfacePtr = NULL;
	EsifActIface iface = {0};

	ESIF_ASSERT(entryPtr != NULL);
	ESIF_ASSERT(typePtr != NULL);

	/* If we already have the type from a previous search, return it */
	if (entryPtr->type != 0) {
		*typePtr = entryPtr->type;
	}

	/* If we don't already have the type; load the library and get the type */
	rc = EsifActMgr_LoadAction(entryPtr, &getIfacePtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	iface.hdr.fIfaceType = eIfaceTypeAction;
	iface.hdr.fIfaceVersion = ACTION_UPE_IFACE_VERSION;
	iface.hdr.fIfaceSize = sizeof(iface);

	rc = getIfacePtr(&iface);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Check EsifActIface */
	if (iface.hdr.fIfaceType != eIfaceTypeAction ||
		iface.hdr.fIfaceVersion > ESIF_ACT_FACE_VER_MAX ||
		iface.hdr.fIfaceSize != EsifActIface_Sizeof(iface.hdr.fIfaceVersion)) {
		ESIF_TRACE_ERROR("The action interface does not meet requirements\n");
		rc = ESIF_E_IFACE_NOT_SUPPORTED;
		goto exit;
	}

	/* Check if this type is already available; we only allow one instance */
	rc = EsifActIface_GetType(&iface, &entryPtr->type);
	if (rc != ESIF_OK) {
		goto exit;
	}

	*typePtr = entryPtr->type;
exit:
	if (entryPtr->lib != NULL) {
		EsifActMgr_UnloadAction(entryPtr);
	}
	return rc;
}
	

/* Loads a pluggable UPE action by library name */
eEsifError EsifActMgr_StartUpe(
	EsifString upeName
	)
{
	eEsifError rc = ESIF_OK;
	EsifActMgrEntryPtr existingEntryPtr = NULL;
	struct esif_link_list_node *existingNodePtr = NULL;
	EsifActMgrEntryPtr entryPtr = NULL;
	GetIfaceFuncPtr getIfacePtr = NULL;
	EsifActIface iface = {0};
	enum esif_action_type actType = 0;
	EsifData eventData = { ESIF_DATA_UINT32, &actType, sizeof(actType), sizeof(actType) };

	if (NULL == upeName) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Check to see if the action is already running only one instance per action allowed */
	esif_ccb_write_lock(&g_actMgr.mgrLock);
	existingEntryPtr = EsifActMgr_GetActEntryByLibname_Locked(upeName);
	esif_ccb_write_unlock(&g_actMgr.mgrLock);
	if (existingEntryPtr != NULL) {
		rc = ESIF_E_ACTION_ALREADY_STARTED;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Adding new UPE action %s\n", upeName);

	rc = EsifActMgr_CreateEntry(&entryPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	entryPtr->libName = (esif_string)esif_ccb_strdup(upeName);

	rc = EsifActMgr_LoadAction(entryPtr, &getIfacePtr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	iface.hdr.fIfaceType = eIfaceTypeAction;
	iface.hdr.fIfaceVersion = ACTION_UPE_IFACE_VERSION;
	iface.hdr.fIfaceSize = sizeof(iface);

	rc = getIfacePtr(&iface);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Check EsifActIface */
	if (!EsifActIface_IsSupported(&iface)) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;	
	}

	/*
	 * If we already have an action of the given type loaded, remove the
	 * current entry and replace it with the new action.
	 */
	rc = EsifActIface_GetType(&iface, &actType);
	if (rc != ESIF_OK) {
		goto exit;
	}

	esif_ccb_write_lock(&g_actMgr.mgrLock);
	existingEntryPtr = EsifActMgr_GetActionEntry_Locked(actType);

	if (existingEntryPtr != NULL) {
		existingNodePtr = EsifActMgr_GetNodeFromEntry_Locked(existingEntryPtr);
		esif_link_list_node_remove(g_actMgr.actions, existingNodePtr);
		g_actMgr.numActions--;
	}

	esif_ccb_write_unlock(&g_actMgr.mgrLock);
	EsifActMgr_DestroyEntry(existingEntryPtr);

	rc = EsifActMgr_CreateAction(entryPtr, &iface);
	if (ESIF_OK != rc) {
		goto exit;
	}

	EsifAct_MarkAsPlugin(entryPtr->actPtr);

	rc = EsifActMgr_AddEntry(entryPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_ACTION_LOADED, &eventData);

	ESIF_TRACE_DEBUG("Added action %s\n", upeName);
exit:
	if (rc != ESIF_OK) {
		ESIF_TRACE_WARN("Failure adding action %s\n", esif_rc_str(rc));
		EsifActMgr_DestroyEntry(entryPtr);
	}
	return rc;
}


static eEsifError EsifActMgr_StopUnusedUpes()
{
	eEsifError rc = ESIF_OK;
	EsifActMgrEntryPtr curEntryPtr = NULL;
	struct esif_link_list_node *curNodePtr = NULL;
	enum esif_action_type type = 0;
	EsifData eventData = { ESIF_DATA_UINT32, &type, sizeof(type), sizeof(type) };

	if (g_actMgr.actions == NULL) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	do {
		esif_ccb_write_lock(&g_actMgr.mgrLock);
		curNodePtr = g_actMgr.actions->head_ptr;

		while (curNodePtr != NULL) {
			curEntryPtr = (EsifActMgrEntryPtr)curNodePtr->data_ptr;
			//
			// If this is a UPE and not used by the current participants, unload it and remark it for dynamic loading
			//
			if (curEntryPtr && EsifAct_IsPlugin(curEntryPtr->actPtr) && !EsifUpPm_IsActionUsedByParticipants(curEntryPtr->type)) {
				ESIF_TRACE_DEBUG("Stopping Action: %s\n", curEntryPtr->libName);

				esif_link_list_node_remove(g_actMgr.actions, curNodePtr);
				g_actMgr.numActions--;
				break;
			}
			curNodePtr = curNodePtr->next_ptr;
		}

		esif_ccb_write_unlock(&g_actMgr.mgrLock);

		if (curNodePtr != NULL) {
			type = curEntryPtr->type;
			EsifActMgr_RegisterDelayedLoadAction(curEntryPtr->type);
			EsifActMgr_DestroyEntry(curEntryPtr);
			EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_ACTION_UNLOADED, &eventData);
		}
	} while (curNodePtr != NULL);
exit:
	return rc;
}


/* Unloads a pluggable UPE action by library name */
eEsifError EsifActMgr_StopUpe(EsifString upeName)
{
	eEsifError rc = ESIF_OK;
	EsifActMgrEntryPtr entryPtr = NULL;
	struct esif_link_list_node *nodePtr = NULL;
	enum esif_action_type actType = 0;
	EsifData eventData = { ESIF_DATA_UINT32, &actType, sizeof(actType), sizeof(actType) };

	if (NULL == upeName) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_write_lock(&g_actMgr.mgrLock);

	entryPtr = EsifActMgr_GetActEntryByLibname_Locked(upeName);
	if (NULL == entryPtr) {
		esif_ccb_write_unlock(&g_actMgr.mgrLock);
		rc = ESIF_E_ACTION_NOT_IMPLEMENTED;
		ESIF_TRACE_WARN("Failed To Find Action: %s\n", upeName);
		goto exit;
	}

	nodePtr = EsifActMgr_GetNodeFromEntry_Locked(entryPtr);
	esif_link_list_node_remove(g_actMgr.actions, nodePtr);
	g_actMgr.numActions--;

	esif_ccb_write_unlock(&g_actMgr.mgrLock);

	actType = entryPtr->type;
	EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_D0, ESIF_EVENT_ACTION_UNLOADED, &eventData);

	EsifActMgr_DestroyEntry(entryPtr);

	ESIF_TRACE_DEBUG("Stopped Action: %s\n", upeName);
exit:
	return rc;
}


static eEsifError EsifActMgr_LoadAction(
	EsifActMgrEntryPtr entryPtr,
	GetIfaceFuncPtr *getIfacePtr
	)
{
	eEsifError rc = ESIF_OK;
	GetIfaceFuncPtr ifaceFuncPtr = NULL;
	EsifString ifaceFuncName = ACTION_UPE_GET_INTERFACE_FUNCTION;
	char libPath[ESIF_LIBPATH_LEN];
	char altLibPath[ESIF_LIBPATH_LEN] = { 0 };

	ESIF_ASSERT(entryPtr != NULL);
	ESIF_ASSERT(getIfacePtr != NULL);

	ESIF_TRACE_DEBUG("Name=%s\n", entryPtr->libName);
	esif_build_path(libPath, sizeof(libPath), ESIF_PATHTYPE_DLL, entryPtr->libName, ESIF_LIB_EXT);
	entryPtr->lib = esif_ccb_library_load(libPath);

	if (NULL == entryPtr->lib || NULL == entryPtr->lib->handle) {

		// Try the alternate path for loadable libraries if different from normal path
		esif_build_path(altLibPath, ESIF_LIBPATH_LEN, ESIF_PATHTYPE_DLL_ALT, entryPtr->libName, ESIF_LIB_EXT);
		if (esif_ccb_strcmp(altLibPath, libPath) != 0) {
			rc = esif_ccb_library_error(entryPtr->lib);
			ESIF_TRACE_WARN("esif_ccb_library_load() %s failed [%s (%d)]: %s\n", libPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(entryPtr->lib));

			esif_ccb_library_unload(entryPtr->lib);
			entryPtr->lib = NULL;
			entryPtr->lib = esif_ccb_library_load(altLibPath);
		}
		if (NULL == entryPtr->lib || NULL == entryPtr->lib->handle) {
			rc = esif_ccb_library_error(entryPtr->lib);
			ESIF_TRACE_ERROR("esif_ccb_library_load() %s failed [%s (%d)]: %s\n", altLibPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(entryPtr->lib));
			goto exit;
		}
		esif_ccb_strcpy(libPath, altLibPath, sizeof(libPath));
		rc = ESIF_OK;
	}

	ESIF_TRACE_DEBUG("esif_ccb_library_load() %s completed.\n", libPath);

	ifaceFuncPtr = (GetIfaceFuncPtr)esif_ccb_library_get_func(entryPtr->lib, (EsifString)ifaceFuncName);
	if (NULL == ifaceFuncPtr) {
		rc = esif_ccb_library_error(entryPtr->lib);
		ESIF_TRACE_ERROR("esif_ccb_library_get_func() %s failed [%s (%d)]: %s\n", libPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(entryPtr->lib));
		goto exit;
	}
	ESIF_TRACE_DEBUG("esif_ccb_library_get_func() %s completed.\n", ifaceFuncName);

	*getIfacePtr = ifaceFuncPtr;
exit:
	return rc;
}


static eEsifError EsifActMgr_UnloadAction(EsifActMgrEntryPtr entryPtr)
{
	eEsifError rc = ESIF_OK;

	if (entryPtr == NULL) {
		ESIF_TRACE_ERROR("\tactPtr is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_library_unload(entryPtr->lib);
	entryPtr->lib = NULL;
exit:
	return rc;
}


static eEsifError EsifActMgr_CreateEntry(
	EsifActMgrEntryPtr* entryPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifActMgrEntryPtr newEntryPtr = NULL;

	if (NULL == entryPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	newEntryPtr = esif_ccb_malloc(sizeof(*newEntryPtr));
	if (NULL == newEntryPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	*entryPtr = newEntryPtr;
exit:
	return rc;
}


static void EsifActMgr_DestroyEntry(
	EsifActMgrEntryPtr entryPtr
	)
{
	if (NULL == entryPtr) {
		goto exit;
	}

	EsifAct_DestroyAction(entryPtr->actPtr);

	EsifActMgr_UnloadAction(entryPtr);

	esif_ccb_free(entryPtr->libName);
	esif_ccb_free(entryPtr);
exit:
	return;
}


static eEsifError EsifActMgr_AddEntry(
	EsifActMgrEntryPtr entryPtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(entryPtr != NULL);

	esif_ccb_write_lock(&g_actMgr.mgrLock);

	//
	// We add at front so that UPE's to minimize the O^2 loop in EsifActMgr_StopUnusedUpes
	//
	rc = esif_link_list_add_at_front(g_actMgr.actions, (void *)entryPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}
	g_actMgr.numActions++;
exit:
	esif_ccb_write_unlock(&g_actMgr.mgrLock);
	return rc;
}


static eEsifError EsifActMgr_CreateAction(
	EsifActMgrEntryPtr entryPtr,
	EsifActIfacePtr actIfacePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifActPtr actPtr = NULL;

	ESIF_ASSERT(entryPtr != NULL);
	ESIF_ASSERT(actIfacePtr != NULL);

	rc = EsifAct_CreateAction(actIfacePtr, &actPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	entryPtr->type = actPtr->type;
	entryPtr->actCtx = actPtr->actCtx;
	entryPtr->actPtr = actPtr;
exit:
	return rc;
}


static EsifActMgrEntryPtr EsifActMgr_GetActionEntry_Locked(
	enum esif_action_type type
	)
{
	EsifActMgrEntryPtr entryPtr = NULL;
	EsifActMgrEntryPtr curEntryPtr = NULL;
	struct esif_link_list_node *curNodePtr = NULL;

	if (g_actMgr.actions == NULL) {
		goto exit;
	}

	curNodePtr = g_actMgr.actions->head_ptr;
	while (curNodePtr) {
		curEntryPtr = (EsifActMgrEntryPtr)curNodePtr->data_ptr;
		if (curEntryPtr != NULL) {
			if (type == curEntryPtr->type) {
				entryPtr = curEntryPtr;
				break;
			}
		}
		curNodePtr = curNodePtr->next_ptr;
	}
exit:
	return entryPtr;
}


static EsifActMgrEntryPtr EsifActMgr_GetActEntryByLibname_Locked(
	EsifString libName
	)
{
	EsifActMgrEntryPtr entryPtr = NULL;
	struct esif_link_list_node *curNodePtr = NULL;

	if (g_actMgr.actions == NULL) {
		goto exit;
	}

	curNodePtr = g_actMgr.actions->head_ptr;
	while (curNodePtr) {
		EsifActMgrEntryPtr curEntryPtr = (EsifActMgrEntryPtr)curNodePtr->data_ptr;
		if ((curEntryPtr != NULL) && (curEntryPtr->libName != NULL)){
			if (!strcmp(curEntryPtr->libName, libName)) {
				entryPtr = curEntryPtr;
				break;
			}
		}
		curNodePtr = curNodePtr->next_ptr;
	}
exit:
	return entryPtr;
}


static struct esif_link_list_node *EsifActMgr_GetNodeFromEntry_Locked(
	EsifActMgrEntryPtr entryPtr
	)
{
	struct esif_link_list_node *curNodePtr = NULL;
	EsifActMgrEntryPtr curEntryPtr = NULL;

	if ((NULL == g_actMgr.actions) || (NULL == entryPtr)) {
		goto exit;
	}

	curNodePtr = g_actMgr.actions->head_ptr;
	while (curNodePtr) {
		curEntryPtr = (EsifActMgrEntryPtr)curNodePtr->data_ptr;

		if (curEntryPtr == entryPtr) {
			break;
		}
		curNodePtr = curNodePtr->next_ptr;
	}
exit:
	return curNodePtr;
}


eEsifError EsifActMgrInit()
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_actMgr.mgrLock);

	g_actMgr.actions = esif_link_list_create();
	if (NULL == g_actMgr.actions) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	EsifEventMgr_RegisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER_COMPLETE,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifActMgr_EventCallback,
		0);

	/* Action manager must be initialized before this call */
	EsifActMgr_InitActions();
exit:
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifActMgrExit()
{
	ESIF_TRACE_ENTRY_INFO();

	EsifEventMgr_UnregisterEventByType(ESIF_EVENT_PARTICIPANT_UNREGISTER_COMPLETE,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_DOMAIN_D0,
		EsifActMgr_EventCallback,
		0);

	/* Call before destroying action manager */
	EsifActMgr_UninitActions();

	esif_ccb_write_lock(&g_actMgr.mgrLock);

	esif_link_list_free_data_and_destroy(g_actMgr.actions, EsifActMgr_LLEntryDestroyCallback);
	g_actMgr.actions = NULL;

	esif_link_list_free_data_and_destroy(g_actMgr.possibleActions, EsifActMgr_LLEntryDestroyCallback);
	g_actMgr.possibleActions = NULL;

	esif_ccb_write_unlock(&g_actMgr.mgrLock);

	esif_ccb_lock_uninit(&g_actMgr.mgrLock);

	ESIF_TRACE_EXIT_INFO();
}


static eEsifError ESIF_CALLCONV EsifActMgr_EventCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(participantId);
	UNREFERENCED_PARAMETER(domainId);
	UNREFERENCED_PARAMETER(eventDataPtr);

	if (NULL == fpcEventPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	switch (fpcEventPtr->esif_event) {
	case ESIF_EVENT_PARTICIPANT_UNREGISTER_COMPLETE:
		EsifActMgr_StopUnusedUpes();
		break;
	default:
		break;
	}
exit:
	return rc;
}


static void EsifActMgr_LLEntryDestroyCallback(
	void *dataPtr
	)
{
	esif_ccb_write_unlock(&g_actMgr.mgrLock);

	EsifActMgr_DestroyEntry((EsifActMgrEntryPtr)dataPtr);

	esif_ccb_write_lock(&g_actMgr.mgrLock);
}


static eEsifError EsifActMgr_InitActions()
{
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_DPTFWWAN);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_USBFAN);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_JAVA);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_SIM);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_NVME);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_RFPWIFI);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_IOC);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_BATTERY);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_SOCWC);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_HWPF);
	EsifActMgr_RegisterDelayedLoadAction(ESIF_ACTION_SYSMAN);

	EsifActConfigInit();
	EsifActConstInit();
	EsifActSystemInit();
	EsifActDelegateInit();
	esif_ccb_imp_spec_actions_init();
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


static void EsifActMgr_UninitActions()
{
	EsifActConfigExit();
	EsifActConstExit();
	EsifActSystemExit();
	EsifActDelegateExit();
	esif_ccb_imp_spec_actions_exit();
	ESIF_TRACE_EXIT_INFO();
}




/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
