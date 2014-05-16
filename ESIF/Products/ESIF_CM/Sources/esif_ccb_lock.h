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
typedef struct rw_semaphore esif_ccb_low_priority_thread_lock_t;
#define esif_ccb_lock_init(lockPtr)    rwlock_init(lockPtr)
#define esif_ccb_lock_uninit(lockPtr)	/* not used */
#define esif_ccb_write_lock(lockPtr)   write_lock(lockPtr)
#define esif_ccb_write_unlock(lockPtr) write_unlock(lockPtr)
#define esif_ccb_read_lock(lockPtr)    read_lock(lockPtr)
#define esif_ccb_read_unlock(lockPtr)  read_unlock(lockPtr)
#define esif_ccb_low_priority_thread_lock_init(lockPtr)    init_rwsem(lockPtr)
#define esif_ccb_low_priority_thread_lock_uninit(lockPtr)	/* not used */
#define esif_ccb_low_priority_thread_write_lock(lockPtr)   down_write(lockPtr)
#define esif_ccb_low_priority_thread_write_unlock(lockPtr) up_write(lockPtr)
#define esif_ccb_low_priority_thread_read_lock(lockPtr)    down_read(lockPtr)
#define esif_ccb_low_priority_thread_read_unlock(lockPtr)  up_read(lockPtr)

#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS
typedef struct esif_ccb_lock {
	WDFSPINLOCK  lock;	/* Spin Lock */
} esif_ccb_lock_t;

typedef struct esif_ccb_low_priority_thread_lock {
	WDFWAITLOCK  lock;	/* Wait Lock */
} esif_ccb_low_priority_thread_lock_t;

static ESIF_INLINE void esif_ccb_lock_init (esif_ccb_lock_t *lockPtr)
{
	/* TODO: Change creation functions to return status and check in code */
	NTSTATUS stat;
	stat = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &lockPtr->lock);
	NT_SUCCESS(stat);
}

#define esif_ccb_lock_uninit(lockPtr)	/* not used */



_IRQL_raises_(DISPATCH_LEVEL)
_IRQL_saves_global_(OldIrql, lockPtr->lock)
_Requires_lock_not_held_(lockPtr->lock)
_Acquires_lock_(lockPtr->lock)
static ESIF_INLINE void esif_ccb_write_lock (esif_ccb_lock_t *lockPtr)
{WdfSpinLockAcquire(lockPtr->lock);}

_IRQL_restores_global_(OldIrql, lockPtr->lock)
_IRQL_requires_(DISPATCH_LEVEL) 
_Requires_lock_held_(lockPtr->lock)
_Releases_lock_(lockPtr->lock)
static ESIF_INLINE void esif_ccb_write_unlock (esif_ccb_lock_t *lockPtr)
{WdfSpinLockRelease(lockPtr->lock);}


_IRQL_raises_(DISPATCH_LEVEL)
_IRQL_saves_global_(OldIrql, lockPtr)
_Acquires_lock_(lockPtr->lock)
static ESIF_INLINE void esif_ccb_read_lock (esif_ccb_lock_t *lockPtr)
{WdfSpinLockAcquire(lockPtr->lock);}

_IRQL_restores_global_(OldIrql, lockPtr)
_IRQL_requires_(DISPATCH_LEVEL) 
_Releases_lock_(lockPtr->lock)
static ESIF_INLINE void esif_ccb_read_unlock (esif_ccb_lock_t *lockPtr)
{WdfSpinLockRelease(lockPtr->lock);}

static ESIF_INLINE void esif_ccb_low_priority_thread_lock_init (
	esif_ccb_low_priority_thread_lock_t *lockPtr)
{
	/* TODO: Change creation functions to return status and check in code */
	NTSTATUS stat;
	stat = WdfWaitLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &lockPtr->lock);
	NT_SUCCESS(stat);
}

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
#include "esif_ccb_atomic.h"

/* Spinlock */
typedef atomic_t esif_ccb_spinlock_t;
#define esif_ccb_spinlock_init(lockPtr)  *(lockPtr) = ATOMIC_INIT(0)
#define esif_ccb_spinlock_uninit(lockPtr) (lockPtr)
#define	esif_ccb_spinlock_lock(lockPtr)   while (atomic_set(lockPtr, 1) == 1) {;}
#define	esif_ccb_spinlock_unlock(lockPtr) atomic_set(lockPtr, 0)

#ifdef ESIF_ATTR_OS_LINUX
#include <pthread.h>

#define PTHREAD_MUTEX_INIT_TYPE(mutex, type)	do { static pthread_mutexattr_t attr; pthread_mutexattr_settype(&attr, type); pthread_mutex_init(mutex, &attr); } while (0)

/* Emulate Critical Section */
typedef pthread_mutex_t esif_ccb_critical_section_t;
#define esif_ccb_critical_section_init(csPtr) PTHREAD_MUTEX_INIT_TYPE(csPtr, PTHREAD_MUTEX_RECURSIVE)
#define esif_ccb_critical_section_uninit(csPtr) pthread_mutex_destroy(csPtr)
#define esif_ccb_critical_section_enter(csPtr) pthread_mutex_lock(csPtr) /* reentrant */
#define esif_ccb_critical_section_leave(csPtr) pthread_mutex_unlock(csPtr) /* reentrant */

/* Mutex */
typedef pthread_mutex_t esif_ccb_mutex_t;
#define esif_ccb_mutex_init(mutexPtr) PTHREAD_MUTEX_INIT_TYPE(mutexPtr, PTHREAD_MUTEX_RECURSIVE)
#define esif_ccb_mutex_uninit(mutexPtr) pthread_mutex_destroy(mutexPtr)
#define esif_ccb_mutex_lock(mutexPtr) pthread_mutex_lock(mutexPtr) /* reentrant */
#define esif_ccb_mutex_unlock(mutexPtr) pthread_mutex_unlock(mutexPtr) /* reentrant */

