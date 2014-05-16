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

#include "esif_queue.h"

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

#ifdef ESIF_ATTR_KERNEL

#define INIT_DEBUG    0
#define Q_DEBUG       1
#define PULL_DEBUG    2
#define SEM_DEBUG     3

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_QUEUE, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_Q(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_QUEUE, Q_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_PULL(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_QUEUE, PULL_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_SEM(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_QUEUE, SEM_DEBUG, format, ##__VA_ARGS__)

#else /* ESIF_ATTR_KERNEL */

/*
 * TODO:  User mode does not currently use the queue code.
 * Need to update when user mode unified debug infrastructre
 * is in place.
 */
#define ESIF_TRACE_DYN_INIT NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_Q NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_PULL NO_ESIF_DEBUG
#define ESIF_TRACE_DYN_SEM NO_ESIF_DEBUG

#endif /* ESIF_ATTR_USER */

/* Queue Node */
struct esif_queue_node {
	struct esif_queue_node  *next_ptr;	/* Next Item In The Queue */
	struct esif_ipc         *ipc_ptr;	/* Queued IPC/Event       */
};

/* Queue Create */
struct esif_queue_instance *esif_queue_create(
	u32 depth,
	char *name_ptr
	)
{
	struct esif_queue_instance *queue_ptr = (struct esif_queue_instance *)
					esif_ccb_malloc(sizeof(*queue_ptr));
	if (NULL == queue_ptr)
		return NULL;

	esif_ccb_lock_init(&queue_ptr->lock);
	esif_ccb_sem_init(&queue_ptr->semaphore);
	queue_ptr->max_size   = depth;
	queue_ptr->us_timeout = 1000000;/* Second */
	queue_ptr->name_ptr   = name_ptr;

	ESIF_TRACE_DYN_Q("%s: Create %s Queue %p depth = %d\n",
			 ESIF_FUNC,
			 name_ptr,
			 queue_ptr,
			 depth);
	return queue_ptr;
}


/* Queue Destroy */
void esif_queue_destroy(struct esif_queue_instance *queue_ptr)
{
	struct esif_queue_node *current_ptr = queue_ptr->head_ptr;

	esif_ccb_write_lock(&queue_ptr->lock);
	for (current_ptr = queue_ptr->head_ptr;
	     current_ptr != queue_ptr->tail_ptr;
	     current_ptr = queue_ptr->head_ptr) {
		queue_ptr->head_ptr = current_ptr->next_ptr;

		ESIF_TRACE_DYN_Q("%s: %s Queue Clean ipc %p\n",
				 ESIF_FUNC, queue_ptr->name_ptr, current_ptr);

		esif_ipc_free(current_ptr->ipc_ptr);
		esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_QUEUE, current_ptr);
	}

	if (current_ptr != NULL) {
		ESIF_TRACE_DYN_Q("%s: %s Queue Clean ipc %p\n",
				 ESIF_FUNC, queue_ptr->name_ptr, current_ptr);

		esif_ipc_free(current_ptr->ipc_ptr);

		esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_QUEUE, current_ptr);
	}
	esif_ccb_write_unlock(&queue_ptr->lock);
	esif_ccb_lock_uninit(&queue->lock);
	esif_ccb_sem_uninit(&queue->semaphore);
	ESIF_TRACE_DYN_Q("%s: %s Destroy Queue %p\n",
			 ESIF_FUNC,
			 queue_ptr->name_ptr,
			 queue_ptr);
	esif_ccb_free(queue_ptr);
}


/* Queue Config */
enum esif_rc esif_queue_config(
	struct esif_queue_instance *queue_ptr,
	u32 us_timeout,
	u32 max_size
	)
{
	esif_ccb_write_lock(&queue_ptr->lock);
	queue_ptr->us_timeout = us_timeout;
	queue_ptr->max_size   = max_size;
	esif_ccb_write_unlock(&queue_ptr->lock);
	return ESIF_OK;
}


/* Queue Push (Puts in back of linked list)*/
enum esif_rc esif_queue_push(
	struct esif_queue_instance *queue_ptr,
	struct esif_ipc *ipc_ptr
	)
{
	struct esif_queue_node *node_ptr = NULL;

	if (queue_ptr->current_size >= queue_ptr->max_size) {
		ESIF_TRACE_DYN_Q("%s: No queue space, max size = %d\n",
				 ESIF_FUNC,
				 queue_ptr->max_size);
		return ESIF_E_NO_MEMORY;
	}

	node_ptr = esif_ccb_mempool_zalloc(ESIF_MEMPOOL_TYPE_QUEUE);
	if (NULL == node_ptr)
		return ESIF_E_NO_MEMORY;

	node_ptr->ipc_ptr = ipc_ptr;

	esif_ccb_write_lock(&queue_ptr->lock);
	node_ptr->next_ptr = NULL;
	if (NULL == queue_ptr->head_ptr) {
		queue_ptr->head_ptr = node_ptr;
		queue_ptr->tail_ptr = node_ptr;
	} else {
		queue_ptr->tail_ptr->next_ptr = node_ptr;
		queue_ptr->tail_ptr = node_ptr;
	}
	queue_ptr->current_size++;
	esif_ccb_write_unlock(&queue_ptr->lock);

	/* Wakeup */
	ESIF_TRACE_DYN_Q("%s: %s Queue Push q %p ipc %p SEM UP\n",
			 ESIF_FUNC, queue_ptr->name_ptr, queue_ptr, ipc_ptr);

	esif_ccb_sem_up(&queue_ptr->semaphore);
	return ESIF_OK;
}


/* Queue Push (Put at front of linked list, used for returning item to queue) */
enum esif_rc esif_queue_requeue(
	struct esif_queue_instance *queue_ptr,
	struct esif_ipc *ipc_ptr
	)
{
	struct esif_queue_node *node_ptr = NULL;

	if (queue_ptr->current_size >= queue_ptr->max_size) {
		ESIF_TRACE_DYN_Q("%s: No queue space, max size = %d\n",
				 ESIF_FUNC,
				 queue_ptr->max_size);
		return ESIF_E_NO_MEMORY;
	}

	node_ptr = esif_ccb_mempool_zalloc(ESIF_MEMPOOL_TYPE_QUEUE);
	if (NULL == node_ptr)
		return ESIF_E_NO_MEMORY;

	node_ptr->ipc_ptr = ipc_ptr;

	esif_ccb_write_lock(&queue_ptr->lock);
	node_ptr->next_ptr = NULL;
	if (NULL == queue_ptr->head_ptr) {
		queue_ptr->head_ptr = node_ptr;
		queue_ptr->tail_ptr = node_ptr;
	} else {
		node_ptr->next_ptr  = queue_ptr->head_ptr;
		queue_ptr->head_ptr = node_ptr;
	}
	queue_ptr->current_size++;
	esif_ccb_write_unlock(&queue_ptr->lock);

	/* Wakeup */
	ESIF_TRACE_DYN_Q("%s: %s Queue Push q %p ipc %p SEM UP\n",
			 ESIF_FUNC, queue_ptr->name_ptr, queue_ptr, ipc_ptr);

	esif_ccb_sem_up(&queue_ptr->semaphore);
	return ESIF_OK;
}


/* Queue Pull */
struct esif_ipc *esif_queue_pull(struct esif_queue_instance *queue_ptr)
{
	struct esif_queue_node *node_ptr = NULL;
	struct esif_ipc *ipc_ptr         = NULL;

	ESIF_TRACE_DYN_SEM("%s: semophore timeout %d\n",
			   ESIF_FUNC,
			   queue_ptr->us_timeout);

	/*  Wait Forever */
	if (0 == queue_ptr->us_timeout) {
		esif_ccb_sem_down(&queue_ptr->semaphore);

	/* Wait For N Usecs and If No Queue Entry Availabe Return False */
	} else if (esif_ccb_sem_try_down(&queue_ptr->semaphore,
					 queue_ptr->us_timeout) != 0) {
		return NULL;
	}

	esif_ccb_write_lock(&queue_ptr->lock);
	if (NULL == queue_ptr->head_ptr) {
		esif_ccb_write_unlock(&queue_ptr->lock);
		return NULL; /* Could happen if a flush occurs after semaphore*/
	}
	node_ptr = queue_ptr->head_ptr;
	ipc_ptr  = node_ptr->ipc_ptr;

	queue_ptr->head_ptr = node_ptr->next_ptr;

	if (NULL == queue_ptr->head_ptr)
		queue_ptr->tail_ptr = NULL;

	esif_ccb_write_unlock(&queue_ptr->lock);
	queue_ptr->current_size--;

	esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_QUEUE, node_ptr);

	ESIF_TRACE_DYN_Q("%s: %s Queue Pull q %p ipc %p\n",
			 ESIF_FUNC, queue_ptr->name_ptr, queue_ptr, ipc_ptr);

	return ipc_ptr;
}


