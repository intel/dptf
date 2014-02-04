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

#ifndef _ESIF_CCB_SEM_H_
#define _ESIF_CCB_SEM_H_

#include "esif.h"
#include "esif_ccb.h"
#include "esif_ccb_lock.h"

/*******************************************************************************
** Kernel
*******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

    #ifdef ESIF_ATTR_OS_LINUX
        typedef struct semaphore esif_ccb_sem_t;

        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
            #define esif_ccb_sem_init(semPtr) sema_init(semPtr, 0)
        #else /* linux version else */
                #define esif_ccb_sem_init(semPtr) init_MUTEX_LOCKED(semPtr)
        #endif /* LINUX_VERSION_CODE */
        #define esif_ccb_sem_uninit(semPtr)
        #define esif_ccb_sem_up(semPtr) up(semPtr)
        #define esif_ccb_sem_down(semPtr) down(semPtr)

        static ESIF_INLINE int
        esif_ccb_sem_try_down(esif_ccb_sem_t *sem_ptr, u32 us_timeout)
        {
            return down_timeout(sem_ptr, usecs_to_jiffies(us_timeout));
        }
    #endif /* ESIF_ATTR_OS_LINUX */

    #ifdef ESIF_ATTR_OS_WINDOWS
        typedef u32 esif_ccb_sem_t;
        #define esif_ccb_sem_init(semPtr)
        #define esif_ccb_sem_uninit(semPtr)
        #define esif_ccb_sem_up(semPtr)
        #define esif_ccb_sem_down(semPtr)

        static ESIF_INLINE int
        esif_ccb_sem_try_down(esif_ccb_sem_t *sem_ptr, u32 us_timeout)
        {
            UNREFERENCED_PARAMETER(sem_ptr);
            UNREFERENCED_PARAMETER(us_timeout);
            return 0;
        }
    #endif /* ESIF_ATTR_OS_WINDOWS */

#endif /* ESIF_ATTR_KERNEL */

/*******************************************************************************
** User
*******************************************************************************/

#ifdef ESIF_ATTR_USER

    #ifdef ESIF_ATTR_OS_LINUX

        #include <semaphore.h>

        /* Semaphore */
        typedef sem_t esif_ccb_sem_t;
        #define esif_ccb_sem_init(semPtr) sem_init(semPtr, 0, 0);
        #define esif_ccb_sem_uninit(semPtr) sem_destroy(semPtr);
        #define esif_ccb_sem_up(semPtr) sem_wait(semPtr)
        #define esif_ccb_sem_down(semPtr) sem_post(semPtr)

        /* Event */
        typedef sem_t esif_ccb_event_t;
        #define esif_ccb_event_create(eventPtr) sem_init(eventPtr, 0, 0);
        #define esif_ccb_event_destroy(eventPtr) sem_destroy(semPtr);
        #define esif_ccb_event_set(eventPtr) sem_wait(*eventPtr)
        #define esif_ccb_event_reset(eventPtr) sem_post(*eventPtr)

        /* Conditional Variable */
        typedef pthread_cond_t esif_ccb_cond_t;
        #define esif_ccb_cond_init(cond) pthread_cond_init(cond, NULL)
        #define esif_ccb_cond_destroy pthread_cond_destroy
        #define esif_ccb_cond_wait pthread_cond_wait
        #define esif_ccb_cond_signal pthread_cond_signal
        #define esif_ccb_cond_broadcast pthread_cond_broadcast

    #endif /* ESIF_ATTR_OS_LINUX */

    #ifdef ESIF_ATTR_OS_WINDOWS

        /* Semaphore */
        typedef HANDLE esif_ccb_sem_t;
        #define esif_ccb_sem_init(semPtr) *semPtr = CreateSemaphore(NULL, 0, 0x7fffffff, NULL)
        #define esif_ccb_sem_uninit(semPtr) CloseHandle(*semPtr)
        #define esif_ccb_sem_up(semPtr) WaitForSingleObject(*semPtr, INFINITE)
        #define esif_ccb_sem_down(semPtr) ReleaseSemaphore(*semPtr, 1, NULL)

        /* Event */
        typedef HANDLE esif_ccb_event_t;
        #define esif_ccb_event_create(eventPtr) *eventPtr = CreateEvent(NULL, FALSE, FALSE, NULL)
        #define esif_ccb_event_destroy(eventPtr) CloseHandle(*eventPtr)
        #define esif_ccb_event_set(eventPtr) SetEvent(*eventPtr)
        #define esif_ccb_event_reset(eventPtr) ResetEvent(*eventPtr)

    #endif /* ESIF_ATTR_OS_WINDOWS */

#endif /* ESIF_ATTR_USER */

#endif /* _ESIF_CCB_SEM_H_ */