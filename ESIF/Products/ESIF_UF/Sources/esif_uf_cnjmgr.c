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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_CONJURE

#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_cnjmgr.h"	/* Application Manager */

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif


/* Friends */
EsifCnjMgr g_cnjMgr = {0};

eEsifError EsifCnjMgrInit()
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_cnjMgr.fLock);

	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifCnjMgrExit()
{
	u8 i = 0;
	EsifCnjPtr cnjrPtr = NULL;

	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_read_lock(&g_cnjMgr.fLock);
	for (i = 0; i < ESIF_MAX_CONJURES; i++) {
		cnjrPtr = &g_cnjMgr.fEnrtries[i];
		if (NULL == cnjrPtr->fLibNamePtr) {
			continue;
		}
		if (NULL != cnjrPtr->fInterface.fConjureDestroyFuncPtr) {
			cnjrPtr->fInterface.fConjureDestroyFuncPtr(cnjrPtr->fHandle);
		}
		esif_ccb_free(cnjrPtr->fLibNamePtr);
		esif_ccb_library_unload(cnjrPtr->fLibHandle);
		esif_ccb_memset(cnjrPtr, 0, sizeof(*cnjrPtr));
	}
	esif_ccb_read_unlock(&g_cnjMgr.fLock);
	esif_ccb_lock_uninit(&g_cnjMgr.fLock);

	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/