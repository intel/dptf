/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "ipf_dll.h"
#include "ipf_dll_ipc.h"
#include "ipf_sdk_version.h"

/*
** IPC Interface
*/

#ifdef ESIF_ATTR_DEBUG
#define IPCAPI_DEBUG(fmt, ...)	fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define IPCAPI_DEBUG(fmt, ...)	(void)(0)
#endif

esif_error_t ESIF_CALLCONV IpfIpcLib_Load(IpfIpcLib *self, const char *libPath)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		if (libPath) {
			libPath = IpfDll_GetFullPath(libPath, self->libPath, sizeof(self->libPath));
		}
		self->libModule = esif_ccb_library_load((esif_string)libPath);

		if (esif_ccb_library_isloaded(self->libModule)) {
			rc = IpfIpcLib_GetInterface(self);
		}
		else {
			rc = esif_ccb_library_error(self->libModule);
		}

		if (rc != ESIF_OK) {
			IPCAPI_DEBUG("\n*** LoadLibrary Failure: %s (%s): %s\n", esif_rc_str(rc), self->libPath, esif_ccb_library_errormsg(self->libModule));
			IpfIpcLib_Unload(self);
		}
	}
	return rc;
}

esif_error_t ESIF_CALLCONV IpfIpcLib_GetInterface(IpfIpcLib *self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		rc = ESIF_E_IFACE_NOT_SUPPORTED;

		if (self->GetIpcInterface == NULL) {
			self->GetIpcInterface = (GetIpcInterfaceFuncPtr)esif_ccb_library_get_func(self->libModule, GET_IPC_INTERFACE_FUNCTION);
		}

		if (self->GetIpcInterface) {
			IpcInterface *iface = &self->iface;
			iface->hdr.fIfaceType = eIfaceTypeIpfIpc;
			iface->hdr.fIfaceVersion = IPC_INTERFACE_VERSION;
			iface->hdr.fIfaceSize = (UInt16)sizeof(*iface);

			// Fill in Client SDK Version
			esif_ccb_strcpy(iface->Ipc_ClientSdkVersion, IPF_SDK_VERSION, sizeof(iface->Ipc_ClientSdkVersion));

			// Get IPC Interface
			rc = self->GetIpcInterface(iface);

			// Check IpcInterface Version
			if ((rc == ESIF_OK) &&
				(iface->hdr.fIfaceType != eIfaceTypeIpfIpc ||
				 iface->hdr.fIfaceVersion != IPC_INTERFACE_VERSION ||
				 iface->hdr.fIfaceSize != (UInt16)sizeof(*iface))) {
				rc = ESIF_E_NOT_SUPPORTED;
			}

			// Check Required Functions Pointers
			if ((rc == ESIF_OK) &&
				(iface->Ipc_Init == NULL ||
				 iface->Ipc_Exit == NULL ||
				 iface->Ipc_SetAppIface == NULL ||
				 iface->IpcSession_Create == NULL ||
				 iface->IpcSession_Destroy == NULL ||
				 iface->IpcSession_Connect == NULL ||
				 iface->IpcSession_Disconnect == NULL ||
				 iface->IpcSession_WaitForStop == NULL)) {
				rc = ESIF_E_CALLBACK_IS_NULL;
			}

			// Fill in version if none specified
			if ((rc == ESIF_OK) && iface->Ipc_Version[0] == 0) {
				esif_ccb_strcpy(iface->Ipc_Version, IPF_APP_VERSION, sizeof(iface->Ipc_Version));
			}
		}
	}
	return rc;
}

esif_error_t ESIF_CALLCONV IpfIpcLib_GetPathname(IpfIpcLib *self, char *libPath, size_t path_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && libPath && path_len) {
		esif_ccb_memset(libPath, 0, path_len);
		if (self->GetIpcInterface == NULL) {
			rc = IpfIpcLib_GetInterface(self);
		}
		if (self->GetIpcInterface) {
			rc = esif_ccb_library_getpath(self->libModule, libPath, path_len, (const void *)self->GetIpcInterface);
		}
	}
	return rc;
}

void ESIF_CALLCONV IpfIpcLib_Unload(IpfIpcLib *self)
{
	if (self) {
		esif_ccb_library_unload(self->libModule);
		self->libModule = NULL;
		self->GetIpcInterface = NULL;
		esif_ccb_memset(&self->iface, 0, sizeof(self->iface));
	}
}
