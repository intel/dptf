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

#if defined(ESIF_ATTR_OS_LINUX) && defined(ESIF_ATTR_USER)

#include <semaphore.h>
#include <pthread.h>

/* Semphore */
typedef sem_t esif_ccb_sem_t;
#define esif_ccb_sem_init(semPtr) sem_init(semPtr, 0, 0)
#define esif_ccb_sem_uninit(semPtr) sem_destroy(semPtr)
#define esif_ccb_sem_up(semPtr) sem_post(semPtr)
#define esif_ccb_sem_down(semPtr) sem_wait(semPtr)
/* Defined only for the queue code to build, but not actually used */
static ESIF_INLINE int esif_ccb_sem_try_down (
	esif_ccb_sem_t *sem_ptr,
	u32 us_timeout
	)
{
	UNREFERENCED_PARAMETER(sem_ptr);
	UNREFERENCED_PARAMETER(us_timeout);
	return 0;
}

/* Conditional Variable */
typedef pthread_cond_t esif_ccb_cond_t;
#define esif_ccb_cond_init(cond) pthread_cond_init(cond, NULL)
#define esif_ccb_cond_destroy pthread_cond_destroy
#define esif_ccb_cond_wait pthread_cond_wait
#define esif_ccb_cond_signal pthread_cond_signal
#define esif_ccb_cond_broadcast pthread_cond_broadcast

#endif /* LINUX */
