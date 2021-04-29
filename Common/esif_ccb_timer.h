/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "esif_ccb_rc.h"
#include "esif_ccb_time.h"
#include "esif_ccb_lock.h"
#include "esif_ccb_sem.h"
#include "esif_ccb_memory.h"

/*
 * INTERFACE - The interface consists of the following:
 *
 *    esif_ccb_timer_t - Opaque timer type
 *
 *    esif_ccb_timer_init - Initializes esif_ccb_timer_t objects, must be
 *        called befor use of the objec in other interface functions
 *
 *    esif_ccb_timer_kill- Destroys a timer object. After being killed, a timer
 *        object should not be used again without first calling esif_ccb_timer
 *        init. Note:  Returns after marking the timer for delete, but full
 *        destruction may not occur until after returning.  If synchronous
 *        destruction is required, use esif_ccb_timer_kill_w_wait.
 *
 *    esif_ccb_timer_kill_w_wait- Kills a timer object and does not return until
 *        the callback is complete or timer is cancelled.  (For Windows kernel,
 *        it is not recommended to kill the timer in the callback function
 *        frequently due to repeated calls to flush the DPC queue.)
 *
 *    esif_ccb_timer_set_msec - Sets the timeout and starts the timer
 *
 *    esif_ccb_timer_cb - Timer callback typedef
 *
 *    esif_ccb_tmrm_init - May optionally be called before any timer is
 *        initialized (The code will self-initialize if not called, but calling
 *        this will prevent issues if more than one thread may attempt to create
 *        the first timer at the same time.)
 *
 *    esif_ccb_tmrm_exit - Should be called to cleanup the timer manager after
 *        all timers are destroyed and no other timers will be created.
 *        This functions is expected to be called when the system is in a
 *        "known" state where not attempts to create any timers are in flight
 *        as this function destroys the lock controlling synchronization
 *
 * IMPORTANT!!!!
 *
 *     In order to use this code, the "C" implementation code may be included in one
 *     of two ways:  Include the "C" implementation file, esif_ccb_timer.c, in the
 *     compilation, or include only this header file and declare the #define
 *     ESIF_CCB_TIMER_MAIN  before inclusion of all other CCB header files in
 *     ONE AND ONLY ONE file of an execution unit.
 *
 *     In addition, this code requires the inclusion of the CCB linked list
 *     implementation by similar methods.  See esif_link_list.h.
 */


#define esif_ccb_timer_handle_t esif_os_handle_t

/* Agnostic timer object */

#pragma pack(push, 1)

/*
 * Opaque timer object that contains only the handle used to reference the timer.
 */
typedef struct esif_ccb_timer {
	esif_ccb_timer_handle_t timer_handle;
} esif_ccb_timer_t;

#pragma pack(pop)


typedef void (*esif_ccb_timer_cb)(void* context_ptr);


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initializes the timer manager.  Should be called before any timer is
 * initialized.
 */
enum esif_rc esif_ccb_tmrm_init(void);

/*
 * Releases resources used by the the timer manager.  Should be called after all
 * timers have been destroyed as this code will wait for timer destruction if
 * any active timers remain.
 */
void esif_ccb_tmrm_exit(void);

/*
 * Every init should have a matching kill to release resources.
 * Also, a timer should not be re-init'd without first killing it
 */
enum esif_rc esif_ccb_timer_init(
	esif_ccb_timer_t *timer_ptr,
	esif_ccb_timer_cb function_ptr,		/* Callback when timer fires */
	void *context_ptr			/* Callback context if any */
	);

enum esif_rc esif_ccb_timer_kill(
	esif_ccb_timer_t *timer_ptr
	);

enum esif_rc esif_ccb_timer_kill_w_wait(
	esif_ccb_timer_t *timer_ptr
	);

/*
 * Sets a timer timeout in ms
 * Notes:  0 is an invalid timeout
 * The code will cancel any current timeout if possible before setting the new
 * timeout. If in the callback, the timeout will not be set until the function
 * exits.
 */
enum esif_rc esif_ccb_timer_set_msec(
	esif_ccb_timer_t *timer_ptr,
	const esif_ccb_time_t timeout	/* Timeout in msec */
	);

void esif_ccb_tmrm_callback(
	esif_ccb_timer_handle_t cb_handle
	);

#ifdef __cplusplus
}
#endif

/* 
 * Timer build options to enable use of different timer types in Windows
 * Normally, these options should be specified in the project settings in
 * order to allow different options depending on need for a given binary.
 */
#if defined(ESIF_ATTR_OS_WINDOWS)

 /*
  * Build option to enable use of the WDF timers in Windows
  * If not enabled, legacy kernel timers are used in kernel mode and
  * and the specified user timer type is used in user mode.
  * (See user section below.)
  */
 /* #define ESIF_FEAT_OPT_WDFTIMER - Enable in project settings */

#if defined(ESIF_ATTR_USER)

/* Build option to enable use of the "waitable" timers in Windows */
/* #define ESIF_FEAT_OPT_WAITABLE_TIMERS - Enable in project settings */

#if defined(ESIF_FEAT_OPT_WAITABLE_TIMERS)
#undef ESIF_FEAT_OPT_WDFTIMER
#endif

#endif /* ESIF_ATTR_USER */
#endif /* ESIF_ATTR_OS_WINDOWS */

/*
 * Bring in OS-specific implementation code
 */
#if defined(ESIF_ATTR_KERNEL)

#if defined(ESIF_ATTR_OS_WINDOWS)
#include "esif_ccb_timer_win_kern.h"
#elif defined(ESIF_ATTR_OS_LINUX)
#include "esif_ccb_timer_lin_kern.h"
#endif

#elif defined(ESIF_ATTR_USER)

#if defined(ESIF_ATTR_OS_WINDOWS)
#include "esif_ccb_timer_win_user.h"
#elif defined(ESIF_ATTR_OS_LINUX)
#include "esif_ccb_timer_lin_user.h"
#endif
#endif

/*
 * IMPORTANT!!!!!!!!!
 *
 * The #define ESIF_CCB_TIMER_MAIN must be included before this header file
 * in ONE AND ONLY ONE file of an execution unit in order to bring in the
 * required code, or the "C" implementation file, esif_ccb_timer.c, must be
 * compiled as part of the execution unit.
 *
 */
#ifdef ESIF_CCB_TIMER_MAIN

#include "esif_ccb_timer.c"

#endif