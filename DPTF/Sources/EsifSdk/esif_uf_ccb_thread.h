/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_CCB_THREAD_H_
#define _ESIF_CCB_THREAD_H_

#include "esif.h"
#include "esif_rc.h"
#include "esif_ccb.h"
#include "esif_ccb_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ESIF_ATTR_OS_LINUX
    #include <pthread.h>
    typedef pthread_t esif_thread_t;                                /* PTHREAD */
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
    typedef UInt32 esif_thread_t;                                   /* Windows Thread */
#endif

/* Get Current Thread ID */
#ifdef ESIF_ATTR_OS_LINUX
    #define esif_ccb_thread_id_current pthread_self
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
    #define esif_ccb_thread_id_current GetCurrentThreadId
#endif

/* Work Function */
typedef void *(ESIF_CALLCONV * work_func_t)(void *);

/*
 * The callback function signature is different in Windows, so we use a wrapper
 * function to call the work function.
 */
#ifdef ESIF_ATTR_OS_WINDOWS

typedef struct esif_thread_wrapper_ctx {
    work_func_t work_funct_ptr;
    void *work_funct_ctx_ptr;
} esif_thread_wrapper_ctx_t;


static ESIF_INLINE DWORD WINAPI thread_wrapper(void *wrapper_ctx_ptr)
{
    esif_thread_wrapper_ctx_t *ctx_ptr = NULL;
    if (NULL == wrapper_ctx_ptr) {
        goto exit;
    }
    ctx_ptr = (esif_thread_wrapper_ctx_t *) wrapper_ctx_ptr;

    if (NULL == ctx_ptr->work_funct_ptr) {
        goto exit;
    }
    (*ctx_ptr->work_funct_ptr)(ctx_ptr->work_funct_ctx_ptr);
exit:
    if (wrapper_ctx_ptr != NULL) {
        esif_ccb_free(wrapper_ctx_ptr);
    }
    return 0;
}

#endif

static ESIF_INLINE eEsifError esif_ccb_thread_create(
    esif_thread_t *thread_ptr,
    work_func_t function_ptr,
    void *argument_ptr
    )
{
    eEsifError rc = ESIF_OK;

#ifdef ESIF_ATTR_OS_LINUX
    pthread_attr_t attr;                                            /* Thread Attribute     */
    pthread_attr_init(&attr);                                       /* Initialize Attributes */

    if (pthread_create(
            thread_ptr,                                             /* Thread                    */
            &attr,                                                  /* Attributes                */
            function_ptr,                                           /* Function To Start         */
            argument_ptr                                            /* Function Arguments If Any */
            ) != 0) {
        rc = ESIF_E_UNSPECIFIED;
    }
#endif /* Linux */

#ifdef ESIF_ATTR_OS_WINDOWS
    esif_thread_wrapper_ctx_t *wrapper_ctx_ptr = NULL;
    HANDLE hThread =  NULL;

    wrapper_ctx_ptr = (esif_thread_wrapper_ctx_t *)esif_ccb_malloc(sizeof(*wrapper_ctx_ptr));
    if (NULL == wrapper_ctx_ptr) {
        rc = ESIF_E_NO_MEMORY;
        goto exit;
    }

    wrapper_ctx_ptr->work_funct_ptr = function_ptr;
    wrapper_ctx_ptr->work_funct_ctx_ptr = argument_ptr;

    hThread = CreateThread(NULL,                                    /* Security / Cannot NOT be inherited */
                     0,                                             /* Default Stack Size                 */
                     thread_wrapper,                                /* Function To Start                  */
                     wrapper_ctx_ptr,                               /* Function Arguments If Any          */
                     0,                                             /* Flags = Run Immediately            */
                     (LPDWORD)thread_ptr);                          /* Thread                             */
    if (hThread != NULL) {
        CloseHandle(hThread);
    } else {
        esif_ccb_free(wrapper_ctx_ptr);
        rc = ESIF_E_UNSPECIFIED;
    }
exit:
#endif

    return rc;
}


static ESIF_INLINE eEsifError esif_ccb_thread_join (
    esif_thread_t *thread_ptr
    )
{
    eEsifError rc = ESIF_OK;

#ifdef ESIF_ATTR_OS_LINUX
    pthread_join(*thread_ptr, NULL);
#endif /* Linux */
#ifdef ESIF_ATTR_OS_WINDOWS
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, (*(DWORD *)thread_ptr));
    if (hThread != NULL) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    } else {
        rc = ESIF_E_UNSPECIFIED;
    }
#endif

    return rc;
}


/* this is a copy of thread_join, using the term destroy for clarity */
/* we may want to alter it to be a brute force destroy later */
static ESIF_INLINE eEsifError esif_ccb_thread_destroy (
    esif_thread_t *thread_ptr
    )
{
    return esif_ccb_thread_join(thread_ptr);
}


#ifdef __cplusplus
}
#endif /* Windows */
#endif /* _ESIF_CCB_THREAD_H_ */