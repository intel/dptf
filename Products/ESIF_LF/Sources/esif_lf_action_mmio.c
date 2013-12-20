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
#define SET_DEBUG        2

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_MMIO, \
		       INIT_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_GET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_MMIO, \
		       GET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)
#define ESIF_TRACE_DYN_SET(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_ACTION_MMIO, \
		       SET_DEBUG, \
		       format, \
		       ##__VA_ARGS__)

/* Lock */
static esif_ccb_lock_t g_esif_action_mmio_lock;

/* Get */
enum esif_rc esif_get_action_mmio(
	const void __iomem *base_addr,
	const u32 offset,
	const u8 bit_from,
	const u8 bit_to,
	const struct esif_data *req_data_ptr,
	struct esif_data *rsp_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	int i = 0;		/* Loop Counter                            */
	u32 val = 0;		/* Temporary MMIO Value MMIO Always 32 Bit */
	u32 bit_mask = 0;	/* Bit Mask For MMIO Value                 */

	UNREFERENCED_PARAMETER(req_data_ptr);
	ESIF_TRACE_DYN_GET("%s: base %p offset %x bit_from %d bit_to %d\n",
			   ESIF_FUNC, base_addr, offset, bit_from, bit_to);

	if (NULL == base_addr)
		return ESIF_E_NO_MMIO_SUPPORT;

	/* Read MMIO 32-Bit Always */
	esif_ccb_read_lock(&g_esif_action_mmio_lock);
	esif_ccb_mmio_read(base_addr, offset, &val);
	esif_ccb_read_unlock(&g_esif_action_mmio_lock);

	/* Mask Bits */
	for (bit_mask = 0, i = bit_from; i <= bit_to; i++)
		bit_mask |= (1 << i);

	val = val & bit_mask;
	val = val >> bit_from;

	switch (rsp_data_ptr->type) {
	case ESIF_DATA_UINT8:
		rsp_data_ptr->data_len = sizeof(u8);
		if (rsp_data_ptr->buf_len >= sizeof(u8))
			*((u8 *)rsp_data_ptr->buf_ptr) = (u8)val;
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_UINT16:
		rsp_data_ptr->data_len = sizeof(u16);
		if (rsp_data_ptr->buf_len >= sizeof(u16))
			*((u16 *)rsp_data_ptr->buf_ptr) = (u16)val;
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_UINT32:
		rsp_data_ptr->data_len = sizeof(u32);
		if (rsp_data_ptr->buf_len >= sizeof(u32))
			*((u32 *)rsp_data_ptr->buf_ptr) = (u32)val;
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	default:
		rc = ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
		break;
	}
	return rc;
}


/* Set */
enum esif_rc esif_set_action_mmio(
	const void __iomem *base_addr,
	const u32 offset,
	const u8 bit_from,
	const u8 bit_to,
	const struct esif_data *req_data_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	int i = 0;	        /* Loop Counter                            */
	u32 req_val     = 0;	/* Request MMIO Value                      */
	u32 orig_val    = 0;	/* Original Value Of MMIO                  */
	u32 bit_mask    = 0;	/* Bit Mask                                */

	if (NULL == base_addr)
		return ESIF_E_NO_MMIO_SUPPORT;

	ESIF_TRACE_DYN_SET(
		"%s: req type %s, mmio base %p, offset 0x%x, bit_from %d, "
		"bit_to %d\n",
		ESIF_FUNC,
		esif_data_type_str(req_data_ptr->type),
		base_addr,
		offset,
		bit_from,
		bit_to);

	switch (req_data_ptr->type) {
	case ESIF_DATA_UINT8:
		if (req_data_ptr->buf_len >= sizeof(u8))
			req_val = *((u8 *)req_data_ptr->buf_ptr);
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_UINT16:
		if (req_data_ptr->buf_len >= sizeof(u16))
			req_val = *((u16 *)req_data_ptr->buf_ptr);
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	case ESIF_DATA_TEMPERATURE:
	case ESIF_DATA_UINT32:
		if (req_data_ptr->buf_len >= sizeof(u32))
			req_val = *((u32 *)req_data_ptr->buf_ptr);
		else
			rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
		break;

	default:
		rc = ESIF_E_UNSUPPORTED_REQUEST_DATA_TYPE;
		break;
	}

	/* Read The Current Value Of The MMIO */
	esif_ccb_write_lock(&g_esif_action_mmio_lock);
	esif_ccb_mmio_read(base_addr, offset, &orig_val);

	/* Mask Bits */
	for (bit_mask = 0UL, i = bit_from; i <= bit_to; i++)
		bit_mask |= (1UL << i);

	/* Shift Bits */
	orig_val &= ~(bit_mask);

	/* Get the New Value */
	req_val = (req_val << bit_from) | orig_val;

	/* Write MMIO 32-Bit Always */
	esif_ccb_mmio_write(base_addr, offset, req_val);
	esif_ccb_write_unlock(&g_esif_action_mmio_lock);

	return rc;
}


/* Init */
enum esif_rc esif_action_mmio_init(void)
{
	ESIF_TRACE_DYN_INIT("%s: Initialize MMIO Action\n", ESIF_FUNC);
	esif_ccb_lock_init(&g_esif_action_mmio_lock);
	return ESIF_OK;
}


/* Exit */
void esif_action_mmio_exit(void)
{
	esif_ccb_lock_uninit(&g_esif_action_mmio_lock);
	ESIF_TRACE_DYN_INIT("%s: Exit MMIO Action\n", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
