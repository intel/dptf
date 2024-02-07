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
#include "esif_ccb_timer.h"
#include "esif_link_list.h"

/*
 * Limits the number of attempts to find a handle not in use as our use case is
 * expected to be small number of counters, but the handle counter may
 * eventually roll over.
 */
#define ESIF_CNT_HNDL_RETRIES_MAX 10000

/*
* Time delay between destroying all timers at exit and when the manager is
* destroyed.  This is required because timers which are destroyed may or may
* not already be queued for callback when the binary is being unloaded;
* specifically DLL/SO.  The code protects against the use of invalid contexts
* being used in the callback, but cannot protect against the code being
* removed without disallowing waiting for timer exits in callbacks.
*/
#define ESIF_TIMER_DISABLE_DELAY 50

/*
 * STRUCTURE DECLARATIONS
 */

struct esif_timer_manager {
	u8 enabled;	/* Indicates the manager lock is initialized */
	u8 marked_for_delete; /* Indicates no additional timers may be created */
	esif_ccb_lock_t mgr_lock;
	struct esif_link_list *timer_list_ptr; /* List of initialized timers */
};

struct esif_tmrm_item {
	struct esif_timer_obj *timer_obj_ptr;

	esif_ccb_timer_handle_t timer_handle;	/* Look-up handle */
	/*
	 * When the timer is set, a new CB handle is assigned
	 * It is used to uniquely identify a "set" instance so that you do not
	 * handle multiple threads associated with a single timer because
	 * cancelling was unsuccesful at the OS level when a timer was reset.
	 */
	esif_ccb_timer_handle_t timer_cb_handle;

	u8 is_in_cb; /* Is in CallBack */
	u8 marked_for_delete;

	/* List threads waiting for the timer callback to complete */
	struct esif_link_list *destroy_list_ptr;
};


/*
 * GLOBAL OBJECTS
 */
struct esif_timer_manager g_tmrm = {0};

u32 g_next_timer_handle = 0;
u32 g_next_timer_cb_handle = 0;


static enum esif_rc esif_ccb_timer_kill_w_event(
	esif_ccb_timer_t *timer_ptr,
	esif_ccb_event_t *event_ptr
	);

/*
 * TIMER MANAGER FUNCTION PROTOTYPES
 */
static enum esif_rc esif_ccb_tmrm_is_ready(void);

static enum esif_rc esif_ccb_tmrm_create_tmrm_item(
	const esif_ccb_timer_cb function_ptr,
	void *context_ptr,
	struct esif_tmrm_item **tmrm_item_ptr
	);

static void esif_ccb_tmrm_destroy_tmrm_item(
	struct esif_tmrm_item *self
	);

static void esif_ccb_tmrm_add_destroy_event(
	struct esif_tmrm_item *self,
	esif_ccb_event_t *event_ptr
	);

static void esif_ccb_tmrm_signal_waiters(
	void *data_ptr
	);

static struct esif_link_list_node *esif_ccb_tmrm_find_timer_node_wlock(
	esif_ccb_timer_handle_t handle
	);

static struct esif_link_list_node *esif_ccb_tmrm_find_timer_node_by_cb_wlock(
	esif_ccb_timer_handle_t cb_handle
	);

static enum esif_rc esif_ccb_tmrm_get_next_handle(
	esif_ccb_timer_handle_t *handle_ptr
	);
	
static void esif_ccb_tmrm_get_next_cb_handle_wlock(
	esif_ccb_timer_handle_t *handle_ptr
	);
	
static void esif_ccb_tmrm_destroy_timer_node_wlock(
	struct esif_link_list_node *node_ptr
	);


static enum esif_rc esif_ccb_tmrm_get_first_timer(
	esif_ccb_timer_t *timer_ptr
	);
/*
 * TIMER OBJECT FUNCTION PROTOTYPES
 */
static enum esif_rc esif_ccb_timer_obj_create(
	const esif_ccb_timer_cb function_ptr,
	void *context_ptr,
	struct esif_timer_obj **timer_obj_ptr
	);

static void esif_ccb_timer_obj_destroy(
	struct esif_timer_obj *self
	);

