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

#include "esif_primitive.h"
#include "esif_action.h"
#include "esif_ipc.h"

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
#define PRIMITIVE_DEBUG     1	/* Primitive Debug */
#define DECODE_DEBUG        2	/* Decode Debug    */
#define PRIMITIVE_DEBUG_DSP 3	/* Debug DSP       */

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_PRIMITIVE, INIT_DEBUG, \
		      format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_PRIM(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_PRIMITIVE, PRIMITIVE_DEBUG, \
		      format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_DECODE(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_PRIMITIVE, DECODE_DEBUG, \
		      format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_PRIM_DPF(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_PRIMITIVE, PRIMITIVE_DEBUG_DSP, \
		      format, ##__VA_ARGS__)

/*
 ******************************************************************************
 * PRIVATE
 ******************************************************************************
 */

/* u16 To String */
static char *esif_primitive_domain_str(
	u16 domain,
	char *str_ptr
	)
{
	esif_ccb_memcpy(str_ptr, &domain, 2);	/* Two Byte String */
	*(str_ptr + 2) = 0;			/* Null Terminate */
	return str_ptr;
}


/* u32 To String */
static char *esif_action_acpi_str(
	u32 method,
	char *str_ptr
	)
{
	esif_ccb_memcpy(str_ptr, &method, 4);	/* Four Byte String */
	*(str_ptr + 4) = 0;			/* Null Terminate */
	return str_ptr;
}


/* Print Action */
static void esif_action_dump(struct esif_lp_action *action_ptr)
{
	/* Done this way to fix compiler warning in release build */
	char tmp[8];
	tmp[0] = '\0';

	/* First action that succeeds wins */
	ESIF_TRACE_DYN_DECODE(
		"ACTION TYPE                   P1         P2         "
		"P3         P4         P5\n"
		"----------------------------- ---------- ---------- "
		"---------- ---------- ----------\n"
		"%-25s(%02d) 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		esif_action_type_str(action_ptr->type),
		action_ptr->type,
		action_ptr->get_p1_u32(action_ptr),
		action_ptr->get_p2_u32(action_ptr),
		action_ptr->get_p3_u32(action_ptr),
		action_ptr->get_p4_u32(action_ptr),
		action_ptr->get_p5_u32(action_ptr));

	if (ESIF_ACTION_ACPI == action_ptr->type) {
		ESIF_TRACE_DYN_DECODE("ACPI Method: %s\n",
			esif_action_acpi_str(action_ptr->get_p1_u32(action_ptr),
					     tmp));
	} else if (ESIF_ACTION_KODE == action_ptr->type) {
		ESIF_TRACE_DYN_DECODE("Code Method: %s\n",
			esif_action_acpi_str(action_ptr->get_p1_u32(action_ptr),
					     tmp));
	}
	return;
}


static enum esif_rc esif_lf_primitive_get_and_execute_action(
	struct esif_lp_primitive *primitive_ptr,
	struct esif_lp *lp_ptr,
	const struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr,
	u8 action
	)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_lp_action *action_ptr = NULL;

	if (NULL == lp_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	action_ptr = lp_ptr->dsp_ptr->get_action(lp_ptr->dsp_ptr,
						 primitive_ptr,
						 action);
	if (NULL == action_ptr) {
		rc = ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;
		goto exit;
	}

	esif_action_dump(action_ptr);

	/* Have Action Now Execute */
	rc = esif_execute_action(primitive_ptr,
				 action_ptr,
				 lp_ptr,
				 (struct esif_data *)req_data_ptr,
				 rsp_data_ptr);

	ESIF_TRACE_DYN_PRIM("%s: PRIMITIVE ACTION(%d) return type: %s(%d)\n",
			    ESIF_FUNC, action, esif_rc_str(rc), rc);
exit:
	if (action_ptr != NULL)
		esif_ccb_free(action_ptr);

	return rc;
}


/*
 ******************************************************************************
 * PUBLIC
 ******************************************************************************
 */

/*
 * Execute IPC Primitive
 * Gets primitive data from IPC structure, looks up the participatn associated
 * with the primitive, and then executes the primitive.  After execution,
 * sets the response data in the primitive for return to caller.
 */
struct esif_ipc *esif_execute_ipc_primitive(struct esif_ipc *ipc_ptr)
{
	enum esif_rc rc  = ESIF_OK;
	struct esif_lp *lp_ptr = NULL;
	struct esif_ipc_primitive *primitive_ptr =
		(struct esif_ipc_primitive *)(ipc_ptr + 1);

	u8 *data_ptr = (u8 *)primitive_ptr + sizeof(*primitive_ptr);
	u32 data_size = 0;

	struct esif_data req_data = {
		primitive_ptr->req_data_type,
		data_ptr + primitive_ptr->req_data_offset,
		primitive_ptr->req_data_len,
		0
	};
	struct esif_data rsp_data = {
		primitive_ptr->rsp_data_type,
		data_ptr + primitive_ptr->rsp_data_offset,
		primitive_ptr->rsp_data_len,
		0
	};
	/* Done this way to fix compiler warning in release build */
	char tmp[8];
	tmp[0] = '\0';

	data_size = ipc_ptr->data_len - sizeof(*primitive_ptr);
	if(((primitive_ptr->req_data_offset + primitive_ptr->req_data_len) > 
	     data_size) ||
	   ((primitive_ptr->rsp_data_offset + primitive_ptr->rsp_data_len) > 
	     data_size)) {
		rc = ESIF_E_NEED_LARGER_BUFFER;
		goto exit;
	}

	ESIF_TRACE_DYN_PRIM("%s: PRIMITIVE Received:\n", ESIF_FUNC);
	ESIF_TRACE_DYN_DECODE(
		"  Version:                   %d\n"
		"  Primitive:             %s(%d)\n"
		"  Primitive Domain:      %s(%d)\n"
		"  Primitive Instance:    %d\n"
		"  Source Type/ID:        %s/%d\n"
		"  Destination Type/ID:   %s/%d\n"
		"  Action Hint:           %s(%d)\n"
		"  Payload Length:        %d\n"
		"  Request  Data Type:    %s(%d)\n"
		"  Request  Data Offset:  %d\n"
		"  Request  Data Length:  %d\n"
		"  Response Data Type:    %s(%d)\n"
		"  Response Data Offset:  %d\n"
		"  Response Data Length:  %d\n",
		(int)primitive_ptr->version,
		esif_primitive_str((enum esif_primitive_type)primitive_ptr->id),
		(int)primitive_ptr->id,
		esif_primitive_domain_str(primitive_ptr->domain, tmp),
		(int)primitive_ptr->domain,
		(int)primitive_ptr->instance,
		esif_primitive_src_str(primitive_ptr->src_id),
		(int)primitive_ptr->src_id,
		esif_primitive_dst_str(primitive_ptr->dst_id),
		(int)primitive_ptr->dst_id,
		esif_action_type_str(primitive_ptr->kern_action),
		(int)primitive_ptr->kern_action,
		(int)primitive_ptr->payload_len,
		esif_data_type_str(primitive_ptr->req_data_type),
		(int)primitive_ptr->req_data_type,
		(int)primitive_ptr->req_data_offset,
		(int)primitive_ptr->req_data_len,
		esif_data_type_str(primitive_ptr->rsp_data_type),
		(int)primitive_ptr->rsp_data_type,
		(int)primitive_ptr->rsp_data_offset,
		(int)primitive_ptr->rsp_data_len);

	/* Lookup the requsted participant instance */
	lp_ptr = esif_lf_pm_lp_get_by_instance_id(primitive_ptr->dst_id);
	if (NULL == lp_ptr) {
		rc = ESIF_E_PRIMITIVE_DST_UNAVAIL;
	} else {
		struct esif_primitive_tuple tuple = {
			(u16)primitive_ptr->id,
			primitive_ptr->domain,
			primitive_ptr->instance
		};

		/*
		 * 5th param - user specified action, or all actions (if NULL)
		 */
		rc = esif_execute_primitive(lp_ptr,
					    &tuple,
					    &req_data,
					    &rsp_data,
					    &primitive_ptr->kern_action);
	}

	primitive_ptr->return_code = rc;

	/* Update Response Data Length and Type (May be Changed) */
	primitive_ptr->rsp_data_type = rsp_data.type;
	primitive_ptr->rsp_data_len  = rsp_data.data_len;

	ESIF_TRACE_DYN_PRIM("%s:\n"
			    "    PRIMITIVE Return: %s(%d)\n"
			    "    Response Data Type: %s(%d)\n"
			    "    Buffer Length: %d\n"
			    "    Data Length  : %d\n",
			    ESIF_FUNC, esif_rc_str(rc), rc,
			    esif_data_type_str(primitive_ptr->rsp_data_type),
			    rsp_data.type, rsp_data.data_len,
			    rsp_data.data_len);
exit:
	return ipc_ptr;
}


/* Execute Primitive */
enum esif_rc esif_execute_primitive(
	struct esif_lp *lp_ptr,
	const struct esif_primitive_tuple *tuple_ptr,
	const struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr,
	const u16 *action_index_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_lp_primitive *primitive_ptr = NULL;
	u8 action = 0;

	/* Done this way to fix compiler warning in release build */
	char tmp[8];
	tmp[0] = '\0';

	/* Special load our DSP */
	if (ESIF_DATA_DSP == req_data_ptr->type) {
		ESIF_TRACE_DYN_PRIM_DPF("%s: Load DSP buf=%p size=%d\n",
					ESIF_FUNC,
					req_data_ptr->buf_ptr,
					req_data_ptr->buf_len);

		/* Unload? */
		if (NULL != lp_ptr->dsp_ptr && 0 == req_data_ptr->buf_len) {
			ESIF_TRACE_DYN_PRIM_DPF("%s: Unload DSP %p\n",
						ESIF_FUNC,
						lp_ptr->dsp_ptr);
			esif_dsp_unload(lp_ptr);
			return ESIF_OK;
		}

		/* Reload? */
		if (lp_ptr->dsp_ptr != NULL) {
			ESIF_TRACE_DYN_PRIM_DPF(
				"%s: Reload DSP buf=%p size=%d\n",
				ESIF_FUNC,
				req_data_ptr->buf_ptr,
				req_data_ptr->buf_len);
			esif_dsp_unload(lp_ptr);
		}
		rc = esif_dsp_load(lp_ptr, req_data_ptr);
		return rc;
	}

	/* Can't continue without DSP! */
	if (NULL == lp_ptr->dsp_ptr)
		return ESIF_E_NEED_DSP;

	/* Free Me! */
	primitive_ptr = lp_ptr->dsp_ptr->get_primitive(lp_ptr->dsp_ptr,
						       tuple_ptr);
	if (NULL == primitive_ptr)
		return ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;

	ESIF_TRACE_DYN_PRIM("%s: PRIMITIVE Lookup Found in DSP (%s)\n",
		ESIF_FUNC, lp_ptr->dsp_ptr->get_code(lp_ptr->dsp_ptr));

	ESIF_TRACE_DYN_DECODE("  Primitive:            %s(%d)\n"
		"  Primitive Domain:     %s(%d)\n"
		"  Primitive Instance:   %d\n"
		"  Opcode:               %s(%d)\n"
		"  Actions:              %d\n",
		esif_primitive_str((enum esif_primitive_type)
					primitive_ptr->tuple.id),
		(int)primitive_ptr->tuple.id,
		esif_primitive_domain_str(primitive_ptr->tuple.domain, tmp),
		(int)primitive_ptr->tuple.domain,
		(int)primitive_ptr->tuple.instance,
		esif_primitive_opcode_str(primitive_ptr->opcode),
		(int)primitive_ptr->opcode,
		(int)primitive_ptr->action_count);

	/* Have Action Specified */
	if (NULL != action_index_ptr) {
		rc = esif_lf_primitive_get_and_execute_action(primitive_ptr,
							lp_ptr,
							req_data_ptr,
							rsp_data_ptr,
							(u8) *action_index_ptr);
		goto exit;
	}

	/* No action specified, so call actions one by one until one fires */
	for (action = 0; action < primitive_ptr->action_count; action++) {
		rc = esif_lf_primitive_get_and_execute_action(primitive_ptr,
							lp_ptr,
							req_data_ptr,
							rsp_data_ptr,
							action);
		if ((ESIF_OK == rc) ||
		    (ESIF_E_NEED_LARGER_BUFFER == rc) ||
		    (ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP == rc)) {
				break;
		}
	}

exit:
	if (primitive_ptr)
		esif_ccb_free(primitive_ptr);

	return rc;
}


/*
 * Simple helper function to execute a primitive that takes no special
 * parameters.
 */
enum esif_rc esif_get_simple_primitive(
	struct esif_lp *lp_ptr,
	u16 id,
	u16 domain,
	u16 instance,
	enum esif_data_type esif_type,
	void *buffer_ptr,
	u32 buffer_size
	)
{
	enum esif_rc rc = ESIF_OK;

	struct esif_primitive_tuple tuple_get = {id, domain, instance};
	struct esif_data esif_void = {ESIF_DATA_VOID, NULL, 0, 0};
	struct esif_data esif_return = {esif_type,
					buffer_ptr,
					buffer_size,
					buffer_size};

	if ((NULL == lp_ptr) || (NULL == buffer_ptr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	rc = esif_execute_primitive(lp_ptr,
				    &tuple_get,
				    &esif_void, &esif_return,
				    NULL);
exit:
	return rc;
}



/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
