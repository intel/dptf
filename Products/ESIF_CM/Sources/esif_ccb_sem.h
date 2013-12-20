/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#ifndef _ESIF_CCB_SEM_H_
#define _ESIF_CCB_SEM_H_

#ifdef ESIF_ATTR_KERNEL

#ifdef ESIF_ATTR_OS_LINUX
typedef struct semaphore esif_ccb_sem_t;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)
#define esif_ccb_sem_init(semPtr) sema_init(semPtr, 0)
#else /* linux version else */
#define esif_ccb_sem_init(semPtr) init_MUTEX_LOCKED(semPtr)
#endif /* LINUX_VERSION_CODE */
#define esif_ccb_sem_uninit(semPtr)
#define esif_ccb_sem_up(semPtr) up(semPtr)
#define esif_ccb_sem_down(semPtr) down(semPtr)

static ESIF_INLINE int esif_ccb_sem_try_down (
	esif_ccb_sem_t *sem_ptr,
	u32 us_timeout
	)
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

static ESIF_INLINE int esif_ccb_sem_try_down (
	esif_ccb_sem_t *sem_ptr,
	u32 us_timeout
	)
{
	UNREFERENCED_PARAMETER(sem_ptr);
	UNREFERENCED_PARAMETER(us_timeout);
	return 0;
}


#endif /* ESIF_ATTR_OS_WINDOWS */

#endif /* ESIF_ATTR_KERNEL */
#ifdef ESIF_ATTR_USER

#include "esif.h"

#ifdef ESIF_ATTR_OS_LINUX

    #include <semaphore.h>

/* Semphore */
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
#define esif_ccb_sem_init(semPtr) *semPtr = CreateSemaphore(NULL, 0, \
							    0x7fffffff, NULL)
#define esif_ccb_sem_uninit(semPtr) CloseHandle(*semPtr)
#define esif_ccb_sem_up(semPtr) WaitForSingleObject(*semPtr, INFINITE)
#define esif_ccb_sem_down(semPtr) ReleaseSemaphore(*semPtr, 1, NULL)

/* Event */
typedef HANDLE esif_ccb_event_t;
#define esif_ccb_event_create(eventPtr) *eventPtr = CreateEvent(NULL, FALSE, \
								FALSE, NULL)
#define esif_ccb_event_destroy(eventPtr) CloseHandle(*eventPtr)
#define esif_ccb_event_set(eventPtr) SetEvent(*eventPtr)
#define esif_ccb_event_reset(eventPtr) ResetEvent(*eventPtr)

#endif /* ESIF_ATTR_OS_WINDOWS */
#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_CCB_SEM_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
