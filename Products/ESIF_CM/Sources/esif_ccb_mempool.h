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

#ifndef _ESIF_CCB_MEMPOOL_H_
#define _ESIF_CCB_MEMPOOL_H_

extern esif_ccb_lock_t g_mempool_lock;

#ifdef ESIF_ATTR_KERNEL

struct esif_ccb_mempool {
#ifdef ESIF_ATTR_OS_LINUX
	struct kmem_cache  *cache;
#endif /* Linux */

#ifdef ESIF_ATTR_OS_WINDOWS
	/* TODO: Look aside List? */
#endif /* Windows */
	esif_string  name_ptr;		/* Name                         */
	u32          pool_tag;		/* Pool Tag                     */
	u32          object_size;	/* Size Of Pool Object In Bytes */
	u32          alloc_count;	/* Object Allocation Count      */
	u32          free_count;	/* Object Free Count            */
};


/* Memory Pool Create */
static ESIF_INLINE struct esif_ccb_mempool *esif_ccb_mempool_create(
	enum esif_mempool_type pool_type,
	u32 pool_tag,
	u32 object_size
	)
{
	struct esif_ccb_mempool *pool_ptr = NULL;

	if (pool_type >= ESIF_MEMPOOL_TYPE_MAX)
		goto exit;

	pool_ptr = (struct esif_ccb_mempool *)
			esif_ccb_malloc(sizeof(*pool_ptr));

	if (NULL == pool_ptr)
		return NULL;

	pool_ptr->name_ptr    = esif_mempool_str(pool_tag);
	pool_ptr->pool_tag    = pool_tag;
	pool_ptr->alloc_count = 0;
	pool_ptr->free_count  = 0;
	pool_ptr->object_size = object_size;

#ifdef ESIF_ATTR_OS_LINUX
	pool_ptr->cache = kmem_cache_create(pool_ptr->name_ptr,
					    pool_ptr->object_size,
					    0,
					    SLAB_HWCACHE_ALIGN,
					    NULL);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
/* TODO  Look aside List? */
#endif
	esif_ccb_write_lock(&g_mempool_lock);

	g_mempool[pool_type] = pool_ptr;
	memstat_inc(&g_memstat.memPoolAllocs);

	MEMPOOL_DEBUG("%s: Memory Pool %s Create Object Size=%d\n", ESIF_FUNC,
		      pool_ptr->name_ptr, pool_ptr->object_size);

	esif_ccb_write_unlock(&g_mempool_lock);

exit:
	return pool_ptr;
}


/* Memory Pool Destroy */
static ESIF_INLINE void esif_ccb_mempool_destroy(
	enum esif_mempool_type pool_type
	)
{
	struct esif_ccb_mempool *pool_ptr = NULL;
	int remain = 0;

	if (pool_type >= ESIF_MEMPOOL_TYPE_MAX)
		goto exit;

	esif_ccb_write_lock(&g_mempool_lock);

	pool_ptr = g_mempool[pool_type];

	if (NULL == pool_ptr) {
		esif_ccb_write_unlock(&g_mempool_lock);
		goto exit;
	}

	remain = pool_ptr->alloc_count - pool_ptr->free_count;
	MEMPOOL_DEBUG("%s: Memory Pool %s Destroy alloc=%d free=%d remain=%d\n",
		      ESIF_FUNC,
		      pool_ptr->name_ptr,
		      pool_ptr->alloc_count,
		      pool_ptr->free_count,
		      remain);

#ifdef ESIF_ATTR_OS_LINUX
	if (NULL != pool_ptr->cache)
		kmem_cache_destroy(pool_ptr->cache);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	/* Nothing To Do */
#endif
	g_mempool[pool_type] = NULL;
	memstat_inc(&g_memstat.memPoolFrees);

	esif_ccb_write_unlock(&g_mempool_lock);

	esif_ccb_free(pool_ptr);
exit:
	;
}


