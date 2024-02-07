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
#include "esif_uf.h"
#include "esif_sdk_iface.h"
#include "esif_sdk_iface_upe.h"
#include "esif_dsp.h"
#include "esif_participant.h"

#define ESIF_ACTION_VERSION_DEFAULT 1
#define ESIF_ACTION_VERSION_INVALID ((UInt16)-1)
#define ESIF_ACTION_FLAGS_DEFAULT 0


typedef eEsifError(ESIF_CALLCONV *ActGetFunctionStatic)(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

typedef eEsifError(ESIF_CALLCONV *ActSetFunctionStatic)(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr
	);


#pragma pack(push,1)

typedef struct EsifActIfaceStatic_s {
	EsifIfaceHdr hdr;

	enum esif_action_type type;
	esif_flags_t flags;

	char name[ESIF_NAME_LEN];
	char desc[ESIF_DESC_LEN];

	UInt16 actVersion; /* Version of the action (not the interface) */

	ActCreateFunction	createFuncPtr;
	ActDestroyFunction	destroyFuncPtr;

	ActGetFunctionStatic  getFuncPtr;
	ActSetFunctionStatic  setFuncPtr;
} EsifActIfaceStatic, *EsifActIfaceStaticPtr;


// EsifActIfacePtr typedef is in esif_sdk_iface_upe.h for clang compliance
typedef union EsifActIface_u {
	EsifIfaceHdr hdr;
	EsifActIfaceStatic ifaceStatic;
	EsifActIfaceUpeV4 actIfaceV4; /* Use with loadable actions */
} EsifActIface;


#pragma pack(pop)


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
