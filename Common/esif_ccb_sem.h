/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#pragma once

#include "esif_ccb.h"

#if defined(ESIF_ATTR_KERNEL)

#if defined(ESIF_ATTR_OS_WINDOWS)
#include "esif_ccb_sem_win_kern.h"
#elif defined(ESIF_ATTR_OS_LINUX)
#include "esif_ccb_sem_lin_kern.h"
#endif /* KERNEL */

#elif defined(ESIF_ATTR_USER)

#if defined(ESIF_ATTR_OS_WINDOWS)
#include "esif_ccb_sem_win_user.h"
#elif defined(ESIF_ATTR_OS_LINUX)
#include "esif_ccb_sem_lin_user.h"
#endif
#endif /* USER */

#include "esif_ccb_lock.h"
typedef struct esif_ccb_event_s
{
	esif_ccb_lock_t state_lock;
	u8 signaled;
	u32 waiters;
	esif_ccb_sem_t sem_obj;
} esif_ccb_event_t;


static ESIF_INLINE void esif_ccb_event_init(esif_ccb_event_t *event_ptr)
{
	esif_ccb_lock_init(&event_ptr->state_lock);
	esif_ccb_sem_init(&event_ptr->sem_obj);
	event_ptr->waiters = 0;
	event_ptr->signaled = ESIF_FALSE;
}


#if defined(ESIF_ATTR_OS_WINDOWS)
#pragma warning(suppress: 28167)
#endif
/*
 * Suppresses SCA false positive where IRQL level is not properly tracked.
 */
static ESIF_INLINE void esif_ccb_event_uninit(esif_ccb_event_t *event_ptr)
{
	/*
	 * Release all waiters when destroying the event
	 * (If waiters were not already released, this inidicates a possible
	 * synchronization issue.)
	 */
	esif_ccb_write_lock(&event_ptr->state_lock);
	event_ptr->signaled = ESIF_TRUE;

	while(event_ptr->waiters) {
		ESIF_ASSERT(0);
		esif_ccb_sem_up(&event_ptr->sem_obj);
		event_ptr->waiters--;
	}
	esif_ccb_sem_uninit(&event_ptr->sem_obj);
	esif_ccb_write_unlock(&event_ptr->state_lock);
	esif_ccb_lock_uninit(&event_ptr->state_lock);
}


#if defined(ESIF_ATTR_OS_WINDOWS)
#pragma warning(suppress: 28167)
#endif
/*
 * Suppresses SCA false positive where IRQL level is not properly tracked.
 */
static ESIF_INLINE void esif_ccb_event_wait(esif_ccb_event_t *event_ptr)
{
	esif_ccb_write_lock(&event_ptr->state_lock);

	if (event_ptr->signaled) {
		esif_ccb_write_unlock(&event_ptr->state_lock);
		goto exit;
	}

	event_ptr->waiters++;
	esif_ccb_write_unlock(&event_ptr->state_lock);

	esif_ccb_sem_down(&event_ptr->sem_obj);
exit:
	return;
}


#if defined(ESIF_ATTR_OS_WINDOWS)
#pragma warning(suppress: 28167)
#endif
/*
 * Suppresses SCA false positive where IRQL level is not properly tracked.
 */
static ESIF_INLINE void esif_ccb_event_set(esif_ccb_event_t *event_ptr)
{
	esif_ccb_write_lock(&event_ptr->state_lock);

	event_ptr->signaled  = ESIF_TRUE;

	while(event_ptr->waiters) {
		esif_ccb_sem_up(&event_ptr->sem_obj);
		event_ptr->waiters--;
	}

	esif_ccb_write_unlock(&event_ptr->state_lock);
}


static ESIF_INLINE void esif_ccb_event_reset(esif_ccb_event_t *event_ptr)
{
	esif_ccb_write_lock(&event_ptr->state_lock);
	event_ptr->signaled  = ESIF_FALSE;
	esif_ccb_write_unlock(&event_ptr->state_lock);
}

/*
 * Special use case function which allows all waiters to proceed, but does not
 * change the signaled state.
 */
#if defined(ESIF_ATTR_OS_WINDOWS)
#pragma warning(suppress: 28167)
#endif
/*
 * Suppresses SCA false positive where IRQL level is not properly tracked.
 */
static ESIF_INLINE void esif_ccb_event_release_waiters(esif_ccb_event_t *event_ptr)
{
	esif_ccb_write_lock(&event_ptr->state_lock);

	while(event_ptr->waiters) {
		esif_ccb_sem_up(&event_ptr->sem_obj);
		event_ptr->waiters--;
	}

	esif_ccb_write_unlock(&event_ptr->state_lock);
}