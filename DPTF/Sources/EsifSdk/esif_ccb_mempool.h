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

#ifndef _ESIF_CCB_MEMPOOL_H_
#define _ESIF_CCB_MEMPOOL_H_

#include "esif.h"
#include "esif_ccb.h"
#include "esif_ccb_memory.h"
#include "esif_mempool.h"

/*******************************************************************************
** Kernel
*******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

    struct esif_ccb_mempool {
    #ifdef ESIF_ATTR_OS_LINUX
        struct kmem_cache* cache;
    #endif /* Linux */

    #ifdef ESIF_ATTR_OS_WINDOWS
        /* TODO: Look aside List? */
    #endif /* Windows */
        esif_string name_ptr;               /* Name                         */
        u32         pool_tag;               /* Pool Tag                     */
        u32         object_size;            /* Size Of Pool Object In Bytes */
        u32         alloc_count;            /* Object Allocation Count      */
        u32         free_count;             /* Object Free Count            */
    };

    /* Memory Pool Create */
    static ESIF_INLINE struct esif_ccb_mempool
    *esif_ccb_mempool_create(u32 pool_tag,
                             u32 object_size)
    {
        struct esif_ccb_mempool *pool_ptr = esif_ccb_malloc(sizeof(struct esif_ccb_mempool));
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

        g_memstat.memPoolAllocs++;
        MEMPOOL_DEBUG(("%s: Memory Pool %s Create Object Size=%d\n", ESIF_FUNC,
            pool_ptr->name_ptr, pool_ptr->object_size));
        return pool_ptr;
    }

    /* Memory Pool Destroy */
    static ESIF_INLINE void
    esif_ccb_mempool_destroy(struct esif_ccb_mempool *pool_ptr)
    {
        int remain = 0;
        if (NULL == pool_ptr)
            return;

        remain = pool_ptr->alloc_count - pool_ptr->free_count;
        MEMPOOL_DEBUG(("%s: Memory Pool %s Destroy alloc=%d free=%d remain=%d\n", ESIF_FUNC,
            pool_ptr->name_ptr, pool_ptr->alloc_count, pool_ptr->free_count, remain));

    #ifdef ESIF_ATTR_OS_LINUX
        if (NULL != pool_ptr->cache)
            kmem_cache_destroy (pool_ptr->cache);
    #endif

    #ifdef ESIF_ATTR_OS_WINDOWS
      /* Nothing To Do */
    #endif

        g_memstat.memPoolFrees++;

        esif_ccb_free(pool_ptr);
        if (remain != 0) {} // ASSERT();
    }

    /* Memory Pool Alloc */
    static ESIF_INLINE void
    *esif_ccb_mempool_alloc(struct esif_ccb_mempool *pool_ptr)
    {
        void *mem_ptr = NULL;

        if (NULL == pool_ptr)
            return NULL;

    #ifdef ESIF_ATTR_OS_LINUX
        mem_ptr = kmem_cache_alloc(pool_ptr->cache, GFP_ATOMIC);
    #endif
    #ifdef ESIF_ATTR_OS_WINDOWS
        mem_ptr = ExAllocatePoolWithTag(NonPagedPool,
                                            pool_ptr->object_size,
                                            pool_ptr->pool_tag);
    #endif
        pool_ptr->alloc_count++;
        g_memstat.memPoolObjAllocs++;
        MEMPOOL_DEBUG(("%s: MP Entry Allocated(%d)=%p From Mempool %s\n", ESIF_FUNC,
            pool_ptr->alloc_count, mem_ptr, pool_ptr->name_ptr));
        return mem_ptr;
    }

    /* Memory Pool ZERO Alloc */
    static ESIF_INLINE void
    *esif_ccb_mempool_zalloc(struct esif_ccb_mempool *pool_ptr)
    {
        void *mem_ptr = NULL;

        if (NULL == pool_ptr)
                    return NULL;
    #ifdef ESIF_ATTR_OS_LINUX
        mem_ptr = kmem_cache_zalloc(pool_ptr->cache, GFP_ATOMIC);
    #endif

    #ifdef ESIF_ATTR_OS_WINDOWS
        mem_ptr = ExAllocatePoolWithTag(NonPagedPool,
                                            pool_ptr->object_size,
                                            pool_ptr->pool_tag);
        if (NULL != mem_ptr) esif_ccb_memset(mem_ptr, 0, pool_ptr->object_size);
    #endif

        pool_ptr->alloc_count++;
        g_memstat.memPoolObjAllocs++;

        MEMPOOL_DEBUG(("%s: MPZ Entry Allocated(%d)=%p From Mempool %s\n", ESIF_FUNC,
            pool_ptr->alloc_count, mem_ptr, pool_ptr->name));
        return mem_ptr;
    }

    /* Memory Pool Free */
    static ESIF_INLINE void esif_ccb_mempool_free(struct esif_ccb_mempool *pool_ptr, void* mem_ptr)
    {
        if (NULL == pool_ptr || NULL == mem_ptr)
            return;

        MEMPOOL_DEBUG(("%s: MP Entree Freed(%d)=%p From Mempool %s\n", ESIF_FUNC,
            pool_ptr->free_count, mem_ptr, pool_ptr->name));

    #ifdef ESIF_ATTR_OS_LINUX
        kmem_cache_free (pool_ptr->cache, mem_ptr);
    #endif

    #ifdef ESIF_ATTR_OS_WINDOWS
        ExFreePoolWithTag(mem_ptr, pool_ptr->pool_tag);
    #endif

        pool_ptr->free_count++;
        g_memstat.memPoolObjFrees++;
    }

