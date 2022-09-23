/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

/*
 * Please see esif_ccb_timer.h for the interface definition.
 * This file contains OS-specific implementation code and NOT interface code.
 */


#include <signal.h>

typedef void (*esif_ccb_tmrm_cb_t)(esif_ccb_timer_handle_t cb_handle);

#pragma pack(push, 1)

struct esif_timer_obj {
	/* TODO: OS-specific items*/
	timer_t  timer;

	esif_ccb_timer_cb function_ptr;		/* Callback when timer fires */
	void *context_ptr;			/* Callback context if any */
	esif_ccb_time_t timeout;		/* Informative only */

	/*
	 * If the timer is set once, or multiple times while in the timer
	 * callback function, the timer will not actually be set until the
	 * callback is exited and then it will only be set with the last
	 * timeout value.
	 */
	u8 set_is_pending;
	esif_ccb_time_t pending_timeout;
	esif_ccb_timer_handle_t timer_cb_handle;
};

struct esif_tmrm_cb_obj {
	esif_ccb_tmrm_cb_t func;
	esif_ccb_timer_handle_t cb_handle;
};

#pragma pack(pop)

static ESIF_INLINE void esif_ccb_timer_obj_disable_timer(
	struct esif_timer_obj *self
	);

static ESIF_INLINE enum esif_rc esif_ccb_timer_obj_create_timer(
	struct esif_timer_obj *self
	)
{
	UNREFERENCED_PARAMETER(self);

	return ESIF_OK;
}

static ESIF_INLINE enum esif_rc esif_ccb_timer_obj_enable_timer(
	struct esif_timer_obj *self,
	const esif_ccb_time_t timeout	/* Timeout in msec */
	)
{
	enum esif_rc rc = ESIF_OK;
	struct sigevent se;
	struct itimerspec its;
	u64 freq_nanosecs = timeout * 1000 * 1000; /* convert msec to nsec */
	struct esif_tmrm_cb_obj *cb_object_ptr = NULL;

	ESIF_ASSERT(self != NULL);

	esif_ccb_timer_obj_disable_timer(self);

	/* Use native malloc() because if the associated timer is disabled
	 * (due to ESIF UF exiting for example), then the esif_ccb_memtrace functions
	 * will report memory leaks. This is not a real issue though because once
	 * the daemon process exists, all the heaps allocated by esif_ufd will
	 * be reclaimed by the OS.
	 */
	cb_object_ptr = (struct esif_tmrm_cb_obj *) malloc(sizeof(*cb_object_ptr));
	if (NULL == cb_object_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	cb_object_ptr->func = esif_ccb_tmrm_callback;
	cb_object_ptr->cb_handle = self->timer_cb_handle;

	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = SIGRTMIN;
	se.sigev_value.sival_ptr = cb_object_ptr;

	if (0 != timer_create(CLOCK_REALTIME,
		&se,
		(timer_t *)&self->timer)) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	} 

	its.it_value.tv_sec     = freq_nanosecs / 1000000000;
	its.it_value.tv_nsec    = freq_nanosecs % 1000000000;
	its.it_interval.tv_sec  = 0;
	its.it_interval.tv_nsec = 0;

	if (0 != timer_settime(self->timer, 0, &its, NULL)) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}
exit:
	return rc;
}


static ESIF_INLINE void esif_ccb_timer_obj_disable_timer(
	struct esif_timer_obj *self
	)
{
	ESIF_ASSERT(self != NULL);

	if (self->timer != 0) {
		timer_delete(self->timer);
		self->timer = 0;
	}
}

