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

#ifndef _ESIF_MEMTYPE_H_
#define _ESIF_MEMTYPE_H_

#include "esif.h"
#include "esif_ccb_memory.h"

/* Variable Length Common Memory Types */
enum esif_memtype_type {
    ESIF_MEMTYPE_TYPE_EVENT = 0,            /* Event */
    ESIF_MEMTYPE_TYPE_IPC,                  /* IPC   */
    ESIF_MEMTYPE_TYPE_MAX
};

static ESIF_INLINE char
*esif_memtype_str(u32 type)
{
    #define ESIF_CREATE_MEMTYPE(mp, mpd) case mp: str = (esif_string) mpd; break;
    esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
    switch(type) {
    ESIF_CREATE_MEMTYPE(ESIF_MEMTYPE_TYPE_EVENT, "esif_event_dynamic" )
    ESIF_CREATE_MEMTYPE(ESIF_MEMTYPE_TYPE_IPC,   "esif_ipc_dynamic"   )
    }
    return str;
}

/* Memory Type Not Fixed Size Use Mempool For That */
struct esif_ccb_memtype {
    esif_string name_ptr;                   /* Name             */
    u32         type_tag;                   /* Type Of Memory   */
    u32         alloc_count;                /* Allocation Count */
    u32         free_count;                 /* Free Count       */
};

/* Memory Type Create */
static ESIF_INLINE struct esif_ccb_memtype
*esif_ccb_memtype_create(u32  type_tag)
{
    struct esif_ccb_memtype *type_ptr = (struct esif_ccb_memtype*) esif_ccb_malloc(sizeof(struct esif_ccb_memtype));
    if (NULL == type_ptr)
        return NULL;

    type_ptr->type_tag    = type_tag;
    type_ptr->name_ptr    = esif_memtype_str(type_tag);
    type_ptr->alloc_count = 0;
    type_ptr->free_count  = 0;

    g_memstat.memTypeAllocs++;

    MEMTYPE_DEBUG(("%s: Memory Type %s Create\n", ESIF_FUNC, type_ptr->name_ptr));
    return type_ptr;
}

/* Memory Type Destroy */
static ESIF_INLINE
void esif_ccb_memtype_destroy(struct esif_ccb_memtype *type_ptr)
{
    u32 remain = 0;
    if (NULL == type_ptr)
        return;

    remain = type_ptr->alloc_count - type_ptr->free_count;
    MEMPOOL_DEBUG(("%s: Memory Type %s Destroy alloc=%d free=%d remain=%d\n", ESIF_FUNC,
        type_ptr->name_ptr, type_ptr->alloc_count, type_ptr->free_count, remain));

    g_memstat.memTypeFrees++;

    esif_ccb_free(type_ptr);
    if (remain != 0) {} // ASSERT();
}

/* Memory Type Allocate */
static ESIF_INLINE void
*esif_ccb_memtype_alloc(struct esif_ccb_memtype *type_ptr, u32 size)
{
    void *mem_ptr = esif_ccb_malloc(size);
    if (NULL == type_ptr || NULL == mem_ptr)
        return NULL;

    type_ptr->alloc_count++;
    g_memstat.memTypeObjAllocs++;

    MEMPOOL_DEBUG(("%s: MT Entry Allocated(%d)=%p From Memtype %s\n", ESIF_FUNC,
        type_ptr->alloc_count, mem_ptr, type_ptr->name_ptr));
    return mem_ptr;
}

/* Memory Type ZERO Allocate */
static ESIF_INLINE void
*esif_ccb_memtype_zalloc(struct esif_ccb_memtype *type_ptr, u32 size)
{
    void *mem_ptr = esif_ccb_memtype_alloc(type_ptr, size);
    if (NULL == mem_ptr)
        return NULL;

    esif_ccb_memset(mem_ptr, 0, size);
    MEMPOOL_DEBUG(("%s: MT ZERO(%d)=%p For Memtype %s\n", ESIF_FUNC,
            type_ptr->alloc_count, mem_ptr, type_ptr->name_ptr));

    return mem_ptr;
}

/* Memory Type Free */
static ESIF_INLINE void
esif_ccb_memtype_free(struct esif_ccb_memtype *type_ptr, void* mem_ptr)
{
    if(NULL == type_ptr || NULL == mem_ptr)
        return;

    type_ptr->free_count++;
    MEMPOOL_DEBUG(("%s: MT Entree Freed(%d)=%p From Memtype %s\n", ESIF_FUNC,
            type_ptr->free_count, mem_ptr, type_ptr->name_ptr));

    esif_ccb_free(mem_ptr);
    g_memstat.memTypeObjFrees++;
}

/* Implemented in esif_lf.c */
extern struct esif_ccb_memtype *g_memtype[ESIF_MEMTYPE_TYPE_MAX];

#endif  /* _ESIF_MEMTYPE_H_ */