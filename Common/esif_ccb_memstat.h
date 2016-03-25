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

/*
* Kernel Memory Statistics
*/
#pragma pack(push, 1)
struct esif_memory_stats {
	u32  allocs;		/* Total number of allocations */
	u32  frees;		/* Total number of frees       */
	u32  memPoolAllocs;	/* Total number of allocations */
	u32  memPoolFrees;	/* Total number of frees       */
	u32  memPoolObjAllocs;	/* Total number of allocations */
	u32  memPoolObjFrees;	/* Total number of frees       */
};
#pragma pack(pop)

#if defined(ESIF_ATTR_KERNEL)

/* Enable Memory Statistics for ESIF_LF Kernel Driver? */
#ifdef ESIF_FEAT_OPT_USE_VIRT_DRVRS

/* Placeholder macros for memstats when not building ESIF_LF */
#define memstat_inc(ptr)
#define memstat_read(ptr)	(0)
#define memstat_set(ptr)

#else

/* Kernel Memory Stats */
#include "esif_ccb_lock.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct esif_memory_stats g_memstat;
extern esif_ccb_lock_t g_memstat_lock;

#ifdef __cplusplus
}
#endif

static ESIF_INLINE void memstat_inc(u32 *intptr)
{
	esif_ccb_write_lock(&g_memstat_lock);
	(*(intptr))++;
	esif_ccb_write_unlock(&g_memstat_lock);
}

static ESIF_INLINE u32 memstat_read(u32 *intptr)
{
	u32 rc;
	esif_ccb_read_lock(&g_memstat_lock);
	rc = *(intptr);
	esif_ccb_read_unlock(&g_memstat_lock);
	return rc;
}
static ESIF_INLINE void memstat_set(u32 *intptr, u32 val)
{
	esif_ccb_write_lock(&g_memstat_lock);
	*(intptr) = val;
	esif_ccb_write_unlock(&g_memstat_lock);
}

#endif /* not ESIF_FEAT_OPT_USE_VIRT_DRVRS */

#endif /* KERNEL */
