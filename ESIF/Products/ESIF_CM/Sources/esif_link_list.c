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

#include "esif_link_list.h"

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

#define LINK_LIST_DEBUG NO_ESIF_DEBUG


/* Destroy Node */
void esif_link_list_destroy_node(struct esif_link_list_node *node_ptr)
{
	esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_LIST_NODE, node_ptr);
}


/* Create Node */
struct esif_link_list_node *esif_link_list_create_node(void *data_ptr)
{
	struct esif_link_list_node *new_node_ptr = NULL;

	new_node_ptr = (struct esif_link_list_node *)
			esif_ccb_mempool_zalloc(ESIF_MEMPOOL_TYPE_LIST_NODE);
	if (NULL == new_node_ptr)
		return NULL;

	new_node_ptr->data_ptr = data_ptr;
	new_node_ptr->next_ptr = NULL;
	new_node_ptr->prev_ptr = NULL;
	return new_node_ptr;
}


/* Create Linked List */
struct esif_link_list *esif_link_list_create(void)
{
	struct esif_link_list *new_link_list_ptr = NULL;

	new_link_list_ptr = (struct esif_link_list *)
				esif_ccb_mempool_zalloc(ESIF_MEMPOOL_TYPE_LIST);
	if (NULL == new_link_list_ptr)
		return NULL;

	new_link_list_ptr->head_ptr = NULL;
	new_link_list_ptr->tail_ptr = NULL;
	new_link_list_ptr->nodes    = 0;
	return new_link_list_ptr;
}


/* Destroy Linked List */
void esif_link_list_destroy(struct esif_link_list *list_ptr)
{
	struct esif_link_list_node *curr_ptr = list_ptr->head_ptr;
	while (curr_ptr) {
		struct esif_link_list_node *next_node_ptr = curr_ptr->next_ptr;
		esif_link_list_destroy_node(curr_ptr);
		curr_ptr = next_node_ptr;
	}

	esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_LIST, list_ptr);
}


/* Add Linked List Node */
void esif_link_list_node_add(
	struct esif_link_list *list_ptr,
	struct esif_link_list_node *new_node_ptr
	)
{
	if (NULL == list_ptr->head_ptr) {
		list_ptr->head_ptr = new_node_ptr;
		list_ptr->head_ptr->next_ptr = NULL;
		list_ptr->head_ptr->prev_ptr = NULL;
		list_ptr->tail_ptr = list_ptr->head_ptr;
	} else {
		new_node_ptr->prev_ptr       = list_ptr->tail_ptr;
		list_ptr->tail_ptr->next_ptr = new_node_ptr;
		list_ptr->tail_ptr = new_node_ptr;
		list_ptr->tail_ptr->next_ptr = NULL;
	}
	list_ptr->nodes++;
}


/* Init */
enum esif_rc esif_link_list_init(void)
{
	struct esif_ccb_mempool *mempool_ptr = NULL;
	LINK_LIST_DEBUG("%s: Initialize Primitive Link List\n", ESIF_FUNC);

	mempool_ptr =
		esif_ccb_mempool_create(ESIF_MEMPOOL_TYPE_LIST,
					ESIF_MEMPOOL_FW_LIST,
					sizeof(struct esif_link_list));
	if (NULL == mempool_ptr)
		return ESIF_E_NO_MEMORY;

	mempool_ptr =
		esif_ccb_mempool_create(ESIF_MEMPOOL_TYPE_LIST_NODE,
					ESIF_MEMPOOL_FW_LIST_NODE,
					sizeof(struct esif_link_list_node));
	if (NULL == mempool_ptr)
		return ESIF_E_NO_MEMORY;

	return ESIF_OK;
}


/* Exit */
void esif_link_list_exit(void)
{
	LINK_LIST_DEBUG("%s: Exit Primitive Link List\n", ESIF_FUNC);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
