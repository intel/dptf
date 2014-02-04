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

#ifndef _ESIF_CCB_TIMER_H_
#define _ESIF_CCB_TIMER_H_

#include "esif.h"
#include "esif_rc.h"
#include "esif_ccb.h"
#include "esif_ccb_time.h"
#include "esif_ccb_memory.h"
#include "esif_uf_ccb_thread.h"

/******************************************************************************
    KERNEL TIMER
******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

/* Timer Callback Function */
typedef void (*esif_ccb_timer_cb)(void *context);

/* 
 * Timer Is Defined to be opaque here to  accommodate different
 * OS and implementation approaches for timers
 */

#ifdef ESIF_ATTR_OS_LINUX
typedef struct {
    struct delayed_work work;
    esif_ccb_timer_cb   function_ptr;
    esif_ccb_lock_t     context_lock;
    esif_flags_t        exit_flag;
    void                *context_ptr;
} esif_ccb_timer_t;
#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS

typedef struct {
    esif_ccb_timer_cb   function_ptr;
    esif_ccb_lock_t     context_lock;
    esif_flags_t        exit_flag;
    void                *context_ptr;
} esif_ccb_timer_context_t;

typedef WDFTIMER esif_ccb_timer_t;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(esif_ccb_timer_context_t, esif_ccb_get_timer_context)

#endif /* ESIF_ATTR_OS_WINDOWS */

#ifdef ESIF_ATTR_OS_LINUX
/* Timer Callback Wrapper  Find And Fire Function */
static ESIF_INLINE void 
esif_ccb_timer_cb_wrapper(struct work_struct* work)
{
    esif_ccb_timer_t *timer_ptr = container_of(
        (struct delayed_work*) work,
        esif_ccb_timer_t,
        work);

    TIMER_DEBUG(("%s: timer fired!!!!!\n", ESIF_FUNC));
    if ((NULL != timer_ptr) && (!timer_ptr->exit_flag)) {
        esif_ccb_read_lock(&timer_ptr->context_lock);
        timer_ptr->function_ptr(timer_ptr->context_ptr);
        esif_ccb_read_unlock(&timer_ptr->context_lock);
    }
}
#endif /* ESIF_ATTR_OS_LINUX */

#ifdef ESIF_ATTR_OS_WINDOWS
/* Timer Callback Wrapper  Find And Fire Function */

EVT_WDF_TIMER esif_ccb_timer_cb_wrapper;

ESIF_INLINE void 
esif_ccb_timer_cb_wrapper(WDFTIMER timer)
{
    esif_ccb_timer_context_t *timer_context_ptr = esif_ccb_get_timer_context(timer);

    TIMER_DEBUG(("%s: timer fired!!!!!\n", ESIF_FUNC));
    if ((NULL != timer_context_ptr) && (!timer_context_ptr->exit_flag)) {
        esif_ccb_read_lock(&timer_context_ptr->context_lock);
        timer_context_ptr->function_ptr(timer_context_ptr->context_ptr);
        esif_ccb_read_unlock(&timer_context_ptr->context_lock);
    }
}
#endif

/* Timer Initialize */
static ESIF_INLINE 
enum esif_rc esif_ccb_timer_init(esif_ccb_timer_t *timer_ptr)
{
    enum esif_rc rc = ESIF_E_UNSPECIFIED;
#ifdef ESIF_ATTR_OS_LINUX
    TIMER_DEBUG(("%s: timer %p\n", ESIF_FUNC, timer_ptr));
    INIT_DELAYED_WORK(&timer_ptr->work, NULL);
    esif_ccb_lock_init(&timer_ptr->context_lock);
    timer_ptr->exit_flag = FALSE;
    rc = ESIF_OK;
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
    NTSTATUS status;
    WDF_TIMER_CONFIG      timer_config     = {0};
    WDF_OBJECT_ATTRIBUTES timer_attributes = {0};
    esif_ccb_timer_context_t *timer_context_ptr = NULL;

    TIMER_DEBUG(("%s: timer %p\n", ESIF_FUNC, timer_ptr));

