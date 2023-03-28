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

#include "esif_ccb.h"
#include "esif_sdk_iface_app.h"

#ifdef __cplusplus
extern "C" {
#endif
	//
	// Exchanges ESIF application interface pointers to allow use of the ESIF
	// application interface directly; instead of using the SDK APIs
	//
	// Note(s):
	// 1) The APP portion should be filled out by the callee in order to
	// receive interface calls directly
	// 2) The ESIF interface portion will NOT be filled/returned.  The ESIF
	// interface function pointers are provided when the AppCreate function
	// supplied is called (fAppCreateFuncPtr)
	// 3) This function is not documented and should only be used by those
	// who understand the raw interface
	// 4) The interface will not become functional until a session is
	// established (at that point, the app create function, fAppCreateFuncPtr,
	// will be called.)
	//
	esif_error_t ESIF_CALLCONV IpfCore_SetAppIface(
		const AppInterfaceSet *appIfaceSetPtr
	);

#ifdef __cplusplus
}
#endif
