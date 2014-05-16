/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#include "esif_lf_action.h"
#include "esif_participant.h"
#include "esif_lf_ccb_gen_action.h"

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified against Windows SDK/DDK included headers which
 * we have no control over.
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Debug Logging Defintions */
#define INIT_DEBUG          0	/* Init Debug      */
#define ACTION_DEBUG        1	/* Primitive Debug */
#define DECODE_DEBUG        2	/* Decode Debug    */

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION, \
		       INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_ACTION(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION, \
		       ACTION_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_DECODE(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION, \
		       DECODE_DEBUG, format, ##__VA_ARGS__)

/* Execute Action */
enum esif_rc esif_execute_action(
	struct esif_lp *lp_ptr,
	struct esif_lp_primitive *primitive_ptr,
	const struct esif_lp_action *action_ptr,
	struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	esif_temp_t orig_temp = 0;
	u8 was_temp  = ESIF_FALSE;
	u8 was_power = ESIF_FALSE;
	u8 was_percent = ESIF_FALSE;
	enum esif_data_type rsp_xform_type = 0;

	if ((NULL == lp_ptr) || (NULL == primitive_ptr) ||
	    (NULL == action_ptr) || (NULL == req_data_ptr) ||
	    (NULL == rsp_data_ptr)) {
		    rc = ESIF_E_PARAMETER_IS_NULL;
		    goto exit;
	}

	ESIF_TRACE_DYN_ACTION(
		"---->LF ACTION:  Type = %s, Opcode = %s Tuple = %d.%d.%d\n",
		esif_action_type_str(action_ptr->type),
		esif_primitive_opcode_str(primitive_ptr->opcode),
		primitive_ptr->tuple.id,
		primitive_ptr->tuple.domain,
		primitive_ptr->tuple.instance);

	ESIF_TRACE_DYN_ACTION(
		"LP Name = %s, req_type = %s, rsp_type = %s\n",
		lp_ptr->pi_name,
		esif_data_type_str(req_data_ptr->type),
		esif_data_type_str(rsp_data_ptr->type));

	/*
	 * If it is a s SET request and we have a valide temp or power xform,
	 * normalize the requested tmperature (_t) and power (_pw) in the
	 * request.
	 */
	if (primitive_ptr->opcode == ESIF_PRIMITIVE_OP_SET) {
		/* Unnormalize Temperature Example C -> Deci Kelvin */
		if (req_data_ptr->type == ESIF_DATA_TEMPERATURE) { 
			orig_temp = *(u32 *)req_data_ptr->buf_ptr;
			rc = esif_execute_xform_func(lp_ptr,
						primitive_ptr,
						action_ptr->type,
						ESIF_DATA_TEMPERATURE,
						(u64 *)req_data_ptr->buf_ptr);

			if (rc != ESIF_OK)
				goto exit;
		}
		/* Unnormalize Power Example Milliwatts -> Deci Watts*/
		if (req_data_ptr->type == ESIF_DATA_POWER) {
			rc = esif_execute_xform_func(lp_ptr,
						primitive_ptr,
						action_ptr->type,
						ESIF_DATA_POWER,
						(u64 *) req_data_ptr->buf_ptr);

			if (rc != ESIF_OK)
				goto exit;
		}
	}

	/*
	 * Okay If we have gotten here we have a DSP and have found our
	 * primitive so now we will execute the primitive based on the
	 * request and the DSP Metadata.
	 */

	/* Transform data type for temp into native UINT32 data type for NOW */
	if (ESIF_PRIMITIVE_OP_GET == primitive_ptr->opcode &&
	    ESIF_DATA_TEMPERATURE == rsp_data_ptr->type) {
		ESIF_TRACE_DYN_ACTION("CHANGE RSP TEMPERATURE >> UINT32\n");
		rsp_data_ptr->type = ESIF_DATA_UINT32;
		was_temp = ESIF_TRUE;
	}

	/* Transform data type for power into native UINT32 data type for NOW */
	if (ESIF_PRIMITIVE_OP_GET == primitive_ptr->opcode &&
	    ESIF_DATA_POWER == rsp_data_ptr->type) {
		ESIF_TRACE_DYN_ACTION("CHANGE RSP POWER >> UINT32\n");
		rsp_data_ptr->type = ESIF_DATA_UINT32;
		was_power = ESIF_TRUE;
	}

	/* Transform data type for temp into native UINT32 data type for NOW */
	if (ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode &&
	    ESIF_DATA_TEMPERATURE == req_data_ptr->type) {
		ESIF_TRACE_DYN_ACTION("CHANGE REQ TEMPERATURE >> UINT32\n");
		req_data_ptr->type = ESIF_DATA_UINT32;
		was_temp = ESIF_TRUE;
	}

	/* Transform data type for power into native UINT32 data type for NOW */
	if (ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode &&
	    ESIF_DATA_POWER == req_data_ptr->type) {
		ESIF_TRACE_DYN_ACTION("CHANGE REQ POWER >> UINT32\n");
		req_data_ptr->type = ESIF_DATA_UINT32;
		was_power = ESIF_TRUE;
	}

	if ((ESIF_PRIMITIVE_OP_GET == primitive_ptr->opcode) &&
	    (ESIF_DATA_PERCENT == rsp_data_ptr->type)) {
		was_percent = ESIF_TRUE;
		rsp_data_ptr->type = ESIF_DATA_UINT32;
	}

	if ((ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode) &&
	    (ESIF_DATA_PERCENT == req_data_ptr->type)) {
		was_percent = ESIF_TRUE;
		*(u32 *)req_data_ptr->buf_ptr /= ESIF_PERCENT_CONV_FACTOR;
		req_data_ptr->type = ESIF_DATA_UINT32;
	}

	/* Handle Actions */
	switch (action_ptr->type) {
	case ESIF_ACTION_ACPI:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_acpi(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr,
						rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_acpi(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of ACPI */

	/*
	 * Handle ACPI LPAT Actions
	 */
	case ESIF_ACTION_ACPILPAT:
	{
		rsp_data_ptr->data_len = sizeof(u32);
		/* Fake Data Example 596 -> ~32C */
		/* ((u32 *)rsp_data_ptr->buf_ptr) = 596; */
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_acpi(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr,
						rsp_data_ptr);
			break;
		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of ACPILPAT */

	/*
	 * Handle Constant Actions
	 */
	case ESIF_ACTION_KONST:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_const(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr,
						rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of CONST */

	/*
	 * Handle CODE Actions
	 */
	case ESIF_ACTION_KODE:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_code(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr,
						rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_code(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of CODE */

	/*
	 *  Handle SYSTEMIO Actions
	 */
	case ESIF_ACTION_SYSTEMIO:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_systemio(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr,
						rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_systemio(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of SYSTEMIO */

	/*
	 * Handle VAR Actions
	 */
	case ESIF_ACTION_VAR:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_var(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr,
						rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_var(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr);

			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of VAR */

	/*
	 * Handle MBI Actions
	 */
	case ESIF_ACTION_IOSF:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_mbi(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr,
						rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_mbi(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of MBI */

	/*
	 * Handle MMIO Actions
	 */
	case ESIF_ACTION_MMIO:
	case ESIF_ACTION_MMIOTJMAX:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_mmio(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr,
						rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_mmio(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of MMIO */

	/*
	 * Handle MSR Actions
	 */
	case ESIF_ACTION_MSR:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:

			/* Keep original respond type needed by msr action */
			if (ESIF_TRUE == was_temp)
				rsp_xform_type = ESIF_DATA_TEMPERATURE;
			else if (ESIF_TRUE == was_power)
				rsp_xform_type = ESIF_DATA_POWER;
			else 
				rsp_xform_type = rsp_data_ptr->type;

			rc = esif_get_action_msr(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr,
						rsp_xform_type,
						rsp_data_ptr);		
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_msr(lp_ptr,
						primitive_ptr,
						action_ptr,
						req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of MSR */

	/*
	 * Handle DDIGFXPERF Actions
	 */
	case ESIF_ACTION_DDIGFXPERF:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_ddigfxperf(lp_ptr,
							primitive_ptr,
							action_ptr,
							req_data_ptr,
							rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_ddigfxperf(lp_ptr,
							primitive_ptr,
							action_ptr,
							req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of DDIGFXPERF */


	/*
	 * Handle DDIGFXDISP Actions
	 */
	case ESIF_ACTION_DDIGFXDISP:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_ddigfxdisp(lp_ptr,
							primitive_ptr,
							action_ptr,
							req_data_ptr,
							rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_ddigfxdisp(lp_ptr,
							primitive_ptr,
							action_ptr,
							req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of DDIGFXDISP */

	default:
		ESIF_TRACE_DYN_ACTION(
			"prim %d NOT supported, p1 %x, result %d\n",
			action_ptr->type,
			action_ptr->get_p1_u32(action_ptr),
			*(unsigned int *)rsp_data_ptr->buf_ptr);
		rc = ESIF_E_UNSUPPORTED_ACTION_TYPE;
		break;
	}

	/* Put back Temperature Type? */
	if ((ESIF_PRIMITIVE_OP_GET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_temp)) {
		ESIF_TRACE_DYN_ACTION("CHANGE RSP UINT32 >> TEMPERATURE\n");
		rsp_data_ptr->type = ESIF_DATA_TEMPERATURE;
	}

	/* Put back Power Type? */
	if ((ESIF_PRIMITIVE_OP_GET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_power)) {
		ESIF_TRACE_DYN_ACTION("CHANGE RSP UINT32 >> POWER\n");
		rsp_data_ptr->type = ESIF_DATA_POWER;
	}

	if ((ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_temp)) {
		ESIF_TRACE_DYN_ACTION("CHANGE REQ UINT32 >> TEMPERATURE\n");
		req_data_ptr->type = ESIF_DATA_TEMPERATURE;
	}

	if ((ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_power)) {
		ESIF_TRACE_DYN_ACTION("CHANGE REQ UINT32 >> POWER\n");
		req_data_ptr->type = ESIF_DATA_POWER;
	}

	if ((ESIF_PRIMITIVE_OP_GET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_percent)) {
		rsp_data_ptr->type = ESIF_DATA_PERCENT;
		if(ESIF_OK == rc) {
			*(u32 *)rsp_data_ptr->buf_ptr *= 
				ESIF_PERCENT_CONV_FACTOR;
		}
	}

	if ((ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_percent)) {
		req_data_ptr->type = ESIF_DATA_PERCENT;
	}

	ESIF_TRACE_DYN_ACTION("LF ACTION RC (Before type conversion): %s(%d)\n",
			       esif_rc_str(rc), rc);
	if (rc != ESIF_OK)
		goto exit;
	/*
	 * Transform temperature/power if the response type told us to do so.
	 */
	if (primitive_ptr->opcode == ESIF_PRIMITIVE_OP_GET) {
		/* Transform Temperature */
		if (rsp_data_ptr->type == ESIF_DATA_TEMPERATURE) { 
			rc = esif_execute_xform_func(lp_ptr,
					primitive_ptr,
					action_ptr->type,
					ESIF_DATA_TEMPERATURE,
					(u64 *) rsp_data_ptr->buf_ptr);

			if (rc != ESIF_OK)
				goto exit;
		}

		/* Transform Power */
		if (rsp_data_ptr->type == ESIF_DATA_POWER) {
			rc = esif_execute_xform_func(lp_ptr,
					primitive_ptr,
					action_ptr->type,
					ESIF_DATA_POWER, 
					(u64 *) rsp_data_ptr->buf_ptr);
			if (rc != ESIF_OK)
				goto exit;
		}

		/*
		 * TODO: Inconsistent BIOS behavior here BIOS reports .1C,
		 * we need C
		 */
		if ((ESIF_ACTION_ACPI == action_ptr->type) &&
		    (GET_TEMPERATURE_THRESHOLD_HYSTERESIS ==
		     primitive_ptr->tuple.id)) {
			*(u32 *)rsp_data_ptr->buf_ptr =
				((*(u32 *)rsp_data_ptr->buf_ptr) / 10);
		}
	} else {/* ESIF_PRIMITIVE_OP_SET */
		/*
		 * Cache aux0/aux1 temperature for devices that don't support
		 * GET. For SET_TEMP_THRESHOLDS and ACTION NOT Code IF ESIF_OK
		 */
		if ((req_data_ptr->type == ESIF_DATA_TEMPERATURE) &&
		    (primitive_ptr->tuple.id == SET_TEMPERATURE_THRESHOLDS) &&
		    (action_ptr->type != ESIF_ACTION_KODE)) {
			u8 domain_index = 0;
			rc = esif_lp_domain_index(primitive_ptr->tuple.domain,
						  &domain_index);
			if ((ESIF_OK != rc) ||
			    (domain_index > lp_ptr->domain_count)) {
				goto exit;
			}
			if (0 == primitive_ptr->tuple.instance) 
				lp_ptr->domains[domain_index].temp_cache0 =
					orig_temp;
			if (1 == primitive_ptr->tuple.instance)
				lp_ptr->domains[domain_index].temp_cache1 =
					orig_temp;
		}
	}
exit:
	ESIF_TRACE_DYN_ACTION("<----LF ACTION FINAL RC: %s(%d)\n",
			       esif_rc_str(rc), rc);
	return rc;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

