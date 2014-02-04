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

#ifndef _ESIF_UF_ACTMGR_
#define _ESIF_UF_ACTMGR_

#include "esif.h"
#include "esif_link_list.h"
#include "esif_uf_action.h"

#define ESIF_MAX_ACTIONS 5

typedef eEsifError (*ActGetFunction)(const void *actionHandle, const EsifString devicePathPtr, const EsifDataPtr p1Ptr, const EsifDataPtr p2Ptr,
									 const EsifDataPtr p3Ptr, const EsifDataPtr p4Ptr, const EsifDataPtr p5Ptr, const EsifDataPtr requestPtr,
									 EsifDataPtr responsePtr);

typedef eEsifError (*ActSetFunction)(const void *actionHandle, const EsifString devicePathPtr, const EsifDataPtr p1Ptr, const EsifDataPtr p2Ptr,
									 const EsifDataPtr p3Ptr, const EsifDataPtr p4Ptr, const EsifDataPtr p5Ptr, const EsifDataPtr requestPtr);

typedef struct _t_EsifActType {
	void   *fHandle;/* NULL for built in actions */
	UInt8  fType;
	UInt8  fReserved[3];
	char   fName[ESIF_NAME_LEN];
	char   fDesc[ESIF_DESC_LEN];
	char   fOsType[ESIF_NAME_LEN];
	char   fVersion[ESIF_DESC_LEN];
	esif_guid_t fGuid;
	UInt8  fIsKernel;
	UInt8  fIsPlugin;
	UInt8  fReserved2[2];
	ActGetFunction  fGetFuncPtr;
	ActSetFunction  fSetFuncPtr;
} EsifActType, *EsifActTypePtr, **EsifActTypePtrLocation;

#define THIS struct _t_EsifActMgr *THIS
typedef struct _t_EsifActMgr {
	UInt8    fEntryCount;
	EsifAct  fEnrtries[ESIF_MAX_ACTIONS];
	EsifActPtr  fSelectedActionPtr;
	esif_ccb_lock_t  fLock;

	/* Action Accessors */
	eEsifError (*AddActType)(THIS, EsifActTypePtr typePtr);
	eEsifError (*RemoveActType)(THIS, EsifActTypePtr typePtr);
	EsifActTypePtr (*GetActType)(THIS, UInt8 type);
	EsifActPtr (*GetActFromName)(THIS, EsifString name);

	/* List of Actions */
	EsifLinkListPtr  fActTypes;	/* Action Types */
} EsifActMgr, *EsifActMgrPtr, **EsifActMgrPtrLocation;
#undef THIS

/* Init / Exit */
eEsifError EsifActMgrInit();
void EsifActMgrExit();

#endif /* ESIF_UF_ACTMGR */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