    WDF_TIMER_CONFIG_INIT(&timer_config, esif_ccb_timer_cb_wrapper);
    timer_config.AutomaticSerialization = TRUE;

    WDF_OBJECT_ATTRIBUTES_INIT(&timer_attributes);
    timer_attributes.ParentObject = g_wdf_ipc_queue_handle;
    timer_attributes.ExecutionLevel = WdfExecutionLevelPassive;

    WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&timer_attributes, esif_ccb_timer_context_t);
    status  = WdfTimerCreate(&timer_config, &timer_attributes, timer_ptr);
    TIMER_DEBUG(("%s: timer %p status %08x\n", ESIF_FUNC, timer_ptr, status));
    if (STATUS_SUCCESS == status) {
        rc = ESIF_OK;
    }

    timer_context_ptr = esif_ccb_get_timer_context(*timer_ptr);
    if(timer_context_ptr == NULL) {
        rc = ESIF_E_UNSPECIFIED;
        goto exit;
    }

    esif_ccb_lock_init(&timer_context_ptr->context_lock);
    timer_context_ptr->exit_flag = FALSE;
exit:
#endif
    return rc;
}

/* Timer Set */
static ESIF_INLINE enum esif_rc 
esif_ccb_timer_set_msec(esif_ccb_timer_t  *timer_ptr, 
                        esif_ccb_time_t   timeout, 
                        esif_ccb_timer_cb function_ptr, 
                        void              *context_ptr)
{
    enum esif_rc rc = ESIF_E_UNSPECIFIED;
#ifdef ESIF_ATTR_OS_LINUX
    u64 x;
    u64 y;
    u8 result;
#
    timer_ptr->function_ptr = function_ptr;
    timer_ptr->context_ptr = context_ptr;

    TIMER_DEBUG(("%s: timer %p timeout %u\n", ESIF_FUNC, timer_ptr, (int) timeout));

    PREPARE_DELAYED_WORK(&timer_ptr->work, esif_ccb_timer_cb_wrapper);

    /* Keep 32 bit Linux Happy */
    x = timeout * HZ;
    y = 1000;

    do_div(x,y);

    result = schedule_delayed_work(&timer_ptr->work, x);
    if( ESIF_TRUE == result) {
        rc = ESIF_OK;
    }
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
    esif_ccb_timer_context_t *timer_context_ptr = NULL;
    BOOLEAN status;

    TIMER_DEBUG(("%s: timer %p timeout %u\n", ESIF_FUNC, timer_ptr, timeout));

    /* Set Context */
    timer_context_ptr = esif_ccb_get_timer_context(*timer_ptr);
    if (NULL != timer_context_ptr) {
            timer_context_ptr->function_ptr = function_ptr;
            timer_context_ptr->context_ptr  = context_ptr;
            timer_context_ptr->exit_flag = FALSE;
    }

    /* Finally Start Timer */
    status = WdfTimerStart(*timer_ptr, (LONG) timeout * -10000);
    TIMER_DEBUG(("%s: timer %p status %08x\n", ESIF_FUNC, timer_ptr, status));
    if (TRUE == status) {
        rc = ESIF_OK;
    }

#endif 
    return rc;
}

/* Timer Stop And Destroy */
static ESIF_INLINE 
enum esif_rc esif_ccb_timer_kill(esif_ccb_timer_t* timer)
{
    enum esif_rc rc = ESIF_E_UNSPECIFIED;
#ifdef ESIF_ATTR_OS_LINUX
    TIMER_DEBUG(("%s: timer %p\n", ESIF_FUNC, timer));

    esif_ccb_read_lock(&timer->context_lock);
    timer->exit_flag = TRUE;
    esif_ccb_read_unlock(&timer->context_lock);

    if (ESIF_TRUE == cancel_delayed_work(&timer->work)) {
        rc = ESIF_OK;
    }
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
    esif_ccb_timer_context_t *timer_context_ptr = NULL;
    BOOLEAN timer_stop_success;

    TIMER_DEBUG(("%s: timer %p\n", ESIF_FUNC, timer));

