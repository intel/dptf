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

#include "esif_event.h"
#include "esif_queue.h"

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified against Windows SDK/DDK included headers which we
 * have no control over.
 */
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define INIT_DEBUG       0
#define EVENT_DEBUG      1
#define DECODE_DEBUG     2

#define ESIF_TRACE_DYN_INIT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_EVENT, INIT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_EVENT(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_EVENT, EVENT_DEBUG, format, ##__VA_ARGS__)
#define ESIF_TRACE_DYN_DECODE(format, ...) \
	ESIF_TRACE_DYN(ESIF_DEBUG_MOD_EVENT, DECODE_DEBUG, format, ##__VA_ARGS__)

/* Event Lock */
static esif_ccb_lock_t g_event_lock;

/* Transaction ID each event will have a unique ID */
static u64 g_event_transaction_id; /* No need to set 0 */

/* Queue */
struct esif_queue_instance *g_event_queue; /* No need to set NULL */;
static char *g_event_queue_name = "EVENT";

/* Check Queue */
u32 esif_event_queue_size(void)
{
	if (NULL != g_event_queue)
		return esif_queue_size(g_event_queue);

	return 0;
}


/* Poll Event Queue */
struct esif_ipc *esif_event_queue_pull()
{
	if (NULL != g_event_queue)
		return esif_queue_pull(g_event_queue);

	return NULL;
}


/* Push Event Queue */
enum esif_rc esif_event_queue_push(struct esif_ipc *ipc_ptr)
{
	if (NULL == g_event_queue || NULL == ipc_ptr)
		return ESIF_E_PARAMETER_IS_NULL;

	return esif_queue_push(g_event_queue, ipc_ptr);
}


/* Put Event Back in Queue */
enum esif_rc esif_event_queue_requeue(struct esif_ipc *ipc_ptr)
{
	if (NULL == g_event_queue || NULL == ipc_ptr)
		return ESIF_E_PARAMETER_IS_NULL;

	return esif_queue_requeue(g_event_queue, ipc_ptr);
}


/* Documented In Header */
struct esif_event *esif_event_allocate(
	const enum esif_event_type type,
	const u16 size,
	const enum esif_event_priority priority,
	const u8 src,
	const u8 dst,
	const u16 dst_domain_id,
	const void *data_ptr
	)
{
	u16 new_size = size + sizeof(struct esif_event);
	struct esif_event *event_ptr = NULL;
	event_ptr = esif_ccb_memtype_zalloc(ESIF_MEMTYPE_TYPE_EVENT, new_size);

	if (event_ptr) {
		event_ptr->version       = ESIF_EVENT_VERSION;
		event_ptr->size          = new_size;
		event_ptr->type          = type;
		event_ptr->priority      = priority;
		event_ptr->src           = src;
		event_ptr->dst           = dst;
		event_ptr->dst_domain_id = dst_domain_id;
		event_ptr->data_size     = size;

		/*
		 *  Assign Function Pointers If Any
		 */
#ifdef ESIF_EVENT_DEBUG
		event_ptr->get_type_str     = esif_event_type_str;
		event_ptr->get_priority_str = esif_event_priority_str;
		event_ptr->dump = esif_dump_event;
#endif

		/*
		** Transaction ID
		*/
		esif_ccb_write_lock(&g_event_lock);
		event_ptr->id = g_event_transaction_id++;
		esif_ccb_write_unlock(&g_event_lock);

		/*
		** Time Stamp
		*/
		esif_ccb_system_time(&event_ptr->timestamp);

		/*
		** Make A Copy Of The Data To Make Sure It Is Contigous
		** In The Buffer
		*/
		if (NULL != data_ptr)
			esif_ccb_memcpy((event_ptr + 1), data_ptr, size);

		ESIF_TRACE_DYN_EVENT("%s: buf %p bytes %d\n",
				     ESIF_FUNC,
				     event_ptr,
				     new_size);
		ESIF_TRACE_DYN_DECODE("Version:     %d\n"
				      "Type:        %s(%d)\n"
				      "ID:          %llu\n"
				      "Timestamp:   %llu\n"
				      "Priority:    %s(%d)\n"
				      "Source:      %d\n"
				      "Destination: %d\n"
				      "Data Size: %d\n",
				      event_ptr->version,
				      esif_event_type_str(event_ptr->type),
				      event_ptr->type,
				      event_ptr->id,
				      (u64)event_ptr->timestamp,
				      esif_event_priority_str(
					event_ptr->priority),
				      event_ptr->priority,
				      event_ptr->src,
				      event_ptr->dst,
				      event_ptr->data_size);
	}
	return event_ptr;
}


/* Event Free */
void esif_event_free(const struct esif_event *event)
{
	ESIF_TRACE_DYN_EVENT("%s: buf %p bytes %d\n",
			     ESIF_FUNC,
			     event,
			     event->size);

	esif_ccb_memtype_free(ESIF_MEMTYPE_TYPE_EVENT, (void *)event);
}


/* Init */
enum esif_rc esif_event_init(void)
{
	ESIF_TRACE_DYN_INIT("%s: Initialize Event\n", ESIF_FUNC);


	g_event_queue = esif_queue_create(1024, g_event_queue_name);
	if (NULL == g_event_queue)
		return ESIF_E_NO_MEMORY;

	esif_ccb_lock_init(&g_event_lock);
	return ESIF_OK;
}


/* Exit */
void esif_event_exit(void)
{
	esif_queue_destroy(g_event_queue);

	esif_ccb_lock_uninit(&g_event_lock);

	ESIF_TRACE_DYN_INIT("%s: Exit Event\n", ESIF_FUNC);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
