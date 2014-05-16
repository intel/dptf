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
#include "esif_primitive.h"
#include "esif_participant.h"

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified against Windows SDK/DDK included headers which we
 * have no control over.
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Debug Logging Defintions */
#define INIT_DEBUG       0
#define GET_DEBUG        1
#define SET_DEBUG        2

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_VAR, \
		       INIT_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_GET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_VAR, \
		       GET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_SET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_VAR, \
		       SET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)

/* Get Action VAR */
enum esif_rc esif_get_action_var(
	const struct esif_lp *lp_ptr,
	const struct esif_lp_primitive *primitive_ptr,
	const struct esif_lp_action *action_ptr,
	struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u32 val;
	struct esif_data local = {ESIF_DATA_UINT32, &val, sizeof(val)};
	struct esif_data *var_ptr  = NULL;

	UNREFERENCED_PARAMETER(lp_ptr);
	UNREFERENCED_PARAMETER(req_data_ptr);

	if ((NULL == primitive_ptr) || (NULL == action_ptr) ||
	    (NULL == rsp_data_ptr)) {
		    rc = ESIF_E_PARAMETER_IS_NULL;
		    goto exit;
	}

	val = action_ptr->get_p1_u32(action_ptr);

	/* Use Context Variable If Available */
	if (primitive_ptr->context_ptr != NULL)
		var_ptr = primitive_ptr->context_ptr;
	else
		var_ptr = &local;

	if (NULL == var_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (rsp_data_ptr->buf_len < var_ptr->buf_len) {
		rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		goto exit;
	}

	rsp_data_ptr->type     = var_ptr->type;
	rsp_data_ptr->data_len = var_ptr->buf_len;

	esif_ccb_memcpy(rsp_data_ptr->buf_ptr,
			var_ptr->buf_ptr,
			var_ptr->buf_len);

	ESIF_TRACE_DYN_GET("Value 0x%x, buf_type - %s, data_len - %d\n",
			   val, esif_data_type_str(rsp_data_ptr->type),
			   rsp_data_ptr->data_len);

exit:
	ESIF_TRACE_DYN_GET("RC: %s(%d)\n", esif_rc_str(rc), rc);
	return rc;
}


/* Set Action VAR */
enum esif_rc esif_set_action_var(
	const struct esif_lp *lp_ptr,
	struct esif_lp_primitive *primitive_ptr,
	const struct esif_lp_action *action_ptr,
	struct esif_data *req_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_data *var_ptr = NULL;
	struct esif_primitive_tuple tup;
	struct esif_lp_primitive *p = NULL;

	if ((NULL == lp_ptr) || (NULL == primitive_ptr) ||
	    (NULL == action_ptr) || (NULL == req_data_ptr) ||
	    (NULL == lp_ptr->dsp_ptr)) {
		    rc = ESIF_E_PARAMETER_IS_NULL;
		    goto exit;
	}

	/* Create a State Variable To Hold Our Context */
	if (NULL == primitive_ptr->context_ptr) {
		primitive_ptr->context_ptr = esif_data_alloc(req_data_ptr->type,
							req_data_ptr->buf_len);
	}

	var_ptr = primitive_ptr->context_ptr;
	if (NULL == var_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (req_data_ptr->buf_len > var_ptr->buf_len) {
		rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		goto exit;
	}

	var_ptr->type    = req_data_ptr->type;
	var_ptr->buf_len = req_data_ptr->buf_len;
	esif_ccb_memcpy(var_ptr->buf_ptr,
			req_data_ptr->buf_ptr,
			req_data_ptr->buf_len);

	ESIF_TRACE_DYN_SET("buf_type - %s, data_len - %d\n",
			   esif_data_type_str(req_data_ptr->type),
			   req_data_ptr->data_len);

	/* Find Get To Go With This Set Relationship */
	tup.id       = (u16)action_ptr->get_p1_u32( action_ptr);
	tup.domain   = (u16)action_ptr->get_p2_u32( action_ptr);
	tup.instance = (u8)action_ptr->get_p3_u32( action_ptr);

	ESIF_TRACE_DYN_SET("Find Get For Set, data %d.%d.%d\n",
			tup.id,
			tup.domain,
			tup.instance);

	p = lp_ptr->dsp_ptr->get_primitive(lp_ptr->dsp_ptr, &tup);
	p->context_ptr = primitive_ptr->context_ptr;

exit:
	ESIF_TRACE_DYN_SET("RC: %s(%d)\n", esif_rc_str(rc), rc);
	return rc;
}


/* Init */
enum esif_rc esif_action_var_init(void)
{
	ESIF_TRACE_DYN_INIT("Initialize VAR Action\n");
	return ESIF_OK;
}


/* Exit */
void esif_action_var_exit(void)
{
	ESIF_TRACE_DYN_INIT("Exit VAR Action\n");
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

