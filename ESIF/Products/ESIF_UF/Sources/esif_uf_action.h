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

#ifndef _ESIF_UF_ACTION_
#define _ESIF_UF_ACTION_

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


/* Map App Data To ESIF Prticipants */
typedef struct EsifAct_s {
	void  *fHandle;			/* The Action Handle Opaque To Us */
	EsifActInterface  fInterface;		/* The Action Interface */
	EsifString fLibNamePtr;		/* The Name Of The Library To Load */
	esif_lib_t fLibHandle;		/* Loadable Library Handle */
} EsifAct, *EsifActPtr, **EsifActPtrLocation;


typedef struct DataItem_s {
	UInt8 data_type;
	UInt16 data_length_in_bytes;
	UInt8 data[1];
} DataItem, *DataItemPtr, **DataItemPtrLocation;


#ifdef __cplusplus
extern "C" {
#endif

/* Control */
eEsifError EsifActStart(EsifActPtr actPtr);
eEsifError EsifActStop(EsifActPtr actPtr);

/* Init/Exit */
eEsifError EsifActInit(void);
void EsifActExit(void);

eEsifError EsifActConfigInit(void);
void EsifActConfigExit(void);

eEsifError EsifActConstInit(void);
void EsifActConstExit(void);

eEsifError EsifActSystemInit(void);
void EsifActSystemExit(void);
eEsifError EsifActDelegateInit(void);
void EsifActDelegateExit(void);

eEsifError EsifActSysfsInit(void);
void EsifActSysfsExit(void);

eEsifError EsifActionGetParams(
	EsifFpcActionPtr actionPtr,
	EsifDataPtr paramsPtr,
	UInt8 numParams
	);

eEsifError EsifActionGetParamAsEsifData(
	EsifFpcActionPtr actionPtr,
	UInt8 paramNum,
	EsifDataPtr paramPtr
	);

EsifString EsifActionCreateTokenReplacedParamString(
	const EsifString paramStr,
	const EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr
	);

DataItemPtr EsifActionGetParam(
	const EsifFpcActionPtr actionPtr,
	const UInt8 paramNum
	);

eEsifError EsifActCallPluginGet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcActionPtr actionPtr,
	ActExecuteGetFunction actGetFuncPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	);

eEsifError EsifActCallPluginSet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcActionPtr actionPtr,
	ActExecuteSetFunction actSetFuncPtr,
	const EsifDataPtr requestPtr
	);

eEsifError EsifActionCopyIntToBufBySize(
	size_t typeSize,
	void *dstPtr,
	u64 val
	);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_ACTION_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
