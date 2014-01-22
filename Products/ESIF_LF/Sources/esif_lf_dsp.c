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

#include "esif_lf.h"
#include "esif_dsp.h"
#include "esif_hash_table.h"
#include "esif_action.h"
#include "esif_participant.h"
#include "esif_pm.h"

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
#define DSP_DEBUG        1

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_DSP, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_DSP(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_DSP, DSP_DEBUG, format, ##__VA_ARGS__)

/*
 *****************************************************************************
 * PRIVATE
 *****************************************************************************
 */

/* Insert Primitive Into Hash */
static enum esif_rc insert_primitive(
	const struct esif_lp_dsp *dsp_ptr,
	struct esif_cpc_primitive *primitive_ptr
	)
{
	if (NULL == primitive_ptr)
		return ESIF_E_PARAMETER_IS_NULL;

	return esif_hash_table_put_item((u8 *)&primitive_ptr->tuple,/* Key */
					sizeof(struct esif_primitive_tuple),
					primitive_ptr,
					dsp_ptr->ht_ptr);
}


/* Inspect Tuple */
static int is_same_primitive(
	const struct esif_primitive_tuple *t1_ptr,
	const struct esif_primitive_tuple *t2_ptr
	)
{
	if (NULL == t1_ptr || NULL == t2_ptr)
		return 0;

	ESIF_TRACE_DYN_DSP("%s: tuple 1 %d.%d.%d tuple 2 %d.%d.%d\n",
			   ESIF_FUNC,
			   t1_ptr->id, t1_ptr->domain, t1_ptr->instance,
			   t2_ptr->id, t2_ptr->domain, t2_ptr->instance);

	return memcmp(t1_ptr, t2_ptr, sizeof(struct esif_primitive_tuple)) == 0;
}


/* Find Primitive In List */
static struct esif_cpc_primitive *find_primitive_in_list(
	struct esif_link_list *list_ptr,
	const struct esif_primitive_tuple *tuple_ptr
	)
{
	struct esif_cpc_primitive *cur_primitive_ptr = NULL;
	struct esif_cpc_primitive *primitive_ptr     = NULL;
	struct esif_link_list_node *curr_ptr         = list_ptr->head_ptr;

	ESIF_TRACE_DYN_DSP("%s: number of linked list nodes %d\n",
			   ESIF_FUNC,
			   list_ptr->nodes);
	while (curr_ptr) {
		cur_primitive_ptr = (struct esif_cpc_primitive *)
			curr_ptr->data_ptr;
		if (cur_primitive_ptr != NULL) {
			if (is_same_primitive(tuple_ptr,
					      &cur_primitive_ptr->tuple)) {
				primitive_ptr = cur_primitive_ptr;
				goto end;
			}
		}
		curr_ptr = curr_ptr->next_ptr;
	}
end:
	return primitive_ptr;
}


/* Get Primitive */
static struct esif_lp_primitive *get_primitive(
	const struct esif_lp_dsp *dsp_ptr,
	const struct esif_primitive_tuple *tuple_ptr
	)
{
	struct esif_cpc_primitive *cpc_primitive_ptr = NULL;
	struct esif_lp_primitive *primitive_ptr      = NULL;
	struct esif_link_list *row_ptr =
			esif_hash_table_get_item(
					 (u8 *)tuple_ptr,
					 sizeof(struct esif_primitive_tuple),
					 dsp_ptr->ht_ptr);

	if (NULL == row_ptr)
		return NULL;

	ESIF_TRACE_DYN_DSP("%s: tuple %d.%d.%d row %p\n",
			   ESIF_FUNC,
			   tuple_ptr->id,
			   tuple_ptr->domain,
			   tuple_ptr->instance,
			   row_ptr);

	cpc_primitive_ptr = find_primitive_in_list(row_ptr, tuple_ptr);
	if (NULL == cpc_primitive_ptr)
		return NULL;

	ESIF_TRACE_DYN_DSP("%s: have cpc primtive %p\n",
			   ESIF_FUNC,
			   cpc_primitive_ptr);

	/*
	 * Okay now we have a CPC primitive pointer convert to standard ESIF
	 * primitive pointer and return.  Note we can optimize this for these to
	 * be the same in the future.
	 */
	ESIF_TRACE_DYN_DSP(
		"%s: have cpc primtive convert to esif primtive %p\n",
		ESIF_FUNC,
		cpc_primitive_ptr);

	primitive_ptr = (struct esif_lp_primitive *)
		esif_ccb_malloc(sizeof(*primitive_ptr));
	if (NULL == primitive_ptr)
		return NULL;

	esif_ccb_memcpy((void *)&primitive_ptr->tuple,
			&cpc_primitive_ptr->tuple,
			sizeof(struct esif_primitive_tuple));

	primitive_ptr->opcode = (enum esif_primitive_opcode)
				cpc_primitive_ptr->operation;
	primitive_ptr->action_count = (u8)cpc_primitive_ptr->number_of_actions;
	return primitive_ptr;
}


/* Insert Algorithm Into Link List */
static enum esif_rc insert_algorithm(
	const struct esif_lp_dsp *dsp_ptr,
	struct esif_cpc_algorithm *algo_ptr
	)
{
	if (NULL == algo_ptr)
		return ESIF_E_PARAMETER_IS_NULL;

	esif_link_list_node_add(dsp_ptr->algo_ptr,
				esif_link_list_create_node((void *)algo_ptr));
	return ESIF_OK;
}


/* Get Algorithm Of An Action From Linked List */
struct esif_cpc_algorithm *get_algorithm(
	const struct esif_lp_dsp *dsp_ptr,
	const enum esif_action_type action_type
	)
{
	struct esif_link_list *list_ptr = dsp_ptr->algo_ptr;
	struct esif_link_list_node *curr_ptr     = list_ptr->head_ptr;
	struct esif_cpc_algorithm *curr_algo_ptr = NULL;

	while (curr_ptr) {
		curr_algo_ptr = (struct esif_cpc_algorithm *)curr_ptr->data_ptr;
		if (curr_algo_ptr != NULL) {
			if (curr_algo_ptr->action_type == action_type)
				return curr_algo_ptr;
		}
		curr_ptr = curr_ptr->next_ptr;
	}
	return NULL;
}

/* Has This Algorithm In DSP From Linked List Or Not */
static u32 dsp_has_algorithm(
	const struct esif_lp_dsp *dsp_ptr,
	const enum esif_algorithm_type algo)
{
	struct esif_link_list *list_ptr = dsp_ptr->algo_ptr;
	struct esif_link_list_node *curr_ptr = list_ptr->head_ptr;
	struct esif_cpc_algorithm *curr_algo_ptr = NULL;

	while (curr_ptr) {
		curr_algo_ptr = (struct esif_cpc_algorithm *)curr_ptr->data_ptr;
		if (curr_algo_ptr != NULL) {
			if ((enum esif_algorithm_type) curr_algo_ptr->temp_xform == algo || 
			    (enum esif_algorithm_type) curr_algo_ptr->power_xform == algo ||
			    (enum esif_algorithm_type) curr_algo_ptr->time_xform == algo ) 
				return ESIF_TRUE;
		}
		curr_ptr = curr_ptr->next_ptr;
	}
	return ESIF_FALSE; 
}

/* Insert Events Into Link List */
static enum esif_rc insert_event(
	const struct esif_lp_dsp *dsp_ptr,
	struct esif_cpc_event *evt_ptr
	)
{
	if (NULL == evt_ptr)
		return ESIF_E_PARAMETER_IS_NULL;

	esif_link_list_node_add(dsp_ptr->evt_ptr,
				esif_link_list_create_node((void *)evt_ptr));
	return ESIF_OK;
}


/* Get Event From Linked List */
struct esif_cpc_event *get_event(
	const struct esif_lp_dsp *dsp_ptr,
	u32 notify_id
	)
{
	struct esif_link_list *list_ptr      = dsp_ptr->evt_ptr;
	struct esif_link_list_node *curr_ptr = list_ptr->head_ptr;
	struct esif_cpc_event *curr_evt_ptr  = NULL;

	while (curr_ptr) {
		curr_evt_ptr = (struct esif_cpc_event *)curr_ptr->data_ptr;
		if (curr_evt_ptr != NULL) {
			if ((u8)curr_evt_ptr->event_key[0] == notify_id)
				return curr_evt_ptr;
		}
		curr_ptr = curr_ptr->next_ptr;
	}
	return NULL;
}


/* DSP Interface */
static esif_string get_code(const struct esif_lp_dsp *dsp_ptr)
{
	if (dsp_ptr->code_ptr)
		return dsp_ptr->code_ptr;
	else
		return ESIF_NOT_AVAILABLE;
}


static u8 get_domain_count(const struct esif_lp_dsp *dsp_ptr)
{
	if (dsp_ptr->domain_count_ptr)
		return *dsp_ptr->domain_count_ptr;
	else
		return 0;
}


static u8 get_ver_major(const struct esif_lp_dsp *dsp_ptr)
{
	if (dsp_ptr->ver_major_ptr)
		return *dsp_ptr->ver_major_ptr;
	else
		return 0;
}


static u8 get_ver_minor(const struct esif_lp_dsp *dsp_ptr)
{
	if (dsp_ptr->ver_minor_ptr)
		return *dsp_ptr->ver_minor_ptr;
	else
		return 0;
}


/* DSP Domain Interface */
static enum esif_domain_type get_domain_type(
	const struct esif_lp_dsp *dsp_ptr,
	u8 domain_index
	)
{
	if (dsp_ptr->domains_ptr)
		return (dsp_ptr->domains_ptr + domain_index)->descriptor.domainType;
	else
		return 0x0;
}


static esif_flags_t get_domain_capability(
	const struct esif_lp_dsp *dsp_ptr,
	u8 domain_index
	)
{
	if (dsp_ptr->domains_ptr)
		return (dsp_ptr->domains_ptr +
			domain_index)->capability_for_domain.capability_flags;
	else
		return 0x0;
}


static u16 get_domain_id(
	const struct esif_lp_dsp *dsp_ptr,
	u8 domain_index
	)
{
	if (dsp_ptr->domains_ptr) {
		return (dsp_ptr->domains_ptr +
			domain_index)->descriptor.qualifier;
	} else {
		return 0x0;
	}
}


static esif_string get_domain_desc(
	const struct esif_lp_dsp *dsp_ptr,
	u8 domain_index
	)
{
	if (dsp_ptr->domains_ptr)
		return (dsp_ptr->domains_ptr + domain_index)->descriptor.description;
	else
		return ESIF_NOT_AVAILABLE;
}


static esif_string get_domain_name(
	const struct esif_lp_dsp *dsp_ptr,
	u8 domain_index
	)
{
	if (dsp_ptr->domains_ptr)
		return (dsp_ptr->domains_ptr + domain_index)->descriptor.name;
	else
		return ESIF_NOT_AVAILABLE;
}


static enum esif_action_type get_type(const struct esif_lp_action *action_ptr)
{
	return action_ptr->type;
}

static u32 get_p1_u32(const struct esif_lp_action *action_ptr)
{
	if (action_ptr->p1_ptr)
		return *action_ptr->p1_ptr;
	else
		return 0;
}


static u32 get_p2_u32(const struct esif_lp_action *action_ptr)
{
	if (action_ptr->p2_ptr)
		return *action_ptr->p2_ptr;
	else
		return 0;
}


static u32 get_p3_u32(const struct esif_lp_action *action_ptr)
{
	if (action_ptr->p3_ptr)
		return *action_ptr->p3_ptr;
	else
		return 0;
}


static u32 get_p4_u32(const struct esif_lp_action *action_ptr)
{
	if (action_ptr->p4_ptr)
		return *action_ptr->p4_ptr;
	else
		return 0;
}


static u32 get_p5_u32(const struct esif_lp_action *action_ptr)
{
	if (action_ptr->p5_ptr)
		return *action_ptr->p5_ptr;
	else
		return 0;
}


/* Temp Constant 1 */
static u32 get_temp_c1(
	const struct esif_lp_dsp *dsp_ptr,
	const enum esif_action_type action
	)
{
	struct esif_cpc_algorithm *algo_ptr = NULL;

	algo_ptr = dsp_ptr->get_algorithm(dsp_ptr, action);
	if (NULL != algo_ptr)
		return algo_ptr->tempC1;
	else
		return 0xffffffff;
}


/* Temp Constant 2 */
static u32 get_temp_c2(
	const struct esif_lp_dsp *dsp_ptr,
	const enum esif_action_type action
	)
{
	/* TODO: Assume tempC2 Is Slope */
	struct esif_cpc_algorithm *algo_ptr = NULL;

	algo_ptr = dsp_ptr->get_algorithm(dsp_ptr, action);
	if (NULL != algo_ptr)
		return algo_ptr->tempC2;
	else
		return 0xffffffff;
}


/* Get Action  Will Change For CPC */
static struct esif_lp_action *get_action(
	const struct esif_lp_dsp *dsp_ptr,
	struct esif_lp_primitive *primitive_ptr,
	u8 index
	)
{
	struct esif_cpc_primitive *cpc_primitive_ptr = NULL;
	struct esif_cpc_action *cpc_action_ptr       = NULL;
	struct esif_lp_action *action_ptr = NULL;
	struct esif_link_list *row_ptr    = NULL;

	if (NULL == dsp_ptr || NULL == primitive_ptr)
		return NULL;

	ESIF_TRACE_DYN_DSP("%s: dsp %p primitive %p index %d\n",
			   ESIF_FUNC, dsp_ptr, primitive_ptr, index);

	/*
	* Find The Original CPC Primtive it contains our actions layed out
	* below it
	*/
	row_ptr = esif_hash_table_get_item((u8 *)&primitive_ptr->tuple,
					   sizeof(struct esif_primitive_tuple),
					   dsp_ptr->ht_ptr);

	if (NULL == row_ptr)
		return NULL;

	ESIF_TRACE_DYN_DSP("%s: tuple %d.%d.%d row %p\n",
			   ESIF_FUNC,
			   primitive_ptr->tuple.id,
			   primitive_ptr->tuple.domain,
			   primitive_ptr->tuple.instance,
			   row_ptr);

	cpc_primitive_ptr = find_primitive_in_list(row_ptr,
						   &primitive_ptr->tuple);

	if (NULL == cpc_primitive_ptr)
		return NULL;

	ESIF_TRACE_DYN_DSP("%s: have cpc primtive %p\n",
			   ESIF_FUNC,
			   cpc_primitive_ptr);

	cpc_action_ptr  = (struct esif_cpc_action *)
		((u8 *)cpc_primitive_ptr + sizeof(struct esif_cpc_primitive));
	cpc_action_ptr += index;

	ESIF_TRACE_DYN_DSP(
		"%s: type %d num_valid_param %d param_valid %x:%x:%x:%x:%x p1 0x%08x p2 0x%08x p3 0x%08x p4 0x%08x p5 %08x\n",
		ESIF_FUNC,
		cpc_action_ptr->type,
		(u32)cpc_action_ptr->num_valid_params,
		(u32)cpc_action_ptr->param_valid[0],
		(u32)cpc_action_ptr->param_valid[1],
		(u32)cpc_action_ptr->param_valid[2],
		(u32)cpc_action_ptr->param_valid[3],
		(u32)cpc_action_ptr->param_valid[4],
		cpc_action_ptr->param[0],
		cpc_action_ptr->param[1],
		cpc_action_ptr->param[2],
		cpc_action_ptr->param[3],
		cpc_action_ptr->param[4]);

	/* Now Convert from a CPC to an ESIF Action */
	action_ptr = (struct esif_lp_action *)esif_ccb_malloc(
						sizeof(*action_ptr));
	if (NULL == action_ptr)
		return NULL;

	action_ptr->type       = (enum esif_action_type)cpc_action_ptr->type;
	action_ptr->p1_ptr     = &cpc_action_ptr->param[0];
	action_ptr->p2_ptr     = &cpc_action_ptr->param[1];
	action_ptr->p3_ptr     = &cpc_action_ptr->param[2];
	action_ptr->p4_ptr     = &cpc_action_ptr->param[3];
	action_ptr->p5_ptr     = &cpc_action_ptr->param[4];

	action_ptr->get_type   = get_type;
	action_ptr->get_p1_u32 = get_p1_u32;
	action_ptr->get_p2_u32 = get_p2_u32;
	action_ptr->get_p3_u32 = get_p3_u32;
	action_ptr->get_p4_u32 = get_p4_u32;
	action_ptr->get_p5_u32 = get_p5_u32;

	ESIF_TRACE_DYN_DSP("%s: HAVE Action %d p1 %p p2 %p p3 %p p4 %p p5 %p\n",
			   ESIF_FUNC,
			   get_type(action_ptr),
			   action_ptr->p1_ptr,
			   action_ptr->p2_ptr,
			   action_ptr->p3_ptr,
			   action_ptr->p4_ptr,
			   action_ptr->p5_ptr);

	return action_ptr;
}


/* Allocate DSP */
static struct esif_lp_dsp *esif_dsp_alloc(void)
{
	struct esif_lp_dsp *dsp_ptr = NULL;

	dsp_ptr = (struct esif_lp_dsp *)
			esif_ccb_mempool_zalloc(ESIF_MEMPOOL_TYPE_DSP);
	if (NULL == dsp_ptr)
		return NULL;

	/* Assign Function Pointers */
	dsp_ptr->get_code         = get_code;
	dsp_ptr->get_ver_minor    = get_ver_minor;
	dsp_ptr->get_ver_major    = get_ver_major;
	dsp_ptr->get_temp_tc1     = get_temp_c1;
	dsp_ptr->get_temp_tc2     = get_temp_c2;
	dsp_ptr->get_domain_count = get_domain_count;

	/* Domains */
	dsp_ptr->get_domain_id         = get_domain_id;
	dsp_ptr->get_domain_capability = get_domain_capability;
	dsp_ptr->get_domain_desc       = get_domain_desc;
	dsp_ptr->get_domain_name       = get_domain_name;
	dsp_ptr->get_domain_type       = get_domain_type;

	dsp_ptr->insert_primitive      = insert_primitive;
	dsp_ptr->insert_algorithm      = insert_algorithm;
	dsp_ptr->insert_event = insert_event;
	dsp_ptr->get_primitive         = get_primitive;

	dsp_ptr->get_algorithm        = get_algorithm;
	dsp_ptr->dsp_has_algorithm    = dsp_has_algorithm;

	dsp_ptr->get_action = get_action;
	dsp_ptr->get_event  = get_event;

	esif_ccb_lock_init(&dsp_ptr->lock);
	return dsp_ptr;
}


/* Free DSP */
static void esif_dsp_free(struct esif_lp_dsp *dsp_ptr)
{
	if (NULL == dsp_ptr)
		return;

	esif_ccb_lock_uninit(&dsp->lock);

	esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_DSP, dsp_ptr);
}


