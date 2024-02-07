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

#pragma once

#include "esif_ccb_library.h"
#include "esif_sdk_iface_app.h"

// TODO: Move this to esif_sdk_iface_app.h
typedef esif_error_t (ESIF_CALLCONV *GetAppInterfaceV2FuncPtr)(AppInterfaceSet *);

/*
** Public ESIF App API
*/

#define DEFAULT_ESIFLIB		"Dptf" ESIF_LIB_EXT

typedef struct EsifAppLib_s {
	char						libPath[MAX_PATH];
	esif_lib_t					libModule;
	GetAppInterfaceV2FuncPtr	GetAppInterfaceV2;
	AppInterfaceSet				ifaceSet;
} EsifAppLib;

#ifdef __cplusplus
extern "C" {
#endif

esif_error_t ESIF_CALLCONV EsifAppLib_Load(EsifAppLib* self, const char* libPath);
esif_error_t ESIF_CALLCONV EsifAppLib_GetInterface(EsifAppLib* self);
esif_error_t ESIF_CALLCONV EsifAppLib_GetPathname(EsifAppLib* self, char* libPath, size_t path_len);
void ESIF_CALLCONV EsifAppLib_Unload(EsifAppLib* self);

#ifdef __cplusplus
}
#endif
