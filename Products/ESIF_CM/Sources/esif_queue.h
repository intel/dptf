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

#ifndef _ESIF_QUEUE_H_
#define _ESIF_QUEUE_H_

#include "esif_ipc.h"

/* Queue Instance */
struct esif_queue_instance {
	u32  us_timeout;	/* Timeout in Microseconds */
	u32  max_size;		/* Maximum allowable queue size in items */
	u32  current_size;	/* Current queeue size in items */
	esif_ccb_lock_t  lock;		/* Lock */
	esif_ccb_sem_t   semaphore;	/* To allow blocking if queue is empty */
	struct esif_queue_node  *head_ptr;	/* First event or NULL if queue Is empty */
	struct esif_queue_node  *tail_ptr;	/* Last event or NULL if queue Is empty */
	char *name_ptr;		/* Queue Name */
};

struct esif_queue_instance *esif_queue_create (u32 depth, char *name_ptr);
void esif_queue_destroy (struct esif_queue_instance *queue_ptr);

enum esif_rc esif_queue_push (struct esif_queue_instance *queue_ptr,
			      struct esif_ipc *ipc_ptr);

enum esif_rc esif_queue_requeue (struct esif_queue_instance *queue_ptr,
				 struct esif_ipc *ipc_ptr);

struct esif_ipc *esif_queue_pull (struct esif_queue_instance *queue_ptr);

u32 esif_queue_size (struct esif_queue_instance *queue_ptr);

/* Init / Exit */
enum esif_rc esif_queue_init (void);
void esif_queue_exit (void);

#endif /* _ESIF_QUEUE_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
