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

#ifndef _ESIF_CCB_TIMER_H_
#define _ESIF_CCB_TIMER_H_

#include "esif_ccb_lock.h"

/******************************************************************************
*   KERNEL TIMER
******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

/* Timer Callback Function */
typedef void (*esif_ccb_timer_cb)(void *context);

/*
 * Timer Is Defined to be opaue here to  acoomondate different
 * OS and implementation approaches for timers
 */

#ifdef ESIF_ATTR_OS_LINUX
typedef struct {
	struct delayed_work  work;
	esif_ccb_timer_cb    function_ptr;
	esif_ccb_low_priority_thread_lock_t context_lock;
	esif_flags_t         exit_flag;
	void *context_ptr;
} esif_ccb_timer_t;
#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS

typedef struct {
	esif_ccb_timer_cb function_ptr;
	esif_ccb_low_priority_thread_lock_t context_lock;
	esif_flags_t exit_flag;
	void *context_ptr;
} esif_ccb_timer_context_t;


#ifdef ESIF_ATTR_USE_COALESCABLE_TIMERS

typedef struct esif_ccb_timer {
	KTIMER  timer;
	KDPC dpc;
	esif_ccb_timer_context_t timer_context;
} esif_ccb_timer_t;

typedef struct esif_ccb_work_item_context {
	void *ptr;
} esif_ccb_work_item_context_t;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(esif_ccb_work_item_context_t,
				   esif_ccb_get_work_item_context)
#else
typedef WDFTIMER esif_ccb_timer_t;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(esif_ccb_timer_context_t,
				   esif_ccb_get_timer_context)
#endif


#endif /* ESIF_ATTR_OS_WINDOWS */

#ifdef ESIF_ATTR_OS_LINUX
/* Timer Callback Wrapper  Find And Fire Function */
static ESIF_INLINE void esif_ccb_timer_cb_wrapper(struct work_struct *work)
{
	esif_ccb_timer_t *timer_ptr = container_of(
			(struct delayed_work *)work,
			esif_ccb_timer_t,
			work);

	TIMER_DEBUG("%s: timer fired!!!!!\n", ESIF_FUNC);
	if ((NULL != timer_ptr) && (!timer_ptr->exit_flag)) {
		esif_ccb_low_priority_thread_read_lock(&timer_ptr->context_lock);
		timer_ptr->function_ptr(timer_ptr->context_ptr);
		esif_ccb_low_priority_thread_read_unlock(
			&timer_ptr->context_lock);
	}
}


#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS
/* Timer Callback Wrapper Find And Fire Function */

#ifdef ESIF_ATTR_USE_COALESCABLE_TIMERS

static KDEFERRED_ROUTINE esif_ccb_timer_dpc;
static EVT_WDF_WORKITEM esif_ccb_timer_cb_wrapper;

static void esif_ccb_timer_cb_wrapper(
    WDFWORKITEM work_item
    )
{
	esif_ccb_work_item_context_t *work_item_context_ptr = NULL;
	esif_ccb_timer_t *timer_ptr = NULL;
	esif_ccb_timer_context_t *timer_context_ptr = NULL;

	TIMER_DEBUG("%s: timer fired!!!!!\n", ESIF_FUNC);

	work_item_context_ptr = esif_ccb_get_work_item_context(work_item);
	if(NULL == work_item_context_ptr) {
		goto exit;
	}

	timer_ptr = (esif_ccb_timer_t *)work_item_context_ptr->ptr;
	if(NULL == timer_ptr) {
		goto exit;
	}

	timer_context_ptr = &timer_ptr->timer_context;
	if ((NULL != timer_context_ptr) && (!timer_context_ptr->exit_flag)) {
		esif_ccb_low_priority_thread_read_lock(
			&timer_context_ptr->context_lock);
		timer_context_ptr->function_ptr(timer_context_ptr->context_ptr);
		esif_ccb_low_priority_thread_read_unlock(
			&timer_context_ptr->context_lock);
	}
exit:
	WdfObjectDelete(work_item);
}

