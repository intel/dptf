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
#include "esif_uf.h"			/* Upper Framework */
#include "esif_uf_actmgr.h"		/* Action Manager */

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

static eEsifError ESIF_CALLCONV ActionConstGet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData p1 = {0};
	UInt32 val    = 0;

	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(upPtr);
	UNREFERENCED_PARAMETER(primitivePtr);
	UNREFERENCED_PARAMETER(requestPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	rc = EsifActionGetParamAsEsifData(actionPtr, 0, &p1);
	if (ESIF_OK != rc) {
		goto exit;
	}

	ESIF_ASSERT(NULL != p1.buf_ptr);
	ESIF_ASSERT(ESIF_DATA_UINT32 == p1.type);

	val = *(UInt32 *)p1.buf_ptr;

	rc = EsifActionCopyIntToBufBySize(esif_data_type_sizeof(responsePtr->type),
		responsePtr->buf_ptr,
		(UInt64)val);
exit:
	return rc;
}


/*
 *******************************************************************************
 ** Register ACTION with ESIF
 *******************************************************************************
 */
static EsifActType g_const = {
	0,
	ESIF_ACTION_CONST,
	{PAD},
	"CONST",
	"User Space Constant",
	"ALL",
	"x1.0.0.1",
	{0},
	ESIF_ACTION_IS_NOT_KERNEL_ACTION,
	ESIF_ACTION_IS_NOT_PLUGIN,
	{PAD},
	ActionConstGet,
	NULL,
	NULL,
	NULL
};

enum esif_rc EsifActConstInit()
{
	if (NULL != g_actMgr.AddActType) {
		g_actMgr.AddActType(&g_actMgr, &g_const);
	}
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifActConstExit()
{
	if (NULL != g_actMgr.RemoveActType) {
		g_actMgr.RemoveActType(&g_actMgr, 0);
	}
	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
