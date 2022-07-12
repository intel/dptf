/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include <dlfcn.h>
#include <ctype.h>
#include <stdlib.h>

#include "esif_ccb_file.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"
#include "esif_ccb_rc.h"

/* Opaque Loadable Library Object. Callers should treat esif_lib_t as a Handle and set to NULL after closing */
struct esif_ccb_library_s {
	void         *handle; /* OS Handle to Loaded Library or NULL */
	enum esif_rc errnum;  /* ESIF Error Message ID for failed Load/Get-Proc */
	char         *errmsg; /* OS Error Message for failed Load/Get-Proc or NULL [Auto-Allocated] */
};
typedef struct esif_ccb_library_s *esif_lib_t;
#define ESIF_LIB_EXT ".so"

/* prototypes */
static ESIF_INLINE char *esif_ccb_library_errormsg(esif_lib_t lib);
static ESIF_INLINE enum esif_rc esif_ccb_library_error(esif_lib_t lib);
static ESIF_INLINE void esif_ccb_library_geterror(esif_lib_t lib);

#if defined(ESIF_ATTR_OS_CHROME) || defined(ESIF_ATTR_OS_ANDROID)
#define ESIF_ATTR_LIB_NOPATHNAME  /* Do not load libname.so files using full pathname */
#endif

/* Load Shared .so/DLL code opaque */
static ESIF_INLINE esif_lib_t esif_ccb_library_load(esif_string lib_name)
{
#ifdef ESIF_ATTR_LIB_NOPATHNAME
	/* Ignore pathname if OS requires libname.so only */
	esif_string slash = (lib_name ? esif_ccb_strrchr(lib_name, *ESIF_PATH_SEP) : NULL);
	if (slash) {
		lib_name = ++slash;
	}
#endif
	/* Remove Symbolic Links */
	if (lib_name && *lib_name == *ESIF_PATH_SEP && esif_ccb_drop_symlink(lib_name) != 0) {
		return NULL;
	}
	esif_lib_t lib = (esif_lib_t)esif_ccb_malloc(sizeof(*lib));
	if (NULL == lib)
		return NULL;

	/* NULL lib_name loads symbol from current module instead of dynamic library */
	Dl_info info = { 0 };
	if (NULL == lib_name && dladdr((const void *)&esif_ccb_library_load, &info)) {
		/* If current module is an .so, use it when loading NULL lib_names instead of main module */
		size_t extlen = esif_ccb_strlen(ESIF_LIB_EXT, MAX_PATH);
		size_t len = esif_ccb_strlen(info.dli_fname, MAX_PATH);
		if (len > extlen && esif_ccb_strcmp(&info.dli_fname[len - extlen], ESIF_LIB_EXT) == 0) {
			lib_name = (esif_string)info.dli_fname;
		}
	}
	
	int dlflags = RTLD_NOW | RTLD_GLOBAL;
	lib->handle = dlopen(lib_name, dlflags);
	esif_ccb_library_geterror(lib);

	/* Try different case-sensitive versions of lib_name */
	if (NULL == lib->handle && NULL != lib_name && lib_name[0] != 0 && esif_ccb_library_error(lib) == ESIF_E_NOT_FOUND && info.dli_fname == NULL) {
		char library[MAX_PATH]={0};
		size_t j=0;
		int start=0;
		char *errmsg = esif_ccb_strdup(esif_ccb_library_errormsg(lib));

		/* Copy full path */
		for (j=0; j < sizeof(library)-1 && lib_name[j]; j++) {
			library[j] = lib_name[j];
			if (lib_name[j] == *ESIF_PATH_SEP) {
				start = j+1;
			}
		}

		/* Capitalized.so */
		for (j=start; library[j]; j++) {
			library[j] = tolower(lib_name[j]);
		}
		library[start] = toupper(library[start]);
		if (library[0] == *ESIF_PATH_SEP) {
			esif_ccb_drop_symlink(library);
		}
		lib->handle = dlopen(library, dlflags);
		esif_ccb_library_geterror(lib);

		/* lowercase.so */
		if (NULL == lib->handle && esif_ccb_library_error(lib) == ESIF_E_NOT_FOUND) {
			library[start] = tolower(library[start]);
			if (library[0] == *ESIF_PATH_SEP) {
				esif_ccb_drop_symlink(library);
			}
			lib->handle = dlopen(library, dlflags);
			esif_ccb_library_geterror(lib);
		}

		/* UPPERCASE.so */
		if (NULL == lib->handle && esif_ccb_library_error(lib) == ESIF_E_NOT_FOUND) {
			for (j=start; library[j] && library[j] != '.'; j++) {
				library[j] = toupper(library[j]);
			}
			if (library[0] == *ESIF_PATH_SEP) {
				esif_ccb_drop_symlink(library);
			}
			lib->handle = dlopen(library, dlflags);
			esif_ccb_library_geterror(lib);
		}

		/* Reset Error Message */
		if (NULL == lib->handle && esif_ccb_library_error(lib) == ESIF_E_NOT_FOUND) {
			esif_ccb_free(lib->errmsg);
			lib->errmsg = errmsg;
		}
		else {
			esif_ccb_free(errmsg);
		}
	}
	return lib;
}


