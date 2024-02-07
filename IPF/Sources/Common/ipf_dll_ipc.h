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
#include "ipf_ipc_iface.h"

/*
** Public IPC API
*/

#define DEFAULT_IPCLIB	"ipfipc" ESIF_LIB_EXT

typedef struct IpfIpcLib_s {
	char					libPath[MAX_PATH];
	esif_lib_t				libModule;
	GetIpcInterfaceFuncPtr	GetIpcInterface;
	IpcInterface			iface;
} IpfIpcLib;


#ifdef __cplusplus
extern "C" {
#endif

	esif_error_t ESIF_CALLCONV IpfIpcLib_Load(IpfIpcLib* self, const char* libPath);
	esif_error_t ESIF_CALLCONV IpfIpcLib_GetInterface(IpfIpcLib* self);
	esif_error_t ESIF_CALLCONV IpfIpcLib_GetPathname(IpfIpcLib* self, char* libPath, size_t path_len);
	void ESIF_CALLCONV IpfIpcLib_Unload(IpfIpcLib* self);

#ifdef __cplusplus
}
#endif
