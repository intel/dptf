/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_APP

#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_handlemgr.h"	/* Application Manager */
#include "esif_link_list.h"


//
// GLOBAL DEFINITIONS
// 

// Singleton Handle Manager
static EsifHandleMgrPtr g_EsifHandleMgr = NULL;

//PRIVATE DEFINITIONS
static eEsifError EsifHandleMgr_GetHandle(EsifHandleMgrPtr self, esif_handle_t *handlePtr);
static eEsifError EsifHandleMgr_AllocateAdditionalHandles(EsifHandleMgrPtr self);
static eEsifError EsifHandleMgr_FindHandle(EsifHandleMgrPtr self, esif_handle_t *handlePtr);
static eEsifError EsifHandleMgr_FindHandleInRegion(
	EsifHandleMgrPtr self,
	size_t startLineNum,
	size_t stopLineNum,
	UInt32 startPos,
	UInt32 stopPos,
	esif_handle_t *handlePtr
);
static EsifHandleMgrPtr EsifHandleMgr_Create();
static void EsifHandleMgr_Destroy(EsifHandleMgrPtr self);
static Bool EsifHandleMgr_IsReservedHandle(esif_handle_t handle);

//
// PUBLIC
//

eEsifError EsifHandleMgr_GetNextHandle(esif_handle_t *handlePtr)
{
	eEsifError rc = ESIF_E_PARAMETER_IS_NULL;

	EsifHandleMgrPtr self = g_EsifHandleMgr;

	if (self && handlePtr) {
		rc = EsifHandleMgr_GetHandle(self, handlePtr);
	}
	
	return rc;
}

eEsifError EsifHandleMgr_PutHandle(esif_handle_t handle) {
	eEsifError rc = ESIF_E_UNSPECIFIED;
	EsifHandleMgrPtr self = g_EsifHandleMgr;
	size_t lineNum = 0;
	UInt32 pos = 0;
	UInt64 handleValue = 0;

	if (EsifHandleMgr_IsReservedHandle(handle)) {
		goto exit;
	}
	if (self) {
		esif_ccb_write_lock(&self->lock);

		if (self->handleMapPtr) {
			handleValue = (UInt64)(size_t)handle;

			handleValue &= ESIF_HNDLMGR_HANDLE_MASK;
			lineNum = (size_t)(handleValue / ESIF_HNDLMGR_HANDLES_PER_LINE);
			pos = (UInt32)(handleValue % ESIF_HNDLMGR_HANDLES_PER_LINE);

			if (lineNum < (self->handleMapSize / ESIF_HNDLMGR_LINE_SIZE)) {
				self->handleMapPtr[lineNum] &= ~((UInt64)1 << pos);
				self->numAvailableHandles++;
			}
		}
		esif_ccb_write_unlock(&self->lock);
	}
exit:
	return rc;
}


eEsifError EsifHandleMgr_Init(void)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	if (!g_EsifHandleMgr) {
		g_EsifHandleMgr = EsifHandleMgr_Create();
		if (!g_EsifHandleMgr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}
	}
exit:
	if (rc != ESIF_OK) {
		EsifHandleMgr_Destroy(g_EsifHandleMgr);
		g_EsifHandleMgr = NULL;
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);

	return rc;
}


void EsifHandleMgr_Exit(void)
{
	ESIF_TRACE_ENTRY_INFO();

	EsifHandleMgr_Destroy(g_EsifHandleMgr);
	g_EsifHandleMgr = NULL;

	ESIF_TRACE_EXIT_INFO();
}


eEsifError EsifHandleMgr_Start(void)
{
	eEsifError rc = ESIF_OK;
	ESIF_TRACE_ENTRY_INFO();
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifHandleMgr_Stop(void)
{
	ESIF_TRACE_ENTRY_INFO();
	ESIF_TRACE_EXIT_INFO();
}


//
// PRIVATE
//

static eEsifError EsifHandleMgr_GetHandle(
	EsifHandleMgrPtr self,
	esif_handle_t *handlePtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self);
	ESIF_ASSERT(handlePtr);

	rc = ESIF_E_INVALID_HANDLE;
	*handlePtr = ESIF_INVALID_HANDLE;

	esif_ccb_write_lock(&self->lock);

	/* If we are out of handles and are at maximum map size; exit */
	if (!self->numAvailableHandles && (self->handleMapSize >= ESIF_HNDLMGR_MAX_HANDLE_MAP_SIZE)) {
		goto exit;
	}

	/* If we don't have a minimum of free handles, but can add more; reallocate the map */
	if ((self->numAvailableHandles < ESIF_HNDLMGR_HANDLES_PER_LINE) && (self->handleMapSize < ESIF_HNDLMGR_MAX_HANDLE_MAP_SIZE)) {
		rc = EsifHandleMgr_AllocateAdditionalHandles(self);
		if (rc != ESIF_OK) {
			goto exit;
		}
	}

	rc = EsifHandleMgr_FindHandle(self, handlePtr);
exit:
	esif_ccb_write_unlock(&self->lock);
	return rc;
}


