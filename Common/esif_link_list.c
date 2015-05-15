/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

enum esif_rc esif_link_list_init(void)
{
	/* Placeholder...should be called in case this is changed in the future */
	return ESIF_OK;
}


void esif_link_list_exit(void)
{
	/* Placeholder...should be called in case this is changed in the future */
}


/* Create Linked List */
struct esif_link_list *esif_link_list_create(void)
{
	struct esif_link_list *list_ptr = NULL;

	list_ptr = (struct esif_link_list *)
		esif_ccb_malloc(sizeof(*list_ptr));
	if (NULL == list_ptr)
		goto exit;

	list_ptr->head_ptr = NULL;
	list_ptr->tail_ptr = NULL;
	list_ptr->nodes    = 0;
exit:
	return list_ptr;
}


/* Destroy Linked List */
void esif_link_list_destroy(struct esif_link_list *self)
{
	struct esif_link_list_node *curr_ptr = NULL;
	
	if (NULL == self)
		goto exit;

	curr_ptr = self->head_ptr;
	while (curr_ptr) {
		struct esif_link_list_node *next_node_ptr = curr_ptr->next_ptr;
		esif_link_list_destroy_node(curr_ptr);
		curr_ptr = next_node_ptr;
	}

	esif_ccb_free(self);
exit:
	return;
}


/* Create Node */
struct esif_link_list_node *esif_link_list_create_node(void *data_ptr)
{
	struct esif_link_list_node *new_node_ptr = NULL;

	new_node_ptr = (struct esif_link_list_node *)
			esif_ccb_malloc(sizeof(*new_node_ptr));
	if (NULL == new_node_ptr)
		goto exit;

	new_node_ptr->data_ptr = data_ptr;
	new_node_ptr->next_ptr = NULL;
	new_node_ptr->prev_ptr = NULL;
exit:
	return new_node_ptr;
}


/* Destroy Node */
void esif_link_list_destroy_node(struct esif_link_list_node *node_ptr)
{
	esif_ccb_free(node_ptr);
}


/* Allocates and adds a new node at the front of the list */
enum esif_rc esif_link_list_add_at_front(
	struct esif_link_list *self,
	void *data_ptr
	)
{
	enum esif_rc rc = ESIF_E_NO_MEMORY;
	struct esif_link_list_node *node_ptr = NULL;
	
	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	node_ptr = esif_link_list_create_node(data_ptr);
	if (NULL == node_ptr)
		goto exit;

	esif_link_list_add_node_at_front(self, node_ptr);

	rc = ESIF_OK;
exit:
	return rc;
}


/* Adds a node at the front of the list */
void esif_link_list_add_node_at_front(
	struct esif_link_list *self,
	struct esif_link_list_node *new_node_ptr
	)
{
	if ((NULL == self) || (NULL == new_node_ptr))
		goto exit;

	if (NULL == self->head_ptr) {
		new_node_ptr->prev_ptr = NULL;
		new_node_ptr->next_ptr = NULL;
		self->head_ptr = new_node_ptr;
		self->tail_ptr = new_node_ptr;
	} else {
		new_node_ptr->prev_ptr = NULL;
		new_node_ptr->next_ptr = self->head_ptr;
		self->head_ptr->prev_ptr = new_node_ptr;
		self->head_ptr = new_node_ptr;
	}
	self->nodes++;
exit:
	return;
}


/* Allocates and adds a new node at the back of the list */
enum esif_rc esif_link_list_add_at_back(
	struct esif_link_list *self,
	void *data_ptr
	)
{
	enum esif_rc rc = ESIF_E_NO_MEMORY;
	struct esif_link_list_node *node_ptr = NULL;
	
	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	node_ptr = esif_link_list_create_node(data_ptr);
	if (NULL == node_ptr)
		goto exit;

	esif_link_list_add_node_at_back(self, node_ptr);

	rc = ESIF_OK;
exit:
	return rc;
}


/* Add Linked List Node at Tail End */
void esif_link_list_add_node_at_back(
	struct esif_link_list *self,
	struct esif_link_list_node *new_node_ptr
	)
{
	if ((NULL == self) || (NULL == new_node_ptr))
		goto exit;

	if (NULL == self->head_ptr) {
		esif_link_list_add_node_at_front(self, new_node_ptr);
	} else {
		new_node_ptr->prev_ptr = self->tail_ptr;
		new_node_ptr->next_ptr = NULL;
		self->tail_ptr->next_ptr = new_node_ptr;
		self->tail_ptr = new_node_ptr;
	}
	self->nodes++;
exit:
	return;
}


void esif_link_list_node_remove(
	struct esif_link_list *self,
	struct esif_link_list_node *node_ptr
	)
{
	struct esif_link_list_node *cur_ptr = NULL;

	if ((NULL == self) || (NULL == node_ptr))
		goto exit;

	cur_ptr = self->head_ptr;
	while (cur_ptr) {
		if (cur_ptr == node_ptr) {

			/* If the head */
			if (cur_ptr->prev_ptr == NULL)
				self->head_ptr = cur_ptr->next_ptr;
			else
				cur_ptr->prev_ptr->next_ptr = cur_ptr->next_ptr;

			/* If the tail */
			if (cur_ptr->next_ptr == NULL)
				self->tail_ptr = cur_ptr->prev_ptr;
			else
				cur_ptr->next_ptr->prev_ptr = cur_ptr->prev_ptr;

			esif_link_list_destroy_node(cur_ptr);
			self->nodes--;

			break;
		}
		cur_ptr = cur_ptr->next_ptr;
	}
exit:
	return;
}