/* Load */
enum esif_rc esif_dsp_load(
	struct esif_lp *lp_ptr,
	const struct esif_data *cpc_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	u8 domain_index;

	if (NULL == lp_ptr)
		return ESIF_E_PARAMETER_IS_NULL;

	ESIF_TRACE_DYN_DSP("%s: DSP Load(%s) CPC=%p, len=%d\n",
			   ESIF_FUNC,
			   lp_ptr->pi_ptr->name,
			   cpc_ptr->buf_ptr,
			   cpc_ptr->buf_len);

	/* Allocate DSP */
	lp_ptr->dsp_ptr = esif_dsp_alloc();
	if (NULL == lp_ptr->dsp_ptr)
		return ESIF_E_NO_MEMORY;

	/* Allocate Hash Table */
	lp_ptr->dsp_ptr->ht_ptr = esif_hash_table_create(31);
	if (NULL == lp_ptr->dsp_ptr->ht_ptr)
		return ESIF_E_NO_MEMORY;

	/* Allocate Linked List */
	lp_ptr->dsp_ptr->algo_ptr = esif_link_list_create();
	if (NULL == lp_ptr->dsp_ptr->algo_ptr)
		return ESIF_E_NO_MEMORY;

	lp_ptr->dsp_ptr->evt_ptr = esif_link_list_create();
	if (NULL == lp_ptr->dsp_ptr->evt_ptr)
		return ESIF_E_NO_MEMORY;

	/* Now Unpack Our CPC */
	rc = esif_cpc_unpack(lp_ptr->dsp_ptr, cpc_ptr);
	if (ESIF_OK == rc) {
		/* Assign For Each Domain */
		lp_ptr->domain_count = lp_ptr->dsp_ptr->get_domain_count(
				lp_ptr->dsp_ptr);

		/*
		** Force one domain for DPTFZ
		*/
		if (lp_ptr->pi_ptr->flags & ESIF_FLAG_DPTFZ)
			lp_ptr->domain_count = 1;

		for (domain_index = 0; domain_index < lp_ptr->domain_count;
		     domain_index++) {
			lp_ptr->domains[domain_index].instance = domain_index;
			lp_ptr->domains[domain_index].lp_ptr = lp_ptr;
			lp_ptr->domains[domain_index].id =
				get_domain_id(lp_ptr->dsp_ptr, domain_index);
			lp_ptr->domains[domain_index].name_ptr =
				get_domain_name(lp_ptr->dsp_ptr, domain_index);
			lp_ptr->domains[domain_index].desc_ptr =
				get_domain_desc(lp_ptr->dsp_ptr, domain_index);
			lp_ptr->domains[domain_index].capabilities =
				get_domain_capability(lp_ptr->dsp_ptr,
						      domain_index);
			lp_ptr->domains[domain_index].domainType =
				get_domain_type(lp_ptr->dsp_ptr, domain_index);

			ESIF_TRACE_DYN_DSP(
				"%s: DOMAIN instance=%d, id=%d, name=%s, desc=%s, capabilities=%08x type=%d\n",
				ESIF_FUNC,
				domain_index,
				lp_ptr->domains[domain_index].id,
				lp_ptr->domains[domain_index].name_ptr,
				lp_ptr->domains[domain_index].desc_ptr,
				lp_ptr->domains[domain_index].capabilities,
				lp_ptr->domains[domain_index].domainType);
		}
		/* Transistion State Instrument Domains */
		esif_lf_pm_lp_set_state(lp_ptr,
					ESIF_PM_PARTICIPANT_STATE_REGISTERED);
	}
	return rc;
}


