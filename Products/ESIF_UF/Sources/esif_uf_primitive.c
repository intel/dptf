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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_PRIMITIVE

#include "esif_uf.h"		/* Upper Framework           */
#include "esif_uf_actmgr.h"	/* Action Manager            */
#include "esif_ipc.h"		/* IPC Abstraction           */
#include "esif_dsp.h"		/* Device Support Package    */
#include "esif_pm.h"		/* Upper Participant Manager */
#include "esif_uf_ipc.h"	/* IPC                       */
#include "esif_temp.h"		/* Temperature               */
#include "esif_power.h"		/* Power                     */
#include "esif_uf_appmgr.h"	/* Application Manager       */
#include "esif_debug.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define MAXPARAMLEN     65536

/* Friend */
extern EsifActMgr g_actMgr;	/* Action Manager */

extern u32 g_uf_xform;


/*
** ===========================================================================
** PRIVATE
** ===========================================================================
*/

/* Temperature Transform */
static ESIF_INLINE enum esif_rc esif_xform_temp(
	const enum esif_temperature_type type,
	esif_temp_t *temp_ptr,
	const enum esif_action_type action,
	const EsifDspPtr dsp_ptr,
	const enum esif_primitive_opcode opcode
	)
{
	enum esif_rc rc = ESIF_OK;
	enum esif_temperature_type temp_in_type  = type;
	enum esif_temperature_type temp_out_type = type;
	struct esif_fpc_algorithm *algo_ptr = NULL;
	esif_temp_t temp_in;
	esif_temp_t temp_out;

	UNREFERENCED_PARAMETER(temp_in);

	if ((temp_ptr == NULL) || (dsp_ptr == NULL)) {
		return ESIF_E_PARAMETER_IS_NULL;
	}
	temp_in  = *temp_ptr;
	temp_out = *temp_ptr;

	algo_ptr = dsp_ptr->get_algorithm(dsp_ptr, action);
	if (algo_ptr == NULL) {
		return ESIF_E_NEED_ALGORITHM;
	}

	switch (algo_ptr->temp_xform) {
	case ESIF_ALGORITHM_TYPE_TEMP_MILLIC:

		/* Convert Temp before/after LAL action
		 *    For Get: From reading returned by LAL in millic
		 *             To   normalized temp (C) back to user response buffer
		 *    For Set: From user request buffer in Celsius
		 *             To   millic passing to LAL
		 */
		ESIF_TRACE_DYN_TEMP("%s: using algorithm MilliC (%s), for LAL temp\n",
							ESIF_FUNC, esif_algorithm_type_str(algo_ptr->temp_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_in_type  = ESIF_TEMP_MILLIC;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);	// Normalized from Kelvin
		} else {/* ESIF_PRIMITIVE_OP_SET */
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_MILLIC;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);	// Normalized to Kelvin
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_DECIK:

		/* Convert Temp before/after ACPI action
		 *    For Get: From reading returned by APCI device driver in Kelvin
		 *             To   normalized temp (C) back to user response buffer
		 *    For Set: From user request buffer in Kelvin or Celsius
		 *             To   Kelvin passing to ACPI device driver to set
		 */
		ESIF_TRACE_DYN_TEMP("%s: using algorithm DeciK (%s), for ACPI temp\n",
							ESIF_FUNC, esif_algorithm_type_str(algo_ptr->temp_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_in_type  = ESIF_TEMP_DECIK;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);	// Normalized from Kelvin
		} else {/* ESIF_PRIMITIVE_OP_SET */
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_DECIK;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);	// Normalized to Kelvin
		}
		break;

	case ESIF_ALGORITHM_TYPE_TEMP_TJMAX_CORE:
	{
		u32 tjmax = dsp_ptr->get_temp_tc1(dsp_ptr, action);
		ESIF_TRACE_DYN_TEMP("%s: using algorithm Tjmax %d %s, for MSR temp\n",
							ESIF_FUNC, tjmax, esif_algorithm_type_str(algo_ptr->temp_xform));

		/* Tjmax must be provided by DSP */
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_out      = tjmax - temp_out;
			temp_in_type  = ESIF_TEMP_C;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);
		} else {
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_C;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);
			temp_out = tjmax - temp_out;
		}
		break;
	}

	case ESIF_ALGORITHM_TYPE_TEMP_PCH_CORE:
	{
		ESIF_TRACE_DYN_TEMP("%s: using algorithm %s, "
							"for PCH MMIO temp\n",
							ESIF_FUNC, esif_algorithm_type_str(algo_ptr->temp_xform));

		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_out      = (temp_out / 2) - 50;
			temp_in_type  = ESIF_TEMP_C;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);
		} else {
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_C;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);
			temp_out = (temp_out / 2) - 50;
		}
		break;
	}

	case ESIF_ALGORITHM_TYPE_TEMP_NONE:
		ESIF_TRACE_DYN_TEMP("%s: using algorithm none (%s), for Code and Konst temp\n",
							ESIF_FUNC, esif_algorithm_type_str(algo_ptr->temp_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			temp_in_type  = ESIF_TEMP_C;
			temp_out_type = type;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);
		} else {/* ESIF_PRIMITIVE_OP_SET */
			temp_in_type  = type;
			temp_out_type = ESIF_TEMP_C;
			esif_convert_temp(temp_in_type, temp_out_type, &temp_out);
		}
		break;

	default:
		ESIF_TRACE_DYN_POWER("%s: Unknown algorithm (%s) to xform temp\n",
							 ESIF_FUNC, esif_algorithm_type_str(algo_ptr->temp_xform));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_TEMP("%s: IN  temp %u %s(%d)\n", ESIF_FUNC, temp_in,
						esif_temperature_type_desc(temp_in_type), temp_in_type);

	ESIF_TRACE_DYN_TEMP("%s: OUT temp %u %s(%d)\n", ESIF_FUNC, temp_out,
						esif_temperature_type_desc(temp_out_type), temp_out_type);

	*temp_ptr = temp_out;
	return rc;
}


