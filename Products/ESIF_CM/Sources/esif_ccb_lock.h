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

#ifndef _ESIF_CCB_LOCK_H_
#define _ESIF_CCB_LOCK_H_

/******************************************************************************
*   KERNEL LOCKS
******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

/* Use Shareable Read/Write Locks */

#ifdef ESIF_ATTR_OS_LINUX
typedef rwlock_t esif_ccb_lock_t;
typedef rwlock_t esif_ccb_low_priority_thread_lock_t;
#define esif_ccb_lock_init(lockPtr)    rwlock_init(lockPtr)
#define esif_ccb_lock_uninit(lockPtr)	/* not used */
#define esif_ccb_write_lock(lockPtr)   write_lock(lockPtr)
#define esif_ccb_write_unlock(lockPtr) write_unlock(lockPtr)
#define esif_ccb_read_lock(lockPtr)    read_lock(lockPtr)
#define esif_ccb_read_unlock(lockPtr)  read_unlock(lockPtr)
#define esif_ccb_low_priority_thread_lock_init(lockPtr)    rwlock_init(lockPtr)
#define esif_ccb_low_priority_thread_lock_uninit(lockPtr)	/* not used */
#define esif_ccb_low_priority_thread_write_lock(lockPtr)   write_lock(lockPtr)
#define esif_ccb_low_priority_thread_write_unlock(lockPtr) write_unlock(lockPtr)
#define esif_ccb_low_priority_thread_read_lock(lockPtr)    read_lock(lockPtr)
#define esif_ccb_low_priority_thread_read_unlock(lockPtr)  read_unlock(lockPtr)

#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS
typedef struct esif_ccb_lock {
	WDFSPINLOCK  lock;	/* Spin Lock */
} esif_ccb_lock_t;

typedef struct esif_ccb_low_priority_thread_lock {
	WDFWAITLOCK  lock;	/* Wait Lock */
} esif_ccb_low_priority_thread_lock_t;

static ESIF_INLINE void esif_ccb_lock_init (esif_ccb_lock_t *lockPtr)
{WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &lockPtr->lock);}

#define esif_ccb_lock_uninit(lockPtr)	/* not used */
static ESIF_INLINE void esif_ccb_write_lock (esif_ccb_lock_t *lockPtr)
{WdfSpinLockAcquire(lockPtr->lock);}

static ESIF_INLINE void esif_ccb_write_unlock (esif_ccb_lock_t *lockPtr)
{WdfSpinLockRelease(lockPtr->lock);}

static ESIF_INLINE void esif_ccb_read_lock (esif_ccb_lock_t *lockPtr)
{WdfSpinLockAcquire(lockPtr->lock);}

static ESIF_INLINE void esif_ccb_read_unlock (esif_ccb_lock_t *lockPtr)
{WdfSpinLockRelease(lockPtr->lock);}

static ESIF_INLINE void esif_ccb_low_priority_thread_lock_init (
	esif_ccb_low_priority_thread_lock_t *lockPtr)
{WdfWaitLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &lockPtr->lock);}

#define esif_ccb_low_priority_thread_lock_uninit(lockPtr)	/* not used */
static ESIF_INLINE void esif_ccb_low_priority_thread_write_lock (
	esif_ccb_low_priority_thread_lock_t *lockPtr)
{WdfWaitLockAcquire(lockPtr->lock, NULL);}	/* Wait forever */

static ESIF_INLINE void esif_ccb_low_priority_thread_write_unlock (
	esif_ccb_low_priority_thread_lock_t *lockPtr)
{WdfWaitLockRelease(lockPtr->lock);}

static ESIF_INLINE void esif_ccb_low_priority_thread_read_lock (
	esif_ccb_low_priority_thread_lock_t *lockPtr)
{WdfWaitLockAcquire(lockPtr->lock, NULL);}	/* Wait forever */

static ESIF_INLINE void esif_ccb_low_priority_thread_read_unlock (
	esif_ccb_low_priority_thread_lock_t *lockPtr)
{WdfWaitLockRelease(lockPtr->lock);}

#endif /* ESIF_ATTR_OS_WINDOWS */


#endif /* ESIF_ATTR_KERNEL */

/******************************************************************************
*   USER LOCKS
******************************************************************************/

#ifdef ESIF_ATTR_USER

#include "esif.h"

#ifdef ESIF_ATTR_OS_LINUX

/* Emulate Critical Section */
typedef pthread_mutex_t esif_ccb_critical_section_t;
#define esif_ccb_critical_section_init(cs) pthread_mutex_init(cs, NULL)
#define esif_ccb_critical_section_uninit pthread_mutex_destroy
#define esif_ccb_critical_section_enter pthread_mutex_lock
#define esif_ccb_critical_section_leave pthread_mutex_unlock

/* Mutex */
typedef pthread_mutex_t esif_ccb_mutex_t;
#define esif_ccb_mutex_init(mutex) pthread_mutex_init(mutex, NULL)
#define esif_ccb_mutex_destroy pthread_mutex_destroy
#define esif_ccb_mutex_lock pthread_mutex_lock
#define esif_ccb_mutex_unlock pthread_mutex_unlock

/* RW Lock */
typedef pthread_rwlock_t esif_ccb_lock_t;
#define esif_ccb_lock_init(lockPtr)     pthread_rwlock_init(lockPtr, NULL)
#define esif_ccb_lock_uninit(lockPtr)   pthread_rwlock_destroy(lockPtr)
#define esif_ccb_write_lock(lockPtr)    pthread_rwlock_wrlock(lockPtr)
#define esif_ccb_write_unlock(lockPtr)  pthread_rwlock_unlock(lockPtr)
#define esif_ccb_read_lock(lockPtr)     pthread_rwlock_rdlock(lockPtr)
#define esif_ccb_read_unlock(lockPtr)   pthread_rwlock_unlock(lockPtr)

#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS
/* Critical Section */
typedef CRITICAL_SECTION esif_ccb_critical_section_t;
#define esif_ccb_critical_section_init InitializeCriticalSection
#define esif_ccb_critical_section_uninit DeleteCriticalSection
#define esif_ccb_critical_section_enter EnterCriticalSection
#define esif_ccb_critical_section_leave LeaveCriticalSection

/* Mutex */
typedef HANDLE esif_ccb_mutex_t;
#define esif_ccb_mutex_init(mutexPtr) *mutexPtr = CreateMutex(NULL, FALSE, NULL)
#define esif_ccb_mutex_destroy(mutexPtr) CloseHandle(*mutexPtr)
#define esif_ccb_mutex_lock(mutexPtr) WaitForSingleObject(*mutexPtr, INFINITE)
#define esif_ccb_mutex_unlock(mutexPtr) ReleaseMutex(*mutexPtr)

/* RW Lock */
typedef SRWLOCK esif_ccb_lock_t;
#define esif_ccb_lock_init(lockPtr)     InitializeSRWLock(lockPtr)
#define esif_ccb_lock_uninit(lockPtr)	/* not used */
#define esif_ccb_write_lock(lockPtr)    AcquireSRWLockExclusive(lockPtr)
#define esif_ccb_write_unlock(lockPtr)  ReleaseSRWLockExclusive(lockPtr)
#define esif_ccb_read_lock(lockPtr)     AcquireSRWLockShared(lockPtr)
#define esif_ccb_read_unlock(lockPtr)   ReleaseSRWLockShared(lockPtr)

#endif /* ESIF_ATTR_OS_WINDOWS */
#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_CCB_LOCK_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
