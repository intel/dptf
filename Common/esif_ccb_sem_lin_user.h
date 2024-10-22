/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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


#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

/* Semphore */
typedef sem_t esif_ccb_sem_t;
#define esif_ccb_sem_init(semPtr) sem_init(semPtr, 0, 0)
#define esif_ccb_sem_uninit(semPtr) sem_destroy(semPtr)
#define esif_ccb_sem_up(semPtr) sem_post(semPtr)

static ESIF_INLINE int esif_ccb_sem_down(sem_t *sem) {
    int result;
    while (((result = sem_wait(sem)) == -1) &&
			(errno == EINTR)) {
        // Retry if interrupted by a signal
        continue;
    }
    return result;
}
/*
 * Returns ESIF_OK on success, ESIF_E_TIMEOUT if the timer expired, else
 * ESIF_E_UNSPECIFIED.
 */
static ESIF_INLINE enum esif_rc esif_ccb_sem_try_down (
	esif_ccb_sem_t *sem_ptr,
	u32 ms_timeout
	)
{
	enum esif_rc rc = ESIF_OK;
	int api_status = 0;
	struct timespec timeout = {0};

	if (0 == clock_gettime(CLOCK_REALTIME, &timeout)) {
		u64 nsec = (u64)timeout.tv_nsec + ((u64)ms_timeout * 1000000);
		if (nsec > 1000000000) {
			timeout.tv_sec += (time_t)(nsec / 1000000000);
			timeout.tv_nsec = (long)(nsec % 1000000000);
		}
		else {
			timeout.tv_nsec = (long)nsec;
		}
		api_status = sem_timedwait(sem_ptr, &timeout);
		if (-1 == api_status) {
			if (ETIMEDOUT == errno)
				rc = ESIF_E_TIMEOUT;
			else
				rc = ESIF_E_UNSPECIFIED;
		}
	}
	return rc;
}

/* Conditional Variable */
typedef pthread_cond_t esif_ccb_cond_t;
#define esif_ccb_cond_init(cond) pthread_cond_init(cond, NULL)
#define esif_ccb_cond_destroy pthread_cond_destroy
#define esif_ccb_cond_wait pthread_cond_wait
#define esif_ccb_cond_signal pthread_cond_signal
#define esif_ccb_cond_broadcast pthread_cond_broadcast