static void esif_ccb_timer_obj_save_pending_timeout(
	struct esif_timer_obj *self,
	esif_ccb_time_t timeout,
	esif_ccb_timer_handle_t timer_cb_handle
	);

static enum esif_rc esif_ccb_timer_obj_set_pending_timeout(
	struct esif_timer_obj *self
	);

static void esif_ccb_timer_obj_call_cb(
	struct esif_timer_obj *self
	);

/*
 * FUNCTION DEFINITIONS
 */

/* Every init should have a matching kill */
enum esif_rc esif_ccb_timer_init(
	esif_ccb_timer_t *timer_ptr,
	esif_ccb_timer_cb function_ptr,	/* Callback when timer fires */
	void *context_ptr		/* Callback context if any */
	)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_tmrm_item *tmrm_item_ptr = NULL;

	if ((NULL == timer_ptr) || (NULL == function_ptr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	rc = esif_ccb_tmrm_is_ready();
	if (rc != ESIF_OK)
		goto exit;

	rc = esif_ccb_tmrm_create_tmrm_item(function_ptr,
		context_ptr,
		&tmrm_item_ptr);
	if (rc != ESIF_OK)
		goto exit;

	/* Place the handle into the timer being initialized*/
	timer_ptr->timer_handle = tmrm_item_ptr->timer_handle;

	/*
	 * We create the manager list dynamically so that we don't have to call
	 * init/exit functions and can destroy the linked list
	 */
	esif_ccb_write_lock(&g_tmrm.mgr_lock);
	if (NULL == g_tmrm.timer_list_ptr) {

		g_tmrm.timer_list_ptr = esif_link_list_create();
		if(NULL == g_tmrm.timer_list_ptr) {
			rc = ESIF_E_NO_MEMORY;
			goto lock_exit;
		}
	}
	rc = esif_link_list_add_at_back(g_tmrm.timer_list_ptr, tmrm_item_ptr);
lock_exit:
	esif_ccb_write_unlock(&g_tmrm.mgr_lock);
exit:
	if (rc != ESIF_OK)
		esif_ccb_tmrm_destroy_tmrm_item(tmrm_item_ptr);

	return rc;
}


enum esif_rc esif_ccb_timer_kill(
	esif_ccb_timer_t *timer_ptr
	)
{
	return esif_ccb_timer_kill_w_event(timer_ptr, NULL);
}


enum esif_rc esif_ccb_timer_kill_w_wait(
	esif_ccb_timer_t *timer_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	esif_ccb_event_t kill_event;

	esif_ccb_event_init(&kill_event);
	rc = esif_ccb_timer_kill_w_event(timer_ptr, &kill_event);
	esif_ccb_event_wait(&kill_event);
	esif_ccb_event_uninit(&kill_event);

	return rc;
}


static enum esif_rc esif_ccb_timer_kill_w_event(
	esif_ccb_timer_t *timer_ptr,
	esif_ccb_event_t *event_ptr
	)
{
	enum esif_rc rc = ESIF_E_UNSPECIFIED;
	struct esif_tmrm_item *tmrm_item_ptr = NULL;
	struct esif_link_list_node *node_ptr = NULL;

	if (NULL == timer_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (!g_tmrm.enabled)
		goto exit;

	esif_ccb_write_lock(&g_tmrm.mgr_lock);

	node_ptr = esif_ccb_tmrm_find_timer_node_wlock(timer_ptr->timer_handle);
	if ((NULL == node_ptr) || (NULL == node_ptr->data_ptr)) {
		rc = ESIF_E_INVALID_HANDLE;
		goto lock_exit;
	}

	tmrm_item_ptr = (struct esif_tmrm_item *)node_ptr->data_ptr;

	/* Mark for delete in case it is in the callback */
	tmrm_item_ptr->marked_for_delete = ESIF_TRUE;

	esif_ccb_tmrm_add_destroy_event(tmrm_item_ptr, event_ptr);

	/* If not in callback, the timer can be destroyed now */
	if (!tmrm_item_ptr->is_in_cb) {
		esif_ccb_tmrm_destroy_timer_node_wlock(node_ptr);
	}
	rc = ESIF_OK;
lock_exit:
	esif_ccb_write_unlock(&g_tmrm.mgr_lock);
exit:
	if ((rc != ESIF_OK) && (event_ptr != NULL)) {
		esif_ccb_event_set(event_ptr);
	}

	return rc;
}


/*
 * Sets a timer timeout in ms
 * Notes:  0 is an invalid timeout
 * The code will cancel any current timeout if possible before setting the new
 * timeout. If in the callback, the timeout will not be set until the function
 * exits.
 */
enum esif_rc esif_ccb_timer_set_msec(
	esif_ccb_timer_t *timer_ptr,
	const esif_ccb_time_t timeout	/* Timeout in msec */
	)
{
	enum esif_rc rc = ESIF_E_UNSPECIFIED;
	struct esif_link_list_node *node_ptr = NULL;
	struct esif_tmrm_item *tmrm_item_ptr = NULL;
	struct esif_timer_obj *timer_obj_ptr = NULL;
	esif_ccb_timer_handle_t timer_cb_handle = {0};

	if (NULL == timer_ptr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (0 == timeout) {
		rc = ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS;
		goto exit;
	}

	/* Order issue check */
	if (!g_tmrm.enabled) 
		goto exit;

	esif_ccb_write_lock(&g_tmrm.mgr_lock);

	node_ptr = esif_ccb_tmrm_find_timer_node_wlock(timer_ptr->timer_handle);
	if ((NULL == node_ptr) || (NULL == node_ptr->data_ptr)) {	
		rc = ESIF_E_INVALID_HANDLE;
		goto lock_exit;
	}

	tmrm_item_ptr = (struct esif_tmrm_item *)node_ptr->data_ptr;

	if (tmrm_item_ptr->marked_for_delete) {
		rc = ESIF_E_INVALID_HANDLE;
		goto lock_exit;
	}

	esif_ccb_tmrm_get_next_cb_handle_wlock(&timer_cb_handle);
	tmrm_item_ptr->timer_cb_handle = timer_cb_handle;

	timer_obj_ptr = tmrm_item_ptr->timer_obj_ptr;
	esif_ccb_timer_obj_save_pending_timeout(timer_obj_ptr,
		timeout,
		timer_cb_handle);

	if (!tmrm_item_ptr->is_in_cb) {
		rc = esif_ccb_timer_obj_set_pending_timeout(timer_obj_ptr);
	} else {
		rc = ESIF_OK;
	}
lock_exit:
	esif_ccb_write_unlock(&g_tmrm.mgr_lock);
exit:
	return rc;
}


/*
 * This functions is expected to be called when the system is in a "known" state
 * before any attempt to create a timer
 */
enum esif_rc esif_ccb_tmrm_init(void)
{
	esif_ccb_lock_init(&g_tmrm.mgr_lock);
	g_tmrm.enabled = ESIF_TRUE;
	return ESIF_OK;
}


static enum esif_rc esif_ccb_tmrm_is_ready(void)
{
	enum esif_rc rc = ESIF_OK;

	if (g_tmrm.marked_for_delete) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	if (g_tmrm.enabled)
		goto exit;

	rc = esif_ccb_tmrm_init();
exit:
	return rc;
}


/*
 * This functions is expected to be called when the system is in a "known" state
 * where no attempts to create any timers are in flight as this function
 * destroys the lock controlling synchronization
 */
void esif_ccb_tmrm_exit(void)
{
	esif_ccb_timer_t cur_timer;

	if (!g_tmrm.enabled)
		goto exit;

	/* Mark to make sure no further timers are created */
	g_tmrm.marked_for_delete = ESIF_TRUE;

	/* Kill all existing timers and wait  for destruction */
	while(esif_ccb_tmrm_get_first_timer(&cur_timer) == ESIF_OK) {
		esif_ccb_timer_kill_w_wait(&cur_timer);
	}

	/* Wait for any possible callbacks to be processed before exiting */
	esif_ccb_sleep_msec(ESIF_TIMER_DISABLE_DELAY);

	g_tmrm.enabled = ESIF_FALSE;
	esif_ccb_lock_uninit(&g_tmrm.mgr_lock);
	g_tmrm.marked_for_delete = ESIF_FALSE;
exit:
	return;
}


static enum esif_rc esif_ccb_tmrm_get_first_timer(
	esif_ccb_timer_t *timer_ptr
	)
{
	enum esif_rc rc = ESIF_E_UNSPECIFIED;
	struct esif_link_list_node *node_ptr = NULL;
	struct esif_tmrm_item *tmrm_item_ptr = NULL;

	esif_ccb_write_lock(&g_tmrm.mgr_lock);

	if (NULL == g_tmrm.timer_list_ptr)
		goto exit;

	node_ptr = g_tmrm.timer_list_ptr->head_ptr;
	if (NULL == node_ptr)
		goto exit;

	tmrm_item_ptr = (struct esif_tmrm_item *)node_ptr->data_ptr;
	if (NULL == tmrm_item_ptr) /* Should never happen */
		goto exit;

	timer_ptr->timer_handle = tmrm_item_ptr->timer_handle;
	rc = ESIF_OK;
exit:
	esif_ccb_write_unlock(&g_tmrm.mgr_lock);
	return rc;
}


void esif_ccb_tmrm_callback(
	esif_ccb_timer_handle_t cb_handle
	)
{
	struct esif_link_list_node *node_ptr = NULL;
	struct esif_tmrm_item *tmrm_item_ptr = NULL;
	struct esif_timer_obj *timer_obj_ptr = NULL;

	esif_ccb_write_lock(&g_tmrm.mgr_lock);

	node_ptr = esif_ccb_tmrm_find_timer_node_by_cb_wlock(cb_handle);

	if ((NULL == node_ptr) || (NULL == node_ptr->data_ptr)) {
		goto lock_exit;	
	}

	tmrm_item_ptr = (struct esif_tmrm_item *)node_ptr->data_ptr;

	/* Clear the CB handle to lower probability of hitting same handle */
	tmrm_item_ptr->timer_cb_handle = 0;
	tmrm_item_ptr->is_in_cb = ESIF_TRUE;

	esif_ccb_write_unlock(&g_tmrm.mgr_lock);

	/* Call the timer callback function */
	timer_obj_ptr = tmrm_item_ptr->timer_obj_ptr;
	esif_ccb_timer_obj_call_cb(timer_obj_ptr);

	/*
	 * Upon return, perform post processing
	 * Note:  The item and node pointers will still be valid as the node
	 * will not be removed while in the callback function
	 */
	esif_ccb_write_lock(&g_tmrm.mgr_lock);

	tmrm_item_ptr->is_in_cb = ESIF_FALSE;

	if (tmrm_item_ptr->marked_for_delete) {
		esif_ccb_tmrm_destroy_timer_node_wlock(node_ptr);
		goto lock_exit;
	}

	 esif_ccb_timer_obj_set_pending_timeout(timer_obj_ptr);
lock_exit:
	esif_ccb_write_unlock(&g_tmrm.mgr_lock);
	return;
}


static enum esif_rc esif_ccb_tmrm_create_tmrm_item(
	const esif_ccb_timer_cb function_ptr,	/* Callback when timer fires */
	void *context_ptr,			/* Callback context if any */
	struct esif_tmrm_item **tmrm_item_ptr
	)
{
	enum esif_rc rc = ESIF_E_NO_MEMORY;
	struct esif_tmrm_item *new_item_ptr = NULL;
	struct esif_timer_obj *timer_obj_ptr = NULL;
	esif_ccb_timer_handle_t handle = 0;

	ESIF_ASSERT(tmrm_item_ptr != NULL);
	ESIF_ASSERT(function_ptr != NULL);

	new_item_ptr = (struct esif_tmrm_item *)
		esif_ccb_malloc(sizeof(*new_item_ptr));
	if (NULL == new_item_ptr)
		goto exit;

	new_item_ptr->destroy_list_ptr = esif_link_list_create();
	if (NULL == new_item_ptr->destroy_list_ptr)
		goto exit;	

	rc = esif_ccb_timer_obj_create(function_ptr,
		context_ptr,
		&timer_obj_ptr);
	if (rc != ESIF_OK)
		goto exit;

	new_item_ptr->timer_obj_ptr = timer_obj_ptr;

	rc = esif_ccb_tmrm_get_next_handle(&handle);
	if (rc != ESIF_OK)
		goto exit;

	new_item_ptr->timer_handle = handle;

	*tmrm_item_ptr = new_item_ptr;
exit:
	if (rc != ESIF_OK)
		esif_ccb_tmrm_destroy_tmrm_item(new_item_ptr);

	return rc;
}


static enum esif_rc esif_ccb_tmrm_get_next_handle(
	esif_ccb_timer_handle_t *handle_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_link_list_node *node_ptr = NULL;
	esif_ccb_timer_t timer = {0};
	u32 try_count = 0;
	/*
	 * Our use case is that we will have a small number of timers ever
	 * active; however the handle counter may eventually roll over to a
	 * counter that has been there the whole time.  So, we make sure that
	 * the handle doesn't overlap. We also protect against an infinite loop.
	 */
	esif_ccb_write_lock(&g_tmrm.mgr_lock);

	do {
		try_count++;
		timer.timer_handle = (esif_ccb_timer_handle_t)(size_t)++g_next_timer_handle;
		node_ptr = esif_ccb_tmrm_find_timer_node_wlock(timer.timer_handle);
	} while ((node_ptr != NULL) && (try_count < ESIF_CNT_HNDL_RETRIES_MAX));

	esif_ccb_write_unlock(&g_tmrm.mgr_lock);

	if(node_ptr != NULL) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	*handle_ptr = timer.timer_handle;
exit:
	return rc;
}


static void esif_ccb_tmrm_get_next_cb_handle_wlock(
	esif_ccb_timer_handle_t *handle_ptr
	)
{
	g_next_timer_cb_handle++;
	if (0 == g_next_timer_cb_handle)
		g_next_timer_cb_handle++;

	*handle_ptr = (esif_ccb_timer_handle_t)(size_t)g_next_timer_cb_handle;
}


static struct esif_link_list_node *esif_ccb_tmrm_find_timer_node_wlock(
	esif_ccb_timer_handle_t handle
	)
{
	struct esif_link_list_node *node_ptr = NULL;
	struct esif_tmrm_item *tmrm_item_ptr = NULL;

	if (NULL == g_tmrm.timer_list_ptr)
		goto exit;

	node_ptr = g_tmrm.timer_list_ptr->head_ptr;
	while (node_ptr != NULL) {
		tmrm_item_ptr = (struct esif_tmrm_item *)node_ptr->data_ptr;
		if (NULL == tmrm_item_ptr) /* Should never happen */
			continue;

		if (handle == tmrm_item_ptr->timer_handle)
			break;	
		node_ptr = node_ptr->next_ptr;
	}
exit:
	return node_ptr;
}


static struct esif_link_list_node *esif_ccb_tmrm_find_timer_node_by_cb_wlock(
	esif_ccb_timer_handle_t cb_handle
	)
{
	struct esif_link_list_node *node_ptr = NULL;
	struct esif_tmrm_item *tmrm_item_ptr = NULL;

	if (NULL == g_tmrm.timer_list_ptr)
		goto exit;

	node_ptr = g_tmrm.timer_list_ptr->head_ptr;
	while (node_ptr != NULL) {
		tmrm_item_ptr = (struct esif_tmrm_item *)node_ptr->data_ptr;
		if (NULL == tmrm_item_ptr) /* Should never happen */
			continue;

		if (cb_handle == tmrm_item_ptr->timer_cb_handle)
			break;	
		node_ptr = node_ptr->next_ptr;
	}
exit:
	return node_ptr;
}


static void esif_ccb_tmrm_destroy_timer_node_wlock(
	struct esif_link_list_node *node_ptr
	)
{
	struct esif_tmrm_item *tmrm_item_ptr = NULL;

	ESIF_ASSERT(node_ptr != NULL);
	ESIF_ASSERT(node_ptr->data_ptr != NULL);

	tmrm_item_ptr = (struct esif_tmrm_item *)node_ptr->data_ptr;
	esif_ccb_tmrm_destroy_tmrm_item(tmrm_item_ptr);

	esif_link_list_node_remove(g_tmrm.timer_list_ptr, node_ptr);
			
	/* If the manager list is empty, destroy it */
	if(NULL == g_tmrm.timer_list_ptr->head_ptr) {
		esif_link_list_destroy(g_tmrm.timer_list_ptr);
		g_tmrm.timer_list_ptr = NULL;
	}
	return;
}


static void esif_ccb_tmrm_destroy_tmrm_item(
	struct esif_tmrm_item *self
	)
{
	if (NULL == self)
		goto exit;

	esif_link_list_free_data_and_destroy(self->destroy_list_ptr, esif_ccb_tmrm_signal_waiters);

	if (self->timer_obj_ptr !=  NULL)
		esif_ccb_timer_obj_destroy(self->timer_obj_ptr);

	esif_ccb_free(self);
exit:
	return;
}

	
static void esif_ccb_tmrm_signal_waiters(
	void *data_ptr
	)
{
	esif_ccb_event_set((esif_ccb_event_t *)data_ptr);
}


static void esif_ccb_tmrm_add_destroy_event(
	struct esif_tmrm_item *self,
	esif_ccb_event_t *event_ptr
	)
{
	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(self->destroy_list_ptr != NULL);

	if (event_ptr != NULL) {
		esif_link_list_add_at_back(self->destroy_list_ptr, event_ptr);
	}
}


static enum esif_rc esif_ccb_timer_obj_create(
	const esif_ccb_timer_cb function_ptr,	/* Callback when timer fires */
	void *context_ptr,			/* Callback context if any */
	struct esif_timer_obj **timer_obj_ptr
	)
{
	enum esif_rc rc = ESIF_OK;
	struct esif_timer_obj *new_timer_obj_ptr = NULL;

	ESIF_ASSERT(function_ptr != NULL);
	ESIF_ASSERT(timer_obj_ptr != NULL);

	new_timer_obj_ptr = (struct esif_timer_obj *)esif_ccb_malloc(sizeof(*new_timer_obj_ptr));
	if (NULL == new_timer_obj_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	rc = esif_ccb_timer_obj_create_timer(new_timer_obj_ptr);
	if (rc != ESIF_OK)
		goto exit;

	new_timer_obj_ptr->function_ptr = function_ptr;
	new_timer_obj_ptr->context_ptr = context_ptr;

	*timer_obj_ptr = new_timer_obj_ptr;
exit:
	if (rc != ESIF_OK)
		esif_ccb_timer_obj_destroy(new_timer_obj_ptr);

	return rc;
}


static void esif_ccb_timer_obj_destroy(
	struct esif_timer_obj *self
	)
{
	/*
	 * The destruction function may have to deal with 'partial' objects, so
	 * it and all called sub-functions should do full NULL checks.
	 */
	if (NULL == self)
		return;

	esif_ccb_timer_obj_disable_timer(self);

	esif_ccb_free(self);
}


static void esif_ccb_timer_obj_call_cb(
	struct esif_timer_obj *self
	)
{
	ESIF_ASSERT(self != NULL);
	self->function_ptr(self->context_ptr);
}


static void esif_ccb_timer_obj_save_pending_timeout(
	struct esif_timer_obj *self,
	esif_ccb_time_t timeout,
	esif_ccb_timer_handle_t timer_cb_handle
	)
{
	ESIF_ASSERT(self != NULL);

	self->set_is_pending = ESIF_TRUE;
	self->pending_timeout = timeout;
	self->timer_cb_handle = timer_cb_handle;
}


static enum esif_rc esif_ccb_timer_obj_set_pending_timeout(
	struct esif_timer_obj *self
	)
{
	enum esif_rc rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	if (!self->set_is_pending)
		goto exit;

	self->set_is_pending = ESIF_FALSE;

	rc = esif_ccb_timer_obj_enable_timer(self, self->pending_timeout);
exit:
	return rc;
}
