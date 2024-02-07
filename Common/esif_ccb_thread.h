/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#pragma once

#include "esif_ccb.h"


#include "esif_ccb_rc.h"

#define THREAD_DEBUG(fmt, ...) /* NOOP */

typedef void *(ESIF_CALLCONV * work_func_t)(void *); /* Worker Function */

#include "esif_ccb_thread_lin_user.h"

/*
** esif_wthread_t - A Waitable Thread object that encapsulates esif_thread_t into a new object that can have more than
** one thread waiting on it to complete, unlike a regular thread which can only have one thread call its thread_join.
**
** This object is intended to be used in place of esif_thread_t when you have a thread that needs to be waited on (joined)
** by more than one thread, and has the following differences from esif_thread_t:
**   1. esif_ccb_wthread_init() must be called before esif_ccb_wthread_create()
**   2. esif_ccb_wthread_uninit() must be called after all calls to esif_ccb_wthread_join() have completed
**   3. esif_ccb_wthread_join() can be called from multiple waiter threads rather than from just one thread.
**  All other functions (esif_ccb_wthread_create, esif_ccb_wthread_id, etc) are used the same way as esif_thread_t functions
*/
#include "esif_ccb_sem.h"

typedef struct esif_wthread_s {
	atomic_t		waiters;	// Number of waiting threads
	esif_ccb_sem_t	signal;		// Semaphore to signal waiting threads
	esif_thread_t	thread;		// Thread Object
	atomic_t		joined;		// Thread Join Initiated
} esif_wthread_t;

/* Constructor - Must be called before esif_ccb_wthread_create() */
static ESIF_INLINE void esif_ccb_wthread_init(esif_wthread_t *self)
{
	if (self) {
		atomic_set(&self->waiters, 0);
		atomic_set(&self->joined, 0);
		esif_ccb_sem_init(&self->signal);
		esif_ccb_thread_init(&self->thread);
	}
}

/* Destructor - Must be called after all calls to esif_ccb_wthread_join() have completed */
static ESIF_INLINE void esif_ccb_wthread_uninit(esif_wthread_t *self)
{
	if (self) {
		atomic_set(&self->waiters, 0);
		atomic_set(&self->joined, 0);
		esif_ccb_sem_uninit(&self->signal);
		esif_ccb_thread_uninit(&self->thread);
	}
}

/* Create a Waitable Thread Object */
static ESIF_INLINE esif_error_t esif_ccb_wthread_create(esif_wthread_t *self, work_func_t worker, void *ctx)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && worker) {
		atomic_set(&self->waiters, 0);
		atomic_set(&self->joined, 0);
		rc = esif_ccb_thread_create(&self->thread, worker, ctx);
	}
	return rc;
}

/* Wait for a Waitable Thread to complete. This may be called from more than one waiter thread. */
static ESIF_INLINE void esif_ccb_wthread_join(esif_wthread_t *self)
{
	if (self) {
		if (atomic_inc(&self->waiters) > 1) {
			esif_ccb_sem_down(&self->signal);
		}
		else {
			if (atomic_set(&self->joined, 1) == 0) {
				esif_ccb_thread_join(&self->thread);
			}
			while (atomic_dec(&self->waiters) > 0) {
				esif_ccb_sem_up(&self->signal);
			}
		}
	}
}

/* Return Unique Thread ID of the given thread object or ESIF_THREAD_ID_NULL */
static ESIF_INLINE esif_thread_id_t esif_ccb_wthread_id(esif_wthread_t *self)
{
	esif_thread_id_t id = ESIF_THREAD_ID_NULL;
	if (self) {
		id = esif_ccb_thread_id(&self->thread);
	}
	return id;
}

