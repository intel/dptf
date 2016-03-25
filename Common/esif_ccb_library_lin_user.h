/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"
#include "esif_ccb_rc.h"

/* Opaque Loadable Library Object. Callers should treat esif_lib_t as a Handle and set to NULL after closing */
struct esif_ccb_library_s {
	void         *handle; /* OS Handle to Loaded Library or NULL */
	enum esif_rc errnum;  /* ESIF Error Message ID for failed Load/Get-Proc or NULL */
	char         *errmsg; /* OS Error Message for failed Load/Get-Proc or NULL [Auto-Allocated] */
};
typedef struct esif_ccb_library_s *esif_lib_t;
#define ESIF_LIB_EXT ".so"

/* prototypes */
static ESIF_INLINE char *esif_ccb_library_errormsg(esif_lib_t lib);
static ESIF_INLINE enum esif_rc esif_ccb_library_error(esif_lib_t lib);

/* Load Shared .so/DLL code opaque */
static ESIF_INLINE esif_lib_t esif_ccb_library_load(esif_string lib_name)
{
	esif_lib_t lib = (esif_lib_t)esif_ccb_malloc(sizeof(*lib));
	if (NULL == lib)
		return NULL;

	lib->handle = dlopen(lib_name, RTLD_NOW);

	/* Try different case-sensitive versions of lib_name */
	if (NULL == lib->handle && lib_name[0] != 0 && esif_ccb_library_error(lib) == ESIF_E_NOT_FOUND) {
		char library[256]={0};
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
		lib->handle = dlopen(library, RTLD_NOW);

		/* lowercase.so */
		if (NULL == lib->handle && esif_ccb_library_error(lib) == ESIF_E_NOT_FOUND) {
			library[start] = tolower(library[start]);
			lib->handle = dlopen(library, RTLD_NOW);
		}

		/* UPPERCASE.so */
		if (NULL == lib->handle && esif_ccb_library_error(lib) == ESIF_E_NOT_FOUND) {
			for (j=start; library[j] && library[j] != '.'; j++) {
				library[j] = toupper(library[j]);
			}
			lib->handle = dlopen(library, RTLD_NOW);
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
		esif_ccb_free(lib->errmsg);
		lib->errmsg = NULL;
		lib->errnum = esif_ccb_library_error(lib);
	}
	return func;
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

/* Get Error Message for failed Library Load or Get Function */
static ESIF_INLINE char *esif_ccb_library_errormsg(esif_lib_t lib)
{
	const char *errmsg = NULL;
	if (NULL == lib)
		return (char *)"null library";

	/* dlerror() resets to NULL after each call so save message for later */
	errmsg = dlerror();
	if (NULL != errmsg) {
		esif_ccb_free(lib->errmsg);
		lib->errmsg = esif_ccb_strdup(errmsg);
	}
	return lib->errmsg;
}

/* Return reason for most recent Load Library or Get Function error */
static ESIF_INLINE enum esif_rc esif_ccb_library_error(esif_lib_t lib)
{
	enum esif_rc rc = ESIF_E_NO_CREATE;
	char *errmsg = esif_ccb_library_errormsg(lib);
	if (NULL == lib)
		return ESIF_E_PARAMETER_IS_NULL;

	if (!errmsg)
		rc = ESIF_OK;
	else if (strstr(errmsg, "No such file") || strstr(errmsg, "not found"))
		rc = ESIF_E_NOT_FOUND;
	else if (strstr(errmsg, "wrong ELF class"))
		rc = ESIF_E_NOT_SUPPORTED;
	else if (strstr(errmsg, "undefined symbol"))
		rc = ESIF_E_NOT_IMPLEMENTED;

	return rc;
}

#endif /* LINUX USER */