/* Queue Size */
u32 esif_queue_size(struct esif_queue_instance *queue_ptr)
{
	ESIF_TRACE_DYN_PULL("%s: %s Queue Size %d\n",
			    ESIF_FUNC,
			    queue_ptr->name_ptr,
			    queue_ptr->current_size);
	return queue_ptr->current_size;
}


/* Queue Flush */
void esif_queue_flush(struct esif_queue_instance *queue_ptr)
{
	struct esif_queue_node *node_ptr = NULL;

	esif_ccb_write_lock(&queue_ptr->lock);
	node_ptr = queue_ptr->head_ptr;
	while (node_ptr != NULL) {
		queue_ptr->head_ptr = node_ptr->next_ptr;
		esif_ipc_free(node_ptr->ipc_ptr);

		esif_ccb_mempool_free(ESIF_MEMPOOL_TYPE_QUEUE, node_ptr);

		node_ptr = queue_ptr->head_ptr;
	}
	queue_ptr->current_size = 0;
	queue_ptr->head_ptr     = NULL;
	queue_ptr->tail_ptr     = NULL;
	esif_ccb_write_unlock(&queue_ptr->lock);
}


/* Init */
enum esif_rc esif_queue_init(void)
{
	struct esif_ccb_mempool *mempool_ptr = NULL;
	ESIF_TRACE_DYN_INIT("%s: Initialize Queue Manager\n", ESIF_FUNC);

	mempool_ptr =
		esif_ccb_mempool_create(ESIF_MEMPOOL_TYPE_QUEUE,
					ESIF_MEMPOOL_FW_QUEUE,
					sizeof(struct esif_queue_node));
	if (NULL == mempool_ptr)
		return ESIF_E_NO_MEMORY;

	return ESIF_OK;
}


/* Exit */
void esif_queue_exit(void)
{
	ESIF_TRACE_DYN_INIT("%s: Exit Queue Manager\n", ESIF_FUNC);
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

