/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
	UInt8 upInstance,
	EsifActPtr *actPtr
	);

void EsifAct_DestroyAction(EsifActPtr actPtr);

eEsifError EsifActIface_GetType(
	EsifActIfacePtr self,
	enum esif_action_type *typePtr
	);
	
void EsifAct_MarkAsPlugin(EsifActPtr self);
UInt16 EsifActIface_Sizeof(EsifActIfaceVer fIfaceVersion);

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
	enum esif_action_type type,
	UInt8 upInstance
	);

static eEsifError EsifActMgr_GetTypeFromPossAct_Locked(
	EsifActMgrEntryPtr entryPtr,
	enum esif_action_type *typePtr
	);

static void EsifActMgr_LLEntryDestroyCallback(
	void *dataPtr
	);

/*
 * FUNCTION DEFINITIONS
 */

EsifActPtr EsifActMgr_GetAction(
	enum esif_action_type type,
	const UInt8 instance
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
	if (entryPtr->loadDelayed != ESIF_TRUE) {
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
		EsifActMgr_LoadDelayLoadAction(type, instance);
		actPtr = EsifActMgr_GetAction(type, instance);
	}
exit:
	return actPtr;
}

/*
 * Used to iterate through the available participants.
 * First call EsifActMgr_InitIterator to initialize the iterator.
 * Next, call EsifActMgr_GetNexAction using the iterator.  Repeat until
 * EsifActMgr_GetNexAction fails. The call will release the reference of the
 * participant from the previous call.  If you stop iteration part way through
 * all participants, the caller is responsible for releasing the reference on
 * the last participant returned.  Iteration is complete when
 * ESIF_E_ITERATOR_DONE is returned.
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


/* See EsifUpPm_InitIterator for usage */
eEsifError EsifActMgr_GetNexAction(
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

	if (iteratorPtr->type == 0) {
		nextNodePtr = curNodePtr;
	} else {
		while (curNodePtr) {
			entryPtr = (EsifActMgrEntryPtr)curNodePtr->data_ptr;
			if (entryPtr != NULL) {
				if (entryPtr->type == iteratorPtr->type) {
					nextNodePtr = curNodePtr->next_ptr;
					break;
				}
			}
			curNodePtr = curNodePtr->next_ptr;
		}
	}

	iteratorPtr->type = 0;
	if (nextNodePtr != NULL) {
		entryPtr = (EsifActMgrEntryPtr)nextNodePtr->data_ptr;
		if (entryPtr != NULL) {
			iteratorPtr->type = entryPtr->type;
		}
	}

	esif_ccb_write_unlock(&g_actMgr.mgrLock);

	if (iteratorPtr->type != 0) {
		nextActPtr = EsifActMgr_GetAction(iteratorPtr->type, ACT_MGR_NO_UPINSTANCE);
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
	EsifActMgrEntryPtr entryPtr = NULL;

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
	g_actMgr.numActions++;
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
	enum esif_action_type type,
	UInt8 upInstance
	)
{
	eEsifError rc = ESIF_OK;
	struct esif_link_list_node *curNodePtr = NULL;
	struct esif_link_list_node *nextNodePtr = NULL;
	EsifActMgrEntryPtr entryPtr = NULL;
	enum esif_action_type possType = 0;

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
		 * If the library supports the action, load the action and remove the
		 * entry from the possible action list
		 */
		if (entryPtr->type == type) {
			esif_link_list_node_remove(g_actMgr.possibleActions, curNodePtr);
			esif_ccb_write_unlock(&g_actMgr.mgrLock);

			rc = EsifActMgr_StartUpe(entryPtr->libName, upInstance);
			if (rc == ESIF_OK) {
				CMD_OUT("Started UPE: %s\n", entryPtr->libName);
			}

			EsifActMgr_DestroyEntry(entryPtr);
			goto exit;
		}
	}
lockExit:
	esif_ccb_write_unlock(&g_actMgr.mgrLock);
exit:
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
	if (INVALID_HANDLE_VALUE == fileIter) {
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
	iface.hdr.fIfaceVersion = ESIF_INTERFACE_VERSION;
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
		goto exit;
	}

	/* Check if this type is already available; we only allow one instance */
	rc = EsifActIface_GetType(&iface, &entryPtr->type);
	if (rc != ESIF_OK) {
		goto exit;
	}

	*typePtr = entryPtr->type;
exit:
	if (rc != ESIF_OK) {
		EsifActMgr_UnloadAction(entryPtr);
	}
	return rc;
}
	

