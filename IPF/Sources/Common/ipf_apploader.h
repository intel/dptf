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

#pragma once

#include "esif_ccb_rc.h"
#include "ipf_core_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// AppLoader Multi-Instance object (opaque CamelCase AppLoader object)
typedef struct AppLoader_s AppLoader;

AppLoader* ESIF_CALLCONV AppLoader_GetInstance();		// Get Singleton Instance (legacy support)
AppLoader* ESIF_CALLCONV AppLoader_Create();			// Create New Uninitialized Multi-Instance object
void ESIF_CALLCONV AppLoader_Destroy(AppLoader* self);	// Destroy Uninitialized Multi-Instance object

esif_error_t ESIF_CALLCONV AppLoader_Init(AppLoader* self, const char* libpath, const char* serveraddr);
esif_error_t ESIF_CALLCONV AppLoader_Start(AppLoader* self);
esif_error_t ESIF_CALLCONV AppLoader_Pause(AppLoader* self);
esif_error_t ESIF_CALLCONV AppLoader_Continue(AppLoader* self);
void ESIF_CALLCONV AppLoader_Stop(AppLoader* self);
void ESIF_CALLCONV AppLoader_Exit(AppLoader* self);

// Older Singleton Interface for legacy support (non-CamelCase Apploader)
#define Apploader_Init(lib, addr)	AppLoader_Init(AppLoader_GetInstance(), lib, addr)
#define Apploader_Start()			AppLoader_Start(AppLoader_GetInstance())
#define Apploader_Pause()			AppLoader_Pause(AppLoader_GetInstance())
#define Apploader_Continue()		AppLoader_Continue(AppLoader_GetInstance())
#define Apploader_Stop()			AppLoader_Stop(AppLoader_GetInstance())
#define Apploader_Exit()			AppLoader_Exit(AppLoader_GetInstance())

#ifdef __cplusplus
}
#endif
