/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_QUEUE_H_
#define _ESIF_QUEUE_H_

#include "esif_ccb.h"
#include "esif_link_list.h"
#include "esif_ccb_sem.h"
#include "esif_ccb_lock.h"

#define ESIF_QUEUE_NAME_LEN 32


/* Queue Instance */
struct esif_queue_instance {
	u32  us_timeout;	/* Timeout in Microseconds */
	u32  max_size;		/* Maximum allowable queue size in items */
	u32  current_size;	/* Current queeue size in items */
	esif_ccb_lock_t lock;	/* Lock */
	esif_ccb_sem_t semaphore;	/* Allow blocking if queue is empty */
	struct esif_link_list	*queue_list_ptr;
	char queue_name[ESIF_QUEUE_NAME_LEN];		/* Queue Name */
};

#ifdef ESIF_ATTR_USER
typedef struct esif_queue_instance EsifQueue, *EsifQueuePtr;
#endif

typedef void (*queue_item_destroy_func) (void *item_ptr);


#ifdef __cplusplus
extern "C" {
#endif

struct esif_queue_instance *esif_queue_create(
	u32 depth,
	char *name_ptr,
	u32 us_timeout
	);

void esif_queue_destroy(
	struct esif_queue_instance *self,
	queue_item_destroy_func destroy_func_ptr
	);

enum esif_rc esif_queue_enqueue(
	struct esif_queue_instance *self,
	void *data_ptr
	);

enum esif_rc esif_queue_requeue(
	struct esif_queue_instance *self,
	void *data_ptr
	);

void *esif_queue_dequeue(struct esif_queue_instance *self);
void *esif_queue_pull(struct esif_queue_instance *self);

/* Used to allow a waiting event thread to exit before destruction */
void esif_queue_signal_event(struct esif_queue_instance *self);

u32 esif_queue_size(struct esif_queue_instance *self);

/* Init / Exit */
enum esif_rc esif_queue_init(void);
void esif_queue_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* _ESIF_QUEUE_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
