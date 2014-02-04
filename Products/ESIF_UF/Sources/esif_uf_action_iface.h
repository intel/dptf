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

#ifndef _ESIF_UF_ACT_IFACE_
#define _ESIF_UF_ACT_IFACE_

#include "esif.h"
#include "esif_uf_iface.h"

#define ACTION_INTERFACE_VERSION 1

typedef enum _t_EsifActState {
	eEsifActStateDisabled = 0,
	eEsifActStateEnabled
} eEsifActState;

typedef eEsifError (*ActGetAboutFunction)(EsifDataPtr actionAbout);
typedef eEsifError (*ActGetDescriptionFunction)(EsifDataPtr actionDescription);
typedef eEsifError (*ActGetIDFunction)(EsifDataPtr actionId);
typedef eEsifError (*ActGetGuidFunction)(EsifDataPtr actionGuid);
typedef eEsifError (*ActGetNameFunction)(EsifDataPtr actionName);
typedef eEsifError (*ActGetVersionFunction)(EsifDataPtr actionVersion);

typedef eEsifError (*ActCreateFunction)(EsifInterfacePtr esifServiceInterface,	/* The App MUST fill in all pointers */
										const void *esifHandle,	/* ESIF will provide App MUST save for use with callbacks */
										void **actionHandleLocation,	/* The App MUST provide esif will save for use with callbacks */
										const eEsifActState initialActionState, const EsifDataPtr P1PtrDescription, const EsifDataPtr P2PtrDescription,
										const EsifDataPtr P3PtrDescription, const EsifDataPtr P4PtrDescription, const EsifDataPtr P5PtrDescription);

/*

    DEPENDENCY
    The remaining funcitons are dependent on application create.
 */

typedef eEsifError (*ActDestroyFunction)(void *actionHandle);

typedef eEsifError (*ActExecuteGetFunction)(const void *actionHandle, const esif_string devicePathPtr, const EsifDataPtr P1Ptr, const EsifDataPtr P2Ptr,
											const EsifDataPtr P3Ptr, const EsifDataPtr P4Ptr, const EsifDataPtr P5Ptr, const EsifDataPtr requestPtr,
											EsifDataPtr responsePtr);

typedef eEsifError (*ActExecuteSetFunction)(const void *actionHandle, const esif_string devicePathPtr, const EsifDataPtr P1Ptr, const EsifDataPtr P2Ptr,
											const EsifDataPtr P3Ptr, const EsifDataPtr P4Ptr, const EsifDataPtr P5Ptr, const EsifDataPtr requestPtr);

typedef eEsifActState (*ActGetStateFunction)(const void *actionHandle);

typedef eEsifError (*ActSetStateFunction)(const void *actionHandle, const eEsifActState actionState);

typedef eEsifError (*ActGetStatusFunction)(const void *actionHandle, EsifDataPtr statusPtr);

typedef struct _t_EsifActInterface {
	// Header
	eIfaceType  fIfaceType;
	UInt16      fIfaceVersion;
	UInt16      fIfaceSize;

	// Get
	ActGetAboutFunction    fActGetAboutFuncPtr;
	ActGetGuidFunction     fActGetGuidFuncPtr;
	ActGetIDFunction       fActGetIDFuncPtr;
	ActGetNameFunction     fActGetNameFuncPtr;
	ActGetDescriptionFunction fActGetDescriptionFuncPtr;
	ActGetVersionFunction  fActGetVersionFuncPtr;

	// Create
	ActCreateFunction   fActCreateFuncPtr;
	ActDestroyFunction  fActDestroyFuncPtr;

	// State
	ActSetStateFunction    fActSetStateFuncPtr;
	ActGetStateFunction    fActGetStateFuncPtr;

	ActExecuteGetFunction  fActGetFuncPtr;
	ActExecuteSetFunction  fActSetFuncPtr;

	ActGetStatusFunction   fActGetStatusFuncPtr;
} EsifActInterface, *EsifActInterfacePtr, **EsifActInterfacePtrLocation;

#endif	// _ESIF_UF_ACT_IFACE_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/