static void esif_ccb_timer_dpc (
	struct _KDPC *Dpc,
	PVOID DeferredContext,
	PVOID SystemArgument1,
	PVOID SystemArgument2
    )
{
	NTSTATUS status = STATUS_SUCCESS;
	esif_ccb_work_item_context_t *work_item_context_ptr = NULL;
	WDF_WORKITEM_CONFIG workItemConfig    = {0};
	WDF_OBJECT_ATTRIBUTES workItemAttribs = {0};
	WDFWORKITEM workItem = NULL;

	UNREFERENCED_PARAMETER(Dpc);
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);

	WDF_WORKITEM_CONFIG_INIT(&workItemConfig, esif_ccb_timer_cb_wrapper);
	workItemConfig.AutomaticSerialization = TRUE;

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&workItemAttribs, 
					        esif_ccb_work_item_context_t);
	workItemAttribs.ParentObject = g_wdf_ipc_queue_handle;

	status = WdfWorkItemCreate(&workItemConfig, &workItemAttribs, &workItem);
	if (NT_SUCCESS(status)) {
		work_item_context_ptr = esif_ccb_get_work_item_context(workItem);
		work_item_context_ptr->ptr = DeferredContext;
		WdfWorkItemEnqueue(workItem);
	}
}



#else /* NOT ESIF_ATTR_USE_COALESCABLE_TIMERS */

EVT_WDF_TIMER esif_ccb_timer_cb_wrapper;

ESIF_INLINE void esif_ccb_timer_cb_wrapper(WDFTIMER timer)
{
	esif_ccb_timer_context_t *timer_context_ptr =
		esif_ccb_get_timer_context(timer);

	TIMER_DEBUG("%s: timer fired!!!!!\n", ESIF_FUNC);
	if ((NULL != timer_context_ptr) && (!timer_context_ptr->exit_flag)) {
		esif_ccb_low_priority_thread_read_lock(
			&timer_context_ptr->context_lock);
		timer_context_ptr->function_ptr(timer_context_ptr->context_ptr);
		esif_ccb_low_priority_thread_read_unlock(
			&timer_context_ptr->context_lock);
	}
}
#endif /* NOT ESIF_ATTR_USE_COALESCABLE_TIMERS */
#endif /* ESIF_ATTR_OS_WINDOWS */


/* Timer Initialize */
static ESIF_INLINE
enum esif_rc esif_ccb_timer_init(esif_ccb_timer_t *timer_ptr)
{
	enum esif_rc rc = ESIF_E_UNSPECIFIED;
#ifdef ESIF_ATTR_OS_LINUX
	TIMER_DEBUG("%s: timer %p\n", ESIF_FUNC, timer_ptr);
	INIT_DELAYED_WORK(&timer_ptr->work, NULL);
	esif_ccb_low_priority_thread_lock_init(&timer_ptr->context_lock);
	timer_ptr->exit_flag = FALSE;
	rc = ESIF_OK;
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
#ifdef ESIF_ATTR_USE_COALESCABLE_TIMERS

	esif_ccb_memset(timer_ptr, 0, sizeof(*timer_ptr));

	KeInitializeDpc(&timer_ptr->dpc, esif_ccb_timer_dpc, timer_ptr);
	KeInitializeTimer(&timer_ptr->timer);

	esif_ccb_low_priority_thread_lock_init(&timer_ptr->timer_context.context_lock);
	timer_ptr->timer_context.exit_flag = FALSE;

	TIMER_DEBUG("%s: timer %p\n", ESIF_FUNC, timer_ptr);

#else /* NOT ESIF_ATTR_USE_COALESCABLE_TIMERS */
	NTSTATUS status;

	WDF_TIMER_CONFIG timer_config = {0};
	WDF_OBJECT_ATTRIBUTES timer_attributes      = {0};
	esif_ccb_timer_context_t *timer_context_ptr = NULL;

	TIMER_DEBUG("%s: timer %p\n", ESIF_FUNC, timer_ptr);

	WDF_TIMER_CONFIG_INIT(&timer_config, esif_ccb_timer_cb_wrapper);
	timer_config.AutomaticSerialization = TRUE;

	WDF_OBJECT_ATTRIBUTES_INIT(&timer_attributes);
	timer_attributes.ParentObject   = g_wdf_ipc_queue_handle;
	timer_attributes.ExecutionLevel = WdfExecutionLevelPassive;

	WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&timer_attributes,
					       esif_ccb_timer_context_t);
	status = WdfTimerCreate(&timer_config, &timer_attributes, timer_ptr);
	TIMER_DEBUG("%s: timer %p status %08x\n", ESIF_FUNC, timer_ptr, status);
	if (STATUS_SUCCESS == status)
		rc = ESIF_OK;

	timer_context_ptr = esif_ccb_get_timer_context(*timer_ptr);
	if (timer_context_ptr == NULL) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	esif_ccb_low_priority_thread_lock_init(&timer_context_ptr->context_lock);
	timer_context_ptr->exit_flag = FALSE;

exit:
#endif /* NOT ESIF_ATTR_USE_COALESCABLE_TIMERS */
#endif /* ESIF_ATTR_OS_WINDOWS */
	return rc;
}


