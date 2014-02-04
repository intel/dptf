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

static eEsifError ActionConstGet(
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
	eEsifError rc = ESIF_OK;
	UInt32 val    = 0;

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
	ESIF_ASSERT(ESIF_DATA_UINT32 == p1Ptr->type);

	val = *(UInt32 *)p1Ptr->buf_ptr;


	/*
	 * Const items are kept as 32-bit values; however, the call may be for
	 * a different size depending on type.  Return data as the largest
	 * type the buffer passed in can accept.
	 */
	if (responsePtr->buf_len >= sizeof(UInt64)) {
		responsePtr->data_len = sizeof(UInt64);
		*((UInt64 *)responsePtr->buf_ptr) = (UInt64)val;

	} else if (responsePtr->buf_len >= sizeof(UInt32)) {
		responsePtr->data_len = sizeof(UInt32);
		*((UInt32 *)responsePtr->buf_ptr) = (UInt32)val;

	} else if (responsePtr->buf_len >= sizeof(UInt16)) {
		responsePtr->data_len = sizeof(UInt16);
		*((UInt16 *)responsePtr->buf_ptr) = (UInt16)val;

	} else if (responsePtr->buf_len >= sizeof(UInt8)) {
		responsePtr->data_len = sizeof(UInt8);
		*((UInt8 *)responsePtr->buf_ptr) = (UInt8)val;

	} else {
		/* Should only happen if buffer len is 0 */
		rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
	}

	return rc;
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

static EsifActType g_const = {
	0,
	ESIF_ACTION_CONST,
	{PAD},
	"CONST",
	"User Space Constant",
	"ALL",
	"x1.0.0.1",
	{0},
	IS_KERNEL,
	IS_PLUGIN,
	{PAD},
	ActionConstGet,
	NULL
};

enum esif_rc EsifActConstInit()
{
	if (NULL != g_actMgr.AddActType) {
		g_actMgr.AddActType(&g_actMgr, &g_const);
	}
	return ESIF_OK;
}


void EsifActConstExit()
{
	if (NULL != g_actMgr.RemoveActType) {
		g_actMgr.RemoveActType(&g_actMgr, 0);
	}
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
