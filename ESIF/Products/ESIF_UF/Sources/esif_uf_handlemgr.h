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

#include <limits.h>
#include "esif.h"
#include "esif_link_list.h"


#define ESIF_HNDLMGR_NUM_HANDLE_BITS 16 /* Configurable - Must be between 6 and 31 */
#define ESIF_HNDLMGR_MAX_HANDLES (1 << ESIF_HNDLMGR_NUM_HANDLE_BITS)
#define ESIF_HNDLMGR_MAX_HANDLE_MAP_SIZE (ESIF_HNDLMGR_MAX_HANDLES / 8) /* Bytes */

#define ESIF_HNDLMGR_MIN_HANDLE_LINES 1 /* Configurable */
#define ESIF_HNDLMGR_HANDLES_PER_LINE 64 /* NOT configurable */
#define ESIF_HNDLMGR_LINE_SIZE (ESIF_HNDLMGR_HANDLES_PER_LINE / 8) /* Bytes */
#define ESIF_HNDLMGR_MIN_HANDLES (ESIF_HNDLMGR_MIN_HANDLE_LINES * ESIF_HNDLMGR_HANDLES_PER_LINE)

#define ESIF_HNDLMGR_INSTANCE_MASK (((UInt64)-1) << ESIF_HNDLMGR_NUM_HANDLE_BITS)
#define ESIF_HNDLMGR_HANDLE_MASK (~ESIF_HNDLMGR_INSTANCE_MASK)


typedef struct _t_EsifHandleManager {
	UInt64 *handleMapPtr; /* Pointer to the handle availability map */
	size_t handleMapSize; /* Number of bytes currently allocated to the map */
	UInt64 numAvailableHandles; /* Number of available handles in the map */

	size_t curLine; /* Current line number in the handle availability map */
	UInt32 curPos;	/* Current bit position in the current handle line */

	UInt64 instanceCounter;
	esif_ccb_lock_t  lock;

} EsifHandleMgr, *EsifHandleMgrPtr, **EsifHandleMgrPtrLocation;

typedef struct HandleMgrIterator_s {
	UInt32 marker;
	size_t index;
	Bool refTaken;
} HandleMgrIterator, *HandleMgrIteratorPtr;

typedef struct EsifHandleListItem_s {
	esif_handle_t esifHandle;
	Bool refTaken;
} EsifHandleListItem, *EsifHandleListItemPtr, **EsifHandleListItemPtrLocation;

#ifdef __cplusplus
extern "C" {
#endif

	/* Init / Start / Stop / Exit */
	eEsifError EsifHandleMgr_Init(void);
	eEsifError EsifHandleMgr_Start(void);
	void EsifHandleMgr_Stop(void);
	void EsifHandleMgr_Exit(void);
	eEsifError EsifHandleMgr_GetNextHandle(esif_handle_t *handlePtr);
	eEsifError EsifHandleMgr_PutHandle(esif_handle_t handle);


#ifdef __cplusplus
}
#endif


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

