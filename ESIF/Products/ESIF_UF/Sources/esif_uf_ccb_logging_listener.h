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

#pragma once
#include "esif_ccb.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"

static ESIF_INLINE eEsifError EsifLogMgr_LogToDebugger(char *outputString)
{
	OutputDebugStringA(outputString);
	return ESIF_OK;
}

static ESIF_INLINE eEsifError EsifLogMgr_LogToEvent(char *buffer)
{
	report_event_to_event_log(ESIF_TRACELEVEL_INFO, "%s", buffer);
	return ESIF_OK;
}

#else  //NOT ESIF_ATTR_OS_WINDOWS

static ESIF_INLINE eEsifError EsifLogMgr_LogToDebugger(char *outputString)
{
	return ESIF_E_NOT_IMPLEMENTED;
}

static ESIF_INLINE eEsifError EsifLogMgr_LogToEvent(char *buffer)
{
	return ESIF_E_NOT_IMPLEMENTED;
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/