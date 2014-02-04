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

#ifndef _ESIF_CCB_MEMORY_H_
#define _ESIF_CCB_MEMORY_H_

#include "esif.h"
#include "esif_ccb.h"

/* Memory Copy */
static ESIF_INLINE void
    esif_ccb_memcpy(void *dest_ptr, const void *src_ptr, size_t size)
{
    MEMORY_DEBUG(("%s: dest = %p src = %p size = %u\n", ESIF_FUNC, dest_ptr, src_ptr, size));
#ifdef ESIF_ATTR_KERNEL
#ifdef ESIF_ATTR_OS_WINDOWS
    memcpy_s(dest_ptr, size, src_ptr, size);
#else
    memcpy(dest_ptr, src_ptr, size);
#endif
#endif

#ifdef ESIF_ATTR_USER
#ifdef ESIF_ATTR_OS_WINDOWS
    memcpy_s(dest_ptr, size, src_ptr, size);
#else
    memcpy(dest_ptr, src_ptr, size);
#endif
#endif
}

/* Memory Move */
static ESIF_INLINE void
    esif_ccb_memmove(void *dest_ptr, const void *src_ptr, size_t size)
{
    MEMORY_DEBUG(("%s: dest = %p src = %p size = %u\n", ESIF_FUNC, dest_ptr, src_ptr, size));
#ifdef ESIF_ATTR_KERNEL
#ifdef ESIF_ATTR_OS_WINDOWS
    memmove_s(dest_ptr, size, src_ptr, size);
#else
    memmove(dest_ptr, src_ptr, size);
#endif
#endif

#ifdef ESIF_ATTR_USER
#ifdef ESIF_ATTR_OS_WINDOWS
    memmove_s(dest_ptr, size, src_ptr, size);
#else
    memmove(dest_ptr, src_ptr, size);
#endif
#endif
}

/* Memory Set */
static ESIF_INLINE void
*esif_ccb_memset(void *s_ptr, int c, size_t count)
{
    MEMORY_DEBUG(("%s: dest = %p value = %02x size = %u\n", ESIF_FUNC, s_ptr, c, count));
    return memset(s_ptr, c, count);
}

/* Memory Allocate */
static ESIF_INLINE void
*esif_ccb_malloc(size_t size)
{
    void* mem_ptr = NULL;

#ifdef ESIF_ATTR_KERNEL
    /* Linux */
    #ifdef ESIF_ATTR_OS_LINUX
        mem_ptr = kzalloc(size, GFP_ATOMIC);
    #endif
    /* Windows */
    #ifdef ESIF_ATTR_OS_WINDOWS
        mem_ptr = ExAllocatePoolWithTag(NonPagedPool, size, ESIF_MEMPOOL_FW_MALLOC);
        if (NULL != mem_ptr) esif_ccb_memset(mem_ptr, 0, size);
    #endif
    #ifndef ESIF_ATTR_OS_LINUX_DRIVER
        g_memstat.allocs++;
    #endif
#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER
    mem_ptr = malloc(size);
    if (NULL != mem_ptr) {
    esif_ccb_memset(mem_ptr, 0, size);
    #ifdef ESIF_TRACE_MALLOC
    atomic_inc(&g_user_memstats.allocs);
    #endif
    }
#endif /* ESIF_ATTR_USER */

    MEMORY_DEBUG(("%s: buf %p size = %u\n", ESIF_FUNC, mem_ptr, size));
    return mem_ptr;
}

/* Memory Free */
static ESIF_INLINE
void esif_ccb_free(void *mem_ptr)
{
    MEMORY_DEBUG(("%s: buf %p\n", ESIF_FUNC, mem_ptr));

#ifdef ESIF_ATTR_KERNEL
    /* Linux */
    #ifdef ESIF_ATTR_OS_LINUX
        kfree(mem_ptr);
    #endif

    /* Windows */
    #ifdef ESIF_ATTR_OS_WINDOWS
        ExFreePoolWithTag(mem_ptr, ESIF_MEMPOOL_FW_MALLOC);
        mem_ptr = NULL;
    #endif

    #ifndef ESIF_ATTR_OS_LINUX_DRIVER
        g_memstat.frees++;
    #endif
#endif /* ESIF_ATTR_KERNEL */

#ifdef ESIF_ATTR_USER
    free(mem_ptr);
    #ifdef ESIF_TRACE_MALLOC
        if (NULL != mem_ptr) atomic_inc(&g_user_memstats.frees);
    #endif
#endif /* ESIF_ATTR_USER */
}

/* Memory Reallocate */
static ESIF_INLINE 
    void *esif_ccb_realloc(void *mem_ptr, size_t new_size)
{
#ifdef ESIF_TRACE_MALLOC
#ifdef ESIF_ATTR_USER
    void* old_ptr = mem_ptr;
#endif
#endif
    MEMORY_DEBUG(("%s: buf %p, new_size=%lld\n", ESIF_FUNC, mem_ptr, (long long)new_size));
#ifdef ESIF_ATTR_KERNEL
    UNREFERENCED_PARAMETER(mem_ptr);
    UNREFERENCED_PARAMETER(new_size);
    return 0; /* Not Implemented in Kernel space */
#endif /* ESIF_ATTR_KERNEL */
#ifdef ESIF_ATTR_USER
    mem_ptr = realloc(mem_ptr, new_size);
#ifdef ESIF_TRACE_MALLOC
    if (NULL == old_ptr && NULL != mem_ptr) atomic_inc(&g_user_memstats.allocs);
#endif
    return mem_ptr;
#endif
}

/* Memory Dump */
static ESIF_INLINE
void esif_ccb_mem_dump(const u8 *ch, const u8 *what, const int size)
{
    int i;
    UNREFERENCED_PARAMETER(what);
    MEMORY_DEBUG(("%s: Dumping Memory for %s\n", ESIF_FUNC, what));
    for (i=0; i < size ; i++, ch++)
        MEMORY_DEBUG(("<%p> 0x%02X\n", (u8 *)ch, (u8)*ch));
    MEMORY_DEBUG(("End of Memory Dump\n"));
}

#endif /* _ESIF_CCB_MEMORY_H_ */