/* Timer Set */
static ESIF_INLINE enum esif_rc esif_ccb_timer_set_msec(
	esif_ccb_timer_t *timer_ptr,
	esif_ccb_time_t timeout,
	esif_ccb_timer_cb function_ptr,
	void *context_ptr
	)
{
	enum esif_rc rc = ESIF_E_UNSPECIFIED;
#ifdef ESIF_ATTR_OS_LINUX
	u64 x;
	u64 y;
	u8 result;
#
	timer_ptr->function_ptr = function_ptr;
	timer_ptr->context_ptr  = context_ptr;

	TIMER_DEBUG("%s: timer %p timeout %u\n",
		    ESIF_FUNC,
		    timer_ptr,
		    (int)timeout);

	PREPARE_DELAYED_WORK(&timer_ptr->work, esif_ccb_timer_cb_wrapper);

	/* Keep 32 bit Linux Happy */
	x = timeout * HZ;
	y = 1000;

	do_div(x, y);

	result = schedule_delayed_work(&timer_ptr->work, x);
	if (ESIF_TRUE == result)
		rc = ESIF_OK;
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
#ifdef ESIF_ATTR_USE_COALESCABLE_TIMERS
	esif_ccb_timer_context_t *timer_context_ptr = NULL;
	LARGE_INTEGER due_time = {0LL};

	TIMER_DEBUG("%s: timer %p timeout %u\n", ESIF_FUNC, timer_ptr, timeout);

	if(NULL == timer_ptr) {
		goto exit;
	}

	timer_context_ptr = &timer_ptr->timer_context;
	timer_context_ptr->function_ptr = function_ptr;
	timer_context_ptr->context_ptr  = context_ptr;
	timer_context_ptr->exit_flag    = FALSE;

	due_time.QuadPart = (LONGLONG)timeout * -10000;
	KeSetCoalescableTimer(&timer_ptr->timer,
			      due_time,
			      0,
			      (ULONG)timeout / 2,
			      &timer_ptr->dpc);

	rc = ESIF_OK;
exit:

#else /* NOT ESIF_ATTR_USE_COALESCABLE_TIMERS */

	esif_ccb_timer_context_t *timer_context_ptr = NULL;
	BOOLEAN status;

	TIMER_DEBUG("%s: timer %p timeout %u\n", ESIF_FUNC, timer_ptr, timeout);

	/* Set Context */
	timer_context_ptr = esif_ccb_get_timer_context(*timer_ptr);
	if (NULL != timer_context_ptr) {
		timer_context_ptr->function_ptr = function_ptr;
		timer_context_ptr->context_ptr  = context_ptr;
		timer_context_ptr->exit_flag    = FALSE;
	}

	/* Finally Start Timer */
	status = WdfTimerStart(*timer_ptr, (LONG)timeout * -10000);
	TIMER_DEBUG("%s: timer %p status %08x\n", ESIF_FUNC, timer_ptr, status);
	if (TRUE == status)
		rc = ESIF_OK;

#endif /* NOT ESIF_ATTR_USE_COALESCABLE_TIMERS */
#endif /* ESIF_ATTR_OS_WINDOWS */
	return rc;
}


/* Timer Stop And Destory */
static ESIF_INLINE
enum esif_rc esif_ccb_timer_kill(esif_ccb_timer_t *timer)
{
	enum esif_rc rc = ESIF_E_UNSPECIFIED;
#ifdef ESIF_ATTR_OS_LINUX
	TIMER_DEBUG("%s: timer %p\n", ESIF_FUNC, timer);

	esif_ccb_low_priority_thread_read_lock(&timer->context_lock);
	timer->exit_flag = TRUE;
	esif_ccb_low_priority_thread_read_unlock(&timer->context_lock);

	if (ESIF_TRUE == cancel_delayed_work(&timer->work))
		rc = ESIF_OK;
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
#ifdef ESIF_ATTR_USE_COALESCABLE_TIMERS
	esif_ccb_timer_context_t *timer_context_ptr = NULL;

	TIMER_DEBUG("%s: timer %p\n", ESIF_FUNC, timer);

	if(NULL == timer) {
		goto exit;
	}

	timer_context_ptr = &timer->timer_context;

	if (timer_context_ptr != NULL) {
		esif_ccb_low_priority_thread_read_lock(
			&timer_context_ptr->context_lock);
		timer_context_ptr->exit_flag = TRUE;
		esif_ccb_low_priority_thread_read_unlock(
			&timer_context_ptr->context_lock);
	}

	KeCancelTimer(&timer->timer);
exit:
	rc = ESIF_OK;

#else /* NOT ESIF_ATTR_USE_COALESCABLE_TIMERS */
	esif_ccb_timer_context_t *timer_context_ptr = NULL;

