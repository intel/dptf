/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "esif_ccb_sem.h"

#if defined(ESIF_ATTR_USER)

#if defined(ESIF_ATTR_OS_WINDOWS)

static ESIF_INLINE eEsifError EsifTimedEventWait(
	esif_ccb_event_t *waitEventPtr,
	esif_ccb_time_t msDelay
	) 
{
	eEsifError rc = ESIF_OK;
	DWORD apiStatus = 0;

	esif_ccb_write_lock(&waitEventPtr->state_lock);

	if (waitEventPtr->signaled) {
		esif_ccb_write_unlock(&waitEventPtr->state_lock);
		goto exit;
	}

	waitEventPtr->waiters++;
	esif_ccb_write_unlock(&waitEventPtr->state_lock);

	apiStatus = WaitForSingleObject(waitEventPtr->sem_obj, (DWORD) msDelay);
	//
	// If thread released, but not due to signaling; decrement the waiter count
	//
	if (apiStatus != WAIT_OBJECT_0) {

		esif_ccb_write_lock(&waitEventPtr->state_lock);
		if (waitEventPtr->waiters) {
			waitEventPtr->waiters--;
		}
		esif_ccb_write_unlock(&waitEventPtr->state_lock);

		if (apiStatus != WAIT_TIMEOUT) {
			rc = ESIF_E_UNSPECIFIED;
		}
	}

exit:
	return rc;
}

#elif defined(ESIF_ATTR_OS_LINUX)

#include "esif_ccb_timer.h"

static ESIF_INLINE void EsifTimedEventWaitTimerCb(void *context)
{
	if (context != NULL) {
		esif_ccb_event_release_waiters((esif_ccb_event_t *)context);
	}
}

static ESIF_INLINE eEsifError EsifTimedEventWait(
	esif_ccb_event_t *waitEventPtr,
	esif_ccb_time_t msDelay
	) 
{
	eEsifError rc = ESIF_OK;
	esif_ccb_timer_t waitTimer = {0};
	
	rc = esif_ccb_timer_init(&waitTimer, EsifTimedEventWaitTimerCb, waitEventPtr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = esif_ccb_timer_set_msec(&waitTimer, msDelay);
	if (ESIF_OK != rc) {
		goto exit;
	}

	esif_ccb_event_wait(waitEventPtr);

	esif_ccb_timer_kill(&waitTimer);
exit:
	return rc;
}

#endif /* LINUX */

#endif /* USER */




/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
