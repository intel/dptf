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

#ifndef _ESIF_CCB_H_
#define _ESIF_CCB_H_

#include "esif.h"
#include "esif_debug.h"

/* Debug Options */
#define MEMORY_DEBUG  NO_ESIF_DEBUG
#define MEMPOOL_DEBUG NO_ESIF_DEBUG
#define MEMTYPE_DEBUG NO_ESIF_DEBUG
#define TIMER_DEBUG   NO_ESIF_DEBUG
#define THREAD_DEBUG  NO_ESIF_DEBUG

/*******************************************************************************
** User
*******************************************************************************/

#ifdef ESIF_ATTR_USER

    #include <stdlib.h>
    #include <string.h>

    #ifdef ESIF_ATTR_OS_LINUX
        #define esif_ccb_sleep              sleep
        #define esif_ccb_sleep_msec(x)      usleep(x * 1000);
        #define esif_ccb_stat               stat
        // FIXME : This must be commented out so that it doesn't collide with
        // the STL implementation of MAX
        //#define max(a,b)                    (((a) > (b)) ? (a) : (b))
    #endif

    #ifdef ESIF_ATTR_OS_WINDOWS
        #include <time.h>
        #include <winsock2.h>
        #define esif_ccb_sleep(x)           Sleep(x * 1000)
        #define esif_ccb_sleep_msec         Sleep
        #define esif_ccb_stat(a, b)         _stat(a, (struct _stat*) b)
    #endif

#endif /* ESIF_ATTR_USER */

#endif /* _ESIF_CCB_H_ */
