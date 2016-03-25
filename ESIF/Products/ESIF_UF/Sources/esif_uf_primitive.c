/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_PRIMITIVE

#include "esif_uf.h"		/* Upper Framework           */
#include "esif_pm.h"		/* Upper Participant Manager */
#include "esif_uf_trace.h"
#include "esif_dsp.h"
#include "esif_uf_action.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif


void EsifUfDumpPrimitive(
	EsifDspPtr dspPtr,
	EsifFpcPrimitivePtr primitivePtr
	)
{
	EsifFpcActionPtr fpcActionPtr;
	DataItemPtr dataItemPtr;
	char msg[128];
	char *paramStr = NULL;
	u32 param = 0;
	int i, j;

	/*
	 * This check is placed here so that if tracing isn't active, we don't do all
	 * the computation below; even though nothing will be output...
	 */
	if (!ESIF_TRACEACTIVE(ESIF_TRACEMASK_CURRENT, ESIF_TRACELEVEL_DEBUG)) {
		return;
	}

	if ((NULL == dspPtr) || (NULL == primitivePtr)) {
		goto exit;
	}

	ESIF_TRACE_DEBUG("esif_uf_execute_primitive: Primitive METADATA From DSP:\n"
					   "\tOperation:        : %s(%u)\n"
					   "\tRequest Data Type : %s(%u)\n"
					   "\tResponse Data Type: %s(%u)\n"
					   "\tAction Count      : %u\n"
					   "\tPrimitive Size    : %u\n",
					   esif_primitive_opcode_str((enum esif_primitive_opcode)primitivePtr->operation),
					   primitivePtr->operation,
					   esif_data_type_str((enum esif_data_type)primitivePtr->request_type),
					   primitivePtr->request_type,
					   esif_data_type_str((enum esif_data_type)primitivePtr->result_type),
					   primitivePtr->result_type,
					   primitivePtr->num_actions,
					   primitivePtr->size);

	for (i = 0; i < (int)primitivePtr->num_actions; i++) {
		fpcActionPtr = dspPtr->get_action(dspPtr, primitivePtr, (u8)i);
		ESIF_TRACE_DEBUG("Action[%u]: size %u type %d(%s) is_kern %u "
						   "param_valid %x:%x:%x:%x:%x\n",
						   i, fpcActionPtr->size,
						   fpcActionPtr->type, esif_action_type_str(fpcActionPtr->type),
						   fpcActionPtr->is_kernel,
						   (u32)fpcActionPtr->param_valid[0], (u32)fpcActionPtr->param_valid[1],
						   (u32)fpcActionPtr->param_valid[2], (u32)fpcActionPtr->param_valid[3],
						   (u32)fpcActionPtr->param_valid[4]);

		for (j = 0; j < 5; j++) {
			dataItemPtr = EsifFpcAction_GetParam(fpcActionPtr, (const UInt8)j);
			if (NULL == dataItemPtr) {
				continue;
			}
			esif_ccb_sprintf(128, msg, "\tparam[%u]: type %u(%s) len %u data ",
							 j, dataItemPtr->data_type,
							 esif_data_type_str(dataItemPtr->data_type),
							 dataItemPtr->data_length_in_bytes);
			switch (dataItemPtr->data_type) {
			case 1:	// String
				paramStr = (char *)&dataItemPtr->data;
				esif_ccb_sprintf_concat(128, msg, "%s\n", paramStr);
				break;

			case 4:	// 32 Bit Integer
				param = *(u32 *)&dataItemPtr->data;
				esif_ccb_sprintf_concat(128, msg, "0x%08x\n", param);
				break;

			default:
				break;
			}
			ESIF_TRACE_DEBUG("%s", msg);
		}
	}
exit:
	return;
}


/*
** ===========================================================================
** PUBLIC
** ===========================================================================
*/


/*
 * Execute Primitive
 * NOTE: This version should only be called by functions external to the
 * participant, as it will require acquisition of locks for the
 * Participant Manager and participant. EsifUp_ExecutePrimitive must be called
 * from within the participant/domain when locks are already held or when
 * executing a primitive from within the context of an action, for
 * example within the "delegate" or "sysfs" actions.
 */
eEsifError EsifExecutePrimitive(
	const UInt8 participantId,
	const UInt32 primitiveId,
	const EsifString domainStr,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr upPtr   = NULL;
	EsifPrimitiveTuple tuple = {0};
	UInt16 domain = domain_str_to_short(domainStr);

	ESIF_TRACE_DEBUG("\n\n"
		"Primitive Request:\n"
		"  Participant ID       : %u\n"
		"  Primitive            : %s(%u)\n"
		"  Domain               : %s\n"
		"  Instance             : %u\n"
		"  Request              : %p\n"
		"  Response             : %p\n",
		participantId,
		esif_primitive_str((enum esif_primitive_type)primitiveId), primitiveId,
		domainStr,
		instance,
		requestPtr,
		responsePtr);

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (NULL == upPtr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}

	tuple.id       = (u16)primitiveId;
	tuple.domain   = domain;
	tuple.instance = instance;
	
	rc = EsifUp_ExecutePrimitive(upPtr, &tuple, requestPtr, responsePtr);
exit:
	ESIF_TRACE_DEBUG("Primitive result = %s\n", esif_rc_str(rc));
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


Bool EsifPrimitiveVerifyOpcode(
	const UInt8 participantId,
	const UInt32 primitiveId,
	const EsifString domain_str,
	const UInt8 instance,
	enum esif_primitive_opcode opcode)
{
	Bool rc = ESIF_TRUE; // return TRUE by default so caller will only report invalid opcode if primitive is found
	EsifDspPtr dspPtr = NULL;
	EsifUpPtr upPtr = NULL;
	EsifFpcPrimitivePtr primitivePtr = NULL;
	EsifPrimitiveTuple tuple = { 0 };
	UInt16 domain = domain_str_to_short(domain_str);

	upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);
	if (NULL == upPtr) {
		goto exit;
	}

	dspPtr = EsifUp_GetDsp(upPtr);
	if (NULL == dspPtr) {
		goto exit;
	}

	tuple.id = (u16)primitiveId;
	tuple.domain = domain;
	tuple.instance = instance;

	primitivePtr = dspPtr->get_primitive(dspPtr, &tuple);
	if (NULL == primitivePtr) {
		goto exit;
	}

	if (opcode != (enum esif_primitive_opcode) primitivePtr->operation) {
		rc = ESIF_FALSE;
	}

exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}
	return rc;
}


char *esif_primitive_domain_str(
	u16 domain,
	char *str,
	u8 str_len
	)
{
	u8 *ptr = (u8 *)&domain;
	esif_ccb_sprintf(str_len, str, "%c%c", *ptr, *(ptr + 1));
	return str;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