/* Memory Pool Alloc */
static ESIF_INLINE void *esif_ccb_mempool_alloc(
	enum esif_mempool_type pool_type
	)
{
	struct esif_ccb_mempool *pool_ptr = NULL;
	void *mem_ptr = NULL;

	if (pool_type >= ESIF_MEMPOOL_TYPE_MAX)
		goto exit;

	esif_ccb_write_lock(&g_mempool_lock);

	pool_ptr = g_mempool[pool_type];
	if (NULL == pool_ptr) {
		esif_ccb_write_unlock(&g_mempool_lock);
		goto exit;
	}

#ifdef ESIF_ATTR_OS_LINUX
	mem_ptr = kmem_cache_alloc(pool_ptr->cache, GFP_ATOMIC);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	mem_ptr = ExAllocatePoolWithTag(NonPagedPool,
					pool_ptr->object_size,
					pool_ptr->pool_tag);
#endif

	if (NULL == mem_ptr) {
		esif_ccb_write_unlock(&g_mempool_lock);
		goto exit;
	}

	pool_ptr->alloc_count++;
	memstat_inc(&g_memstat.memPoolObjAllocs);

	MEMPOOL_DEBUG("%s: MP Entry Allocated(%d)=%p From Mempool %s\n",
		      ESIF_FUNC,
		      pool_ptr->alloc_count,
		      mem_ptr,
		      pool_ptr->name_ptr);

	esif_ccb_write_unlock(&g_mempool_lock);
exit:
	return mem_ptr;
}


/* Memory Pool Free */
static ESIF_INLINE void esif_ccb_mempool_free(
	enum esif_mempool_type pool_type,
	void *mem_ptr
	)
{
	struct esif_ccb_mempool *pool_ptr = NULL;

	if ((NULL == mem_ptr) || (pool_type >= ESIF_MEMPOOL_TYPE_MAX))
		goto exit;

	esif_ccb_write_lock(&g_mempool_lock);

	pool_ptr = g_mempool[pool_type];

	if (NULL == pool_ptr) {
		esif_ccb_write_unlock(&g_mempool_lock);
		goto exit;
	}

#ifdef ESIF_ATTR_OS_LINUX
	kmem_cache_free(pool_ptr->cache, mem_ptr);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	ExFreePoolWithTag(mem_ptr, pool_ptr->pool_tag);
#endif

	pool_ptr->free_count++;
	memstat_inc(&g_memstat.memPoolObjFrees);

	MEMPOOL_DEBUG("%s: MP Entry Freed(%d)=%p From Mempool %s\n", ESIF_FUNC,
		      pool_ptr->free_count, mem_ptr, pool_ptr->name);

	esif_ccb_write_unlock(&g_mempool_lock);
exit:
	;
}


#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER

#include "esif.h"

struct esif_ccb_mempool {
	esif_string  name_ptr;		/* Name                         */
	UInt32       pool_tag;		/* Pool Tag                     */
	UInt32       object_size;	/* Size Of Pool Object In Bytes */
	UInt32       alloc_count;	/* Object Allocation Count      */
	UInt32       free_count;	/* Object Free Count            */
};

/* Memory Pool Create */
static ESIF_INLINE struct esif_ccb_mempool *esif_ccb_mempool_create(
	enum esif_mempool_type pool_type,
	UInt32 pool_tag,
	UInt32 object_size
	)
{
	struct esif_ccb_mempool *pool_ptr = NULL;

	if (pool_type >= ESIF_MEMPOOL_TYPE_MAX)
		goto exit;

	pool_ptr = (struct esif_ccb_mempool *)
			esif_ccb_malloc(sizeof(*pool_ptr));

	if (NULL == pool_ptr)
		return NULL;

	pool_ptr->name_ptr    = esif_mempool_str(pool_tag);
	pool_ptr->pool_tag    = pool_tag;
	pool_ptr->alloc_count = 0;
	pool_ptr->free_count  = 0;
	pool_ptr->object_size = object_size;

	esif_ccb_write_lock(&g_mempool_lock);

	g_mempool[pool_type] = pool_ptr;

	MEMPOOL_DEBUG("%s: Memory Pool %s Create Object Size=%d\n", ESIF_FUNC,
		      pool_ptr->name_ptr, pool_ptr->object_size);

	esif_ccb_write_unlock(&g_mempool_lock);
exit:
	return pool_ptr;
}

/* Memory Pool Destroy */
static ESIF_INLINE void esif_ccb_mempool_destroy(
	enum esif_mempool_type pool_type
	)
{
	struct esif_ccb_mempool *pool_ptr = NULL;
	int remain = 0;

	if (pool_type >= ESIF_MEMPOOL_TYPE_MAX)
		goto exit;

	esif_ccb_write_lock(&g_mempool_lock);

	pool_ptr = g_mempool[pool_type];

	if (NULL == pool_ptr) {
		esif_ccb_write_unlock(&g_mempool_lock);
		goto exit;
	}

	remain = pool_ptr->alloc_count - pool_ptr->free_count;
	MEMPOOL_DEBUG("%s: Memory Pool %s Destroy alloc=%d free=%d remain=%d\n",
		      ESIF_FUNC,
		      pool_ptr->name_ptr,
		      pool_ptr->alloc_count,
		      pool_ptr->free_count,
		      remain);

	g_mempool[pool_type] = NULL;

	esif_ccb_write_unlock(&g_mempool_lock);

	esif_ccb_free(pool_ptr);
exit:
	;
}


