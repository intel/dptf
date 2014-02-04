/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_CCB_LOCK_H_
#define _ESIF_CCB_LOCK_H_

#include "esif.h"
#include "esif_ccb.h"
#include "esif_uf_ccb_thread.h"

/*******************************************************************************
** Kernel
*******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

    /* Use Shareable Read/Write Locks */

    #ifdef ESIF_ATTR_OS_LINUX
        typedef rwlock_t esif_ccb_lock_t;
        #define esif_ccb_lock_init(lockPtr)         rwlock_init(lockPtr)
        #define esif_ccb_lock_uninit(lockPtr)       /* not used */
        #define esif_ccb_write_lock(lockPtr)        write_lock(lockPtr)
        #define esif_ccb_write_unlock(lockPtr)      write_unlock(lockPtr)
        #define esif_ccb_read_lock(lockPtr)         read_lock(lockPtr)
        #define esif_ccb_read_unlock(lockPtr)       read_unlock(lockPtr)
    #endif /* ESIF_ATTR_OS_LINUX */

    #ifdef ESIF_ATTR_OS_WINDOWS
        #if 1
            typedef struct esif_ccb_lock_s {
                WDFSPINLOCK lock;                                   /* Spin Lock */
            } esif_ccb_lock_t;

            static ESIF_INLINE void esif_ccb_lock_init(esif_ccb_lock_t* lockPtr)
            { WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &lockPtr->lock); }
            #define esif_ccb_lock_uninit(lockPtr)                   /* not used */
            static ESIF_INLINE void esif_ccb_write_lock(esif_ccb_lock_t* lockPtr)
            { WdfSpinLockAcquire (lockPtr->lock); }
            static ESIF_INLINE void esif_ccb_write_unlock(esif_ccb_lock_t* lockPtr)
            { WdfSpinLockRelease(lockPtr->lock); }
            static ESIF_INLINE void esif_ccb_read_lock(esif_ccb_lock_t* lockPtr)
            { WdfSpinLockAcquire (lockPtr->lock); }
            static ESIF_INLINE void esif_ccb_read_unlock(esif_ccb_lock_t* lockPtr)
            { WdfSpinLockRelease(lockPtr->lock) ; }
        #else
            typedef u32 esif_ccb_lock_t;
            #define esif_ccb_lock_init(lockPtr)
            #define esif_ccb_lock_uninit(lockPtr)
            #define esif_ccb_write_lock(lockPtr)
            #define esif_ccb_write_unlock(lockPtr)
            #define esif_ccb_read_lock(lockPtr)
            #define esif_ccb_read_unlock(lockPtr)
        #endif
    #endif /* ESIF_ATTR_OS_WINDOWS */

#endif /* ESIF_ATTR_KERNEL */

/*******************************************************************************
** User
*******************************************************************************/

#ifdef ESIF_ATTR_USER

    #ifdef ESIF_ATTR_OS_LINUX
        /* Emulate Critical Section */
        typedef pthread_mutex_t esif_ccb_critical_section_t;
        #define esif_ccb_critical_section_init(cs)  pthread_mutex_init(cs, NULL)
        #define esif_ccb_critical_section_uninit    pthread_mutex_destroy
        #define esif_ccb_critical_section_enter     pthread_mutex_lock
        #define esif_ccb_critical_section_leave     pthread_mutex_unlock

        /* Mutex */
        typedef pthread_mutex_t esif_ccb_mutex_t;
        #define esif_ccb_mutex_init(mutex)          pthread_mutex_init(mutex, NULL)
        #define esif_ccb_mutex_destroy              pthread_mutex_destroy
        #define esif_ccb_mutex_lock                 pthread_mutex_lock
        #define esif_ccb_mutex_unlock               pthread_mutex_unlock

        /* RW Lock */
        typedef pthread_rwlock_t esif_ccb_lock_t;
        #define esif_ccb_lock_init(lockPtr)         pthread_rwlock_init(lockPtr, NULL)
        #define esif_ccb_lock_uninit(lockPtr)       pthread_rwlock_destroy(lockPtr)
        #define esif_ccb_write_lock(lockPtr)        pthread_rwlock_wrlock(lockPtr)
        #define esif_ccb_write_unlock(lockPtr)      pthread_rwlock_unlock(lockPtr)
        #define esif_ccb_read_lock(lockPtr)         pthread_rwlock_rdlock(lockPtr)
        #define esif_ccb_read_unlock(lockPtr)       pthread_rwlock_unlock(lockPtr)
    #endif /* ESIF_ATTR_OS_LINUX */

    #ifdef ESIF_ATTR_OS_WINDOWS
        /* Critical Section */
        typedef CRITICAL_SECTION esif_ccb_critical_section_t;
        #define esif_ccb_critical_section_init      InitializeCriticalSection
        #define esif_ccb_critical_section_uninit    DeleteCriticalSection
        #define esif_ccb_critical_section_enter     EnterCriticalSection
        #define esif_ccb_critical_section_leave     LeaveCriticalSection

        /* Mutex */
        typedef HANDLE esif_ccb_mutex_t;
        #define esif_ccb_mutex_init(mutexPtr)       *mutexPtr = CreateMutex(NULL, FALSE, NULL)
        #define esif_ccb_mutex_destroy(mutexPtr)    CloseHandle(*mutexPtr)
        #define esif_ccb_mutex_lock(mutexPtr)       WaitForSingleObject(*mutexPtr, INFINITE)
        #define esif_ccb_mutex_unlock(mutexPtr)     ReleaseMutex(*mutexPtr)

        /* RW Lock */
        typedef SRWLOCK esif_ccb_lock_t;
        #define esif_ccb_lock_init(lockPtr)         InitializeSRWLock(lockPtr)
        #define esif_ccb_lock_uninit(lockPtr)       /* not used */
        #define esif_ccb_write_lock(lockPtr)        AcquireSRWLockExclusive(lockPtr)
        #define esif_ccb_write_unlock(lockPtr)      ReleaseSRWLockExclusive(lockPtr)
        #define esif_ccb_read_lock(lockPtr)         AcquireSRWLockShared(lockPtr)
        #define esif_ccb_read_unlock(lockPtr)       ReleaseSRWLockShared(lockPtr)
    #endif /* ESIF_ATTR_OS_WINDOWS */

#endif /* ESIF_ATTR_USER */

#endif /* _ESIF_CCB_LOCK_H_ */