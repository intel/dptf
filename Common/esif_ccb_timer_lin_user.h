/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#if defined(ESIF_ATTR_OS_LINUX) && defined(ESIF_ATTR_USER)

#include <signal.h>


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

#pragma pack(pop)

static ESIF_INLINE void esif_ccb_timer_obj_disable_timer(
	struct esif_timer_obj *self
	);


static ESIF_INLINE void esif_ccb_tmrm_cb_wrapper(
	const union sigval sv
	)
{
	esif_ccb_timer_handle_t cb_handle = {0};

	cb_handle = (esif_ccb_timer_handle_t) sv.sival_int;

	esif_ccb_tmrm_callback(cb_handle);
}


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
	pthread_attr_t attr;
	u64 freq_nanosecs = timeout * 1000 * 1000; /* convert msec to nsec */

	ESIF_ASSERT(self != NULL);

	pthread_attr_init(&attr);

	esif_ccb_timer_obj_disable_timer(self);

	se.sigev_notify = SIGEV_THREAD;
	se.sigev_notify_function   = esif_ccb_tmrm_cb_wrapper;
	se.sigev_value.sival_int   = self->timer_cb_handle;
	se.sigev_notify_attributes = &attr;

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
	pthread_attr_destroy(&attr);
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

#endif /* LINUX USER */
