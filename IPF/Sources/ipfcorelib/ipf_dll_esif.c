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

#include "ipf_dll.h"
#include "ipf_dll_esif.h"
#include "esif_ccb_string.h"

/*
** ESIF App API
*/

#ifdef ESIF_ATTR_DEBUG
#define ESIFAPP_DEBUG(fmt, ...)	fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define ESIFAPP_DEBUG(fmt, ...)	(void)(0)
#endif

esif_error_t ESIF_CALLCONV EsifAppLib_Load(EsifAppLib *self, const char *libPath)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		if (libPath) {
			libPath = IpfDll_GetFullPath(libPath, self->libPath, sizeof(self->libPath));
		}
		self->libModule = esif_ccb_library_load((esif_string)libPath);

		if (esif_ccb_library_isloaded(self->libModule)) {
			rc = EsifAppLib_GetInterface(self);
		}
		else {
			rc = esif_ccb_library_error(self->libModule);
		}

		if (rc != ESIF_OK) {
			ESIFAPP_DEBUG("\n*** LoadLibrary Failure: %s (%s): %s\n", esif_rc_str(rc), self->libPath, esif_ccb_library_errormsg(self->libModule));
			EsifAppLib_Unload(self);
		}
	}
	return rc;
}

esif_error_t ESIF_CALLCONV EsifAppLib_GetInterface(EsifAppLib *self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		rc = ESIF_E_IFACE_NOT_SUPPORTED;

		if (self->GetAppInterfaceV2 == NULL) {
			self->GetAppInterfaceV2 = (GetAppInterfaceV2FuncPtr)esif_ccb_library_get_func(self->libModule, GET_APPLICATION_INTERFACE_FUNCTION);
		}

		if (self->GetAppInterfaceV2) {
			AppInterfaceSet *ifaceSet = &self->ifaceSet;
			ifaceSet->hdr.fIfaceType = eIfaceTypeApplication;
			ifaceSet->hdr.fIfaceVersion = APP_INTERFACE_VERSION;
			ifaceSet->hdr.fIfaceSize = (UInt16)sizeof(*ifaceSet);

			// Get Application Interface
			rc = self->GetAppInterfaceV2(ifaceSet);

			// Check AppInterface Version
			if ((rc == ESIF_OK) &&
				(ifaceSet->hdr.fIfaceType != eIfaceTypeApplication ||
				 ifaceSet->hdr.fIfaceVersion != APP_INTERFACE_VERSION ||
				 ifaceSet->hdr.fIfaceSize != (UInt16)sizeof(*ifaceSet))) {
				rc = ESIF_E_NOT_SUPPORTED;
			}

			// Check Required Functions Pointers
			if ((rc == ESIF_OK) &&
				(ifaceSet->appIface.fAppCreateFuncPtr == NULL ||
				 ifaceSet->appIface.fAppDestroyFuncPtr == NULL ||
				 ifaceSet->appIface.fAppGetNameFuncPtr == NULL ||
				 ifaceSet->appIface.fParticipantCreateFuncPtr == NULL ||
				 ifaceSet->appIface.fParticipantDestroyFuncPtr == NULL ||
				 ifaceSet->appIface.fDomainCreateFuncPtr == NULL ||
				 ifaceSet->appIface.fDomainDestroyFuncPtr == NULL)) {
				rc = ESIF_E_CALLBACK_IS_NULL;
			}
		}
	}
	return rc;
}

esif_error_t ESIF_CALLCONV EsifAppLib_GetPathname(EsifAppLib *self, char *libPath, size_t path_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && libPath && path_len) {
		esif_ccb_memset(libPath, 0, path_len);
		if (self->GetAppInterfaceV2 == NULL) {
			rc = EsifAppLib_GetInterface(self);
		}
		if (self->GetAppInterfaceV2) {
			rc = esif_ccb_library_getpath(self->libModule, libPath, path_len, (const void *)self->GetAppInterfaceV2);
		}
	}
	return rc;
}

void ESIF_CALLCONV EsifAppLib_Unload(EsifAppLib *self)
{
	if (self) {
		esif_ccb_library_unload(self->libModule);
		self->libModule = NULL;
		self->GetAppInterfaceV2 = NULL;
		esif_ccb_memset(&self->ifaceSet, 0, sizeof(self->ifaceSet));
	}
}
