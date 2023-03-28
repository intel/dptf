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

#include "esif_sdk_data.h"
#include "esif_sdk_event_type.h"
#include "ipf_ipc_iface.h"
#include "ipf_core_api.h"

#define GET_IPF_INTERFACE_FUNCTION "GetIpfInterface"
#define IPF_INTERFACE_VERSION_1 1 /* Initial Version */
#define IPF_INTERFACE_VERSION   IPF_INTERFACE_VERSION_1


// IpfSession Members
typedef esif_error_t (ESIF_CALLCONV *IpfCore_InitFuncPtr)();
typedef void (ESIF_CALLCONV *IpfCore_ExitFuncPtr)();
typedef esif_error_t (ESIF_CALLCONV *IpfCore_SetAppIfaceFuncPtr)(const AppInterfaceSet *appIfacePtr);
typedef IpfSession_t (ESIF_CALLCONV *IpfCore_SessionCreateFuncPtr)(IpfSessionInfo *sessionInfoPtr);
typedef void (ESIF_CALLCONV *IpfCore_SessionDestroyFuncPtr)(IpfSession_t session);
typedef esif_error_t (ESIF_CALLCONV *IpfCore_SessionConnectFuncPtr)(IpfSession_t session);
typedef void (ESIF_CALLCONV *IpfCore_SessionDisconnectFuncPtr)(IpfSession_t session);
typedef void (ESIF_CALLCONV *IpfCore_SessionWaitForStopFuncPtr)(IpfSession_t session);

typedef esif_error_t (ESIF_CALLCONV *IpfCore_SessionRegisterEventFuncPtr)(
	IpfSession_t session,
	esif_event_type_t eventType,
	char *participantName,
	IPFAPI_EVENT_CALLBACK callback,
	esif_context_t context
	);

typedef esif_error_t (ESIF_CALLCONV *IpfCore_SessionUnregisterEventFuncPtr)(
	IpfSession_t session,
	esif_event_type_t eventType,
	char *participantName,
	IPFAPI_EVENT_CALLBACK callback,
	esif_context_t context
	);

typedef esif_error_t (ESIF_CALLCONV *IpfCore_SessionExecuteFuncPtr)(
	IpfSession_t session,
	EsifData *cmdPtr,
	EsifData *reqPtr,
	EsifData *respPtr
	);


//
// IPF API INTERFACE STRUCTURE
//
// Filled in by the GetIpfInterface function exported by the DLL
// Notes(s):
// 1) Must provide a buffer large enough to contain the supported
// interface version
// 2) The "hdr" member must be initialied prior to calling GetIpfInterface
//   a) hdr.fIfaceType = eIfaceTypeIpfClient
//   b) hdr.fIfaceVersion = IPF_INTERFACE_VERSION
//   c) hdr.fIfaceSize = sizeof(IpfIface)
//
#pragma pack(push, 1)
typedef struct IpfIface_s {
	EsifIfaceHdr hdr;

	char IpfCore_Version[ESIF_VERSION_LEN];			// Build Version: Filled in by ipfcore.dll (server)
	char IpfCoreSdk_Version[ESIF_VERSION_LEN];		// SDK Version: Filled in by ipfcore.dll (server)
	char IpfClientSdk_Version[ESIF_VERSION_LEN];	// SDK Version: Filled in by ipfcorelib.lib [ipfcoresdk.dll] (client)

	IpfCore_InitFuncPtr IpfCore_Init;
	IpfCore_ExitFuncPtr IpfCore_Exit;
	IpfCore_SetAppIfaceFuncPtr IpfCore_SetAppIface;
	IpfCore_SessionCreateFuncPtr IpfCore_SessionCreate;
	IpfCore_SessionDestroyFuncPtr IpfCore_SessionDestroy;
	IpfCore_SessionConnectFuncPtr IpfCore_SessionConnect;
	IpfCore_SessionDisconnectFuncPtr IpfCore_SessionDisconnect;
	IpfCore_SessionWaitForStopFuncPtr IpfCore_SessionWaitForStop;
	IpfCore_SessionExecuteFuncPtr IpfCore_SessionExecute;
	IpfCore_SessionRegisterEventFuncPtr IpfCore_SessionRegisterEvent;
	IpfCore_SessionUnregisterEventFuncPtr IpfCore_SessionUnregisterEvent;

} IpfIface;
#pragma pack(pop)

typedef esif_error_t(ESIF_CALLCONV *GetIpfInterfaceFuncPtr)(IpfIface *);

#ifdef __cplusplus
extern "C" {
#endif
	//
	// Interface function exported by IpfCore.dll to retrieve the function
	// pointers used to access the API.
	// Notes(s):
	// 1) Expected to be the first function called to get the supported
	// interface.
	// 2) The caller should verify compatibility before using the interface
	// pointers returned in the IpfIface structure
	//   Recommended interface header checks:
	//    a) Verify the type is eIfaceTypeIpfClient
	//    b) Verify the version is IPF_INTERFACE_VERSION
	//    c) Validate size to be >= sizeof(IpfIface)
	//
	ESIF_EXPORT eEsifError ESIF_CALLCONV GetIpfInterface(IpfIface *ifacePtr);

#ifdef __cplusplus
}
#endif
