/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "esif_uf_action_iface.h"
#include "esif_uf_fpc.h"
#include "esif_participant.h"

#define MAXPARAMLEN	65536

/* From the DSP compiler */
typedef enum DataItemType_e {
	DATA_ITEM_TYPE_STRING = 1,
	DATA_ITEM_TYPE_UINT08,
	DATA_ITEM_TYPE_UINT16,
	DATA_ITEM_TYPE_UINT32,
	DATA_ITEM_TYPE_UINT64
} DataItemType, *DataItemTypePtr;



typedef struct DataItem_s {
	UInt8 data_type;
	UInt16 data_length_in_bytes;
	UInt8 data[1];
} DataItem, *DataItemPtr;


typedef struct EsifAct_s {
	enum esif_action_type type; /* Quick access to the type defined by the interface */
	esif_context_t actCtx; /* Action context (returned from the action during creation) */

	Bool isPlugin;
	Bool createCalled;

	EsifActIface iface;

	/* life control */
	UInt32 refCount;		
	Bool markedForDelete;
	esif_ccb_event_t deleteEvent;
	esif_ccb_lock_t objLock;
} EsifAct, *EsifActPtr;

#ifdef __cplusplus
extern "C" {
#endif


/*
 * EsifAct object public interface
 */

/*
 * Takes an additional reference on an action object.  (The function is
 * called for you by the Action Manager when one of the functions are
 * called which returns a pointer to an action.)  After using the
 * action, EsifAct_PutRef must be called to release the reference.
 */
eEsifError EsifAct_GetRef(EsifActPtr self);

/*
 * Releases a reference on an action object.  This function should be
 * called when done using an action pointer obtained through any of the
 * Action Manager interfaces.
 */
void EsifAct_PutRef(EsifActPtr self);

EsifActIfacePtr EsifAct_GetIface(EsifActPtr self);
EsifActIfaceVer EsifAct_GetIfaceVersion(EsifActPtr self);
enum esif_action_type EsifAct_GetType(EsifActPtr self);
EsifString EsifAct_GetName(EsifActPtr self);
EsifString EsifAct_GetDesc(EsifActPtr self);
UInt16 EsifAct_GetVersion(EsifActPtr self);
Bool EsifAct_IsPlugin(EsifActPtr self);
esif_context_t EsifAct_GetActCtx(EsifActPtr self);


/* Control */

eEsifError EsifActConfigInit(void);
void EsifActConfigExit(void);
eEsifError EsifActConfigSignalChangeEvents(
	EsifUpPtr upPtr,
	const EsifPrimitiveTuple tuple,
	const EsifDataPtr requestPtr
	);

eEsifError EsifActConstInit(void);
void EsifActConstExit(void);

eEsifError EsifActSystemInit(void);
void EsifActSystemExit(void);

eEsifError EsifActDelegateInit(void);
void EsifActDelegateExit(void);

eEsifError EsifFpcAction_GetParams(
	EsifFpcActionPtr fpcActionPtr,
	EsifDataPtr paramsPtr,
	UInt8 numParams
	);

eEsifError EsifFpcAction_GetParamAsEsifData(
	EsifFpcActionPtr fpcActionPtr,
	UInt8 paramNum,
	EsifDataPtr paramPtr
	);

DataItemPtr EsifFpcAction_GetParam(
	const EsifFpcActionPtr fpcActionPtr,
	const UInt8 paramNum
	);

eEsifError EsifActCallPluginGet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcActionPtr fpcActionPtr,
	ActExecuteGetFunction actGetFuncPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	);

eEsifError EsifActCallPluginSet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcActionPtr fpcActionPtr,
	ActExecuteSetFunction actSetFuncPtr,
	const EsifDataPtr requestPtr
	);

eEsifError EsifCopyIntToBufBySize(
	size_t typeSize,
	void *dstPtr,
	u64 val
	);

#ifdef __cplusplus
}
#endif


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
