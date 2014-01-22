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

#ifndef _ESIF_MEMTYPE_H_
#define _ESIF_MEMTYPE_H_

#ifdef ESIF_ATTR_USER
    #include "esif.h"
#endif


/* Variable Length Common Memory Types */
enum esif_memtype_type {
	ESIF_MEMTYPE_TYPE_EVENT = 0,	/* Event */
	ESIF_MEMTYPE_TYPE_MAX
};


#ifdef __cplusplus
extern "C" {
#endif

/* Implemented in esif_lf.c */
extern struct esif_ccb_memtype *g_memtype[ESIF_MEMTYPE_TYPE_MAX];
extern esif_ccb_lock_t g_memtype_lock;

static ESIF_INLINE char *esif_memtype_str(u32 type)
{
	#define ESIF_CREATE_MEMTYPE(mp, mpd, str) case mp: str = (esif_string)mpd; break;

	esif_string str = (esif_string)ESIF_NOT_AVAILABLE;
	switch (type) {
		ESIF_CREATE_MEMTYPE(ESIF_MEMTYPE_TYPE_EVENT, "esif_event_dynamic", str)
	}
	return str;
}

static ESIF_INLINE struct esif_ccb_memtype *esif_ccb_memtype_create(
	enum esif_memtype_type type_tag);
static ESIF_INLINE void esif_ccb_memtype_destroy(
	enum esif_memtype_type type_tag);

/* Memory Type Not Fixed Size Use Mempool For That */
struct esif_ccb_memtype {
	esif_string  name_ptr;		/* Name             */
	u32          type_tag;		/* Type Of Memory   */
	u32          alloc_count;	/* Allocation Count */
	u32          free_count;	/* Free Count       */
};


static ESIF_INLINE void esif_ccb_memtype_init_tracking(void)
{
	u32 type_tag;

	esif_ccb_lock_init(&g_memtype_lock);
	/*
	 * Create the tracking array for all supported types.
	 */
	for (type_tag = 0; type_tag < ESIF_MEMTYPE_TYPE_MAX; type_tag++)
		esif_ccb_memtype_create((enum esif_memtype_type)type_tag);
}


static ESIF_INLINE void esif_ccb_memtype_uninit_tracking(void)
{
	u32 type_tag;

	/*
	 * Create the tracking array for all supported types.
	 */
	for (type_tag = 0; type_tag < ESIF_MEMTYPE_TYPE_MAX; type_tag++)
		esif_ccb_memtype_destroy((enum esif_memtype_type)type_tag);

	esif_ccb_lock_uninit(&g_memtype_lock);
}


/* Memory Type Create */
static ESIF_INLINE struct esif_ccb_memtype *esif_ccb_memtype_create(
	enum esif_memtype_type type_tag
	)
{
	struct esif_ccb_memtype *memtype_ptr = NULL;

	if (type_tag >= ESIF_MEMTYPE_TYPE_MAX)
		goto exit;

	memtype_ptr = (struct esif_ccb_memtype *)
			esif_ccb_malloc(sizeof(*memtype_ptr));

	if (NULL == memtype_ptr)
		goto exit;

	memtype_ptr->type_tag = type_tag;
	memtype_ptr->name_ptr = esif_memtype_str(type_tag);
	memtype_ptr->alloc_count = 0;
	memtype_ptr->free_count  = 0;

	esif_ccb_write_lock(&g_memtype_lock);

	g_memtype[type_tag] = memtype_ptr;
#ifdef ESIF_ATTR_KERNEL
	memstat_inc(&g_memstat.memTypeAllocs);
#endif
	esif_ccb_write_unlock(&g_memtype_lock);

	MEMTYPE_DEBUG("%s: Memory Type %s Create\n",
		      ESIF_FUNC,
		      memtype_ptr->name_ptr);
exit:
	return memtype_ptr;
}


/* Memory Type Destroy */
static ESIF_INLINE void esif_ccb_memtype_destroy(
	enum esif_memtype_type type_tag
	)
{
	u32 remain = 0;
	struct esif_ccb_memtype *memtype_ptr = NULL;

	if (type_tag >= ESIF_MEMTYPE_TYPE_MAX)
		goto exit;

	esif_ccb_write_lock(&g_memtype_lock);

	memtype_ptr = g_memtype[type_tag];

	if (NULL == memtype_ptr) {
		esif_ccb_write_unlock(&g_memtype_lock);
		goto exit;
	}

	remain = memtype_ptr->alloc_count - memtype_ptr->free_count;
	MEMPOOL_DEBUG("%s: Memory Type %s Destroy alloc=%d free=%d remain=%d\n",
		      ESIF_FUNC,
		      memtype_ptr->name_ptr,
		      memtype_ptr->alloc_count,
		      memtype_ptr->free_count,
		      remain);

#ifdef ESIF_ATTR_KERNEL
	memstat_inc(&g_memstat.memTypeFrees);
#endif

	esif_ccb_free(memtype_ptr);
	g_memtype[type_tag] = NULL;

	esif_ccb_write_unlock(&g_memtype_lock);
exit:
	;
}


/* Memory Type Allocate */
static ESIF_INLINE void *esif_ccb_memtype_alloc(
	enum esif_memtype_type type_tag,
	u32 size
	)
{
	void *mem_ptr = NULL;
	struct esif_ccb_memtype *memtype_ptr = NULL;

	/*
	 * Attempt allocation first, so if unsuccessful updating stats we still
	 * return usable memory.
	 */
	mem_ptr = esif_ccb_malloc(size);
	if (NULL == mem_ptr)
		goto exit;

	if (type_tag >= ESIF_MEMTYPE_TYPE_MAX)
		goto exit;

	esif_ccb_write_lock(&g_memtype_lock);

	memtype_ptr = g_memtype[type_tag];

	if (NULL == memtype_ptr) {
		esif_ccb_write_unlock(&g_memtype_lock);
		goto exit;
	}

	memtype_ptr->alloc_count++;
#ifdef ESIF_ATTR_KERNEL
	memstat_inc(&g_memstat.memTypeObjAllocs);
#endif

	MEMPOOL_DEBUG("%s: MT Entry Allocated(%d)=%p From Memtype %s\n",
		      ESIF_FUNC,
		      memtype_ptr->alloc_count,
		      mem_ptr,
		      memtype_ptr->name_ptr);

	esif_ccb_write_unlock(&g_memtype_lock);

exit:
	return mem_ptr;
}


/* Memory Type ZERO Allocate */
static ESIF_INLINE void *esif_ccb_memtype_zalloc(
	enum esif_memtype_type type_tag,
	u32 size
	)
{
	void *mem_ptr = NULL;

	mem_ptr = esif_ccb_memtype_alloc(type_tag, size);
	if (NULL == mem_ptr)
		goto exit;

	esif_ccb_memset(mem_ptr, 0, size);

exit:
	return mem_ptr;
}


/* Memory Type Free */
static ESIF_INLINE void esif_ccb_memtype_free (
	enum esif_memtype_type type_tag,
	void *mem_ptr
	)
{
	struct esif_ccb_memtype *memtype_ptr = NULL;

	/*
	 * Deallocate first, so even if we fail to update stats, the memory
	 * is still freed.
	 */
	if (mem_ptr != NULL)
		esif_ccb_free(mem_ptr);


	if (type_tag >= ESIF_MEMTYPE_TYPE_MAX)
		goto exit;


	esif_ccb_write_lock(&g_memtype_lock);

	memtype_ptr = g_memtype[type_tag];

	if (NULL == memtype_ptr) {
		esif_ccb_write_unlock(&g_memtype_lock);
		goto exit;
	}

	memtype_ptr->free_count++;

#ifdef ESIF_ATTR_KERNEL
	memstat_inc(&g_memstat.memTypeObjFrees);
#endif
	MEMPOOL_DEBUG("%s: MT Entry Freed(%d)=%p From Memtype %s\n", ESIF_FUNC,
		      memtype_ptr->free_count, mem_ptr, memtype_ptr->name_ptr);

	esif_ccb_write_unlock(&g_memtype_lock);

exit:
	;
}


#ifdef __cplusplus
}
#endif

#endif /* _ESIF_MEMTYPE_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
