/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include <pthread.h>
#include <errno.h>

/* Required for OS Abstraction and Klocwork Compliance */

/* Emulate Critical Section */
typedef pthread_mutex_t esif_ccb_critical_section_t;

static ESIF_INLINE void esif_ccb_critical_section_init(esif_ccb_critical_section_t *csPtr)
{
	pthread_mutexattr_t attr;
	if (pthread_mutexattr_init(&attr) == 0) {
		if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0) {}
		if (pthread_mutex_init(csPtr, &attr) != 0) {}
		if (pthread_mutexattr_destroy(&attr) != 0) {}
	}
}
static ESIF_INLINE void esif_ccb_critical_section_uninit(esif_ccb_critical_section_t *csPtr)
{
	if (pthread_mutex_destroy(csPtr) != 0) {}
}
/* reentrant */
static ESIF_INLINE void esif_ccb_critical_section_lock(esif_ccb_critical_section_t *csPtr)
{
	if (pthread_mutex_lock(csPtr) != 0) {}
}
/* reentrant */
static ESIF_INLINE void esif_ccb_critical_section_unlock(esif_ccb_critical_section_t *csPtr)
{
	if (pthread_mutex_unlock(csPtr) != 0) {}
}

/* Mutex */
typedef pthread_mutex_t esif_ccb_mutex_t;

static ESIF_INLINE void esif_ccb_mutex_init(esif_ccb_mutex_t *mutexPtr)
{
	pthread_mutexattr_t attr;
	if (pthread_mutexattr_init(&attr) == 0) {
		if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0) {}
		if (pthread_mutex_init(mutexPtr, &attr) != 0) {}
		if (pthread_mutexattr_destroy(&attr) != 0) {}
	}
}
static ESIF_INLINE void esif_ccb_mutex_uninit(esif_ccb_mutex_t *mutexPtr)
{

	if (pthread_mutex_destroy(mutexPtr) != 0) {}
}
/* reentrant */
static ESIF_INLINE void esif_ccb_mutex_lock(esif_ccb_mutex_t *mutexPtr)
{
	if (pthread_mutex_lock(mutexPtr) != 0) {}
}
/* reentrant */
static ESIF_INLINE void esif_ccb_mutex_unlock(esif_ccb_mutex_t *mutexPtr)
{
	if (pthread_mutex_unlock(mutexPtr) != 0) {}
}

/* RW Lock */
typedef pthread_rwlock_t esif_ccb_lock_t;

static ESIF_INLINE void esif_ccb_lock_init(esif_ccb_lock_t *lockPtr)
{
	if (pthread_rwlock_init(lockPtr, NULL) != 0) {}
}
static ESIF_INLINE void esif_ccb_lock_uninit(esif_ccb_lock_t *lockPtr)
{
	if (pthread_rwlock_destroy(lockPtr) != 0) {}
}
/* NOT reentrant */
static ESIF_INLINE void esif_ccb_write_lock(esif_ccb_lock_t *lockPtr)
{
	/* Infinite Retry Required for OS Abstraction and Klocwork Compliance */
	int err = 0;
	while ((err = pthread_rwlock_wrlock(lockPtr)) != 0 && err == EDEADLK) {
		sleep(1);
	}
}
/* NOT reentrant */
static ESIF_INLINE void esif_ccb_write_unlock(esif_ccb_lock_t *lockPtr)
{
	if (pthread_rwlock_unlock(lockPtr) != 0) {}
}
/* reentrant */
static ESIF_INLINE void esif_ccb_read_lock(esif_ccb_lock_t *lockPtr)
{
	/* Infinite Retry Required for OS Abstraction and Klocwork Compliance */
	int err = 0;
	while ((err = pthread_rwlock_rdlock(lockPtr)) != 0 && err == EDEADLK) {
		sleep(1);
	}
}
/* reentrant */
static ESIF_INLINE void esif_ccb_read_unlock(esif_ccb_lock_t *lockPtr)
{
	if (pthread_rwlock_unlock(lockPtr) != 0) {}
}

#endif /* LINUX */
