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

#include "esif_action.h"
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
#define MMIO_ACCESS_SIZE    4

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
	struct esif_lp_primitive *primitive_ptr,
	const struct esif_lp_action *action_ptr,
	struct esif_lp *lp_ptr,
	struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	enum esif_rc rc       = ESIF_OK;
	esif_temp_t orig_temp = 0;
	u8 was_temp  = ESIF_FALSE;
	u8 was_power = ESIF_FALSE;


	ESIF_TRACE_DYN_ACTION(
		"lp name %s req_type %s rsp_type %s, opcode %s action %s "
		"tuple %d.%d.%d\n",
		lp_ptr->pi_name,
		esif_data_type_str(req_data_ptr->type),
		esif_data_type_str(rsp_data_ptr->type),
		esif_primitive_opcode_str(primitive_ptr->opcode),
		esif_action_type_str(action_ptr->type),
		primitive_ptr->tuple.id,
		primitive_ptr->tuple.domain,
		primitive_ptr->tuple.instance);

	/*
	 * If it is a s SET request and we have a valide temp or power xform,
	 * normalize the requested tmperature (_t) and power (_pw) in the
	 * request.
	 */
	if (primitive_ptr->opcode == ESIF_PRIMITIVE_OP_SET) {
		/* Unnormalize Temperature Example C -> Deci Kelvin */
		if (req_data_ptr->type == ESIF_DATA_TEMPERATURE &&
		    lp_ptr->xform_temp != NULL) {
			orig_temp = *(u32 *)req_data_ptr->buf_ptr;
			rc = lp_ptr->xform_temp(NORMALIZE_TEMP_TYPE,
					(esif_temp_t *)req_data_ptr->buf_ptr,
					action_ptr->type,
					lp_ptr->dsp_ptr,
					primitive_ptr,
					lp_ptr);

			if (rc != ESIF_OK)
				goto exit;
		}
		/* Unnormalize Power Example Milliwatts -> Deci Watts*/
		if (req_data_ptr->type == ESIF_DATA_POWER &&
		    lp_ptr->xform_power != NULL) {
			rc = lp_ptr->xform_power(NORMALIZE_POWER_UNIT_TYPE,
					(esif_power_t *)req_data_ptr->buf_ptr,
					action_ptr->type,
					lp_ptr->dsp_ptr,
					primitive_ptr->opcode);

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
		ESIF_TRACE_DYN_ACTION("%s: TRANSFORM RSP TEMP >> UINT32\n",
				      ESIF_FUNC);
		rsp_data_ptr->type = ESIF_DATA_UINT32;
		was_temp = ESIF_TRUE;
	}

	/* Transform data type for power into native UINT32 data type for NOW */
	if (ESIF_PRIMITIVE_OP_GET == primitive_ptr->opcode &&
	    ESIF_DATA_POWER == rsp_data_ptr->type) {
		ESIF_TRACE_DYN_ACTION("%s: TRANSFORM RSP POWER >> UINT32\n",
				      ESIF_FUNC);
		rsp_data_ptr->type = ESIF_DATA_UINT32;
		was_power = ESIF_TRUE;
	}

	/* Transform data type for temp into native UINT32 data type for NOW */
	if (ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode &&
	    ESIF_DATA_TEMPERATURE == req_data_ptr->type) {
		ESIF_TRACE_DYN_ACTION(
			"%s: TRANSFORM REQ TEMPERATURE >> UINT32\n",
			ESIF_FUNC);
		req_data_ptr->type = ESIF_DATA_UINT32;
		was_temp = ESIF_TRUE;
	}

	/* Transform data type for power into native UINT32 data type for NOW */
	if (ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode &&
	    ESIF_DATA_POWER == req_data_ptr->type) {
		ESIF_TRACE_DYN_ACTION("%s: TRANSFORM REQ POWER >> UINT32\n",
				      ESIF_FUNC);
		req_data_ptr->type = ESIF_DATA_UINT32;
		was_power = ESIF_TRUE;
	}

	/* Handle Actions */
	switch (action_ptr->type) {
	case ESIF_ACTION_ACPI:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_acpi(lp_ptr->pi_ptr->acpi_handle,
					action_ptr->get_p1_u32(action_ptr),
					req_data_ptr,
					rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_acpi(lp_ptr->pi_ptr->acpi_handle,
					action_ptr->get_p1_u32(action_ptr),
					(struct esif_data *)req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_ACPI, (acpi_string) p1 %x, "
			"result type %s(%d)\n",
			ESIF_FUNC,
			action_ptr->get_p1_u32(action_ptr),
			esif_rc_str(rc),
			rc);
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
			rc = esif_get_action_acpi(lp_ptr->pi_ptr->acpi_handle,
					action_ptr->get_p1_u32(action_ptr),
					req_data_ptr,
					rsp_data_ptr);
			break;
		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_ACPILPAT, (acpi_string) p1 %x, "
			"result type %s(%d)\n",
			ESIF_FUNC,
			action_ptr->get_p1_u32(action_ptr),
			esif_rc_str(rc),
			rc);
	}
	break;	/* End of ACPILPAT */

	/*
	 * Handle Constant Actions
	 */
	case ESIF_ACTION_KONST:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_const(
					action_ptr->get_p1_u32(action_ptr),
					req_data_ptr,
					rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_KONST, (string) p1 %x result "
			"type %s(%d)\n",
			ESIF_FUNC,
			action_ptr->get_p1_u32(action_ptr),
			esif_rc_str(rc),
			rc);
	}
	break;	/* End of CONST */

	/*
	 * Handle CODE Actions
	 */
	case ESIF_ACTION_KODE:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_code((struct esif_lp *)lp_ptr,
						  &primitive_ptr->tuple,
						  action_ptr,
						  req_data_ptr,
						  rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_code((struct esif_lp *)lp_ptr,
						  &primitive_ptr->tuple,
						  action_ptr,
						  req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_KODE, (method) p1 %x result "
			"type %s(%d)\n",
			ESIF_FUNC,
			action_ptr->get_p1_u32(action_ptr),
			esif_rc_str(rc),
			rc);
	}
	break;	/* End of CODE */

	/*
	 *  Handle SYSTEMIO Actions
	 */
	case ESIF_ACTION_SYSTEMIO:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_systemio(req_data_ptr,
						      rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_systemio(req_data_ptr,
						      rsp_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_SYSTEMIO, result type %s(%d)\n",
			ESIF_FUNC,
			esif_rc_str(rc),
			rc);
	}
	break;	/* End of SYSTEMIO */

	/*
	 * Hande VAR Actions
	 */
	case ESIF_ACTION_VAR:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
		{
			u32 val = action_ptr->get_p1_u32(action_ptr);
			struct esif_data local = {ESIF_DATA_UINT32, &val,
						sizeof(val)};
			struct esif_data *var  = NULL;

			/* Use Context Variable If Available */
			if (primitive_ptr->context_ptr == NULL)
				var = &local;
			else
				var = primitive_ptr->context_ptr;

			rc = esif_get_action_var(var,
						 req_data_ptr,
						 rsp_data_ptr);
			ESIF_TRACE_DYN_ACTION(
				"%s: ESIF_ACTION_VAR_GET, result %d\n",
				ESIF_FUNC,
				*(u32 *)rsp_data_ptr->buf_ptr);
		}
		break;

		case ESIF_PRIMITIVE_OP_SET:
		{
			ESIF_TRACE_DYN_ACTION(
				"%s: ESIF_ACTION_VAR_SET, data %d\n",
				ESIF_FUNC,
				*(unsigned int *)req_data_ptr->buf_ptr);

			/* Create a State Variable To Hold Our Context */
			if (NULL == primitive_ptr->context_ptr) {
				primitive_ptr->context_ptr = esif_data_alloc(
						req_data_ptr->type,
						req_data_ptr->buf_len);
			}

			rc = esif_set_action_var(primitive_ptr->context_ptr,
						 req_data_ptr);

			if (ESIF_OK == rc) {
				struct esif_primitive_tuple tup;
				struct esif_lp_primitive *p = NULL;

				/* Find Get To Go With This Set Relationship */
				tup.id       = (u16)action_ptr->get_p1_u32(
						action_ptr);
				tup.domain   = (u16)action_ptr->get_p2_u32(
						action_ptr);
				tup.instance = (u8)action_ptr->get_p3_u32(
						action_ptr);
				ESIF_TRACE_DYN_ACTION(
					"%s: Find Get For Set, data %d.%d.%d\n",
					ESIF_FUNC,
					tup.id,
					tup.domain,
					tup.instance);

				p = lp_ptr->dsp_ptr->get_primitive(
						lp_ptr->dsp_ptr,
						&tup);
				p->context_ptr = primitive_ptr->context_ptr;
			}
		}
		break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
	}
	break;	/* End of VAR */

	/*
	 * Hande MBI Actions
	 */
	case ESIF_ACTION_IOSF:
	{
		u8 port      = (u8)action_ptr->get_p1_u32(action_ptr);
		u8 punit     = (u8)action_ptr->get_p2_u32(action_ptr);
		u8 bit_start = (u8)action_ptr->get_p4_u32(action_ptr);
		u8 bit_stop  = (u8)action_ptr->get_p3_u32(action_ptr);

		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_mbi(port,
						 punit,
						 bit_start,
						 bit_stop,
						 req_data_ptr,
						 rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_mbi(port,
						 punit,
						 bit_start,
						 bit_stop,
						 req_data_ptr);
			ESIF_TRACE_DYN_ACTION(
				"%s: MBI SET port 0x%x punit 0x%x rc %d\n",
				ESIF_FUNC,
				port,
				punit,
				rc);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_MBI, prim->p1 %d, prim->p2 %d, "
			"prim->p3 %d, result %d\n",
			ESIF_FUNC,
			action_ptr->get_p1_u32(action_ptr),
			action_ptr->get_p2_u32(action_ptr),
			action_ptr->get_p3_u32(action_ptr),
			*(unsigned int *)rsp_data_ptr->buf_ptr);
	}
	break;	/* End of MBI */

	/*
	 * Hande MMIO Actions
	 */
	case ESIF_ACTION_MMIO:
	{
		u32 offset   = action_ptr->get_p1_u32(action_ptr);
		u8 bit_start = (u8)action_ptr->get_p3_u32(action_ptr);
		u8 bit_stop  = (u8)action_ptr->get_p2_u32(action_ptr);

#ifdef ESIF_ATTR_OS_WINDOWS
		if ((offset + MMIO_ACCESS_SIZE) > lp_ptr->pi_ptr->mem_size) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			break;
		}
