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

#include "esif_ccb.h"
#include "esif_ccb_atomic.h"
#include "esif_ccb_library.h"
#include "esif_sdk_iface_app.h"
#include "ipfsrv_appmgr.h"
#include "ipf_appinfo.h"
#include "ipf_version.h"

// IPF Server Application Information
#define IPF_APP_NAME	"ipfsrv"
#define IPF_APP_DESC	"Intel(R) Innovation Platform Framework Server"
#define IPF_APP_BANNER	IPF_APP_DESC " v" IPF_APP_VERSION " [" ESIF_ATTR_OS " " ESIF_PLATFORM_TYPE " " ESIF_BUILD_TYPE "]"

IpfAppInfo g_ipfAppInfo = {
	.appName = IPF_APP_NAME,
	.appVersion = IPF_APP_VERSION,
	.appDescription = IPF_APP_DESC,
	.appBanner = IPF_APP_BANNER
};

// Get Loadable Library (DLL or SO) Name
static Bool GetLoadableLibraryName(char *buf_ptr, size_t buf_len)
{
	Bool result = ESIF_FALSE;
	if (buf_ptr && buf_len) {
		char pathname[MAX_PATH] = {0};
		esif_lib_t lib = esif_ccb_library_load(NULL);
		if (lib) {
			if (esif_ccb_library_getpath(lib, pathname, sizeof(pathname), GetLoadableLibraryName) == ESIF_OK) {
				char *slash = esif_ccb_strrchr(pathname, *ESIF_PATH_SEP);
				char *dot = (slash ? esif_ccb_strchr(slash, '.') : NULL);
				if (slash && dot) {
					slash++;
					*dot = 0;
					size_t buf_left = sizeof(pathname) - ((size_t)slash - (size_t)pathname);
					if (esif_ccb_strlen(slash, buf_left) < buf_len) {
						esif_ccb_strcpy(buf_ptr, slash, buf_len);
						result = ESIF_TRUE;
					}
				}
			}
			esif_ccb_library_unload(lib);
		}
	}
	return result;
}

//
// Exported ESIF App Interface Function. 
// This Fucntion Serves two different AppInterfaceSets, depending on when it is called:
// 1. IPF Server Application (On First Load, When ESIF starts IPFSRV)
// 2. IPF Client Session (On Additional Loads, When IPFSRV starts a new ESIF App to manage a new Client Connection)
//
ESIF_EXPORT eEsifError GetApplicationInterfaceV2(AppInterfaceSet *ifaceSetPtr)
{
	static atomic_t isServerLoaded = ATOMIC_INIT(0);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (ifaceSetPtr) {
		// Use different AppInterfaceSet depending on whether we are a Server (first DLL load) or Client (subsequent DLL loads)
		AppInterfaceSet myIfaceSet = IpfSrv_GetInterface();
		if (atomic_set(&isServerLoaded, 1) == 1) {
			myIfaceSet = IpfCli_GetInterface();
		}
		else {
			static char appName[ESIF_NAME_LEN] = {0};
			if (GetLoadableLibraryName(appName, sizeof(appName))) {
				g_ipfAppInfo.appName = appName;
			}
		}

		// Verify Interface Header
		if ((myIfaceSet.hdr.fIfaceType != ifaceSetPtr->hdr.fIfaceType) ||
			(myIfaceSet.hdr.fIfaceVersion != ifaceSetPtr->hdr.fIfaceVersion) ||
			(myIfaceSet.hdr.fIfaceSize != ifaceSetPtr->hdr.fIfaceSize)) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else {
			ifaceSetPtr->appIface = myIfaceSet.appIface;
			rc = ESIF_OK;
		}
	}
	return rc;
}