/* Memory Pool Alloc */
static ESIF_INLINE void *esif_ccb_mempool_alloc(
	enum esif_mempool_type pool_type
	)
{
	struct esif_ccb_mempool *pool_ptr = NULL;
	void *mem_ptr = NULL;

	if (pool_type >= ESIF_MEMPOOL_TYPE_MAX)
		goto exit;

	esif_ccb_write_lock(&g_mempool_lock);

	pool_ptr = g_mempool[pool_type];
	if (NULL == pool_ptr) {
		esif_ccb_write_unlock(&g_mempool_lock);
		goto exit;
	}

	mem_ptr = esif_ccb_malloc(pool_ptr->object_size);
	if (NULL == mem_ptr) {
		esif_ccb_write_unlock(&g_mempool_lock);
		goto exit;
	}

	pool_ptr->alloc_count++;

	MEMPOOL_DEBUG("%s: MP Entry Allocated(%d)=%p From Mempool %s\n",
		      ESIF_FUNC,
		      pool_ptr->alloc_count,
		      mem_ptr,
		      pool_ptr->name_ptr);

	esif_ccb_write_unlock(&g_mempool_lock);

exit:
	return mem_ptr;
}


/* Memory Pool Free */
static ESIF_INLINE void esif_ccb_mempool_free(
	enum esif_mempool_type pool_type,
	void *mem_ptr
	)
{
	struct esif_ccb_mempool *pool_ptr = NULL;

	if (mem_ptr != NULL)
		esif_ccb_free(mem_ptr);

	if (pool_type >= ESIF_MEMPOOL_TYPE_MAX)
		goto exit;

	esif_ccb_write_lock(&g_mempool_lock);

	pool_ptr = g_mempool[pool_type];

	if (NULL == pool_ptr) {
		esif_ccb_write_unlock(&g_mempool_lock);
		goto exit;
	}

	pool_ptr->free_count++;

	MEMPOOL_DEBUG("%s: MP Entry Freed(%d)=%p From Mempool %s\n", ESIF_FUNC,
		      pool_ptr->free_count, mem_ptr, pool_ptr->name_ptr);

	esif_ccb_write_unlock(&g_mempool_lock);

exit:
		;
}


#endif /* ESIF_ATTR_USER */


/* NOTE:  This function is common to user and kernel mode */
static ESIF_INLINE void esif_ccb_mempool_init_tracking(void)
{
	esif_ccb_lock_init(&g_mempool_lock);
}


/* NOTE:  This function is common to user and kernel mode */
static ESIF_INLINE void esif_ccb_mempool_uninit_tracking(void)
{
	u32 type_tag;

	/*
	 * Create the tracking array for all supported types.
	 */
	for (type_tag = 0; type_tag < ESIF_MEMPOOL_TYPE_MAX; type_tag++)
		esif_ccb_mempool_destroy((enum esif_mempool_type)type_tag);

	esif_ccb_lock_uninit(&g_mempool_lock);
}

/* Memory Pool ZERO Alloc */
/* NOTE:  This function is common to user and kernel mode */
static ESIF_INLINE void *esif_ccb_mempool_zalloc(
	enum esif_mempool_type pool_type
	)
{
	struct esif_ccb_mempool *pool_ptr = NULL;
	void *mem_ptr = NULL;

	mem_ptr = esif_ccb_mempool_alloc(pool_type);

	if (NULL == mem_ptr)
		goto exit;

	/*
	 * If the allocation above is successful, we know the pool_type is in
	 * bounds, so no check required in this function.
	 */
	esif_ccb_read_lock(&g_mempool_lock);

	pool_ptr = g_mempool[pool_type];

	if (NULL == pool_ptr) {
		esif_ccb_read_unlock(&g_mempool_lock);
		esif_ccb_mempool_free(pool_type, mem_ptr);
		mem_ptr = NULL;
		goto exit;
	}

	esif_ccb_memset(mem_ptr, 0, pool_ptr->object_size);
	esif_ccb_read_unlock(&g_mempool_lock);
exit:
	return mem_ptr;
}


#endif /* _ESIF_CCB_MEMPOOL_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
