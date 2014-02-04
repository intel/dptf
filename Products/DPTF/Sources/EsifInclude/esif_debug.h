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

#ifndef _ESIF_DEBUG_H_
#define _ESIF_DEBUG_H_

#define NO_ESIF_DEBUG(fmt)

/*******************************************************************************
** Kernel
*******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

    /* Linux */
    #ifdef ESIF_ATTR_OS_LINUX
        #define LINUX_LOG_INFO(format, arg...)  \
            printk(KERN_INFO  "=INF= " format, ## arg)
        #define LINUX_LOG_ERROR(format, arg...) \
            printk(KERN_ERR   "!ERR! " format, ## arg)
        #define LINUX_LOG_DEBUG(format, arg...) \
            printk(KERN_DEBUG "~DBG~ " format, ## arg)

        #define ESIF_INFO(fmt)      LINUX_LOG_INFO  fmt
        #define ESIF_ERROR(fmt)     LINUX_LOG_ERROR fmt
        #define ESIF_DEBUG(fmt)     LINUX_LOG_DEBUG fmt
        #define ESIF_DYN_DEBUG(fmt) LINUX_LOG_DEBUG fmt
    #endif /* ESIF_ATTR_OS_LINUX */

    /* Windows */
    #ifdef ESIF_ATTR_OS_WINDOWS
        #define WINDOWS_LOG_INFO(fmt, ...) \
            DbgPrint("=INF= " fmt, __VA_ARGS__)
        #define WINDOWS_LOG_ERROR(fmt, ...) \
            DbgPrint("!ERR! " fmt, __VA_ARGS__)
        #define WINDOWS_LOG_DEBUG(fmt, ...) \
            DbgPrint("~DBG~ " fmt, __VA_ARGS__)

        #define ESIF_INFO(fmt)      WINDOWS_LOG_INFO  fmt
        #define ESIF_ERROR(fmt)     WINDOWS_LOG_ERROR fmt
        #define ESIF_DEBUG(fmt)     WINDOWS_LOG_DEBUG fmt
        #define ESIF_DYN_DEBUG(fmt) WINDOWS_LOG_DEBUG fmt
    #endif /* ESIF_ATTR_OS_WINDOWS */

    #define ESIF_DEBUG_LEVEL_ON(module, module_level) \
        (((1 << (module)) & esif_module_mask) != 0 && \
        ((1 << (module_level)) & esif_module_level_mask[(module)]) != 0)

    #define ESIF_LOG_DEBUG_LEVEL(module, module_level, format) \
        if (ESIF_DEBUG_LEVEL_ON(module, module_level)) \
            ESIF_DYN_DEBUG(format)

    #define ESIF_DEBUG_MOD_MAX 32

    extern u32 esif_module_mask;
    extern u32 esif_module_level_mask[ESIF_DEBUG_MOD_MAX];

    /* Sets the mask of modules with debugging on */
    void esif_debug_set_modules(u32 module_mask);

    /* Gets the mask of modules with debugging on */
    void esif_debug_get_modules(u32* module_mask_ptr);

    /* Sets the debugging level mask of a particular module */
    void esif_debug_set_module_level(u32 module, u32 moduleLevelMask);

    /* Gets the debugging level mask of a particular module */
    void esif_debug_get_module_level(u32 module, u32* module_level_mask_ptr);

#else /* ESIF_ATTR_KERNEL */

/*******************************************************************************
** User
*******************************************************************************/

    /* Linux */
    #ifdef ESIF_ATTR_OS_LINUX
        #define LINUX_LOG_INFO(format, arg...) \
            printf("=INF= " format, ## arg)
        #define LINUX_LOG_ERROR(format, arg...) \
            printf("!ERR! " format, ## arg)
        #define LINUX_LOG_DEBUG(format, arg...) \
            printf("~DBG~ " format, ## arg)

        #define ESIF_INFO(fmt)      LINUX_LOG_INFO fmt
        #define ESIF_ERROR(fmt)     LINUX_LOG_ERROR fmt
        #define ESIF_DEBUG(fmt)     LINUX_LOG_DEBUG fmt
        #define ESIF_DYN_DEBUG(fmt) LINUX_LOG_DEBUG fmt
    #endif /* ESIF_ATTR_OS_LINUX */

    /* Windows */
    #ifdef ESIF_ATTR_OS_WINDOWS

        #define WINDOWS_LOG_INFO(fmt, ...)  \
            printf("=INF= " fmt, __VA_ARGS__)
        #define WINDOWS_LOG_ERROR(fmt, ...) \
            printf("!ERR! " fmt, __VA_ARGS__)
        #define WINDOWS_LOG_DEBUG(fmt, ...) \
            printf("~DBG~ " fmt, __VA_ARGS__)

        #define ESIF_INFO(fmt)      WINDOWS_LOG_INFO  fmt
        #define ESIF_ERROR(fmt)     WINDOWS_LOG_ERROR fmt
        #define ESIF_DEBUG(fmt)     WINDOWS_LOG_DEBUG fmt
        #define ESIF_DYN_DEBUG(fmt) WINDOWS_LOG_DEBUG fmt
    #endif /* ESIF_ATTR_OS_WINDOWS */

#endif /* ESIF_ATTR_KERNEL */


enum esif_debug_mod {
    ESIF_DEBUG_MOD_ACTION_VAR      = 0,     /* Action Variable        */
    ESIF_DEBUG_MOD_ACTION_CONST    = 1,     /* Action Constant        */
    ESIF_DEBUG_MOD_ACTION_MSR      = 2,     /* Action MSR             */
    ESIF_DEBUG_MOD_ACTION_MMIO     = 3,     /* Action MMIO            */
    ESIF_DEBUG_MOD_ACTION_ACPI     = 4,     /* Action ACPI            */
    ESIF_DEBUG_MOD_IPC             = 5,     /* IPC                    */
    ESIF_DEBUG_MOD_COMMAND         = 6,     /* Command Pre DSP        */
    ESIF_DEBUG_MOD_PRIMITIVE       = 7,     /* Primitive Requires DSP */
    ESIF_DEBUG_MOD_ACTION          = 8,     /* Primitive Requires DSP */
    ESIF_DEBUG_MOD_CPC             = 9,     /* Loads DSP              */
    ESIF_DEBUG_MOD_DATA            = 10,    /* Data Operations        */
    ESIF_DEBUG_MOD_DSP             = 11,    /* DSP Operations         */
    ESIF_DEBUG_MOD_EVENT           = 12,    /* Event Processing       */
    ESIF_DEBUG_MOD_ELF             = 13,    /* ESIF Lower Framework   */
    ESIF_DEBUG_MOD_PMG             = 14,    /* Package Manager        */
    ESIF_DEBUG_MOD_QUEUE           = 15,    /* Queue Manager          */
    ESIF_DEBUG_MOD_HASH            = 16,    /* Hash Tables            */
    ESIF_DEBUG_MOD_ACTION_SYSTEMIO = 17,    /* Action SYSTEM IO       */
    ESIF_DEBUG_MOD_ACTION_CODE     = 18,    /* Action Code            */
    ESIF_DEBUG_MOD_POL             = 19,    /* Polling Code           */
    ESIF_DEBUG_MOD_ACTION_MBI      = 20,    /* Action MBI (ATOM)      */
};

static ESIF_INLINE char
*esif_debug_mod_str(enum esif_debug_mod mod)
{
    #define ESIF_CREATE_MOD(mod,ab) case mod: str = (esif_string) #ab; break;
    esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
    switch(mod) {
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_VAR,      VAR)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_CONST,    CON)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_MSR,      MSR)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_MMIO,     MMI)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_ACPI,     ACP)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_IPC,             IPC)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_COMMAND,         CMD)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_PRIMITIVE,       PRI)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION,          ACT)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_CPC,             CPC)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_DATA,            DAT)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_DSP,             DSP)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_EVENT,           EVE)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ELF,             ELF)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_PMG,             PMG)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_QUEUE,           QUE)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_HASH,            HSH)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_SYSTEMIO, SIO)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_CODE,     COD)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_POL,             POL)
    ESIF_CREATE_MOD(ESIF_DEBUG_MOD_ACTION_MBI,      MBI)
    }
    return str;
}

struct esif_memory_stats
{
    u32 allocs;                             /* Total number of allocations */
    u32 frees;                              /* Total number of frees       */
    u32 memPoolAllocs;                      /* Total number of allocations */
    u32 memPoolFrees;                       /* Total number of frees       */
    u32 memPoolObjAllocs;                   /* Total number of allocations */
    u32 memPoolObjFrees;                    /* Total number of frees       */
    u32 memTypeAllocs;                      /* Total number of allocations */
    u32 memTypeFrees;                       /* Total number of frees       */
    u32 memTypeObjAllocs;                   /* Total number of allocations */
    u32 memTypeObjFrees;                    /* Total number of frees       */
};
extern struct esif_memory_stats g_memstat;
extern int background;

#endif // _ESIF_DEBUG_H_