#endif /* ESIF_ATTR_KERNEL */

/*******************************************************************************
** User
*******************************************************************************/

#ifdef ESIF_ATTR_USER

struct esif_ccb_mempool {
    esif_string name_ptr;                   /* Name                         */
    UInt32      pool_tag;                   /* Pool Tag                     */
    UInt32      object_size;                /* Size Of Pool Object In Bytes */
    UInt32      alloc_count;                /* Object Allocation Count      */
    UInt32      free_count;                 /* Object Free Count            */
};

/* Memory Pool Create */
static ESIF_INLINE struct esif_ccb_mempool
*esif_ccb_mempool_create(UInt32 pool_tag,
                         UInt32 object_size)
{
    struct esif_ccb_mempool *pool_ptr = (struct esif_ccb_mempool*) esif_ccb_malloc(sizeof(struct esif_ccb_mempool));
    if (NULL == pool_ptr)
        return NULL;

    pool_ptr->name_ptr    = esif_mempool_str(pool_tag);
    pool_ptr->pool_tag    = pool_tag;
    pool_ptr->alloc_count = 0;
    pool_ptr->free_count  = 0;
    pool_ptr->object_size = object_size;

    MEMPOOL_DEBUG(("%s: Memory Pool %s Create Object Size=%d\n", ESIF_FUNC,
        pool_ptr->name_ptr, pool_ptr->object_size));

    return pool_ptr;
}

/* Memory Pool Destroy */
static ESIF_INLINE void
esif_ccb_mempool_destroy(struct esif_ccb_mempool *pool_ptr)
{
    int remain = 0;
    if (NULL == pool_ptr)
        return;

    remain = pool_ptr->alloc_count - pool_ptr->free_count;
    MEMPOOL_DEBUG(("%s: Memory Pool %s Destroy alloc=%d free=%d remain=%d\n", ESIF_FUNC,
        pool_ptr->name_ptr, pool_ptr->alloc_count, pool_ptr->free_count, remain));

    esif_ccb_free(pool_ptr);
    if(remain != 0) {} // ASSERT();
}

/* Memory Pool Alloc */
static ESIF_INLINE void
*esif_ccb_mempool_alloc(struct esif_ccb_mempool *pool_ptr)
{
    void *mem_ptr = NULL;

    if (NULL == pool_ptr)
        return NULL;

    mem_ptr = esif_ccb_malloc(pool_ptr->object_size);
    pool_ptr->alloc_count++;

    MEMPOOL_DEBUG(("%s: MP Entry Allocated(%d)=%p From Mempool %s\n", ESIF_FUNC,
        pool_ptr->alloc_count, mem_ptr, pool_ptr->name_ptr));
    return mem_ptr;
}

/* Memory Pool ZERO Alloc */
static ESIF_INLINE void
*esif_ccb_mempool_zalloc(struct esif_ccb_mempool *pool_ptr)
{
    void *mem_ptr = NULL;

    if (NULL == pool_ptr)
        return NULL;

    mem_ptr = esif_ccb_malloc(pool_ptr->object_size);
    if (NULL != mem_ptr) esif_ccb_memset(mem_ptr, 0, pool_ptr->object_size);
    pool_ptr->alloc_count++;

    MEMPOOL_DEBUG(("%s: MPZ Entry Allocated(%d)=%p From Mempool %s\n", ESIF_FUNC,
        pool_ptr->alloc_count, mem_ptr, pool_ptr->name));
    return mem_ptr;
}

/* Memory Pool Free */
static ESIF_INLINE void esif_ccb_mempool_free(struct esif_ccb_mempool *pool_ptr, void* mem_ptr)
{
    if (NULL == pool_ptr || NULL == mem_ptr)
        return;

    MEMPOOL_DEBUG(("%s: MP Entree Freed(%d)=%p From Mempool %s\n", ESIF_FUNC,
        pool_ptr->free_count, mem_ptr, pool_ptr->name));

    esif_ccb_free(mem_ptr);
    pool_ptr->free_count++;
}

#endif /* ESIF_ATTR_USER */

#endif /* _ESIF_CCB_MEMPOOL_H_ */