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
#include "esif_cpc.h"
#include "esif_domain.h"

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 *
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified
 * against Windows SDK/DDK included headers which we have no control over.
 *
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Debug Logging Defintions */
#define INIT_DEBUG       0
#define CPC_DEBUG        1

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_CPC, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_CPC(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_CPC, CPC_DEBUG, format, ##__VA_ARGS__)

/*
 ******************************************************************************
 * PRIVATE
 ******************************************************************************
 */

/* Assumes hash table has already been created */
static enum esif_rc esif_cpc_to_dsp(struct esif_lp_dsp *dsp_ptr)
{
	struct esif_lp_cpc *cpc_ptr = dsp_ptr->cpc_ptr;
	struct esif_cpc_primitive *primitive_ptr = NULL;
	struct esif_cpc_algorithm *algorithm_ptr;
	struct esif_cpc_event *event_ptr;
	struct domain *domain_ptr;
	enum esif_rc rc = ESIF_OK;
	u32 i;
	u8 *base_ptr;
	u64 offset;

	base_ptr = (u8 *)cpc_ptr;
	dsp_ptr->code_ptr         = (esif_string)cpc_ptr->header.code;

	dsp_ptr->domain_count_ptr = (u8 *)&cpc_ptr->number_of_domains;
	dsp_ptr->ver_major_ptr    = (u8 *)&cpc_ptr->header.ver_major;
	dsp_ptr->ver_minor_ptr    = (u8 *)&cpc_ptr->header.ver_minor;
	dsp_ptr->capability_ptr   = (esif_flags_t *)
					&cpc_ptr->header.cpc.capability;

	ESIF_TRACE_DYN_CPC(
		"%s: <cpc @ %p> CPC name '%s' size %d num_primitives %u, num_algorithms %u, num_domains %u\n",
		ESIF_FUNC,
		cpc_ptr,
		cpc_ptr->header.code,
		cpc_ptr->size,
		cpc_ptr->number_of_basic_primitives,
		cpc_ptr->number_of_algorithms,
		*dsp_ptr->domain_count_ptr);

	/* 1. Insert All Primitives Into Hash */
	/* Locate First Primitive Laid After CPC Struct */
	primitive_ptr =
		(struct esif_cpc_primitive *)((u8 *)cpc_ptr +
					     sizeof(struct esif_lp_cpc));
	for (i = 0; i < cpc_ptr->number_of_basic_primitives; i++) {
		offset = (u64)((u8 *)primitive_ptr - base_ptr);
		ESIF_TRACE_DYN_CPC(
			"<%06llu> Primitive[%03d]: size %03d tuple <%03u %03u %03u> operation %u(%s) num_actions %d\n",
				   offset,
				   i,
				   primitive_ptr->size,
				   primitive_ptr->tuple.id,
				   primitive_ptr->tuple.domain,
				   primitive_ptr->tuple.instance,
				   primitive_ptr->operation,
				   esif_primitive_opcode_str(
					   (enum esif_primitive_opcode)
					   primitive_ptr->operation),
				   primitive_ptr->number_of_actions);

		/* Must Be ESIF Primitive Not CPC Primitive Format */
		rc = dsp_ptr->insert_primitive(dsp_ptr, primitive_ptr);
		if (ESIF_OK != rc)
			goto exit;

		/* Primitives Have Varaible Length Due To Number Of Actions */
		primitive_ptr =
			(struct esif_cpc_primitive *)((u8 *)primitive_ptr +
						     primitive_ptr->size);
	}

	/* 2. Insert All Algorithms Into Linked List */
	/* First Algorithm Laid After the Last Primitive */
	algorithm_ptr = (struct esif_cpc_algorithm *)primitive_ptr;
	for (i = 0; i < cpc_ptr->number_of_algorithms; i++) {
		offset = (u64)((u8 *)algorithm_ptr - base_ptr);
		ESIF_TRACE_DYN_CPC("<%06llu> Algorithm[%3d]: action_type %u(%s) temp_xform %u tempC1 %u tempC2 %u size %u\n",
				   offset,
				   i,
				   algorithm_ptr->action_type,
				   esif_action_type_str(
					   (enum esif_action_type)algorithm_ptr
					   ->action_type),
				   algorithm_ptr->temp_xform,
				   algorithm_ptr->tempC1,
				   algorithm_ptr->tempC2,
				   algorithm_ptr->size);

		rc = dsp_ptr->insert_algorithm(dsp_ptr, algorithm_ptr);
		if (ESIF_OK != rc)
			goto exit;

		/* Next Algorithm. Algorithms Are Fix-Sized */
		algorithm_ptr = (struct esif_cpc_algorithm *)
		       ((u8 *)algorithm_ptr + sizeof(struct esif_cpc_algorithm));
	}

	/* Truncate any unsupported domain conunts */
	if (*dsp_ptr->domain_count_ptr > ESIF_DOMAIN_MAX)
		*dsp_ptr->domain_count_ptr = ESIF_DOMAIN_MAX;

	/* Domains are contained in an array one entry per domain */
	if (*dsp_ptr->domain_count_ptr > 0) {
		dsp_ptr->domains_ptr = (struct domain *)algorithm_ptr;
		ESIF_TRACE_DYN_CPC("%s: First DWORD %08x\n",
				   ESIF_FUNC,
				   *(u32 *)dsp_ptr->domains_ptr);
	}

	/* Domain */
	domain_ptr = (struct domain *)algorithm_ptr;
	for (i = 0; i < cpc_ptr->number_of_domains; i++) {
		offset = (u64)((u8 *)domain_ptr - base_ptr);
		ESIF_TRACE_DYN_CPC("<%06llu> domain size %d name %s\n",
				   offset,
				   domain_ptr->size,
				   domain_ptr->descriptor.name);
		domain_ptr =
			(struct domain *)((u8 *)domain_ptr + domain_ptr->size);
	}

	/* Event */
	event_ptr = (struct esif_cpc_event *)domain_ptr;
	for (i = 0; i < cpc_ptr->number_of_events; i++) {
		offset = (u64)((u8 *)event_ptr - base_ptr);
		ESIF_TRACE_DYN_CPC(
			"<%06llu> name %s notify %x esif_event %s(%d) esif_event_group TBD(%d)\n",
			offset,
			event_ptr->name,
			event_ptr->event_key[i],
			esif_event_type_str(event_ptr->esif_event),
			event_ptr->esif_event,
			event_ptr->esif_group);

		/* Todo Handle Other Types Here */
		if (ESIF_EVENT_GROUP_ACPI == event_ptr->esif_group) {
			rc = dsp_ptr->insert_event(dsp_ptr, event_ptr);
			if (ESIF_OK != rc)
				goto exit;
		}
		/* Next Event. Events Are Fix-Sized */
		event_ptr = (struct esif_cpc_event *)
			((u8 *)event_ptr + sizeof(struct esif_cpc_event));
	}

exit:

	ESIF_TRACE_DYN_CPC(
		"%s: %u primitives, %u algorithms and %u events inserted, status %s! %u domain found\n",
		ESIF_FUNC,
		cpc_ptr->number_of_basic_primitives,
		cpc_ptr->number_of_algorithms,
		cpc_ptr->number_of_events,
		esif_rc_str(rc),
		cpc_ptr->number_of_domains);

	return rc;
}


/*
 ******************************************************************************
 * PUBLIC
 ******************************************************************************
 */

/* CPC Unpack */
enum esif_rc esif_cpc_unpack(
	struct esif_lp_dsp *dsp_ptr,
	const struct esif_data *req_data_ptr
	)
{
	struct esif_lp_cpc *cpc_ptr = NULL;
	struct esif_cpc_header *cpc_header_ptr = NULL;

	ESIF_TRACE_DYN_CPC("%s: START\n", ESIF_FUNC);
	ESIF_TRACE_DYN_CPC("%s: dsp %p req_data %p cpc %p size %d\n",
			   ESIF_FUNC,
			   dsp_ptr,
			   req_data_ptr,
			   req_data_ptr->buf_ptr,
			   req_data_ptr->buf_len);

	/* Validate Thourougly Here */
	if (req_data_ptr->type != ESIF_DATA_DSP)
		return ESIF_E_INVALID_REQUEST_TYPE;

	if (req_data_ptr->buf_len < sizeof(struct esif_cpc_header))
		return ESIF_E_CPC_SHORT;

	cpc_ptr        = (struct esif_lp_cpc *)req_data_ptr->buf_ptr;
	cpc_header_ptr = (struct esif_cpc_header *)&cpc_ptr->header;

	ESIF_TRACE_DYN_CPC(
		"%s: code %s ver %x (%x,%x), signature %08x, version %x\n",
		ESIF_FUNC,
		cpc_header_ptr->code,
		cpc_header_ptr->version,
		cpc_header_ptr->ver_major,
		cpc_header_ptr->ver_minor,
		cpc_header_ptr->cpc.signature,
		cpc_header_ptr->cpc.version);

	if (cpc_header_ptr->cpc.signature != *(unsigned int *)"@CPC")
		return ESIF_E_CPC_SIGNATURE;

	/* Make a copy for now */
	cpc_ptr = (struct esif_lp_cpc *)esif_ccb_malloc(req_data_ptr->buf_len);
	if (NULL != cpc_ptr) {
		esif_ccb_memcpy(cpc_ptr,
				req_data_ptr->buf_ptr,
				req_data_ptr->buf_len);

		dsp_ptr->cpc_ptr = cpc_ptr;
		esif_cpc_to_dsp(dsp_ptr);
	}
	ESIF_TRACE_DYN_CPC(" %s: STOP\n", ESIF_FUNC);
	return ESIF_OK;
}


/* CPC Free */
void esif_cpc_free(struct esif_lp_dsp *dsp_ptr)
{
	ESIF_TRACE_DYN_CPC("%s: START\n", ESIF_FUNC);

	if (NULL == dsp_ptr || NULL == dsp_ptr->cpc_ptr)
		return;

	esif_ccb_free(dsp_ptr->cpc_ptr);
	dsp_ptr->cpc_ptr = NULL;

	ESIF_TRACE_DYN_CPC("%s: STOP\n", ESIF_FUNC);
}


/* Init */
enum esif_rc esif_cpc_init(void)
{
	ESIF_TRACE_DYN_INIT("%s: Initialize Compact Primitive Catalog(CPC)\n",
			    ESIF_FUNC);
	return ESIF_OK;
}


/* Exit */
void esif_cpc_exit(void)
{
	ESIF_TRACE_DYN_INIT("%s: UnInitialize Compact Primitive Catalog(CPC)\n",
			    ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