/* Find address for symbol */
static ESIF_INLINE void *esif_ccb_library_get_func(
	esif_lib_t lib,
	esif_string func_name
	)
{
	void *func = NULL;
	if (NULL == lib || NULL == lib->handle)
		return NULL;

	if ((func = dlsym(lib->handle, func_name)) == NULL) {
		esif_ccb_library_geterror(lib);
	}
	return func;
}

/* Get Full pathname for of loaded library */
static ESIF_INLINE enum esif_rc esif_ccb_library_getpath(
	esif_lib_t lib,
	esif_string pathname,
	size_t path_len,
	const void *func
)
{
	enum esif_rc rc = ESIF_E_PARAMETER_IS_NULL;
	Dl_info info = { 0 };
	if (lib && pathname && path_len && func) {
		if (!dladdr(func, &info)) {
			rc = ESIF_E_NO_CREATE;
		}
		else {
			rc = ESIF_OK;
			esif_ccb_strcpy(pathname, info.dli_fname, path_len);
			if (esif_ccb_strlen(info.dli_fname, MAX_PATH) > path_len) {
				rc = ESIF_E_NEED_LARGER_BUFFER;
			}
		}
	}
	return rc;
}

/* Close/Free Library Object */
static ESIF_INLINE void esif_ccb_library_unload(esif_lib_t lib)
{
	if (NULL != lib) {
		if (NULL != lib->handle)
			dlclose(lib->handle);
		esif_ccb_free(lib->errmsg);
		esif_ccb_memset(lib, 0, sizeof(*lib));
		esif_ccb_free(lib);
	}
}

/* Get Last Libary Load or Get Function Error, if any */
static ESIF_INLINE void esif_ccb_library_geterror(esif_lib_t lib)
{
	if (lib) {
		enum esif_rc rc = ESIF_E_NO_CREATE;

		esif_ccb_free(lib->errmsg);
		lib->errmsg = NULL;

		/* dlerror() resets to NULL after each call so save message for later */
		char *errmsg = dlerror();
		if (NULL != errmsg) {
			lib->errmsg = esif_ccb_strdup(errmsg);
		}

		if (!errmsg)
			rc = ESIF_OK;
		else if (strstr(errmsg, "No such file") || strstr(errmsg, "not found"))
			rc = ESIF_E_NOT_FOUND;
		else if (strstr(errmsg, "wrong ELF class"))
			rc = ESIF_E_NOT_SUPPORTED;
		else if (strstr(errmsg, "undefined symbol"))
			rc = ESIF_E_NOT_IMPLEMENTED;

		lib->errnum = rc;
	}
}

/* Get Error Message for failed Library Load or Get Function */
static ESIF_INLINE char *esif_ccb_library_errormsg(esif_lib_t lib)
{
	char *errmsg = (char *)"N/A";
	if (lib && lib->errnum != ESIF_OK && lib->errmsg) {
		errmsg = lib->errmsg;
	}
	return errmsg;
}

/* Return reason for most recent Load Library or Get Function error */
static ESIF_INLINE enum esif_rc esif_ccb_library_error(esif_lib_t lib)
{
	enum esif_rc rc = ESIF_E_PARAMETER_IS_NULL;
	if (lib) {
		rc = lib->errnum;
	}
	return rc;
}

/* Is Library Loaded? */
static ESIF_INLINE Bool esif_ccb_library_isloaded(esif_lib_t lib)
{
	return (NULL != lib && NULL != lib->handle);
}

#endif /* LINUX USER */
