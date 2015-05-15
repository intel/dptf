/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

/* Initialize a Linux pthread mutex using the given type */
static ESIF_INLINE void pthread_mutex_init_type(pthread_mutex_t *mutex, int type)
{
	static pthread_mutexattr_t attr;
	pthread_mutexattr_settype(&attr, type);
	pthread_mutex_init(mutex, &attr);
}

/* Emulate Critical Section */
typedef pthread_mutex_t esif_ccb_critical_section_t;
#define esif_ccb_critical_section_init(csPtr)	pthread_mutex_init_type(csPtr, PTHREAD_MUTEX_RECURSIVE)
#define esif_ccb_critical_section_uninit(csPtr)	pthread_mutex_destroy(csPtr)
#define esif_ccb_critical_section_enter(csPtr)	pthread_mutex_lock(csPtr) /* reentrant */
#define esif_ccb_critical_section_leave(csPtr)	pthread_mutex_unlock(csPtr) /* reentrant */

/* Mutex */
typedef pthread_mutex_t esif_ccb_mutex_t;
#define esif_ccb_mutex_init(mutexPtr)	pthread_mutex_init_type(mutexPtr, PTHREAD_MUTEX_RECURSIVE)
#define esif_ccb_mutex_uninit(mutexPtr)	pthread_mutex_destroy(mutexPtr)
#define esif_ccb_mutex_lock(mutexPtr)	pthread_mutex_lock(mutexPtr) /* reentrant */
#define esif_ccb_mutex_unlock(mutexPtr)	pthread_mutex_unlock(mutexPtr) /* reentrant */

/* RW Lock */
typedef pthread_rwlock_t esif_ccb_lock_t;
#define esif_ccb_lock_init(lockPtr)     pthread_rwlock_init(lockPtr, NULL)
#define esif_ccb_lock_uninit(lockPtr)   pthread_rwlock_destroy(lockPtr)
#define esif_ccb_write_lock(lockPtr)    pthread_rwlock_wrlock(lockPtr) /* NOT reentrant */
#define esif_ccb_write_unlock(lockPtr)  pthread_rwlock_unlock(lockPtr) /* NOT reentrant */
#define esif_ccb_read_lock(lockPtr)     pthread_rwlock_rdlock(lockPtr) /* reentrant */
#define esif_ccb_read_unlock(lockPtr)   pthread_rwlock_unlock(lockPtr) /* reentrant */

#endif /* LINUX */
