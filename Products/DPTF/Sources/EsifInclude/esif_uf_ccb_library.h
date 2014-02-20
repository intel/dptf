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

#ifndef _ESIF_CCB_LIBRARY_H_
#define _ESIF_CCB_LIBRARY_H_

#include "esif.h"

#ifdef ESIF_ATTR_USER

#ifdef ESIF_ATTR_OS_WINDOWS
typedef HINSTANCE esif_lib_t;
#define ESIF_LIB_EXT "dll"
#else
#include <dlfcn.h>
typedef void *esif_lib_t;
#define ESIF_LIB_EXT "so"
#endif

/* Load Shared .so/DLL code opaque */
static ESIF_INLINE esif_lib_t esif_ccb_library_load(esif_string lib_name)
{
#ifdef ESIF_ATTR_OS_WINDOWS
    return LoadLibraryA(lib_name);
#else
    esif_lib_t handle = dlopen(lib_name, RTLD_NOW);

    /* Try different case-sensitive versions of lib_name */
    if (NULL == handle && lib_name[0] != 0) {
        char library[256]={0};
        int j=0;

        /* lowercase.so */
        for (j=0; j < sizeof(library)-1 && lib_name[j]; j++) {
            library[j] = tolower(lib_name[j]);
        }
        handle = dlopen(library, RTLD_NOW);

        /* Capitalized.so */
        if (NULL == handle) {
            library[0] = toupper(library[0]);
            handle = dlopen(library, RTLD_NOW);
        }

        /* UPPERCASE.so */
        if (NULL == handle) {
            for (j=0; library[j] && library[j] != '.'; j++) {
                library[j] = toupper(library[j]);
            }
            handle = dlopen(library, RTLD_NOW);
        }
    }
    return handle;

#endif
}

/* Find address for symbol */
#ifdef ESIF_ATTR_OS_WINDOWS
static ESIF_INLINE FARPROC esif_ccb_library_get_func(
    esif_lib_t lib,
    esif_string func_name
)
#else
static ESIF_INLINE void *esif_ccb_library_get_func(esif_lib_t lib, esif_string func_name)
#endif
{
    if (NULL == lib)
        return NULL;
#ifdef ESIF_ATTR_OS_WINDOWS
    return GetProcAddress(lib, func_name);
#else
    return dlsym(lib, func_name);
#endif
}

/* Close/Free previously opened library */
static ESIF_INLINE void esif_ccb_library_unload(esif_lib_t lib)
{
    if (NULL == lib)
        return;
#ifdef ESIF_ATTR_OS_WINDOWS
    FreeLibrary(lib);
#else
    dlclose(lib);
#endif
}

#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_CCB_LIBRARY_H_ */