/* Unload */
void esif_dsp_unload(struct esif_lp *lp_ptr)
{
	if (NULL == lp_ptr || NULL == lp_ptr->dsp_ptr)
		return;

	ESIF_TRACE_DYN_DSP("%s: DSP UnLoad = %p\n", ESIF_FUNC, lp_ptr->dsp_ptr);

	/* Transition State Unistrument Domains If Registered */
	if (ESIF_PM_PARTICIPANT_STATE_REGISTERED ==
	    esif_lf_pm_lp_get_state(lp_ptr))
		esif_lf_pm_lp_set_state(lp_ptr,
					ESIF_PM_PARTICIPANT_STATE_NEEDDSP);

	if (NULL != lp_ptr->dsp_ptr->algo_ptr)
		esif_link_list_destroy(lp_ptr->dsp_ptr->algo_ptr);

	if (NULL != lp_ptr->dsp_ptr->evt_ptr)
		esif_link_list_destroy(lp_ptr->dsp_ptr->evt_ptr);

	if (NULL != lp_ptr->dsp_ptr->ht_ptr)
		esif_hash_table_destroy(lp_ptr->dsp_ptr->ht_ptr);

	if (NULL != lp_ptr->dsp_ptr->cpc_ptr)
		esif_cpc_free(lp_ptr->dsp_ptr);

	if (NULL != lp_ptr->dsp_ptr->table) {
		esif_ccb_free(lp_ptr->dsp_ptr->table);
		lp_ptr->dsp_ptr->table = NULL;
		lp_ptr->dsp_ptr->table_size = 0;
	}

	esif_dsp_free(lp_ptr->dsp_ptr);
	lp_ptr->dsp_ptr = NULL;
}


/* Init */
enum esif_rc esif_dsp_init(void)
{
	struct esif_ccb_mempool *mempool_ptr = NULL;

	ESIF_TRACE_DYN_INIT("%s: Initialize DSP Engine\n", ESIF_FUNC);

	mempool_ptr = esif_ccb_mempool_create(ESIF_MEMPOOL_TYPE_DSP,
					      ESIF_MEMPOOL_FW_DSP,
					      sizeof(struct esif_lp_dsp));
	if (NULL == mempool_ptr)
		return ESIF_E_NO_MEMORY;

	esif_cpc_init();
	return ESIF_OK;
}


/* Exit */
void esif_dsp_exit(void)
{
	esif_cpc_exit();

	ESIF_TRACE_DYN_INIT("%s: Exit DSP Engine\n", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