/* Loads a pluggable UPE action by library name */
eEsifError EsifActMgr_StartUpe(
	EsifString upeName,
	UInt8 upInstance
	)
{
	eEsifError rc = ESIF_OK;
	EsifActMgrEntryPtr entryPtr = NULL;
	GetIfaceFuncPtr getIfacePtr = NULL;
	EsifActIface iface = {0};

	if (NULL == upeName) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Check to see if the action is already running only one instance per action allowed */
	esif_ccb_write_lock(&g_actMgr.mgrLock);
	entryPtr = EsifActMgr_GetActEntryByLibname_Locked(upeName);
	esif_ccb_write_unlock(&g_actMgr.mgrLock);
	if (entryPtr != NULL) {
		entryPtr = NULL; /* Keep from being destroy on exit for this failure type */
		rc = ESIF_E_ACTION_ALREADY_STARTED;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Adding new UPE action %s\n", upeName);

	rc = EsifActMgr_CreateEntry(&entryPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}
	entryPtr->upInstance = upInstance;
	entryPtr->libName = (esif_string)esif_ccb_strdup(upeName);

	rc = EsifActMgr_LoadAction(entryPtr, &getIfacePtr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	iface.hdr.fIfaceType = eIfaceTypeAction;
	iface.hdr.fIfaceVersion = ESIF_INTERFACE_VERSION;
	iface.hdr.fIfaceSize = sizeof(iface);

	rc = getIfacePtr(&iface);
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = EsifActMgr_CreateAction(entryPtr, &iface);
	if (ESIF_OK != rc) {
		goto exit;
	}

	EsifAct_MarkAsPlugin(entryPtr->actPtr);

	rc = EsifActMgr_AddEntry(entryPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	ESIF_TRACE_DEBUG("Added action %s\n", upeName);
exit:
	if (rc != ESIF_OK) {
		ESIF_TRACE_ERROR("Failure adding action %s\n", esif_rc_str(rc));
		EsifActMgr_DestroyEntry(entryPtr);
	}
	return rc;
}


/* Unloads a pluggable UPE action by library name */
eEsifError EsifActMgr_StopUpe(EsifString upeName)
{
	eEsifError rc = ESIF_OK;
	EsifActMgrEntryPtr entryPtr = NULL;
	struct esif_link_list_node *nodePtr = NULL;

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
	EsifString ifaceFuncName     = "GetActionInterface";
	char libPath[ESIF_LIBPATH_LEN];

	ESIF_ASSERT(entryPtr != NULL);
	ESIF_ASSERT(getIfacePtr != NULL);

	ESIF_TRACE_DEBUG("Name=%s\n", entryPtr->libName);
	esif_build_path(libPath, sizeof(libPath), ESIF_PATHTYPE_DLL, entryPtr->libName, ESIF_LIB_EXT);
	entryPtr->lib = esif_ccb_library_load(libPath);

	if (NULL == entryPtr->lib || NULL == entryPtr->lib->handle) {
		rc = esif_ccb_library_error(entryPtr->lib);
		ESIF_TRACE_ERROR("esif_ccb_library_load() %s failed [%s (%d)]: %s\n", libPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(entryPtr->lib));
		goto exit;
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

	newEntryPtr->upInstance = ACT_MGR_NO_UPINSTANCE;

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

	rc =  esif_link_list_add_at_back(g_actMgr.actions, (void *)entryPtr);
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
	enum esif_action_type actType = 0;
	EsifActPtr actPtr = NULL;

	ESIF_ASSERT(entryPtr != NULL);
	ESIF_ASSERT(actIfacePtr != NULL);

	/* Check EsifActIface */
	if (actIfacePtr->hdr.fIfaceType != eIfaceTypeAction ||
		actIfacePtr->hdr.fIfaceVersion > ESIF_ACT_FACE_VER_MAX ||
		actIfacePtr->hdr.fIfaceSize != EsifActIface_Sizeof(actIfacePtr->hdr.fIfaceVersion)) {
		ESIF_TRACE_ERROR("The action interface does not meet requirements\n");
		goto exit;
	}

	/* Check if this type is already available; we only allow one instance */
	rc = EsifActIface_GetType(actIfacePtr, &actType);
	if (rc != ESIF_OK) {
		goto exit;
	}

	actPtr = EsifActMgr_GetAction(actType, ACT_MGR_NO_UPINSTANCE);
	if (actPtr != NULL) {
		EsifAct_PutRef(actPtr);
		rc = ESIF_E_ACTION_ALREADY_STARTED;
		goto exit;
	}

	rc = EsifAct_CreateAction(actIfacePtr, entryPtr->upInstance, &actPtr);
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

	/* Action manager must be initialized before this call */
	EsifActMgr_InitActions();
exit:
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifActMgrExit()
{
	ESIF_TRACE_ENTRY_INFO();

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
