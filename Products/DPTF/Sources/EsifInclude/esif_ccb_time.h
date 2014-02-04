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

#ifndef _ESIF_CCB_TIME_H_
#define _ESIF_CCB_TIME_H_

#include "esif.h"
#include "esif_ccb.h"

typedef u64 esif_ccb_time_t;

/*******************************************************************************
** Kernel
*******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

    #ifdef ESIF_ATTR_OS_LINUX
    static void ESIF_INLINE esif_ccb_system_time(esif_ccb_time_t *time)
    {
        struct timeval now = {0};

        do_gettimeofday(&now);
        *time =  (now.tv_sec * 1000);       /* Convert sec to msec */
        *time += (now.tv_usec / 1000);      /* Convert usec to msec */
    }
    #endif /* ESIF_ATTR_OS_LINUX */

    #ifdef ESIF_ATTR_OS_WINDOWS
    static void ESIF_INLINE esif_ccb_system_time(esif_ccb_time_t *time)
    {
        LARGE_INTEGER tickPerSecond = {0};
        LARGE_INTEGER tick = {0};

        /* get the high resolution counter's accuracy and tick's / sec */
        tick = KeQueryPerformanceCounter(&tickPerSecond);

        /* system time in msec not wall clock */
        *time = (tick.QuadPart / (tickPerSecond.QuadPart / 1000));
    }
    #endif /* ESIF_ATTR_OS_WINDOWS */

#endif /* ESIF_ATTR_KERNEL */

/*******************************************************************************
** User
*******************************************************************************/

#ifdef ESIF_ATTR_USER

    #ifdef __cplusplus
    extern "C" {
    #endif /* __cplusplus */

    #ifdef ESIF_ATTR_OS_LINUX

        #include <sys/time.h>

        static void ESIF_INLINE esif_ccb_ctime(EsifString time_str, size_t len, const time_t* time)
        {
            ctime_r(time, time_str);
        }

        static void ESIF_INLINE esif_ccb_system_time(esif_ccb_time_t *time)
        {
            struct timeval now = {0};

            gettimeofday(&now, NULL);
            *time =  (now.tv_sec * 1000);   /* Convert sec to msec */
            *time += (now.tv_usec / 1000);  /* Convert usec to msec */
        }

        static void ESIF_INLINE esif_ccb_get_time(struct timeval *tv)
        {
            gettimeofday(tv, NULL);
        }

    #endif /* ESIF_ATTR_OS_LINUX */

    #ifdef ESIF_ATTR_OS_WINDOWS

        static void ESIF_INLINE esif_ccb_ctime(esif_string time_str, size_t len, const time_t* time)
        {
            ctime_s(time_str, len, time);
        }

        static void ESIF_INLINE esif_ccb_system_time(esif_ccb_time_t *time)
        {
            LARGE_INTEGER tickPerSecond = {0};
            LARGE_INTEGER tick = {0};

            /* get the high resolution counter's accuracy and tick / sec */
            QueryPerformanceFrequency(&tickPerSecond);
            QueryPerformanceCounter(&tick);

            /* system time in msec not wall clock */
            *time = (tick.QuadPart / (tickPerSecond.QuadPart / 1000));
        }

        static void ESIF_INLINE esif_ccb_get_time(struct timeval *tv)
        {
            LARGE_INTEGER tickPerSecond = {0};
            LARGE_INTEGER tick = {0};
            time_t rawtime;

            /* Get Seconds */
            time(&rawtime);
            tv->tv_sec = (long) rawtime;

            /* get the high resolution counter's accuracy and tick/sec */
            QueryPerformanceFrequency(&tickPerSecond);
            QueryPerformanceCounter(&tick);

            /* Get usec */
            tv->tv_usec = (long) (tick.QuadPart / (tickPerSecond.QuadPart / 1000000));
        }

    #endif /* ESIF_ATTR_OS_WINDOWS */

    #ifdef __cplusplus
    }
    #endif /* __cplusplus */

#endif /* ESIF_ATTR_USER */

#endif /* _ESIF_CCB_TIME_H_ */
