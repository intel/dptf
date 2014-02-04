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

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_CONST, \
		       INIT_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_GET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_CONST, \
		       GET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)

enum esif_rc esif_get_action_const(
	const u32 val,
	const struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;

	UNREFERENCED_PARAMETER(req_data_ptr);
	ESIF_TRACE_DYN_GET("%s: type = %d, value = %d\n", ESIF_FUNC,
			   rsp_data_ptr->type, val);
	/*
	 * Const items are kept as 32-bit values; however, the call may be for
	 * a different size depending on type.  Return data as the largest
	 * type the buffer passed in can accept.
	 */
	if (rsp_data_ptr->buf_len >= sizeof(u64)) {
		rsp_data_ptr->data_len = sizeof(u64);
		*((u64 *)rsp_data_ptr->buf_ptr) = (u64)val;

	} else if (rsp_data_ptr->buf_len >= sizeof(u32)) {
		rsp_data_ptr->data_len = sizeof(u32);
		*((u32 *)rsp_data_ptr->buf_ptr) = (u32)val;

	} else if (rsp_data_ptr->buf_len >= sizeof(u16)) {
		rsp_data_ptr->data_len = sizeof(u16);
		*((u16 *)rsp_data_ptr->buf_ptr) = (u16)val;

	} else if (rsp_data_ptr->buf_len >= sizeof(u8)) {
		rsp_data_ptr->data_len = sizeof(u8);
		*((u8 *)rsp_data_ptr->buf_ptr) = (u8)val;

	} else {
		/* Should only happen if buffer len is 0 */
		ESIF_TRACE_DYN_GET("Buffer insufficient for result = %d\n",
				   rsp_data_ptr->buf_len);
		rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
	}

	return rc;
}


/* Init */
enum esif_rc esif_action_const_init(void)
{
	ESIF_TRACE_DYN_INIT("%s: Initialize CONST Action\n", ESIF_FUNC);
	return ESIF_OK;
}


/* Exit */
void esif_action_const_exit(void)
{
	ESIF_TRACE_DYN_INIT("%s: Exit CONST Action\n", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