/* RW Lock */
typedef pthread_rwlock_t esif_ccb_lock_t;
#define esif_ccb_lock_init(lockPtr)     pthread_rwlock_init(lockPtr, NULL)
#define esif_ccb_lock_uninit(lockPtr)   pthread_rwlock_destroy(lockPtr)
#define esif_ccb_write_lock(lockPtr)    pthread_rwlock_wrlock(lockPtr) /* NOT reentrant */
#define esif_ccb_write_unlock(lockPtr)  pthread_rwlock_unlock(lockPtr) /* NOT reentrant */
#define esif_ccb_read_lock(lockPtr)     pthread_rwlock_rdlock(lockPtr) /* reentrant */
#define esif_ccb_read_unlock(lockPtr)   pthread_rwlock_unlock(lockPtr) /* reentrant */

#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS
/* Critical Section */
typedef CRITICAL_SECTION esif_ccb_critical_section_t;
#define esif_ccb_critical_section_init(csPtr) InitializeCriticalSection(csPtr)
#define esif_ccb_critical_section_uninit(csPtr) DeleteCriticalSection(csPtr)
#define esif_ccb_critical_section_enter(csPtr) EnterCriticalSection(csPtr) /* reentrant */
#define esif_ccb_critical_section_leave(csPtr) LeaveCriticalSection(csPtr) /* reentrant */

/* Mutex */
typedef HANDLE esif_ccb_mutex_t;
#define esif_ccb_mutex_init(mutexPtr) *mutexPtr = CreateMutex(NULL, FALSE, NULL)
#define esif_ccb_mutex_uninit(mutexPtr) CloseHandle(*mutexPtr)
#define esif_ccb_mutex_lock(mutexPtr) WaitForSingleObject(*mutexPtr, INFINITE) /* reentrant */
#define esif_ccb_mutex_unlock(mutexPtr) ReleaseMutex(*mutexPtr) /* reentrant */

/* RW Lock */

/* Wrapper for SRWLOCKs to allow nested READ locks in Windows
 * This is necessary to avoid WHQL errors and potential deadlocks in Windows
 * This is unecessary for Linux since pthread_rwlock_t is read reentrant
 */
typedef struct esif_ccb_lock_s {
	SRWLOCK				lock;		/* Slim Read/Write Lock */
	esif_ccb_spinlock_t	spinlock;	/* Spinlock for managing lock and ref_count */
	UInt32				ref_count;	/* Count of Acquired References to shared READ lock */
} esif_ccb_lock_t;

/* Initialize */
static ESIF_INLINE void esif_ccb_lock_init(esif_ccb_lock_t *lockPtr)
{
	InitializeSRWLock(&lockPtr->lock);
	esif_ccb_spinlock_init(&lockPtr->spinlock);
	lockPtr->ref_count = 0;
}

/* Uninitialize */
static ESIF_INLINE void esif_ccb_lock_uninit(esif_ccb_lock_t *lockPtr)
{
	esif_ccb_spinlock_uninit(&lockPtr->spinlock);
}

/* NOT Reentrant Write Lock */
static ESIF_INLINE void esif_ccb_write_lock(esif_ccb_lock_t *lockPtr)
{
	AcquireSRWLockExclusive(&lockPtr->lock);
}

/* NOT Reentrant Write Unlock */
static ESIF_INLINE void esif_ccb_write_unlock(esif_ccb_lock_t *lockPtr)
{
	ReleaseSRWLockExclusive(&lockPtr->lock);
}

/* Reentrant Read Lock */
static ESIF_INLINE void esif_ccb_read_lock(esif_ccb_lock_t *lockPtr)
{
	BOOL release = FALSE;
	
	/* Acquire spinlock to see if any readers have acquired the lock
	 * If so just increment ref_count, otherwise Acquire lock outside spinlock
	 */
	esif_ccb_spinlock_lock(&lockPtr->spinlock);
	if (lockPtr->ref_count > 0) {
		++lockPtr->ref_count;
	}
	else {
		esif_ccb_spinlock_unlock(&lockPtr->spinlock);
		
		AcquireSRWLockShared(&lockPtr->lock);
		
		/* After acquiring lock, update ref_count after re-acquiring spinlock */
		esif_ccb_spinlock_lock(&lockPtr->spinlock);
		if (++lockPtr->ref_count > 1) {
			release = TRUE;
		}
	}
	esif_ccb_spinlock_unlock(&lockPtr->spinlock);

	/* If multiple threads Acquired locks, all but one will release it here */
	if (release) {
		ReleaseSRWLockShared(&lockPtr->lock);
	}
}

/* Reentrant Read Unlock */
static ESIF_INLINE void esif_ccb_read_unlock(esif_ccb_lock_t *lockPtr)
{
	BOOL release = FALSE;

	/* Aquire spinlock to see if this is the last reader referencing this lock
	 * If this is the last reader, Release the lock outside of the spinlock
	 */
	esif_ccb_spinlock_lock(&lockPtr->spinlock);
	if (--lockPtr->ref_count == 0) {
		release = TRUE;
	}
	esif_ccb_spinlock_unlock(&lockPtr->spinlock);

	if (release) {
		ReleaseSRWLockShared(&lockPtr->lock);
	}
}


#endif /* ESIF_ATTR_OS_WINDOWS */
#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_CCB_LOCK_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