    /* WDF Framework Will Cleanup  */    
    timer_stop_success =  WdfTimerStop(*timer, FALSE);
    if (timer_stop_success) {
        rc = ESIF_OK;
        goto exit;
    }

    timer_context_ptr = esif_ccb_get_timer_context(*timer);
    if(timer_context_ptr != NULL) {
        esif_ccb_read_lock(&timer_context_ptr->context_lock);
        timer_context_ptr->exit_flag = TRUE;
        esif_ccb_read_unlock(&timer_context_ptr->context_lock);
    }
exit:
#endif
    return rc;
}

#endif /* ESIF_ATTR_KERNEL*/

/******************************************************************************
    USER TIMER
******************************************************************************/

#ifdef ESIF_ATTR_USER

/* OS Agnostic Callback Function */
typedef void (*esif_ccb_timer_cb)
(
    const void* context_ptr
);

/* OS Agnostic Timer Context */
typedef struct esif_ccb_timer_ctx_s {
    esif_ccb_timer_cb      cb_func;         /* Call Back Function */
    void*                  cb_context_ptr;  /* Call Back Function Context */
} esif_ccb_timer_ctx_t;

#ifdef ESIF_ATTR_OS_LINUX

#include <signal.h>

/* 
    Linux OS Callback Wrapper Prototype
    Each OS Expects its own CALLBACK primitive we use this
    wrapper function to normalize the parameters and and 
    ultimately call our func_ptr and func_context originally 
    provided to the timer.  By example Linux expects a union
    here that contains our timer context pointer which in turn
    contains our function and context for the function.
*/

typedef void (*esif_ccb_timer_cb_wrapper)
(
    union sigval sv
);

/* Linux Timer */
typedef struct esif_ccb_timer_s {
    timer_t                     timer;          /* Linux specific timer */
    esif_ccb_timer_ctx_t*       timer_ctx_ptr;  /* OS Agnostic timer context */
} esif_ccb_timer_t;