#endif
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_mmio(lp_ptr->pi_ptr->mem_base,
						  offset,
						  bit_start,
						  bit_stop,
						  req_data_ptr,
						  rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_mmio(lp_ptr->pi_ptr->mem_base,
						  offset,
						  bit_start,
						  bit_stop,
						  req_data_ptr);
			ESIF_TRACE_DYN_ACTION(
				"%s: MMIO SET offset 0x%x rc %d\n",
				ESIF_FUNC,
				offset,
				rc);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_MMIO, prim->p1 %d, prim->p2 %d, "
			"prim->p3 %d, result %d\n",
			ESIF_FUNC,
			action_ptr->get_p1_u32(action_ptr),
			action_ptr->get_p2_u32(action_ptr),
			action_ptr->get_p3_u32(action_ptr),
			*(unsigned int *)rsp_data_ptr->buf_ptr);
	}
	break;	/* End of MMIO */

	/*
	 * Hande MSR Actions
	 */
	case ESIF_ACTION_MSR:
	{
		u32 msr     = action_ptr->get_p1_u32(action_ptr);
		u8 bit_from = (u8)action_ptr->get_p3_u32(action_ptr);
		u8 bit_to   = (u8)action_ptr->get_p2_u32(action_ptr);
		u32 hint    = action_ptr->get_p4_u32(action_ptr);
		u32 cpus    = 0;	/* Default CPU */

		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:

			/*
			 * getp_pX_u32() = -1 means the parameter isn't set,
			 * 0 is a valid value.
			 * For whose p4 (hint) are set, CPU affinity mask is
			 * required, either provided by UF or use all active
			 * CPUs.
			 */
			if ((int)hint > 0) {
				/* Use cpu_affinity_mask from by UF, or  all */
				if (req_data_ptr->buf_len ==
				    sizeof(ESIF_DATA_UINT32) &&
				    req_data_ptr->type == ESIF_DATA_UINT32) {
					cpus = (*(u32 *)req_data_ptr->buf_ptr);
				} else {
					/* For hint without binary type */
					if (rsp_data_ptr->type == ESIF_DATA_UINT8 ||
					    rsp_data_ptr->type == ESIF_DATA_UINT16 ||
					    rsp_data_ptr->type == ESIF_DATA_UINT32 ||
					    rsp_data_ptr->type == ESIF_DATA_UINT64) {
						cpus = (u32)(0UL); /* Use 0 */
					} else {
						cpus = (u32) ~(0UL); /* All */
					}
				}
			}

			rc = esif_get_action_msr(msr,
						 bit_from,
						 bit_to,
						 cpus,
						 hint,
						 req_data_ptr,
						 rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			if ((hint > 0)) {
				/*
				 * Expected Request Data {msr_value} or
				 * {msr_value, cpu_affinity_mask}
				 */
				if ((req_data_ptr->buf_len ==
				    (sizeof(ESIF_DATA_UINT32) * 2)) &&
				    (req_data_ptr->type == ESIF_DATA_UINT32)) {
					cpus =
					   (*(u32 *)((u8 *)req_data_ptr->buf_ptr + 4));
				} else {
					cpus = (u32) ~(0UL);
				}
			}

			rc = esif_set_action_msr(msr,
						 bit_from,
						 bit_to,
						 cpus,
						 hint,
						 req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_MSR, p1 msr %d,  p2 bits_to %d, p3 bits_from %d, p4 cpu_affinity %d, result 0x%llx\n",
			ESIF_FUNC,
			action_ptr->get_p1_u32(action_ptr),
			action_ptr->get_p2_u32(action_ptr),
			action_ptr->get_p3_u32(action_ptr),
			action_ptr->get_p4_u32(action_ptr),
			*(unsigned long long *)rsp_data_ptr->buf_ptr);
	}
	break;	/* End of MSR */

	/*
	 * Hande DDIGFXPERF Actions
	 */
	case ESIF_ACTION_DDIGFXPERF:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_ddigfxperf(primitive_ptr,
							action_ptr,
							lp_ptr,
							req_data_ptr,
							rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_ddigfxperf(primitive_ptr,
							action_ptr,
							lp_ptr,
							req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_DDIGFXPERF, result type %s(%d)\n",
			ESIF_FUNC,
			esif_rc_str(rc),
			rc);
	}
	break;	/* End of DDIGFXPERF */


	/*
	 * Hande DDIGFXDISP Actions
	 */
	case ESIF_ACTION_DDIGFXDISP:
	{
		switch (primitive_ptr->opcode) {
		case ESIF_PRIMITIVE_OP_GET:
			rc = esif_get_action_ddigfxdisp(primitive_ptr,
							action_ptr,
							lp_ptr,
							req_data_ptr,
							rsp_data_ptr);
			break;

		case ESIF_PRIMITIVE_OP_SET:
			rc = esif_set_action_ddigfxdisp(primitive_ptr,
							action_ptr,
							lp_ptr,
							req_data_ptr);
			break;

		default:
			rc = ESIF_E_OPCODE_NOT_IMPLEMENTED;
			break;
		}
		ESIF_TRACE_DYN_ACTION(
			"%s: ESIF_ACTION_DDIGFXPERF, result type %s(%d)\n",
			ESIF_FUNC,
			esif_rc_str(rc),
			rc);
	}
	break;	/* End of DDIGFXDISP */

	default:
		ESIF_TRACE_DYN_ACTION(
			"%s: prim %d NOT supported, p1 %x, result %d\n",
			ESIF_FUNC,
			action_ptr->type,
			action_ptr->get_p1_u32(action_ptr),
			*(unsigned int *)rsp_data_ptr->buf_ptr);
		rc = ESIF_E_UNSUPPORTED_ACTION_TYPE;
		break;
	}

	/* Put back Temperature Type? */
	if ((ESIF_PRIMITIVE_OP_GET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_temp)) {
		ESIF_TRACE_DYN_ACTION(
			"%s: TRANSFORM RSP UINT32 >> TEMPERATURE\n",
			ESIF_FUNC);
		rsp_data_ptr->type = ESIF_DATA_TEMPERATURE;
	}

	/* Put back Power Type? */
	if ((ESIF_PRIMITIVE_OP_GET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_power)) {
		ESIF_TRACE_DYN_ACTION("%s: TRANSFORM RSP UINT32 >> POWER\n",
				      ESIF_FUNC);
		rsp_data_ptr->type = ESIF_DATA_POWER;
	}

	if ((ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_temp)) {
		ESIF_TRACE_DYN_ACTION(
			"%s: TRANSFORM REQ UINT32 >> TEMPERATURE\n",
			ESIF_FUNC);
		req_data_ptr->type = ESIF_DATA_TEMPERATURE;
	}

	if ((ESIF_PRIMITIVE_OP_SET == primitive_ptr->opcode) &&
	    (ESIF_TRUE == was_power)) {
		ESIF_TRACE_DYN_ACTION("%s: TRANSFORM REQ UINT32 >> POWER\n",
				      ESIF_FUNC);
		req_data_ptr->type = ESIF_DATA_POWER;
	}

	/*
	 * Transform temperature/power if the response type told us to do so.
	 */
	if (primitive_ptr->opcode == ESIF_PRIMITIVE_OP_GET) {
		/* Transform Temperature */
		if ((rsp_data_ptr->type == ESIF_DATA_TEMPERATURE) &&
		    (rc == ESIF_OK) &&
		    (lp_ptr->xform_temp != NULL)) {
			rc = lp_ptr->xform_temp(NORMALIZE_TEMP_TYPE,
						(esif_temp_t *)rsp_data_ptr->buf_ptr,
						action_ptr->type,
						lp_ptr->dsp_ptr,
						primitive_ptr,
						lp_ptr);
			if (rc != ESIF_OK)
				goto exit;
		}

		/* Transform Power */
		if ((rsp_data_ptr->type == ESIF_DATA_POWER) &&
		    (rc == ESIF_OK) &&
		    (lp_ptr->xform_power != NULL)) {
			rc = lp_ptr->xform_power(NORMALIZE_POWER_UNIT_TYPE,
						 (esif_power_t *)rsp_data_ptr->buf_ptr,
						 action_ptr->type,
						 lp_ptr->dsp_ptr,
						 primitive_ptr->opcode);
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
		    (action_ptr->type != ESIF_ACTION_KODE) &&
		    (ESIF_OK == rc)) {
			u8 domain_index = 0;
			rc = esif_lp_domain_index(primitive_ptr->tuple.domain,
						  &domain_index);
			if ((ESIF_OK != rc) ||
			    (domain_index > lp_ptr->domain_count)) {
				goto exit;
			} else {
				if (0 == primitive_ptr->tuple.instance) {
					lp_ptr->domains[domain_index].
					temp_cache0 = orig_temp;
				}
				if (1 == primitive_ptr->tuple.instance) {
					lp_ptr->domains[domain_index].
					temp_cache1 = orig_temp;
				}
			}
		}
	}
exit:
	return rc;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

