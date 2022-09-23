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

#include "esif_ccb.h"
#include "esif_ccb_sem.h"



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






/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
