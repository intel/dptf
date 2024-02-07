/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "esif_queue.h"
#include "esif_ccb_string.h"

/* Queue Create */
struct esif_queue_instance *esif_queue_create(
	u32 depth,
	char *name_ptr,
	u32 ms_timeout
	)
{
	enum esif_rc rc = ESIF_E_NO_MEMORY;
	struct esif_queue_instance *queue_ptr = NULL;
	struct esif_link_list *queue_list_ptr = NULL;
	
	queue_ptr = (struct esif_queue_instance *)
		esif_ccb_malloc(sizeof(*queue_ptr));
	if (NULL == queue_ptr)
		goto exit;

	esif_ccb_lock_init(&queue_ptr->lock);
	esif_ccb_event_init(&queue_ptr->event);
	queue_ptr->max_size   = depth;
	queue_ptr->ms_timeout = ms_timeout;

	esif_ccb_strcpy(queue_ptr->queue_name,
		name_ptr,
		sizeof(queue_ptr->queue_name));

	queue_list_ptr = esif_link_list_create();
	if (NULL == queue_list_ptr)
		goto exit;

	queue_ptr->queue_list_ptr = queue_list_ptr;

	rc = ESIF_OK;
exit:
	if (rc != ESIF_OK) {
		esif_queue_destroy(queue_ptr, NULL);
		queue_ptr = NULL;
	}
	return queue_ptr;
}


/* Queue Destroy */
void esif_queue_destroy(
	struct esif_queue_instance *self,
	queue_item_destroy_func destroy_func_ptr
	)
{
	void *data_ptr = NULL;
	
	if (NULL == self)
		goto exit;

	data_ptr = esif_queue_dequeue(self);
	while (data_ptr != NULL) {

		if (destroy_func_ptr != NULL)
			destroy_func_ptr(data_ptr);

		data_ptr = esif_queue_dequeue(self);
	}

	esif_ccb_event_uninit(&self->event);
	esif_ccb_lock_uninit(&self->lock);

	esif_ccb_free(self->queue_list_ptr);
	esif_ccb_free(self);
exit:
	return;
}


/* Queue Enqueue (Puts at back of linked list) */
enum esif_rc esif_queue_enqueue(
	struct esif_queue_instance *self,
	void *data_ptr
	)
{
	enum esif_rc rc = ESIF_E_NO_MEMORY;

	if (NULL == self)
		goto exit;

	esif_ccb_write_lock(&self->lock);

	if (self->current_size >= self->max_size)
		goto lock_exit;

	rc = esif_link_list_add_at_back(self->queue_list_ptr, data_ptr);
	if (rc != ESIF_OK)
		goto lock_exit;

	self->current_size++;

	/* Wakeup */
	esif_ccb_event_set(&self->event);

	rc = ESIF_OK;
lock_exit:
	esif_ccb_write_unlock(&self->lock);
exit:
	return rc;
}


/*
 * Queue Requeue (Put at front of linked list, used for returning item to queue)
 * Note:  This function does not wake the waiting thread as it is used in the
 * case when processing of the queued item has failed, and we don't want to
 * continue processing in infinite loop.
 */
enum esif_rc esif_queue_requeue(
	struct esif_queue_instance *self,
	void *data_ptr
	)
{
	enum esif_rc rc = ESIF_E_NO_MEMORY;
	struct esif_link_list_node *node_ptr = NULL;

	if (NULL == self)
		goto exit;

	esif_ccb_write_lock(&self->lock);

	if (self->current_size >= self->max_size)
		goto lock_exit;

	node_ptr = esif_link_list_create_node(data_ptr);
	if (NULL == node_ptr)
		goto lock_exit;

	/* Put at front; insted of back for requeue */
	esif_link_list_add_node_at_front(self->queue_list_ptr, node_ptr);
	self->current_size++;

	esif_queue_signal_event(self);

	rc = ESIF_OK;
lock_exit:
	esif_ccb_write_unlock(&self->lock);
exit:
	return rc;
}


/* Queue Dequeue */
void *esif_queue_dequeue(struct esif_queue_instance *self)
{
	void *data_ptr = NULL;
	struct esif_link_list_node *node_ptr = NULL;

	if ((NULL == self)  || (NULL == self->queue_list_ptr))
		goto exit;

	esif_ccb_write_lock(&self->lock);

	node_ptr = self->queue_list_ptr->head_ptr;
	if (NULL == node_ptr) {
		self->current_size = 0;
		goto lock_exit;
	}

	data_ptr = node_ptr->data_ptr;

	esif_link_list_node_remove(self->queue_list_ptr, node_ptr);

	self->current_size--;

lock_exit:
	if (!self->current_size) {
		esif_ccb_event_reset(&self->event);
	}
	esif_ccb_write_unlock(&self->lock);
exit:
	return data_ptr;
}


/* Queue Dequeue */
void *esif_queue_pull(struct esif_queue_instance *self)
{
	void *data_ptr = NULL;

	if (NULL == self)
		goto exit;

	/*
	 * If no timeout, wait forever; else, wait the specified time for an event.
	 */
	if (ESIF_QUEUE_TIMEOUT_INFINITE == self->ms_timeout) {
		esif_ccb_event_wait(&self->event);
	} else {
		esif_ccb_event_try_wait(&self->event, self->ms_timeout);
	}

	data_ptr = esif_queue_dequeue(self);
	if (NULL == data_ptr)
		goto exit;
exit:
	return data_ptr;
}


/* Queue Size */
u32 esif_queue_size(struct esif_queue_instance *self)
{
	if (NULL == self)
		return 0;

	return self->current_size;
}

/* Used to allow a waiting event thread to exit before destruction */
void esif_queue_signal_event(struct esif_queue_instance *self)
{
	if (self != NULL)
		esif_ccb_event_set(&self->event);
}


/* Init */
enum esif_rc esif_queue_init(void)
{
	return ESIF_OK;
}


/* Exit */
void esif_queue_exit(void)
{
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

