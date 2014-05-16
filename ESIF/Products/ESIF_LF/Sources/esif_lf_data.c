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

#include "esif.h"

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
#define DATA_DEBUG       1

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_DATA, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_DATA(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_DATA, DATA_DEBUG, format, ##__VA_ARGS__)

/*
** Agnostic
*/

/* Allocate Data */
struct esif_data *esif_data_alloc(
	enum esif_data_type type,
	u32 data_len
	)
{
	struct esif_data *data_ptr = NULL;

	data_ptr = esif_ccb_mempool_zalloc(ESIF_MEMPOOL_TYPE_DATA);
	if (NULL == data_ptr)
		return NULL;

	data_ptr->type    = type;
	data_ptr->buf_len = data_len;

	if (data_len > 0) {
		data_ptr->buf_ptr = esif_ccb_malloc(data_len);
		if (NULL == data_ptr->buf_ptr) {
			esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_DATA, data_ptr);
			return NULL;
		}
	} else {
		ESIF_TRACE_DYN_DATA(
			"%s: 0 Length Requesed WARNING Caller Must Allocate "
			"Buffer\n",
			ESIF_FUNC);
	}
	ESIF_TRACE_DYN_DATA("%s: type = %d, buf_len = %d, buf_ptr = %p\n",
			    ESIF_FUNC,
			    data_ptr->type,
			    (int)data_ptr->buf_len,
			    data_ptr->buf_ptr);
	return data_ptr;
}


/* Free Data */
void esif_data_free(
	struct esif_data *data_ptr
	)
{
	if (NULL == data_ptr)
		return;

	/* Free Buffer If Any */
	if (NULL != data_ptr->buf_ptr)
		esif_ccb_free(data_ptr->buf_ptr);

	esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_DATA, data_ptr);
}


/* Init */
enum esif_rc esif_data_init(void)
{
	struct esif_ccb_mempool *mempool_ptr = NULL;
	ESIF_TRACE_DYN_DATA("%s: Initialize Data\n", ESIF_FUNC);

	mempool_ptr = esif_ccb_mempool_create(ESIF_MEMPOOL_TYPE_DATA,
					      ESIF_MEMPOOL_FW_DATA,
					      sizeof(struct esif_data));
	if (NULL == mempool_ptr)
		return ESIF_E_NO_MEMORY;

	return ESIF_OK;
}


/* Exit */
void esif_data_exit(void)
{
	ESIF_TRACE_DYN_DATA("%s: Exit Data\n", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