/* Power Transform */
static ESIF_INLINE enum esif_rc esif_xform_power(
	const enum esif_power_unit_type type,
	esif_power_t *power_ptr,
	const enum esif_action_type action,
	const EsifDspPtr dsp_ptr,
	const enum esif_primitive_opcode opcode
	)
{
	enum esif_power_unit_type power_in_type  = type;
	enum esif_power_unit_type power_out_type = type;
	enum esif_rc rc = ESIF_OK;
	struct esif_fpc_algorithm *algo_ptr = NULL;
	esif_power_t power_in;
	esif_power_t power_out;

	if ((power_ptr == NULL) || (dsp_ptr == NULL)) {
		return ESIF_E_PARAMETER_IS_NULL;
	}
	power_in  = *power_ptr;
	power_out = *power_ptr;

	algo_ptr  = dsp_ptr->get_algorithm(dsp_ptr, action);
	if (algo_ptr == NULL) {
		return ESIF_E_NEED_ALGORITHM;
	}

	switch (algo_ptr->power_xform) {
	case ESIF_ALGORITHM_TYPE_POWER_DECIW:
		ESIF_TRACE_DYN_POWER("%s: using algorithm DeciW (%s), for ACPI power\n",
							 ESIF_FUNC, esif_algorithm_type_str(algo_ptr->power_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			/* Tenths Of A Watt To Milli Watts */
			power_in_type  = ESIF_POWER_DECIW;
			power_out_type = type;
			// Normalized from DeciW
			esif_convert_power(power_in_type, power_out_type, &power_out);
		} else {
			/* Milli Watts To Tenths Of A Watt */
			power_in_type  = type;
			power_out_type = ESIF_POWER_DECIW;
			// Normalized to DeciW
			esif_convert_power(power_in_type, power_out_type, &power_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_MILLIW:
		ESIF_TRACE_DYN_POWER("%s: using algorithm MillW (%s), for Code and Konst power\n",
							 ESIF_FUNC, esif_algorithm_type_str(algo_ptr->power_xform));
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			power_in_type  = ESIF_POWER_MILLIW;
			power_out_type = type;
			esif_convert_power(power_in_type, power_out_type, &power_out);
		} else {
			/* Milli Watts To Tenths Of A Watt */
			power_in_type  = type;
			power_out_type = ESIF_POWER_MILLIW;
			esif_convert_power(power_in_type, power_out_type, &power_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_UNIT_ATOM:
		ESIF_TRACE_DYN_POWER("%s: using algorithm %s, for hardware power\n",
							 ESIF_FUNC, esif_algorithm_type_str(algo_ptr->power_xform));
		/* Hardware */
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			power_in_type  = ESIF_POWER_UNIT_ATOM;
			power_out_type = type;
			// Normalized from hardware
			esif_convert_power(power_in_type, power_out_type, &power_out);
		} else {
			power_in_type  = type;
			power_out_type = ESIF_POWER_UNIT_ATOM;
			// Normalized to hardware
			esif_convert_power(power_in_type, power_out_type, &power_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_UNIT_CORE:
		ESIF_TRACE_DYN_POWER("%s: using algorithm %s, for hardware power\n",
							 ESIF_FUNC, esif_algorithm_type_str(algo_ptr->power_xform));
		/* Hardware */
		if (opcode == ESIF_PRIMITIVE_OP_GET) {
			power_in_type  = ESIF_POWER_UNIT_CORE;
			power_out_type = type;
			// Normalized from hardware
			esif_convert_power(power_in_type, power_out_type, &power_out);
		} else {
			power_in_type  = type;
			power_out_type = ESIF_POWER_UNIT_CORE;
			// Normalized to hardware
			esif_convert_power(power_in_type, power_out_type, &power_out);
		}
		break;

	case ESIF_ALGORITHM_TYPE_POWER_NONE:
		/* No algorithm specified, do not perform any xform */
		ESIF_TRACE_DYN_POWER("%s: using algorithm NONE (%s), no xform performed\n",
							 ESIF_FUNC, esif_algorithm_type_str(algo_ptr->power_xform));
		break;

	default:
		ESIF_TRACE_DYN_POWER("%s: Unknown algorithm (%s) to xform power\n",
							 ESIF_FUNC, esif_algorithm_type_str(algo_ptr->power_xform));
		rc = ESIF_E_UNSUPPORTED_ALGORITHM;
	}

	ESIF_TRACE_DYN_POWER("%s: IN  power %u %s(%d)\n", ESIF_FUNC, power_in,
						 esif_power_unit_desc(power_in_type), power_in_type);

	ESIF_TRACE_DYN_POWER("%s: OUT power %u %s(%d)\n", ESIF_FUNC, power_out,
						 esif_power_unit_desc(power_out_type), power_out_type);

	*power_ptr = power_out;
	return rc;
}


/* Need to move to header POC.  Also don't forget to free returned
** memory JDH
*/
char *esif_str_replace(char *orig, char *rep, char *with);


/* Primitive User Get */
static eEsifError PrimitiveActionUFGet(
	const UInt8 participantId,
	eEsifActionType type,
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
	EsifActTypePtr actiontype_ptr = g_actMgr.GetActType(&g_actMgr, type);

	EsifUpPtr up_ptr = EsifUpManagerGetAvailableParticipantByInstance(participantId);

	/*  Participant Check */
	if (NULL == up_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_DEBUG("%s: Participant For Type %d NOT FOUND\n", ESIF_FUNC, participantId);
		goto exit;
	}

	/* Find Action From Action Type LIST */
	if (NULL == actiontype_ptr) {
		rc = ESIF_E_UNSUPPORTED_ACTION_TYPE;
		ESIF_TRACE_DEBUG("%s: Action For Type %d NOT FOUND Skipping...\n", ESIF_FUNC, type);
		goto exit;
	}

	/* Validate Action */
	if (ESIF_TRUE == actiontype_ptr->fIsKernel || NULL == actiontype_ptr->fGetFuncPtr) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	/* Have Action So Execute */
	ESIF_TRACE_DEBUG("%s: Have Action %s(%d)\n", ESIF_FUNC,
						  esif_action_type_str(type), type);

	rc = actiontype_ptr->fGetFuncPtr(actiontype_ptr->fHandle,
									 up_ptr->fMetadata.fDevicePath,
									 p1Ptr,
									 p2Ptr,
									 p3Ptr,
									 p4Ptr,
									 p5Ptr,
									 requestPtr,
									 responsePtr);
	ESIF_TRACE_DEBUG("%s: USER rc %s, Buffer Len %d, Data Len %d\n",
						  ESIF_FUNC, esif_rc_str(rc), responsePtr->buf_len, responsePtr->data_len);
exit:
	return rc;
}


/* Primitive User Set */
static eEsifError PrimitiveActionUFSet(
	const UInt8 participantId,
	eEsifActionType type,
	const EsifDataPtr p1Ptr,
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr,
	const EsifDataPtr p4Ptr,
	const EsifDataPtr p5Ptr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc    = ESIF_OK;
	EsifActTypePtr actiontype_ptr = g_actMgr.GetActType(&g_actMgr, type);
	EsifUpPtr up_ptr = EsifUpManagerGetAvailableParticipantByInstance(participantId);

	/* Participant Check */
	if (NULL == up_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_ERROR("%s: Participant For Type %d NOT FOUND\n", ESIF_FUNC, participantId);
		goto exit;
	}

	/* Find Action From Action Type LIST */
	if (NULL == actiontype_ptr) {
		rc = ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;
		ESIF_TRACE_WARN("%s: Action For Type %d NOT FOUND Skipping...\n", ESIF_FUNC, type);
		goto exit;
	}

	/* Validate Action */
	if (ESIF_TRUE == actiontype_ptr->fIsKernel || NULL == actiontype_ptr->fSetFuncPtr) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	/* Have Action So Execute */
	ESIF_TRACE_DEBUG("%s: Have Action %s(%d)\n", ESIF_FUNC,
						  esif_action_type_str(type), type);
	ESIF_TRACE_DEBUG("%s: USER rc %s, Buffer Len %d, Data Len %d\n",
						  ESIF_FUNC, esif_rc_str(rc), requestPtr->buf_len, requestPtr->data_len);

	rc = actiontype_ptr->fSetFuncPtr(actiontype_ptr->fHandle,
									 up_ptr->fMetadata.fDevicePath,
									 p1Ptr,
									 p2Ptr,
									 p3Ptr,
									 p4Ptr,
									 p5Ptr,
									 requestPtr);
	ESIF_TRACE_DEBUG("%s: USER rc %s, Buffer Len %d, Data Len %d\n",
						  ESIF_FUNC, esif_rc_str(rc), requestPtr->buf_len, requestPtr->data_len);
exit:
	return rc;
}


/* IPC Primitve Get */
static eEsifError PrimitiveActionLFGet(
	const UInt8 participantId,
	const UInt16 kernAction,
	const EsifPrimitiveTuplePtr tuplePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifIpcPtr ipc_ptr = NULL;
	EsifIpcPrimitivePtr primitive_ptr = NULL;
	u8 *addr = NULL;

	ESIF_TRACE_DEBUG("%s: Send To LOWER_FRAMEWORK/KERNEL\n", ESIF_FUNC);

	if ((NULL == requestPtr) || (NULL == responsePtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ipc_ptr = esif_ipc_alloc_primitive(&primitive_ptr, (responsePtr->buf_len + requestPtr->buf_len));
	if (NULL == ipc_ptr || NULL == primitive_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	primitive_ptr->id              = (u32)tuplePtr->id;
	primitive_ptr->domain          = tuplePtr->domain;
	primitive_ptr->instance        = (u8)tuplePtr->instance;
	primitive_ptr->src_id          = ESIF_INSTANCE_UF;
	primitive_ptr->dst_id          = participantId;
	primitive_ptr->kern_action     = kernAction;
	primitive_ptr->action_type     = (enum esif_action_type)actionPtr->type;

	primitive_ptr->rsp_data_type   = responsePtr->type;
	primitive_ptr->rsp_data_offset = 0;
	primitive_ptr->rsp_data_len    = responsePtr->buf_len;

	if (requestPtr->buf_len == 0) {
		primitive_ptr->req_data_type   = ESIF_DATA_VOID;
		primitive_ptr->req_data_offset = 0;
		primitive_ptr->req_data_len    = 0;
	} else {
		primitive_ptr->req_data_type   = requestPtr->type;
		primitive_ptr->req_data_offset = responsePtr->buf_len;	// Data Format: {{rsp_data}, {req_data}}
		primitive_ptr->req_data_len    = requestPtr->buf_len;
		addr = (u8 *)((u8 *)(primitive_ptr + 1) + (responsePtr->buf_len));
		esif_ccb_memcpy(addr, requestPtr->buf_ptr, requestPtr->buf_len);
	}


	rc = ipc_execute(ipc_ptr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	responsePtr->data_len = (u16)primitive_ptr->rsp_data_len;
	ESIF_TRACE_DEBUG("%s: IPC rc %s, Primitive rc %s, Buffer Len %d, Data Len %d\n",
						  ESIF_FUNC, esif_rc_str(ipc_ptr->return_code), esif_rc_str(primitive_ptr->return_code),
						  responsePtr->buf_len, responsePtr->data_len);

	if (ESIF_OK != ipc_ptr->return_code) {
		rc = ipc_ptr->return_code;
		goto exit;
	}

	if (ESIF_OK != primitive_ptr->return_code) {
		rc = primitive_ptr->return_code;
		goto exit;
	}

	// Assign Data
	esif_ccb_memcpy(responsePtr->buf_ptr, (primitive_ptr + 1), responsePtr->data_len);
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return rc;
}


/* IPC Primitve Set */
static eEsifError PrimitiveActionLFSet(
	const UInt8 participantId,
	const UInt16 kernAction,
	const EsifPrimitiveTuplePtr tuplePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifIpcPtr ipc_ptr = NULL;
	EsifIpcPrimitivePtr primitive_ptr = NULL;

	ESIF_TRACE_DEBUG("%s: Send To LOWER_FRAMEWORK/KERNEL\n", ESIF_FUNC);

	ipc_ptr = esif_ipc_alloc_primitive(&primitive_ptr, requestPtr->buf_len);
	if (NULL == ipc_ptr || NULL == primitive_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	primitive_ptr->id              = tuplePtr->id;
	primitive_ptr->domain          = tuplePtr->domain;
	primitive_ptr->instance        = (u8)tuplePtr->instance;
	primitive_ptr->src_id          = ESIF_INSTANCE_UF;
	primitive_ptr->dst_id          = participantId;
	primitive_ptr->kern_action     = kernAction;
	primitive_ptr->req_data_type   = requestPtr->type;
	primitive_ptr->action_type     = (eEsifActionType)actionPtr->type;
	primitive_ptr->req_data_offset = 0;
	primitive_ptr->req_data_len    = requestPtr->buf_len;
	primitive_ptr->rsp_data_type   = responsePtr->type;
	primitive_ptr->rsp_data_offset = 0;
	primitive_ptr->rsp_data_len    = responsePtr->buf_len;

	esif_ccb_memcpy((primitive_ptr + 1), requestPtr->buf_ptr, requestPtr->buf_len);

	rc = ipc_execute(ipc_ptr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	if (ESIF_OK != ipc_ptr->return_code) {
		rc = ipc_ptr->return_code;
		goto exit;
	}

	if (ESIF_OK != primitive_ptr->return_code) {
		rc = primitive_ptr->return_code;
		goto exit;
	}
exit:
	if (NULL != ipc_ptr) {
		esif_ipc_free(ipc_ptr);
	}
	return rc;
}


typedef struct _t_data_item {
	UInt8   data_type;
	UInt16  data_length_in_bytes;
	UInt8   data[1];
} DataItem, *DataItemPtr, **DataItemPtrLocation;


static DataItemPtr get_action_param(
	const EsifFpcActionPtr actionPtr,
	const UInt8 param_number
	)
{
	UInt8 *address_of_data_item = NULL;

	if (param_number >= NUMBER_OF_PARAMETERS_FOR_AN_ACTION) {
		return NULL;
	}

	if (actionPtr->param_valid[param_number] == 0) {
		return NULL;
	}

	address_of_data_item = (UInt8 *)actionPtr + actionPtr->param_offset[param_number];
	return (DataItemPtr)address_of_data_item;
}


void esif_uf_dump_primitive(
	struct esif_up_dsp *dsp_ptr,
	struct esif_fpc_primitive *primitive_ptr
	)
{
	struct esif_fpc_action *action_ptr;
	DataItemPtr data_item_ptr;
	char msg[128];
	char *param_str = NULL;
	u32 param = 0;
	int i, j;


	if (!ESIF_TRACEACTIVE(ESIF_TRACEMODULE_PRIMITIVE, ESIF_TRACELEVEL_DEBUG)) {
		return;
	}


	ESIF_TRACE_DEBUG("esif_uf_execute_primitive: Primitive METADATA From DSP:\n"
					   "\tOperation:        : %s(%u)\n"
					   "\tRequest Data Type : %s(%u)\n"
					   "\tResponse Data Type: %s(%u)\n"
					   "\tAction Count      : %u\n"
					   "\tPrimitive Size    : %u\n",
					   esif_primitive_opcode_str((enum esif_primitive_opcode)primitive_ptr->operation),
					   primitive_ptr->operation,
					   esif_data_type_str((enum esif_data_type)primitive_ptr->request_type),
					   primitive_ptr->request_type,
					   esif_data_type_str((enum esif_data_type)primitive_ptr->result_type),
					   primitive_ptr->result_type,
					   primitive_ptr->num_actions,
					   primitive_ptr->size);

	for (i = 0; i < (int)primitive_ptr->num_actions; i++) {
		action_ptr = dsp_ptr->get_action(dsp_ptr, primitive_ptr, (u8)i);
		ESIF_TRACE_DEBUG("Action[%u]: size %u type %d(%s) is_kern %u "
						   "param_valid %x:%x:%x:%x:%x\n",
						   i, action_ptr->size,
						   action_ptr->type, esif_action_type_str(action_ptr->type),
						   action_ptr->is_kernel,
						   (u32)action_ptr->param_valid[0], (u32)action_ptr->param_valid[1],
						   (u32)action_ptr->param_valid[2], (u32)action_ptr->param_valid[3],
						   (u32)action_ptr->param_valid[4]);

		for (j = 0; j < 5; j++) {
			data_item_ptr = get_action_param(action_ptr, (const UInt8)j);
			if (NULL == data_item_ptr) {
				continue;
			}
			esif_ccb_sprintf(128, msg, "\tparam[%u]: type %u(%s) len %u data ",
							 j, data_item_ptr->data_type,
							 esif_data_type_str(data_item_ptr->data_type),
							 data_item_ptr->data_length_in_bytes);
			switch (data_item_ptr->data_type) {
			case 1:	// String
				param_str = (char *)&data_item_ptr->data;
				esif_ccb_sprintf(128, msg, "%s%s\n", msg, param_str);
				break;

			case 4:	// 32 Bit Integer
				param = *(u32 *)&data_item_ptr->data;
				esif_ccb_sprintf(128, msg, "%s0x%08x\n", msg, param);
				break;

			default:
				break;
			}
			ESIF_TRACE_DEBUG("%s", msg);
		}
	}
}


/*
** ===========================================================================
** PUBLIC
** ===========================================================================
*/

eEsifError EsifExecutePrimitive(
	const UInt8 participantId,
	const UInt32 primitiveId,
	const EsifString domain_str,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	struct esif_fpc_action *action_ptr = NULL;
	struct esif_fpc_algorithm *algo_ptr;
	EsifDspPtr dsp_ptr = NULL;
	EsifUpPtr up_ptr   = NULL;
	EsifFpcPrimitivePtr primitive_ptr = NULL;
	EsifPrimitiveTuple tuple = {0};
	eEsifError rc = ESIF_OK;
	esif_temp_t saved_val    = 0;
	UInt16 kernAct, domain = convert_string_to_short(domain_str);
	UInt32 i, do_retry = ESIF_TRUE;
	UInt32 req_auto  = ESIF_FALSE, rsp_auto = ESIF_FALSE,
		   req_temp  = ESIF_FALSE, rsp_temp = ESIF_FALSE,
		   req_power = ESIF_FALSE, rsp_power = ESIF_FALSE;
	char *replaced_str[5] = {0};
	struct esif_data void_data = {ESIF_DATA_VOID, NULL, 0};

	// Just like what we did in LF, was_temp, was_power, etc
	#define ESIF_UNIT_RETYPE(name, old_type, new_type, set_flag)   \
	if (name == old_type) { \
		name = new_type;set_flag = 1; \
	}

	ESIF_TRACE_DEBUG("%s\n\n"
					   "Participant ID       : %u\n"
					   "Request              : %p\n"
					   "  Data Type:         : %s(%u)\n"
					   "  Data Buffer        : %p\n"
					   "  Data Length        : %u\n"
					   "Response             : %p\n"
					   "  Data Type          : %s(%u)\n"
					   "  Data Buffer        : %p\n"
					   "  Data Length        : %u\n"
					   "Primitive            : %s(%u)\n"
					   "Domain               : %s\n"
					   "Instance             : %u\n\n",
					   ESIF_FUNC,
					   participantId,
					   requestPtr,
					   esif_data_type_str(requestPtr->type), requestPtr->type,
					   requestPtr->buf_ptr,
					   requestPtr->buf_len,
					   responsePtr,
					   esif_data_type_str(responsePtr->type), responsePtr->type,
					   responsePtr->buf_ptr,
					   responsePtr->buf_len,
					   esif_primitive_str((enum esif_primitive_type)primitiveId), primitiveId,
					   domain_str,
					   instance);


	up_ptr = EsifUpManagerGetAvailableParticipantByInstance(participantId);
	if (NULL == up_ptr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}

	dsp_ptr = up_ptr->fDspPtr;
	if (NULL == dsp_ptr) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}

	tuple.id       = (u16)primitiveId;
	tuple.domain   = domain;
	tuple.instance = instance;

	primitive_ptr  = dsp_ptr->get_primitive(dsp_ptr, &tuple);
	if (NULL == primitive_ptr) {
		rc = ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;
		goto exit;
	}

	/* Print */
	esif_uf_dump_primitive(dsp_ptr, primitive_ptr);

	/* Automatically Pick Up The Right Data Type, If Asked To Do So */
	ESIF_UNIT_RETYPE(requestPtr->type, ESIF_DATA_AUTO,
					 (enum esif_data_type)primitive_ptr->request_type, req_auto);
	ESIF_UNIT_RETYPE(responsePtr->type, ESIF_DATA_AUTO,
					 (enum esif_data_type)primitive_ptr->result_type, rsp_auto);

	/* Work around for now */
	if (ESIF_DATA_BIT == responsePtr->type ||
		ESIF_DATA_TIME == responsePtr->type ||
		ESIF_DATA_UINT8 == responsePtr->type ||
		ESIF_DATA_UINT16 == responsePtr->type) {
		responsePtr->type = ESIF_DATA_UINT32;
	}
	if (ESIF_DATA_UNICODE == responsePtr->type) {
		responsePtr->type = ESIF_DATA_STRING;
	}

	if (responsePtr->buf_ptr == NULL && ESIF_DATA_ALLOCATE == responsePtr->buf_len) {
		int size = 4;

		if (ESIF_DATA_UINT64 == responsePtr->type ||
			ESIF_DATA_FREQUENCY == responsePtr->type) {
			size = 8;
		}
		if (ESIF_DATA_STRING == responsePtr->type) {
			size = 128;
		}
		if (ESIF_DATA_UNICODE == responsePtr->type) {
			size = 256;
		}
		if (ESIF_DATA_BINARY == responsePtr->type) {
			size = 4096;
		}

		responsePtr->buf_ptr = esif_ccb_malloc(size);
		responsePtr->buf_len = size;
		if (NULL == responsePtr->buf_ptr) {
			rc = ESIF_E_NO_MEMORY;
			goto exit;
		}

		ESIF_TRACE_DEBUG("Auto allocated response buffer %p size %u\n",
						   responsePtr->buf_ptr, responsePtr->buf_len);
	}

retry:
	/* UF and LF Perform Xform */
	if (g_uf_xform) {
		ESIF_UNIT_RETYPE(requestPtr->type, ESIF_DATA_TEMPERATURE, ESIF_DATA_UINT32, req_temp);
		ESIF_UNIT_RETYPE(requestPtr->type, ESIF_DATA_POWER, ESIF_DATA_UINT32, req_power);
		ESIF_UNIT_RETYPE(responsePtr->type, ESIF_DATA_TEMPERATURE, ESIF_DATA_UINT32, rsp_temp);
		ESIF_UNIT_RETYPE(responsePtr->type, ESIF_DATA_POWER, ESIF_DATA_UINT32, rsp_power);
		if (req_temp || req_power) {
			saved_val = *(u32 *)requestPtr->buf_ptr;
		}
	}

	if (primitive_ptr->num_actions == 0) {
		rc = ESIF_E_UNSUPPORTED_ACTION_TYPE;
		goto exit;
	}

	/*
	 * Going to ESIF LF Service Or User-Level Service
	 */
	for (kernAct = 0, i = 0; i < (int)primitive_ptr->num_actions; i++) {
		action_ptr = dsp_ptr->get_action(dsp_ptr, primitive_ptr, (u8)i);
		if (NULL == action_ptr) {
			rc = ESIF_E_UNSUPPORTED_ACTION_TYPE;
			goto exit;
		}

		algo_ptr = dsp_ptr->get_algorithm(dsp_ptr, action_ptr->type);
		if (req_temp) {
			*(esif_temp_t *)requestPtr->buf_ptr = (esif_temp_t)saved_val;
			esif_xform_temp(NORMALIZE_TEMP_TYPE,
							(esif_temp_t *)requestPtr->buf_ptr,
							action_ptr->type,
							dsp_ptr,
							primitive_ptr->operation);
		}
		if (req_power) {
			*(esif_power_t *)requestPtr->buf_ptr = (esif_power_t)saved_val;
			esif_xform_power(NORMALIZE_POWER_UNIT_TYPE,
							 (esif_power_t *)requestPtr->buf_ptr,
							 action_ptr->type,
							 dsp_ptr,
							 primitive_ptr->operation);
		}


		if (action_ptr->is_kernel > 0) {
			/* ESIF LF via IPC */
			if (ESIF_PRIMITIVE_OP_GET == primitive_ptr->operation) {
				rc = PrimitiveActionLFGet(participantId, kernAct,
										  &tuple, action_ptr, requestPtr, responsePtr);
			} else {
				rc = PrimitiveActionLFSet(participantId, kernAct,
										  &tuple, action_ptr, requestPtr, responsePtr);
			}
			ESIF_TRACE_DEBUG("Using ESIF LF for action %d, kern_action %d! rc %s\n",
							   i, kernAct, esif_rc_str(rc));
			kernAct++;
		} else {
			/* User-Level Only */
			UInt8 j = 0;
			EsifData params[5];

			for (j = 0; j < 5; j++) {
				DataItemPtr data_item_ptr = get_action_param(action_ptr, j);

				params[j].buf_len = 0;
				params[j].buf_ptr = NULL;
				params[j].type    = ESIF_DATA_UINT32;

				if (NULL == data_item_ptr) {
					continue;
				}

				switch (data_item_ptr->data_type) {
				case 1:	// String
				{
					char temp_buf[ESIF_NAME_LEN + 1];
					EsifString param_str = (EsifString) & data_item_ptr->data;
					EsifUpPtr up_ptr     = EsifUpManagerGetAvailableParticipantByInstance(participantId);
					EsifString replaced  = NULL;

					if (NULL != up_ptr) {
						esif_ccb_sprintf(ESIF_NAME_LEN, temp_buf, "%s.%s", up_ptr->fMetadata.fName, domain_str);

						replaced = esif_str_replace(
								param_str,
								"%nm%",
								temp_buf);

						if (NULL != replaced) {
							if (replaced_str[j]) {
								esif_ccb_free(replaced_str[j]);
							}
							param_str = replaced;
							replaced_str[j] = replaced;
							ESIF_TRACE_DEBUG("\tEXPANDED data %s\n", param_str);
						}
					}

					params[j].buf_ptr  = param_str;	// (char*) &data_item_ptr->data;
					params[j].buf_len  = (u32)esif_ccb_strlen(param_str, MAXPARAMLEN);	// data_item_ptr->data_length_in_bytes;
					params[j].data_len = (u32)esif_ccb_strlen(param_str, MAXPARAMLEN);	// data_item_ptr->data_length_in_bytes;
					params[j].type     = ESIF_DATA_STRING;
					break;
				}

				case 4:	// 32 Bit Integer
					params[j].buf_ptr  = (u32 *)&data_item_ptr->data;
					params[j].buf_len  = data_item_ptr->data_length_in_bytes;
					params[j].data_len = data_item_ptr->data_length_in_bytes;
					params[j].type     = ESIF_DATA_UINT32;
					break;

				default:
					break;
				}
			}

			if (ESIF_PRIMITIVE_OP_GET == primitive_ptr->operation) {
				rc = PrimitiveActionUFGet(participantId, (enum esif_action_type)action_ptr->type,
										  &params[0], &params[1], &params[2], &params[3], &params[4],
										  requestPtr, responsePtr);
			} else {
				rc = PrimitiveActionUFSet(participantId, (enum esif_action_type)action_ptr->type,
										  &params[0], &params[1], &params[2], &params[3], &params[4],
										  requestPtr);
				if (ESIF_OK == rc && ESIF_ACTION_CONFIG == action_ptr->type) {
					if (SET_TRIP_POINT_ACTIVE == primitive_ptr->tuple.id ||
						SET_TRIP_POINT_CRITICAL == primitive_ptr->tuple.id ||
						SET_TRIP_POINT_HOT == primitive_ptr->tuple.id ||
						SET_TRIP_POINT_PASSIVE == primitive_ptr->tuple.id ||
						SET_TRIP_POINT_WARM == primitive_ptr->tuple.id) {
						EsifAppsEvent(participantId, primitive_ptr->tuple.domain, ESIF_EVENT_PARTICIPANT_SPEC_INFO_CHANGED, &void_data);
						ESIF_TRACE_DEBUG("Send Event ==> ESIF_EVENT_PARTICIPANT_SPEC_INFO_CHANGED\n");
					}
					else if (SET_THERMAL_RELATIONSHIP_TABLE == primitive_ptr->tuple.id) {
						EsifAppsEvent(participantId, primitive_ptr->tuple.domain, ESIF_EVENT_APP_THERMAL_RELATIONSHIP_CHANGED, &void_data);
						ESIF_TRACE_DEBUG("Send Event ==>ESIF_EVENT_APP_THERMAL_RELATIONSHIP_CHANGED\n");
					}
					else if (SET_ACTIVE_RELATIONSHIP_TABLE == primitive_ptr->tuple.id) {
						EsifAppsEvent(participantId, primitive_ptr->tuple.domain, ESIF_EVENT_APP_ACTIVE_RELATIONSHIP_CHANGED, &void_data);
						ESIF_TRACE_DEBUG("Send Event ==>ESIF_EVENT_APP_ACTIVE_RELATIONSHIP_CHANGED\n");
					}
				}
			}
			ESIF_TRACE_DEBUG("Using User-Level service for action %d! rc %s\n", i, esif_rc_str(rc));
		}

		if (ESIF_OK == rc) {
			if (rsp_temp) {
				ESIF_TRACE_DEBUG("esif_xform_temp\n");
				esif_xform_temp(NORMALIZE_TEMP_TYPE,
								(esif_temp_t *)responsePtr->buf_ptr,
								action_ptr->type,
								dsp_ptr,
								primitive_ptr->operation);
			}
			if (rsp_power) {
				ESIF_TRACE_DEBUG("esif_xform_power\n");
				esif_xform_power(NORMALIZE_POWER_UNIT_TYPE,
								 (esif_temp_t *)responsePtr->buf_ptr,
								 action_ptr->type,
								 dsp_ptr,
								 primitive_ptr->operation);
			}
			break;
		}
	}

	/* Restore Type */
	if (req_temp) {
		requestPtr->type = ESIF_DATA_TEMPERATURE;
	}
	if (req_power) {
		requestPtr->type = ESIF_DATA_POWER;
	}
	if (rsp_temp) {
		responsePtr->type = ESIF_DATA_TEMPERATURE;
	}
	if (rsp_power) {
		responsePtr->type = ESIF_DATA_POWER;
	}

	/*
	 * When Default Buffer Size Is Too Small, Retry Once Using Response Data Size.
	 * This Only Applies To ESIF_DATA_AUTO.
	 */
	if (ESIF_E_NEED_LARGER_BUFFER == rc) {
		if (do_retry-- && ESIF_TRUE == rsp_auto) {
			ESIF_TRACE_DEBUG("Auto re-malloc: original %u new buffer size %u byte\n",
							 responsePtr->buf_len, responsePtr->data_len);

			esif_ccb_free(responsePtr->buf_ptr);

			responsePtr->buf_ptr = esif_ccb_malloc(responsePtr->data_len);
			responsePtr->buf_len = responsePtr->data_len;
			if (NULL == responsePtr->buf_ptr) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
			goto retry;
		}
	}

	/* KLUDGE for inconsistent GTSH behavior here */
	/* LAL reports .001C we need C */
	/* Man I hate code like this */
	if (ESIF_ACTION_LAL == action_ptr->type && GET_TEMPERATURE_THRESHOLD_HYSTERESIS == primitive_ptr->tuple.id) {
		if (responsePtr->buf_ptr != NULL) {
			*(u32 *)responsePtr->buf_ptr = ((*(u32 *)responsePtr->buf_ptr) / 1000);
		}
	}

exit:
	// Free any replaced (allocated) strings
	for (i = 0; i < sizeof(replaced_str) / sizeof(char *); i++) {
		if (replaced_str[i]) {
			esif_ccb_free(replaced_str[i]);
		}
	}

	return rc;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