	TIMER_DEBUG("%s: timer %p\n", ESIF_FUNC, timer);

	timer_context_ptr = esif_ccb_get_timer_context(*timer);
	if (timer_context_ptr != NULL) {
		esif_ccb_low_priority_thread_read_lock(
			&timer_context_ptr->context_lock);
		timer_context_ptr->exit_flag = TRUE;
		esif_ccb_low_priority_thread_read_unlock(
			&timer_context_ptr->context_lock);
	}

	/* WDF Framework Will Cleanup  */
	WdfTimerStop(*timer, FALSE);
	rc = ESIF_OK;

#endif /* NOT ESIF_ATTR_USE_COALESCABLE_TIMERS */
#endif /* ESIF_ATTR_OS_WINDOWS */
	return rc;
}


#endif /* ESIF_ATTR_KERNEL*/

/******************************************************************************
*   USER TIMER
******************************************************************************/

#ifdef ESIF_ATTR_USER

#include "esif.h"

/* OS Agnostic Callback Function */
typedef void (*esif_ccb_timer_cb)(const void *context_ptr);

/* OS Agnostic Timer Context */
typedef struct esif_ccb_timer_ctx {
	esif_ccb_timer_cb  cb_func;		/* Call Back Function */
	void *cb_context_ptr;	/* Call Back Function Context */
} esif_ccb_timer_ctx_t;

#ifdef ESIF_ATTR_OS_LINUX

#include <signal.h>

/*
 *  Linux OS Callback Wrapper Prototype
 *  Each OS Expects its own CALLBACK primitive we use this
 *  wrapper function to normalize the parameters and and
 *  ultimately call our func_ptr and func_context originally
 *  provided to the timer.  By example Linux expects a union
 *  here that contains our timer context poiner which in turn
 *  contains our function and context for the function.
 */

typedef void (*esif_ccb_timer_cb_wrapper)(union sigval sv);

/* Linux Timer */
typedef struct esif_ccb_timer {
	timer_t  timer;		/* Linux specific timer */
	esif_ccb_timer_ctx_t *timer_ctx_ptr;	/* OS Agnostic timer context */
} esif_ccb_timer_t;

/*
 *  Linux Callback Wrapper Function
 *  Declared as void by Linux so there is not much we can do if
 *  something goes wrong.  We simply wrap the parameters with a
 *  validity check and hope for the best.
 */
static ESIF_INLINE void esif_ccb_timer_wrapper(const union sigval sv)
{
	esif_ccb_timer_ctx_t *timer_ctx_ptr =
		(esif_ccb_timer_ctx_t *)sv.sival_ptr;

	ESIF_ASSERT(timer_ctx_ptr != NULL);
	ESIF_ASSERT(timer_ctx_ptr->cb_func != NULL);
	ESIF_ASSERT(timer_ctx_ptr->cb_context_ptr != NULL);

	if (NULL != timer_ctx_ptr &&
	    NULL != timer_ctx_ptr->cb_func &&
	    NULL != timer_ctx_ptr->cb_context_ptr) {
		timer_ctx_ptr->cb_func(timer_ctx_ptr->cb_context_ptr);
	}
}


#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS

/*
 *  Windows OS Callback Wrapper Prototype
 *  Each OS Expects its own CALLBACK primitive we use this
 *  wrapper function to normalize the parameters and and
 *  ultimately call our func_ptr and func_context originally
 *  provided to the timer.  By example Windows expects two
 *  parameters of which we only use one.  The other one is
 *  always true.
 */
typedef void (*esif_ccb_timer_cb_wrapper)(void *context_ptr, BOOLEAN notUsed);

/* Windows Timer */
typedef struct esif_ccb_timer {
	HANDLE  timer;		/* Windows specific timer */
	esif_ccb_timer_ctx_t *timer_ctx_ptr;	/* OS Agnostic timer context */
} esif_ccb_timer_t;

/*
 *  Windows Callback Wrapper Function
 *  Declared as void by windows so there is not much we can do if
 *  something goes wrong.  We simply wrap the parameters with a
 *  validity check and hope for the best.
 */

static ESIF_INLINE void esif_ccb_timer_wrapper(
	const void *context_ptr,
	const BOOLEAN notUsed
	)
{
	esif_ccb_timer_ctx_t *timer_ctx_ptr =
		(esif_ccb_timer_ctx_t *)context_ptr;

	ESIF_ASSERT(timer_ctx_ptr != NULL);
	ESIF_ASSERT(timer_ctx_ptr->cb_func != NULL);
	ESIF_ASSERT(timer_ctx_ptr->cb_context_ptr != NULL);

	UNREFERENCED_PARAMETER(notUsed);

	if (NULL != timer_ctx_ptr &&
	    NULL != timer_ctx_ptr->cb_func &&
	    NULL != timer_ctx_ptr->cb_context_ptr) {
		timer_ctx_ptr->cb_func(timer_ctx_ptr->cb_context_ptr);
	}
}


