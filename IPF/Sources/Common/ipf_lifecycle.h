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

/******************************************************************************
**
** This file contains the declarations necessary for creation of a table-driven
** initialization/de-initialization solution.  The solution is based on two
** loops: one for initialization and one for de-initialization which executes
** the functions specified in a table.  During initialization, the table is
** processed from the first entry to the last; during destruction the table is
** executed in reverse order.  The solution allows for the use of flags to
** specify special handling for a given entry in the table; for example whether
** to ignore a error in processing that specific call.
**
** Note:  The last entry in the table is marked by the both init and exit
** function pointers being NULL.
**
******************************************************************************/

#pragma once

#include "esif_ccb_rc.h"
#include "esif_sdk.h"

// Lifecycle table entry flags:
#define LIFECYCLE_FLAG_NONE					0x00000000  // No flags
#define	LIFECYCLE_FLAG_IGNORE_ERROR			0x00000001  // If error, continue


typedef esif_error_t(*LifeCycleInitFunc)();
typedef void(*LifecycleExitFunc)();

/******************************************************************************
**
** Note:  The last entry in the table is marked by the both init and exit
** function pointers being NULL.
**
******************************************************************************/
typedef struct LifecycleTableEntry_s {
	LifeCycleInitFunc initFunc;
	LifecycleExitFunc exitFunc;
	esif_flags_t flags;
} LifecycleTableEntry;



#ifdef __cplusplus
extern "C" {
#endif

	esif_error_t LifecycleMgr_Init(LifecycleTableEntry *tablePtr);
	void LifecycleMgr_Exit(LifecycleTableEntry *tablePtr);

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/