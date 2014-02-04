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

#include "esif.h"

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

static ESIF_INLINE int esif_ccb_fopen(
	FILE **fp,
	esif_string name,
	esif_string mode
	)
{
#ifdef ESIF_ATTR_OS_LINUX
	*fp = fopen(name, mode);
	return *fp ? 0 : errno;

#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	return (int)fopen_s(fp, name, mode);

#endif
}


#ifdef ESIF_ATTR_OS_LINUX
	#define esif_ccb_fgets(buf, siz, fp)                fgets(buf, siz, fp)
	#define esif_ccb_fread(buf, bsiz, siz, count, fp) fread(buf, siz, count, fp)
	#define esif_ccb_fwrite(buf, siz, count, fp)        fwrite(buf, siz, count, fp)
	#define esif_ccb_fprintf(fp, fmt, ...)              fprintf(fp, fmt, ##__VA_ARGS__)
	#define esif_ccb_vfprintf(fp, fmt, vargs)           vfprintf(fp, fmt, vargs)
	#define esif_ccb_fseek(fp, off, org)                fseek(fp, off, org)
	#define esif_ccb_ftell(fp)                          ftell(fp)
	#define esif_ccb_fclose(fp)                         fclose(fp)
	#define esif_ccb_unlink(fname)                      unlink(fname)
	#define esif_ccb_rename(oldname, newname)           rename(oldname, newname)
#endif
#ifdef ESIF_ATTR_OS_WINDOWS
	#define esif_ccb_fgets(buf, siz, fp)                fgets(buf, siz, fp)
	#define esif_ccb_fread(buf, bsiz, siz, count, fp)   fread_s(buf, bsiz, siz, count, fp)
	#define esif_ccb_fwrite(buf, siz, count, fp)        fwrite(buf, siz, count, fp)
	#define esif_ccb_fprintf(fp, fmt, ...)              fprintf_s(fp, fmt, ##__VA_ARGS__)
	#define esif_ccb_vfprintf(fp, fmt, vargs)           vfprintf_s(fp, fmt, vargs)
	#define esif_ccb_fseek(fp, off, org)                fseek(fp, off, org)
	#define esif_ccb_ftell(fp)                          ftell(fp)
	#define esif_ccb_fclose(fp)                         fclose(fp)
	#define esif_ccb_unlink(fname)                      _unlink(fname)
	#define esif_ccb_rename(oldname, newname)           rename(oldname, newname)
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

struct esif_ccb_file_find {
	DIR   *handle;
	int   matches;
	char  **files;
};

typedef struct esif_ccb_file_find *esif_ccb_file_find_handle;

// qsort() callback to do case-insenstive sort for an array of null-terminated strings
static ESIF_INLINE int esif_ccb_file_find_qsort(
	const void *arg1,
	const void *arg2
	)
{
	return esif_ccb_stricmp(*(char **)arg1, *(char **)arg2);
}


#else
typedef HANDLE esif_ccb_file_find_handle;
#endif

/* CCB File Abstraction */
struct esif_ccb_file {
	char  filename[MAX_PATH];
};

static ESIF_INLINE esif_ccb_file_find_handle esif_ccb_file_enum_first(
	esif_string path,
	esif_string pattern,
	struct esif_ccb_file *file
	)
{
	esif_ccb_file_find_handle find_handle = INVALID_HANDLE_VALUE;
	char full_path[MAX_PATH];

#ifdef ESIF_ATTR_OS_LINUX
	struct dirent *ffd;

	esif_ccb_sprintf(MAX_PATH, full_path, "%s", path);

	find_handle = (esif_ccb_file_find_handle)esif_ccb_malloc(sizeof(struct esif_ccb_file_find));

	find_handle->handle = opendir(full_path);
	if (INVALID_HANDLE_VALUE == find_handle->handle) {
		esif_ccb_free(find_handle);
		find_handle = INVALID_HANDLE_VALUE;
		goto exit;
	}

	// Since readdir does not return sorted results, put all matches in an array and sort before returning the first match
	do {
		ffd = readdir(find_handle->handle);
		if (NULL != ffd && fnmatch(pattern, ffd->d_name, FNM_PATHNAME | FNM_NOESCAPE) == 0) {	// found a match
			find_handle->matches++;
			find_handle->files = (char **)esif_ccb_realloc(find_handle->files, sizeof(char *) * find_handle->matches);
			find_handle->files[find_handle->matches - 1] = esif_ccb_strdup(ffd->d_name);
		}
	} while (NULL != ffd);

	// If any matches, sort the results and return the first match, freeing it from the result array
	if (find_handle->matches) {
		qsort(find_handle->files, find_handle->matches, sizeof(char *), esif_ccb_file_find_qsort);
		esif_ccb_strcpy(file->filename, find_handle->files[0], MAX_PATH);
		esif_ccb_free(find_handle->files[0]);
		find_handle->files[0] = NULL;
		goto exit;
	}
	closedir(find_handle->handle);
	esif_ccb_free(find_handle);
	find_handle = INVALID_HANDLE_VALUE;	// no matches

#else
	WIN32_FIND_DATAA ffd;
	esif_ccb_sprintf(MAX_PATH, full_path, "%s%s%s", path, ESIF_PATH_SEP, pattern);

	find_handle = FindFirstFileA(full_path, &ffd);
	if (INVALID_HANDLE_VALUE == find_handle) {
		goto exit;
	}
	esif_ccb_strcpy(file->filename, ffd.cFileName, MAX_PATH);
#endif

exit:

	return find_handle;
}


static ESIF_INLINE struct esif_ccb_file *esif_ccb_file_enum_next(
	esif_ccb_file_find_handle find_handle,
	esif_string pattern,
	struct esif_ccb_file *file
	)
{
#ifdef ESIF_ATTR_OS_LINUX
	u32 idx;
	// Find the next match in the sorted list, freeing it from the result array
	for (idx = 0; idx < find_handle->matches; idx++) {
		if (find_handle->files[idx] == NULL) {
			continue;
		}
		esif_ccb_strcpy(file->filename, find_handle->files[idx], MAX_PATH);
		esif_ccb_free(find_handle->files[idx]);
		find_handle->files[idx] = NULL;
		return file;
	}
	file = NULL;// no more matches
#else
	WIN32_FIND_DATAA ffd;

	UNREFERENCED_PARAMETER(pattern);

	if (0 == FindNextFileA(find_handle, &ffd)) {
		return NULL;
	}
	esif_ccb_strcpy(file->filename, ffd.cFileName, MAX_PATH);
#endif
	return file;
}

static ESIF_INLINE void esif_ccb_file_enum_close(esif_ccb_file_find_handle find_handle)
{
#ifdef ESIF_ATTR_OS_LINUX
	u32 idx;
	closedir(find_handle->handle);
	for (idx = 0; idx < find_handle->matches; idx++) {
		esif_ccb_free(find_handle->files[idx]);
	}
	esif_ccb_free(find_handle->files);
	esif_ccb_memset(find_handle, 0, sizeof(struct esif_ccb_file_find));
	esif_ccb_free(find_handle);
#else
	FindClose(find_handle);
#endif
}


static ESIF_INLINE int esif_ccb_file_exists(esif_string filename)
{
	struct stat st;
	return 0 == esif_ccb_stat(filename, &st);
}


#endif /* _ESIF_UF_CCB_FILE_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
