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
#include "esif_uf.h"			/* Upper Framework */
#include "esif_uf_actmgr.h"		/* Action Manager */
#include "esif_uf_cfgmgr.h"		/* Configuration */

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/*
** Handle ESIF Action Request
*/

static eEsifError ActionConfigGet(
	const void *actionHandle,
	const EsifString devicePathPtr,
	const EsifDataPtr p1Ptr,
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr,
	const EsifDataPtr p4Ptr,
	const EsifDataPtr p5Ptr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(devicePathPtr);
	UNREFERENCED_PARAMETER(p1Ptr);
	UNREFERENCED_PARAMETER(p2Ptr);
	UNREFERENCED_PARAMETER(p3Ptr);
	UNREFERENCED_PARAMETER(p4Ptr);
	UNREFERENCED_PARAMETER(p5Ptr);
	UNREFERENCED_PARAMETER(requestPtr);
	UNREFERENCED_PARAMETER(responsePtr);

	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr->buf_ptr);
	ESIF_ASSERT(ESIF_DATA_UINT32 == requestPtr->type);

	ESIF_ASSERT(NULL != p1Ptr);
	ESIF_ASSERT(NULL != p1Ptr->buf_ptr);
	ESIF_ASSERT(ESIF_DATA_STRING == p1Ptr->type);

	return EsifConfigGet(p1Ptr, p2Ptr, responsePtr);
}


static eEsifError ActionConfigSet(
	const void *actionHandle,
	const EsifString devicePathPtr,
	const EsifDataPtr p1Ptr,
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr,
	const EsifDataPtr p4Ptr,
	const EsifDataPtr p5Ptr,
	EsifDataPtr requestPtr
	)
{
	esif_flags_t flags = ESIF_SERVICE_CONFIG_PERSIST;
	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(devicePathPtr);
	UNREFERENCED_PARAMETER(p1Ptr);
	UNREFERENCED_PARAMETER(p2Ptr);
	UNREFERENCED_PARAMETER(p3Ptr);
	UNREFERENCED_PARAMETER(p4Ptr);
	UNREFERENCED_PARAMETER(p5Ptr);
	UNREFERENCED_PARAMETER(requestPtr);

	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr->buf_ptr);
	ESIF_ASSERT(ESIF_DATA_UINT32 == requestPtr->type);

	ESIF_ASSERT(NULL != p1Ptr);
	ESIF_ASSERT(NULL != p1Ptr->buf_ptr);
	ESIF_ASSERT(ESIF_DATA_STRING == p1Ptr->type);

	/* 255 To delete temperatuer based key */
	if (ESIF_DATA_TEMPERATURE == requestPtr->type &&
		255 == *(UInt32 *)requestPtr->buf_ptr) {
		flags |= ESIF_SERVICE_CONFIG_DELETE;
	}

	return EsifConfigSet(p1Ptr, p2Ptr, flags, requestPtr);
}


/*
 *******************************************************************************
 ** Register ACTION with ESIF
 *******************************************************************************
 */
extern EsifActMgr g_actMgr;

#define PAD 0
#define IS_KERNEL 0
#define IS_PLUGIN 0

static EsifActType g_config = {
	0,
	ESIF_ACTION_CONFIG,
	{PAD},
	"CONFIG",
	"Configuration Data Management",
	"ALL",
	"x1.0.0.1",
	{0},
	IS_KERNEL,
	IS_PLUGIN,
	{PAD},
	ActionConfigGet,
	ActionConfigSet
};

enum esif_rc EsifActConfigInit()
{
	if (NULL != g_actMgr.AddActType) {
		g_actMgr.AddActType(&g_actMgr, &g_config);
	}
	return ESIF_OK;
}


void EsifActConfigExit()
{
	if (NULL != g_actMgr.RemoveActType) {
		g_actMgr.RemoveActType(&g_actMgr, 0);
	}
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