static eEsifError EsifHandleMgr_AllocateAdditionalHandles(EsifHandleMgrPtr self)
{
	eEsifError rc = ESIF_OK;
	size_t newMapSize = 0;
	UInt64 *handleMapPtr = NULL;

	ESIF_ASSERT(self);

	/* Add a single line to the availability map */
	newMapSize = self->handleMapSize + ESIF_HNDLMGR_LINE_SIZE;

	handleMapPtr = esif_ccb_realloc(self->handleMapPtr, newMapSize);
	if (NULL == handleMapPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	esif_ccb_memset((char *)handleMapPtr + self->handleMapSize, 0, ESIF_HNDLMGR_LINE_SIZE);

	/* Update the map information */
	self->handleMapPtr = handleMapPtr;
	self->handleMapSize = newMapSize;
	self->numAvailableHandles += ESIF_HNDLMGR_HANDLES_PER_LINE;
exit:
	return rc;
}


static eEsifError EsifHandleMgr_FindHandle(
	EsifHandleMgrPtr self,
	esif_handle_t *handlePtr
	)
{
	eEsifError rc = ESIF_E_INVALID_HANDLE;

	ESIF_ASSERT(self);
	ESIF_ASSERT(handlePtr);

	/* If necessary, check remainder of map */
	if ((self->curPos != (ESIF_HNDLMGR_HANDLES_PER_LINE - 1)) || (self->curLine != ((self->handleMapSize / ESIF_HNDLMGR_LINE_SIZE) - 1))) {
		rc = EsifHandleMgr_FindHandleInRegion(self,
			self->curLine,
			(self->handleMapSize / ESIF_HNDLMGR_LINE_SIZE) - 1,
			self->curPos,
			ESIF_HNDLMGR_HANDLES_PER_LINE - 1,
			handlePtr
		);
	}

	/* If not found at end of map, start from beginning */
	if (rc != ESIF_OK) {
		self->instanceCounter++;
		rc = EsifHandleMgr_FindHandleInRegion(self,
			0,
			self->curLine,
			0,
			self->curPos,
			handlePtr
		);
	}
	return rc;
}


static eEsifError EsifHandleMgr_FindHandleInRegion(
	EsifHandleMgrPtr self,
	size_t startLineNum,
	size_t stopLineNum,
	UInt32 startPos,
	UInt32 stopPos,
	esif_handle_t *handlePtr
)
{
	eEsifError rc = ESIF_E_INVALID_HANDLE;
	esif_handle_t handle = 0;
	UInt64 line = 0;
	size_t lineNum = 0;
	UInt32 pos = 0;
	UInt32 lastPos = 0;

	ESIF_ASSERT(self);
	ESIF_ASSERT(handlePtr);


	pos = startPos;
	for (lineNum = startLineNum; lineNum <= stopLineNum; lineNum++) {
		line = self->handleMapPtr[lineNum];

		lastPos = ESIF_HNDLMGR_HANDLES_PER_LINE - 1;
		if (lineNum == stopLineNum) {
			lastPos = stopPos;
		}

		for (; pos <= lastPos; pos++) {
			if (!(line & ((UInt64)1 << pos))) {


				handle = (esif_handle_t)(size_t)((lineNum * ESIF_HNDLMGR_HANDLES_PER_LINE + pos) | (self->instanceCounter << ESIF_HNDLMGR_NUM_HANDLE_BITS));

				if (EsifHandleMgr_IsReservedHandle(handle)) {
					continue;
				}

				self->handleMapPtr[lineNum] |= ((UInt64)1 << pos);
				self->curLine = lineNum;
				self->curPos = pos;
				self->numAvailableHandles--;

				*handlePtr = handle;

				rc = ESIF_OK;
				goto exit;
			}
		}
		pos = 0;
	}
exit:
	return rc;
}


static Bool EsifHandleMgr_IsReservedHandle(esif_handle_t handle)
{
	Bool isReserved = ESIF_FALSE;
	if ((handle == ESIF_HANDLE_DEFAULT) ||
		(handle == ESIF_HANDLE_PRIMARY_PARTICIPANT) ||
		(handle == ESIF_HANDLE_MATCH_ANY_EVENT)	||
		(handle == ESIF_INVALID_HANDLE)) {
		isReserved = ESIF_TRUE;
	}
	return isReserved;
}


static EsifHandleMgrPtr EsifHandleMgr_Create()
{
	EsifHandleMgrPtr self = (EsifHandleMgrPtr)esif_ccb_malloc(sizeof(*self));
	size_t handleMapSize = 0;

	if (self) {
		esif_ccb_lock_init(&self->lock);

		handleMapSize = ESIF_HNDLMGR_MIN_HANDLE_LINES * ESIF_HNDLMGR_LINE_SIZE;

		self->handleMapPtr = (UInt64 *)esif_ccb_malloc(handleMapSize);
		if (NULL == self->handleMapPtr) {
			EsifHandleMgr_Destroy(self);
			self = NULL;
			goto exit;
		}

		self->handleMapSize = handleMapSize;

		self->curLine = 0;
		self->curPos = 0;
		self->instanceCounter = 0;
		self->numAvailableHandles = ESIF_HNDLMGR_MIN_HANDLE_LINES * ESIF_HNDLMGR_HANDLES_PER_LINE;
	}
exit:
	return self;
}

static void EsifHandleMgr_Destroy(EsifHandleMgrPtr self)
{
	if (self) {
		esif_ccb_free(self->handleMapPtr);

		esif_ccb_lock_uninit(&self->lock);
	}
	esif_ccb_free(self);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
