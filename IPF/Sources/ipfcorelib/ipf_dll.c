/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "esif_ccb_file.h"
#include "esif_ccb_string.h"
#include "ipf_dll.h"

/*
** IPF DLL App API
*/

static char *IpfDll_GetInstallDir(const char *libPath, char *fullpath, size_t path_len);


// Get Full Pathname of specified Library Path
char* ESIF_CALLCONV IpfDll_GetFullPath(const char *libPath, char *fullpath, size_t path_len)
{
	char *pathname = NULL;
	if (libPath && fullpath && path_len) {
		char installdir[MAX_PATH] = { 0 };

		if (esif_ccb_strpbrk(libPath, "\\/") == NULL) {
			pathname = IpfDll_GetInstallDir(libPath, installdir, sizeof(installdir));
		}

		if (pathname) {
			esif_ccb_sprintf(path_len, fullpath, "%s%s%s", pathname, ESIF_PATH_SEP, libPath);
		}
		else {
			esif_ccb_strcpy(fullpath, libPath, path_len);
		}
		pathname = fullpath;
	}
	return pathname;
}

// Get Install Path for DPTF/ESIF Loadable Libraries using optional library name
static char *IpfDll_GetInstallDir(const char *libPath, char *fullpath, size_t path_len)
{
	if (fullpath && path_len) {
#if defined(ESIF_ATTR_OS_CHROME)
		#ifdef ESIF_ATTR_64BIT
		# define ARCHBITS	"64"
		#else
		#  define ARCHBITS	"32"
		#endif
		esif_ccb_strcpy(fullpath, "/usr/lib" ARCHBITS, path_len);
		fullpath = NULL; // Do not load .so files using full path
		UNREFERENCED_PARAMETER(libPath);
#else
		#ifdef ESIF_ATTR_64BIT
		# define ARCHNAME	"x64"
		#else
		# define ARCHNAME	"x86"
		#endif
		esif_ccb_strcpy(fullpath, "/usr/share/dptf/uf" ARCHNAME, path_len);
		UNREFERENCED_PARAMETER(libPath);
#endif
	}
	return fullpath;
}

