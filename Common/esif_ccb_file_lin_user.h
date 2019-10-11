/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#pragma once

#if defined(ESIF_ATTR_OS_LINUX) && defined(ESIF_ATTR_USER)

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "esif_ccb_string.h"

/*
 * File Management
 */

 // mode parameters for esif_ccb_fopen() that can be combined
#define FILEMODE_READ		"r"		// Open for Read-only; File must exist
#define FILEMODE_WRITE		"w"		// Open for Write, Overwrite existing file; Create if does not exist
#define FILEMODE_APPEND		"a"		// Open for Write, Append existing file; Create if does not exist
#define FILEMODE_UPDATE		"+"		// Open for Update; Combine with "r", "w", or "a"
#define FILEMODE_BINARY		"b"		// Open in Binary mode
#define FILEMODE_TEXT		"t"		// Open in Text mode (Same as Binary for Linux)
#define FILEMODE_COMMIT		""		// N/A in Linux

static ESIF_INLINE FILE *esif_ccb_fopen(
	esif_string name,
	esif_string mode,
	int *errnoPtr
	)
{
	FILE *fp = fopen(name, mode);
	if (errnoPtr) {
		*errnoPtr = (fp ? 0 : errno);
	}
	return fp;
}

static ESIF_INLINE int esif_ccb_fclose(FILE *fp)
{
	// Commit writable files to disk; No effect for read-only files.
	fflush(fp);
	fsync(fileno(fp));
	return fclose(fp);
}

/* NOTE: SCANFBUF is mandatory when using esif_ccb_fscanf to scan into strings:
*        esif_ccb_fscanf(fp, "%s=%d", SCANFBUF(name, sizeof(name)), &value);
*/
#define esif_ccb_fgets(buf, siz, fp)                fgets(buf, siz, fp)
#define esif_ccb_fread(buf, bsiz, siz, count, fp)   fread(buf, siz, count, fp)
#define esif_ccb_fwrite(buf, siz, count, fp)        fwrite(buf, siz, count, fp)
#define esif_ccb_fprintf(fp, fmt, ...)              fprintf(fp, fmt, ##__VA_ARGS__)
#define esif_ccb_fscanf(fp, fmt, ...)				fscanf(fp, fmt, ##__VA_ARGS__)
#define esif_ccb_vfprintf(fp, fmt, vargs)           vfprintf(fp, fmt, vargs)
#define esif_ccb_fseek(fp, off, org)                fseek(fp, off, org)
#define esif_ccb_ftell(fp)                          ftell(fp)
#define esif_ccb_fflush(fp)                         fflush(fp)
#define esif_ccb_unlink(fname)                      unlink(fname)
#define esif_ccb_rename(oldname, newname)           rename(oldname, newname)
#define esif_ccb_mkdir(dir)                         mkdir(dir, 755)
#define esif_ccb_stat(path, buf)                    stat(path, buf)

/* Recursively create a path if it does not already exist */
static ESIF_INLINE int esif_ccb_makepath(char *path)
{
	int rc = -1;
	struct stat st = { 0 };

	// create path only if it does not already exist
	if ((rc = esif_ccb_stat(path, &st)) != 0) {
		size_t len = esif_ccb_strlen(path, MAX_PATH);
		char dir[MAX_PATH];
		char *slash;

		// trim any trailing slash
		esif_ccb_strcpy(dir, path, MAX_PATH);
		if (len > 0 && dir[len - 1] == *ESIF_PATH_SEP) {
			dir[len - 1] = 0;
		}

		// if path doesn't exist and can't be created, recursively create parent folder(s)
		if ((rc = esif_ccb_stat(dir, &st)) != 0 && (rc = esif_ccb_mkdir(dir)) != 0) {
			if ((slash = esif_ccb_strrchr(dir, *ESIF_PATH_SEP)) != 0) {
				*slash = 0;
				if ((rc = esif_ccb_makepath(dir)) == 0) {
					*slash = *ESIF_PATH_SEP;
					rc = esif_ccb_mkdir(dir);
				}
			}
		}
	}
	return rc;
}

/*
 * File Enumeration
 */

#include <dirent.h>
#include <fnmatch.h>
#include "esif_ccb_memory.h"
#include "esif_ccb_sort.h"