/* 
    Linux Callback Wrapper Function 
    Declared as void by Linux so there is not much we can do if
    something goes wrong.  We simply wrap the parameters with a 
    validity check and hope for the best.
*/
static ESIF_INLINE void esif_ccb_timer_wrapper(
    const union sigval sv)
{
    esif_ccb_timer_ctx_t* timer_ctx_ptr = (esif_ccb_timer_ctx_t *)sv.sival_ptr;
    
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
    Windows OS Callback Wrapper Prototype
    Each OS Expects its own CALLBACK primitive we use this
    wrapper function to normalize the parameters and and 
    ultimately call our func_ptr and func_context originally 
    provided to the timer.  By example Windows expects two
    parameters of which we only use one.  The other one is 
    always true.
*/
typedef void (*esif_ccb_timer_cb_wrapper)
(
    void* context_ptr, 
    BOOLEAN notUsed
);

/* Windows Timer */
typedef struct esif_ccb_timer_s {
    HANDLE                  timer;          /* Windows specific timer */    
    esif_ccb_timer_ctx_t*   timer_ctx_ptr;  /* OS Agnostic timer context */
} esif_ccb_timer_t;

/* 
    Windows Callback Wrapper Function
    Declared as void by windows so there is not much we can do if
    something goes wrong.  We simply wrap the parameters with a 
    validity check and hope for the best.
*/

static ESIF_INLINE void esif_ccb_timer_wrapper(
    const void* context_ptr, 
    const BOOLEAN notUsed)
{
    esif_ccb_timer_ctx_t* timer_ctx_ptr = (esif_ccb_timer_ctx_t*) context_ptr;
    
    ESIF_ASSERT(timer_ctx_ptr != NULL);
    ESIF_ASSERT(timer_ctx_ptr->cb_func != NULL);
    ESIF_ASSERT(timer_ctx_ptr->cb_context_ptr != NULL);

    UNREFERENCED_PARAMETER(notUsed);

    if(NULL != timer_ctx_ptr &&
        NULL != timer_ctx_ptr->cb_func &&
        NULL != timer_ctx_ptr->cb_context_ptr) {

        timer_ctx_ptr->cb_func(timer_ctx_ptr->cb_context_ptr);
    }
}

#endif /* ESIF_ATTR_OS_WINDOWS */

/* Windows Initialize Timer */
static ESIF_INLINE eEsifError esif_ccb_timer_init(
    esif_ccb_timer_t* timer_ptr,            /* Our Timer */
    const esif_ccb_timer_cb function_ptr,   /* Callback when timer fires */
    void*             context_ptr)          /* Callback context if any */
{
    eEsifError rc = ESIF_E_UNSPECIFIED;
    ESIF_ASSERT(timer_ptr != NULL);
    ESIF_ASSERT(function_ptr != NULL);

    /* Allocate and setup new data */
    if (NULL != timer_ptr) {
        timer_ptr->timer_ctx_ptr = (esif_ccb_timer_ctx_t*) esif_ccb_malloc(sizeof(esif_ccb_timer_ctx_t));
        if (NULL == timer_ptr->timer_ctx_ptr) return rc;
    
        /* Store state for timer set */
        timer_ptr->timer_ctx_ptr->cb_func = function_ptr;
        timer_ptr->timer_ctx_ptr->cb_context_ptr  = context_ptr;
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
        se.sigev_notify_function = esif_ccb_timer_wrapper;
        se.sigev_value.sival_ptr = timer_ptr->timer_ctx_ptr;
        se.sigev_notify_attributes = &attr;

        if (0 == timer_create(CLOCK_REALTIME, &se, (timer_t*) &timer_ptr->timer))
            rc = ESIF_OK;
    }
#endif
    return rc;
}

/* Windows Set Timer */
static ESIF_INLINE eEsifError esif_ccb_timer_set_msec(
    esif_ccb_timer_t* timer_ptr,        /* Our Timer */
    const esif_ccb_time_t   timeout)    /* Timeout in msec */
{
    eEsifError rc = ESIF_E_UNSPECIFIED;
    ESIF_ASSERT(timer_ptr != NULL);

    if (NULL != timer_ptr) {
#ifdef ESIF_ATTR_OS_WINDOWS
        if( ESIF_TRUE == CreateTimerQueueTimer(&timer_ptr->timer, 
            NULL,
            (WAITORTIMERCALLBACK) timer_ptr->timer_ctx_ptr->cb_func,
            timer_ptr->timer_ctx_ptr->cb_context_ptr,
            (DWORD) timeout,
            0,
            WT_EXECUTEONLYONCE)) {
                rc = ESIF_OK;
        }
#endif

#ifdef ESIF_ATTR_OS_LINUX 
        struct itimerspec its;
        u64 freq_nanosecs = timeout * 1000 * 1000; // convert msec to nsec

        its.it_value.tv_sec = freq_nanosecs / 1000000000;
        its.it_value.tv_nsec = freq_nanosecs % 1000000000;
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
    
        if(0 == timer_settime(timer_ptr->timer, 0, &its, NULL)) {
            rc = ESIF_OK;
        }
#endif
    }
    return rc;
}

/* Windows Kill Timer */
static ESIF_INLINE eEsifError esif_ccb_timer_kill(
    const esif_ccb_timer_t* timer_ptr)  /* Our Timer */
{
    eEsifError rc = ESIF_E_UNSPECIFIED;
    ESIF_ASSERT(timer_ptr != NULL);

    if (NULL != timer_ptr) {
#ifdef ESIF_ATTR_OS_WINDOWS
        if (ESIF_TRUE == DeleteTimerQueueTimer(NULL, timer_ptr->timer, NULL)) {
            rc = ESIF_OK;
        }
#endif

#ifdef ESIF_ATTR_OS_LINUX
        if (0 == timer_delete(timer_ptr->timer)) {
            rc = ESIF_OK;
        }

#endif
        if (timer_ptr->timer_ctx_ptr != NULL) {
            esif_ccb_free(timer_ptr->timer_ctx_ptr);
        }
    }
    return rc;
}

#endif /* ESIF_ATTR_USER */

#endif /* _ESIF_CCB_TIMER_H_ */