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

#ifndef _ESIF_CCB_H_
#define _ESIF_CCB_H_

#ifdef ESIF_ATTR_USER
    #include "esif.h"
#endif

/* Debug Options */
#define MEMORY_DEBUG  NO_ESIF_DEBUG
#define MEMPOOL_DEBUG NO_ESIF_DEBUG
#define MEMTYPE_DEBUG NO_ESIF_DEBUG
#define TIMER_DEBUG   NO_ESIF_DEBUG
#define THREAD_DEBUG  NO_ESIF_DEBUG

#ifdef ESIF_ATTR_USER
#include <stdlib.h>			/* Standard Library */
#include <string.h>			/* String Library   */

#ifdef ESIF_ATTR_OS_LINUX

#define esif_ccb_sleep sleep	/* Sleep Wrapper */
#define esif_ccb_sleep_msec(x) usleep(x * 1000);
#define esif_ccb_stat stat	/* Stat Wrapper  */
#define max(a, b) (((a) > (b)) ? (a) : (b))

#endif

#ifdef ESIF_ATTR_OS_WINDOWS

#include <time.h>	/* Time Library       */
#include <winsock2.h>	/* Timeval Struct Def */
#define esif_ccb_sleep(x) Sleep(x * 1000)
#define esif_ccb_sleep_msec Sleep
#define esif_ccb_stat(a, b) _stat(a, (struct _stat *)b)

#endif

#include "esif_uf_ccb_thread.h"
#include "esif_uf_ccb_library.h"
#endif /* ESIF_ATTR_USER */

#include "esif_ccb_memory.h"
#include "esif_ccb_lock.h"
#include "esif_ccb_time.h"
#include "esif_ccb_string.h"
#include "esif_ccb_mempool.h"
#include "esif_ccb_sem.h"
#include "esif_ccb_timer.h"

#ifdef ESIF_ATTR_USER
#include "esif_uf_ccb_file.h"
#endif

#ifdef ESIF_ATTR_KERNEL

#include "esif_lf_ccb_mbi.h"
#include "esif_lf_ccb_mmio.h"
#include "esif_lf_ccb_msr.h"
#include "esif_lf_ccb_acpi.h"

#endif /* ESIF_ATTR_KERNEL */
#endif /* _ESIF_CCB_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
