/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

/******************************************************************************
**
** This file contains the implementation of  a table-driven initialization/
** de-initialization solution.  The solution is based on two loops:
** one for initialization and one for de-initialization which executes the
** functions specified in a table.  During initialization, the table is
** processed from the first entry to the last; during destruction the table is
** executed in reverse order.  The solution allows for the use of flags to
** specify special handling for a given entry in the table; for example
** whether to ignore a error in processing that specific call.
**
** Note:  The last entry in the table is marked by the both init and exit
** function pointers being NULL.
**
** Note:  The solution requires the definition of a table named
** g_lifecycleTable containing an array of LifecycleTableEntry entries and
** an int named g_numLifecycleTableEntries specifying the number of entries
** in the table.
**
** Note:  This implemenation serializes the initialization and
** de-initialization process such that de-initialization may not take place
** until initialization completes.
**
******************************************************************************/

#include "esif_ccb.h"
#include "ipf_lifecycle.h"
#include "ipf_trace.h"


esif_error_t LifecycleMgr_Init(LifecycleTableEntry *tablePtr)
{
	esif_error_t rc = ESIF_OK;
	LifecycleTableEntry *curEntryPtr = NULL;
	size_t entryCount = 0;
	size_t index = 0;

	//
	// Initialization with an empty table is allowed and does
	// not result in an error.
	//
	if (NULL == tablePtr) {
		goto exit;
	}

	//
	// Determine table size
	//
	curEntryPtr = tablePtr;
	while (curEntryPtr->initFunc || curEntryPtr->exitFunc) {
		entryCount++;
		curEntryPtr++;
	}
	entryCount++; /* Add one for end marker entry */


	curEntryPtr = tablePtr;
	for (index = 0; index < entryCount; index++, curEntryPtr++) {

		IPF_TRACE_INFO("[INIT] Performing initialization Step %zd\n", index);

		if (curEntryPtr->initFunc == NULL) {
			IPF_TRACE_DEBUG("[INIT] initialization Step %zd complete\n", index);
			continue;
		}

		rc = curEntryPtr->initFunc();
		if ((rc != ESIF_OK) && !(curEntryPtr->flags & LIFECYCLE_FLAG_IGNORE_ERROR)) {
			IPF_TRACE_DEBUG("[INIT] Error in initialization Step %zd; rc = %d\n", index, rc);
			break;
		}
		rc = ESIF_OK;

		IPF_TRACE_INFO("[INIT] Initialization Step %zd complete\n", index);
	}
exit:
	return rc;
}


void LifecycleMgr_Exit(LifecycleTableEntry *tablePtr)
{
	size_t index = 0;
	size_t entryCount = 0;
	LifecycleTableEntry *curEntryPtr = NULL;

	if (!tablePtr) {
		goto exit;
	}
	//
	// Determine table size
	//
	curEntryPtr = tablePtr;
	while (curEntryPtr->initFunc || curEntryPtr->exitFunc) {
		entryCount++;
		curEntryPtr++;
	}
	entryCount++; /* Add one for end marker entry */

	index = entryCount - 1;
	curEntryPtr = tablePtr + index;

	for (; curEntryPtr >= tablePtr; index--, curEntryPtr--) {

		IPF_TRACE_INFO("[EXIT] Performing exit Step %d\n", (UInt32)index);

		if (curEntryPtr->exitFunc != NULL) {
			curEntryPtr->exitFunc();
		}

		IPF_TRACE_INFO("[EXIT] Exit Step %d complete\n", (UInt32)index);
	}
exit:
	return;
}