#pragma pack(push,1)
struct esif_ccb_file_enum {
	DIR    *handle;
	size_t matches;
	char   **files;
};
#pragma pack(pop)

typedef struct esif_ccb_file_enum *esif_ccb_file_enum_t;
#define ESIF_INVALID_FILE_ENUM_HANDLE	NULL

/* CCB File Abstraction */
#pragma pack(push,1)
struct esif_ccb_file {
	char  filename[MAX_PATH];
};
#pragma pack(pop)

// Android C compiler requires forward declaration when it is called by esif_ccb_file_enum_first()
static ESIF_INLINE void esif_ccb_file_enum_close(esif_ccb_file_enum_t find_handle);

static ESIF_INLINE esif_ccb_file_enum_t esif_ccb_file_enum_first(
	esif_string path,
	esif_string pattern,
	struct esif_ccb_file *file
	)
{
	esif_ccb_file_enum_t find_handle = ESIF_INVALID_FILE_ENUM_HANDLE;
	char full_path[MAX_PATH] = {0};

	struct dirent *ffd;

	esif_ccb_sprintf(MAX_PATH, full_path, "%s", path);

	find_handle = (esif_ccb_file_enum_t)esif_ccb_malloc(sizeof(struct esif_ccb_file_enum));
	if (NULL == find_handle) {
		find_handle = ESIF_INVALID_FILE_ENUM_HANDLE;
		goto exit;
	}
	find_handle->handle = opendir(full_path);
	if (ESIF_INVALID_FILE_ENUM_HANDLE == find_handle->handle) {
		esif_ccb_free(find_handle);
		find_handle = ESIF_INVALID_FILE_ENUM_HANDLE;
		goto exit;
	}

	// Since readdir does not return sorted results, put all matches in an array and sort before returning the first match
	do {
		ffd = readdir(find_handle->handle);
		if (NULL != ffd && fnmatch(pattern, ffd->d_name, FNM_PATHNAME | FNM_NOESCAPE) == 0) {	// found a match
			char **oldFiles = find_handle->files;
			find_handle->matches++;
			find_handle->files = (char **)esif_ccb_realloc(find_handle->files, sizeof(char *) * find_handle->matches);
			if (!find_handle->files) {
				// It is such a catastrophic failure that we will just disregard all previous matches, clean up and exit
				find_handle->files = oldFiles;
				esif_ccb_file_enum_close(find_handle);
				find_handle = ESIF_INVALID_FILE_ENUM_HANDLE;
				goto exit;
			}
			find_handle->files[find_handle->matches - 1] = esif_ccb_strdup(ffd->d_name);
		}
	} while (NULL != ffd);

	// If any matches, sort the results and return the first match, freeing it from the result array
	if (find_handle->matches) {
		esif_context_t qsort_ctx = 0;
		esif_ccb_qsort(find_handle->files, find_handle->matches, sizeof(char *), esif_ccb_qsort_stricmp, &qsort_ctx);
		esif_ccb_strcpy(file->filename, find_handle->files[0], MAX_PATH);
		esif_ccb_free(find_handle->files[0]);
		find_handle->files[0] = NULL;
		goto exit;
	}

	closedir(find_handle->handle);
	esif_ccb_free(find_handle);
	find_handle = ESIF_INVALID_FILE_ENUM_HANDLE; // no matches

exit:
	return find_handle;
}


static ESIF_INLINE struct esif_ccb_file *esif_ccb_file_enum_next(
	esif_ccb_file_enum_t find_handle,
	esif_string pattern,
	struct esif_ccb_file *file
	)
{
	size_t idx;
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
	file = NULL; // no more matches
	return file;
}

static ESIF_INLINE void esif_ccb_file_enum_close(esif_ccb_file_enum_t find_handle)
{
	size_t idx;
	closedir(find_handle->handle);
	for (idx = 0; idx < find_handle->matches; idx++) {
		esif_ccb_free(find_handle->files[idx]);
	}
	esif_ccb_free(find_handle->files);
	esif_ccb_memset(find_handle, 0, sizeof(struct esif_ccb_file_enum));
	esif_ccb_free(find_handle);
}


static ESIF_INLINE int esif_ccb_file_exists(esif_string filename)
{
	struct stat st;
	return 0 == esif_ccb_stat(filename, &st);
}

#endif /* LINUX USER */