/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
#ifndef _ESIF_UF_CCB_FILE_H_
#define _ESIF_UF_CCB_FILE_H_

#include "esif_ccb.h"

// FIXME: what's wrong with esif_ccb_string.h?? Cannot include its defined macro!!!
// The problem is that you must include ESIF.H as the first item in a C/CPP file, or
// you can end up with a circular reference in the headers.
#ifdef ESIF_ATTR_OS_WINDOWS
#include "esif_ccb_string.h"
#else
#ifdef ESIF_ATTR_OS_LINUX
#include <errno.h>
#include "esif_ccb_string.h"
#endif
#endif

static ESIF_INLINE int esif_ccb_fopen(FILE **fp, esif_string name, esif_string mode)
{
#ifdef ESIF_ATTR_OS_LINUX
    *fp = fopen(name, mode);
    return (*fp ? 0 : errno);
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
    return (int) fopen_s(fp, name, mode);
#endif
}


#ifdef ESIF_ATTR_OS_LINUX
    #define esif_ccb_fprintf              fprintf
    #define esif_ccb_fread(buf, bsiz, siz, count, fp) fread(buf, siz, count, fp)
    #define esif_ccb_unlink               unlink
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
    #define esif_ccb_fprintf fprintf_s
    #define esif_ccb_fread   fread_s
    #define esif_ccb_unlink  _unlink
#endif

/*
*******************************************************************************
** DIRECTORY FILE SCAN
*******************************************************************************
*/

#ifdef ESIF_ATTR_OS_LINUX
    #include <dirent.h>
    #include <fnmatch.h>
    #define MAX_PATH 260
    #define INVALID_HANDLE_VALUE NULL
    typedef DIR* esif_ccb_file_find_handle;
#else
    typedef HANDLE esif_ccb_file_find_handle;
#endif

/* CCB File Abstraction */
struct esif_ccb_file {
    char filename[MAX_PATH];
};

static ESIF_INLINE esif_ccb_file_find_handle esif_ccb_file_enum_first(esif_string path,
                                                                      esif_string pattern,
                                                                      struct esif_ccb_file *file)
{
    esif_ccb_file_find_handle find_handle = INVALID_HANDLE_VALUE;
    char full_path[MAX_PATH];

#ifdef ESIF_ATTR_OS_LINUX

    struct dirent *ffd;

    esif_ccb_sprintf(MAX_PATH, full_path, "%s", path);
    //ESIF_TRACE("%s: SCAN File System For Files Path = %s, Pattern = %s", ESIF_FUNC, full_path, pattern);

    find_handle = opendir(full_path);
    if (INVALID_HANDLE_VALUE == find_handle) {
        goto exit;
    }

    do {
        ffd = readdir(find_handle);
        if (NULL != ffd && fnmatch(pattern, ffd->d_name, FNM_PATHNAME|FNM_NOESCAPE)==0) { // found a match
            esif_ccb_strcpy(file->filename, ffd->d_name, MAX_PATH);
            goto exit;
        }
    } while (NULL != ffd);
    closedir(find_handle);
    find_handle = INVALID_HANDLE_VALUE; // no matches

#else

    WIN32_FIND_DATAA ffd;
    esif_ccb_sprintf(MAX_PATH, full_path, "%s%s%s", path, ESIF_PATH_SEP, pattern);
    //ESIF_TRACE("%s: SCAN File System For Files Path = %s Pattern = %s", ESIF_FUNC, full_path);

    find_handle = FindFirstFileA(full_path, &ffd);
    if (INVALID_HANDLE_VALUE == find_handle) {
        goto exit;
    }
    esif_ccb_strcpy(file->filename, ffd.cFileName, MAX_PATH);

#endif

exit:

    return find_handle;
}

static ESIF_INLINE struct esif_ccb_file *esif_ccb_file_enum_next(esif_ccb_file_find_handle find_handle,
                                                                 esif_string pattern,
                                                                 struct esif_ccb_file *file)
{
#ifdef ESIF_ATTR_OS_LINUX
    struct dirent *ffd;
    do {
        ffd = readdir(find_handle);
        if (NULL != ffd && fnmatch(pattern, ffd->d_name, FNM_PATHNAME|FNM_NOESCAPE)==0) { // found a match
            esif_ccb_strcpy(file->filename, ffd->d_name, MAX_PATH);
            return file;
        } 
    } while (NULL != ffd);
    file = NULL; // no more matches
#else
    WIN32_FIND_DATAA ffd;

    UNREFERENCED_PARAMETER(pattern);

    if(0 == FindNextFileA(find_handle, &ffd)) return NULL;
    esif_ccb_strcpy(file->filename, ffd.cFileName, MAX_PATH);
#endif

    return file;
}

static ESIF_INLINE void esif_ccb_file_enum_close(esif_ccb_file_find_handle file_handle)
{
#ifdef ESIF_ATTR_OS_LINUX
    closedir(file_handle);
#else
    FindClose(file_handle);
#endif
}

#endif /* _ESIF_UF_CCB_FILE_H_ */
