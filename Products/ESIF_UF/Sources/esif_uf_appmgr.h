/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

#define ESIF_MAX_APPS 5
#undef THIS

#define THIS struct _t_EsifAppManager *THIS
typedef struct _t_EsifAppManager {
	UInt8       fEntryCount;
	EsifApp     fEntries[ESIF_MAX_APPS];
	EsifAppPtr  fSelectedAppPtr;
	esif_ccb_lock_t  fLock;

	EsifAppPtr (*GetAppFromName)(THIS, EsifString name);
	eEsifError (*GetPrompt)(THIS, EsifDataPtr prompt);
} EsifAppMgr, *EsifAppMgrPtr, **EsifAppMgrPtrLocation;
#undef THIS

EsifAppPtr GetAppFromHandle(const void *handle);

/* Send Event */
eEsifError EsifAppsEvent(UInt8 participantId, UInt16 domainId, eEsifEventType eventType, EsifDataPtr eventData);

/* Send event by domain type to all applications */
eEsifError EsifAppsEventByDomainType(enum esif_domain_type domainType, eEsifEventType eventType, EsifDataPtr eventData);

eEsifError EsifAppMgrDestroyParticipantInAllApps(const EsifUpPtr upPtr);
eEsifError EsifAppMgrCreateCreateParticipantInAllApps(const EsifUpPtr upPtr);

/* Init / Exit */
eEsifError EsifAppMgrInit(void);
void EsifAppMgrExit(void);

#endif /* _ESIF_UF_APPMGR */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

