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

#ifndef _ESIF_MEMPOOL_H_
#define _ESIF_MEMPOOL_H_

#include "esif.h"

/* Memory Pools */
enum esif_mempool_type {
    ESIF_MEMPOOL_TYPE_DATA = 0,     /* Data                   */
    ESIF_MEMPOOL_TYPE_DSP,          /* Device Support Package */
    ESIF_MEMPOOL_TYPE_HASH,         /* Hash Table             */
    ESIF_MEMPOOL_TYPE_LIST,         /* Linked List            */
    ESIF_MEMPOOL_TYPE_LIST_NODE,    /* Linked List Node       */
    ESIF_MEMPOOL_TYPE_PM,           /* Participant Manager    */
    ESIF_MEMPOOL_TYPE_QUEUE,        /* Queue                  */
    ESIF_MEMPOOL_TYPE_MAX           /* Max                    */
};

/*
 * Windows Uses a 32 Bit Value To Represent A Tag. Linux allows for a name
 * we acommodate both heree in addition Windows does not have a number authority
 * so we resuse permutations of ESIF to avoid name space collisions.
 * For Windows use Poolmon or ESIF Tools to Monitor
 * For Linux use Slabtop or ESIF tools to Monitor (@ Hutts To Top Of Slabtop)
 * Note you may bave to boot the kernel with slub debug on or Linux will
 * consolidate same size cahches.
 */

/* Driver */
#define ESIF_MEMPOOL_DRIVER_LF        'fise' /* esif Bit 0000 Lower Framework */
#define ESIF_MEMPOOL_DRIVER_ACPI      'Fise' /* esif Bit 0001 ACPI Driver     */
#define ESIF_MEMPOOL_DRIVER_CPU       'fIse' /* esif Bit 0010 CPU Driver      */
#define ESIF_MEMPOOL_DRIVER_PCH       'FIse' /* esif Bit 0011 PCH Driver      */
#define ESIF_MEMPOOL_DRIVER_PLAT      'fiSe' /* esif Bit 0100 PLAT Driver     */
#define ESIF_MEMPOOL_DRIVER_RESERVED1 'FiSe' /* esif Bit 0101 PLAT Driver     */

/* ESIF Framework */
#define ESIF_MEMPOOL_FW_PM            'fisE' /* ESIF Bit 1000 Participant         */
#define ESIF_MEMPOOL_FW_DATA          'FisE' /* ESIF Bit 1001 Data                */
#define ESIF_MEMPOOL_FW_DSP           'FIsE' /* ESIF Bit 1011 Dsp                 */
#define ESIF_MEMPOOL_FW_HASH          'fISe' /* esif Bit 0110 Hash                */
#define ESIF_MEMPOOL_FW_QUEUE         'fiSE' /* ESIF Bit 1100 Queue               */
#define ESIF_MEMPOOL_FW_LIST          'FiSE' /* ESIF Bit 1101 List                */
#define ESIF_MEMPOOL_FW_LIST_NODE     'fISE' /* ESIF Bit 1110 List Node           */
#define ESIF_MEMPOOL_FW_MALLOC        'FISE' /* esif Bit 1111 Generic Malloc Pool */

static ESIF_INLINE char
*esif_mempool_str(u32 pool_tag)
{
    #define ESIF_CREATE_MEMPOOL(mp, mpd) case mp: str = (esif_string) mpd; break;
    esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
    switch(pool_tag) {
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_DRIVER_LF,        "@esif_lf_driver"        )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_DRIVER_ACPI,      "@esif_acpi_driver"      )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_DRIVER_CPU,       "@esif_cpu_driver"       )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_DRIVER_PCH,       "@esif_pch_driver"       )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_DRIVER_PLAT,      "@esif_plat_driver"      )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_DRIVER_RESERVED1, "@esif_rsv1_driver"      )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_FW_HASH,          "@esif_hash_table_cache" )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_FW_PM,            "@esif_pm_cache"         )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_FW_DATA,          "@esif_data_cache"       )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_FW_DSP,           "@esif_dsp_cache"        )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_FW_QUEUE,         "@esif_queue_cache"      )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_FW_LIST,          "@esif_list_cache"       )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_FW_LIST_NODE,     "@esif_list_node_cache"  )
    ESIF_CREATE_MEMPOOL(ESIF_MEMPOOL_FW_MALLOC,        "@esif_malloc_cache"     )
    }
    return str;
}

extern struct esif_ccb_mempool *g_mempool[ESIF_MEMPOOL_TYPE_MAX];

#endif  /* _ESIF_MEMPOOL_H_ */