#endif /* ESIF_ATTR_OS_WINDOWS */

/* Windows Initialize Timer */
static ESIF_INLINE eEsifError esif_ccb_timer_init(
	esif_ccb_timer_t *timer_ptr,		/* Our Timer */
	const esif_ccb_timer_cb function_ptr,	/* Callback when timer fires */
	void *context_ptr
	)		/* Callback context if any */
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	ESIF_ASSERT(timer_ptr != NULL);
	ESIF_ASSERT(function_ptr != NULL);

	/* Allocate and setup new data */
	if (NULL != timer_ptr) {
		timer_ptr->timer_ctx_ptr =
			(esif_ccb_timer_ctx_t *)esif_ccb_malloc(sizeof(
								       esif_ccb_timer_ctx_t));
		if (NULL == timer_ptr->timer_ctx_ptr)
			return rc;

		/* Store state for timer set */
		timer_ptr->timer_ctx_ptr->cb_func        = function_ptr;
		timer_ptr->timer_ctx_ptr->cb_context_ptr = context_ptr;
	}
#ifdef ESIF_ATTR_OS_WINDOWS
	/* Nothing For Windows */
	rc = ESIF_OK;
#endif

#ifdef ESIF_ATTR_OS_LINUX
	if (NULL != timer_ptr) {
		struct sigevent se;
		pthread_attr_t attr;
		pthread_attr_init(&attr);

		se.sigev_notify = SIGEV_THREAD;
		se.sigev_notify_function   = esif_ccb_timer_wrapper;
		se.sigev_value.sival_ptr   = timer_ptr->timer_ctx_ptr;
		se.sigev_notify_attributes = &attr;

		if (0 ==
		    timer_create(CLOCK_REALTIME, &se,
				 (timer_t *)&timer_ptr->timer)) {
			rc = ESIF_OK;
		}
	}
#endif
	return rc;
}


/* Windows Set Timer */
static ESIF_INLINE eEsifError esif_ccb_timer_set_msec(
	esif_ccb_timer_t *timer_ptr,		/* Our Timer */
	const esif_ccb_time_t timeout
	)	/* Timeout in msec */
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	ESIF_ASSERT(timer_ptr != NULL);

	if (NULL != timer_ptr) {
#ifdef ESIF_ATTR_OS_WINDOWS
		if (ESIF_TRUE == CreateTimerQueueTimer(&timer_ptr->timer,
						       NULL,
						       (WAITORTIMERCALLBACK)
						       timer_ptr->timer_ctx_ptr
						       ->cb_func,
						       timer_ptr->timer_ctx_ptr
						       ->cb_context_ptr,
						       (DWORD)timeout,
						       0,
						       WT_EXECUTEONLYONCE)) {
			rc = ESIF_OK;
		}
#endif

#ifdef ESIF_ATTR_OS_LINUX
		struct itimerspec its;
		u64 freq_nanosecs = timeout * 1000 * 1000;	/* convert msec
								 * to nsec */

		its.it_value.tv_sec     = freq_nanosecs / 1000000000;
		its.it_value.tv_nsec    = freq_nanosecs % 1000000000;
		its.it_interval.tv_sec  = 0;
		its.it_interval.tv_nsec = 0;

		if (0 == timer_settime(timer_ptr->timer, 0, &its, NULL))
			rc = ESIF_OK;

#endif
	}
	return rc;
}


/* Windows Kill Timer */
static ESIF_INLINE eEsifError esif_ccb_timer_kill(
	const esif_ccb_timer_t *timer_ptr)	/* Our Timer */
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	ESIF_ASSERT(timer_ptr != NULL);

	if (NULL != timer_ptr) {
#ifdef ESIF_ATTR_OS_WINDOWS
		if (ESIF_TRUE ==
		    DeleteTimerQueueTimer(NULL, timer_ptr->timer, NULL)) {
			rc = ESIF_OK;
		}
#endif

#ifdef ESIF_ATTR_OS_LINUX
		if (0 == timer_delete(timer_ptr->timer))
			rc = ESIF_OK;

#endif
		if (timer_ptr->timer_ctx_ptr != NULL)
			esif_ccb_free(timer_ptr->timer_ctx_ptr);
	}
	return rc;
}


#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_CCB_TIMER_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

