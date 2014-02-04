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

#ifndef _ESIF_UF_CNJMGR_
#define _ESIF_UF_CNJMGR_

#include "esif.h"
#include "esif_uf_conjure.h"

#define ESIF_MAX_CONJURES 5

#define THIS struct _t_EsifCnjMgr *THIS
typedef struct _t_EsifCnjMgr {
	UInt8    fEntryCount;
	EsifCnj  fEnrtries[ESIF_MAX_CONJURES];
	esif_ccb_lock_t fLock;
} EsifCnjMgr, *EsifCnjMgrPtr, **EsifCnjMgrPtrLocation;
#undef THIS

/* Conjure Control */
eEsifError EsifConjureStart(EsifCnjPtr cnjPtr);
eEsifError EsifConjureStop(EsifCnjPtr cnjPtr);

/* Conjure Support */
EsifCnjPtr esif_uf_conjure_get_instance_from_name(EsifString);

/* Init / Exit */
eEsifError EsifCnjMgrInit(void);
void EsifCnjMgrExit(void);

#endif /* _ESIF_UF_CNJMGR */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
