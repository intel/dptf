/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_APPMGR_
#define _ESIF_UF_APPMGR_

#include "esif.h"
#include "esif_uf_app.h"
#include "esif_queue.h"

#define ESIF_MAX_APPS 5
#define APPMGR_ITERATOR_MARKER 'APPM'

typedef struct _t_EsifAppMgrEntry {
	EsifAppPtr appPtr;
	Bool svcRefOnly;
} EsifAppMgrEntry, *EsifAppMgrEntryPtr;

typedef struct _t_EsifAppMgr {
	UInt8 fEntryCount;
	EsifAppMgrEntryPtr fEntries[ESIF_MAX_APPS];
	EsifAppPtr fSelectedAppPtr;
	esif_ccb_lock_t fLock;

	EsifQueuePtr partQueuePtr;
	esif_thread_t partQueueThread;
	Bool partQueueExitFlag;
	UInt32 creationRefCount;
	
	Bool	isInitialized;
} EsifAppMgr, *EsifAppMgrPtr;

typedef struct AppMgrIterator_s {
	UInt32 marker;
	size_t index;
	Bool refTaken;
	EsifAppPtr appPtr;
} AppMgrIterator, *AppMgrIteratorPtr;



#ifdef __cplusplus
extern "C" {
#endif

/* Init / Start / Stop / Exit */
eEsifError EsifAppMgr_Init(void);
eEsifError EsifAppMgr_Start(void);
void EsifAppMgr_Stop(void);
void EsifAppMgr_Exit(void);

/*
* App Retrieval Functions
* WARNING:  The caller is responsible for calling EsifAppMgr_PutRef on the
* returned pointer when done using the app
*/
EsifAppPtr EsifAppMgr_GetAppFromName(EsifString lib_name);

/* Get an app by it handle
* WARNING:  The caller is responsible for calling EsifAppMgr_PutRef on the
* returned pointer when done using the app
*/
EsifAppPtr EsifAppMgr_GetAppFromHandle(const esif_handle_t handle);

/*
* Releases a reference on the app and destroys the app if the reference count
* reaches 0.
* NOTE:  Must be called when done using any app pointer obtained from AppMgr
* functions.
*/
void EsifAppMgr_PutRef(EsifAppPtr appPtr);

/*
* Used to iterate through the available apps.
* First call AppMgr_InitIterator to initialize the iterator.
* Next, call AppMgr_GetNextApp using the iterator.  Repeat until
* AppMgr_GetNextApp fails. The call will release the reference of the
* app from the previous call.  If you stop iteration part way through
* all apps, the caller is responsible for releasing the reference on
* the last app returned.  Iteration is complete when ESIF_E_ITERATOR_DONE
* is returned.
*/
eEsifError AppMgr_InitIterator(
	AppMgrIteratorPtr iteratorPtr
);

/* See AppMgr_InitIterator for usage */
eEsifError AppMgr_GetNextApp(
	AppMgrIteratorPtr iteratorPtr,
	EsifAppPtr *appPtr
);

/* Participant State Reporting Functions */
eEsifError EsifAppMgr_DestroyParticipantInAllApps(const EsifUpPtr upPtr);
eEsifError EsifAppMgr_CreateParticipantInAllApps(const EsifUpPtr upPtr);

/* Start/Stop Apps using AppMgr */
eEsifError EsifAppMgr_AppStart(const EsifString appName);
eEsifError EsifAppMgr_AppStop(const EsifString appName);
eEsifError EsifAppMgr_AppRestartAll(void);

/* Get the command prompt for the currently selected app */
eEsifError EsifAppMgr_GetPrompt(EsifDataPtr promptPtr);

#ifdef __cplusplus
}
#endif


#endif /* _ESIF_UF_APPMGR */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

