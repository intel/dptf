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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ACTION

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
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData p1 = {0};
	UInt32 val    = 0;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(upPtr);
	UNREFERENCED_PARAMETER(primitivePtr);
	UNREFERENCED_PARAMETER(requestPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	rc = EsifFpcAction_GetParamAsEsifData(fpcActionPtr, 0, &p1);
	if ((ESIF_OK != rc) || (NULL == p1.buf_ptr)) {
		goto exit;
	}

	switch (p1.type) {
	case ESIF_DATA_UINT32:
		val = *(UInt32 *)p1.buf_ptr;

		rc = EsifCopyIntToBufBySize(esif_data_type_sizeof(responsePtr->type),
			responsePtr->buf_ptr,
			(UInt64)val);
		break;

	case ESIF_DATA_STRING:
	case ESIF_DATA_BINARY:
		if (responsePtr->type != p1.type) {
			rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
		}
		else if (responsePtr->buf_len < p1.data_len) {
			rc = ESIF_E_NEED_LARGER_BUFFER;
			responsePtr->data_len = p1.data_len;
		}
		else {
			rc = ESIF_OK;
			esif_ccb_memcpy(responsePtr->buf_ptr, p1.buf_ptr, p1.data_len);
			responsePtr->data_len = p1.data_len;
		}
		break;

	default:
		rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
		break;
	}

exit:
	return rc;
}

static eEsifError ESIF_CALLCONV ActionConstSet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	EsifDataPtr requestPtr
)
{
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(upPtr);
	UNREFERENCED_PARAMETER(primitivePtr);
	UNREFERENCED_PARAMETER(fpcActionPtr);
	UNREFERENCED_PARAMETER(requestPtr);

	//this is a way to force a success on a set
	
	return rc;
}


/*
 *******************************************************************************
 ** Register ACTION with ESIF
 *******************************************************************************
 */
static EsifActIfaceStatic g_const = {
	eIfaceTypeAction,
	ESIF_ACT_IFACE_VER_STATIC,
	sizeof(g_const),
	ESIF_ACTION_CONST,
	ESIF_ACTION_FLAGS_DEFAULT,
	"CONST",
	"User Space Constant",
	ESIF_ACTION_VERSION_DEFAULT,
	NULL,
	NULL,
	ActionConstGet,
	ActionConstSet
};

enum esif_rc EsifActConstInit()
{
	EsifActMgr_RegisterAction((EsifActIfacePtr)&g_const);
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifActConstExit()
{
	EsifActMgr_UnregisterAction((EsifActIfacePtr)&g_const);
	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
