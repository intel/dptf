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
#include "esif_sdk_iface.h"
#include "ipf_core_api.h"

#define GET_IPC_INTERFACE_FUNCTION	"GetIpcInterface"
#define IPC_INTERFACE_VERSION_1		1 /* Initial Version */
#define IPC_INTERFACE_VERSION		IPC_INTERFACE_VERSION_1

// Object Types Signatures for struct objects
#define	ObjType_IpcSession			0x53435049	// 'IPCS' = IpcSession Object

typedef IpfSessionInfo IpcSessionInfo;	// IPC Session Configuration Parameters = Use IPF Session Info Definition
typedef esif_handle_t IpcSession_t;		// Opaque IPF Session Object Type returned by Public Interface

#define IPC_INVALID_SESSION			ESIF_INVALID_HANDLE
#define IPC_SESSION_FMT				"0x%llx"
#define IPC_SESSIONINFO_REVISION	IPF_SESSIONINFO_REVISION

/*
** Public IPC Interface
*/
typedef esif_error_t (ESIF_CALLCONV *Ipc_InitFuncPtr)(void);
typedef void (ESIF_CALLCONV *Ipc_ExitFuncPtr)(void);
typedef esif_error_t (ESIF_CALLCONV *Ipc_SetAppIfaceFuncPtr)(const AppInterfaceSet *ifaceSet);
typedef IpcSession_t (ESIF_CALLCONV *IpcSession_CreateFuncPtr)(const IpcSessionInfo *info);
typedef void (ESIF_CALLCONV *IpcSession_DestroyFuncPtr)(IpcSession_t self);
typedef esif_error_t (ESIF_CALLCONV *IpcSessionConnectFuncPtr)(IpcSession_t self);
typedef void (ESIF_CALLCONV *IpcSession_DisconnectFuncPtr)(IpcSession_t self);
typedef void (ESIF_CALLCONV *IpcSession_WaitForStopFuncPtr)(IpcSession_t self);

#pragma pack(push, 1)
typedef struct IpcInterface_s {
	EsifIfaceHdr					hdr;

	char							Ipc_Version[ESIF_VERSION_LEN];			// Build Version: Filled in by ipfipc.dll (server)
	char							Ipc_SdkVersion[ESIF_VERSION_LEN];		// SDK Version: Filled in by ipfipc.dll (server)
	char							Ipc_ClientSdkVersion[ESIF_VERSION_LEN];	// SDK Version: Filled in by ipfcorelib.lib [ipfcoresdk.dll] (client)

	Ipc_InitFuncPtr					Ipc_Init;
	Ipc_ExitFuncPtr					Ipc_Exit;
	Ipc_SetAppIfaceFuncPtr			Ipc_SetAppIface;
	IpcSession_CreateFuncPtr		IpcSession_Create;
	IpcSession_DestroyFuncPtr		IpcSession_Destroy;
	IpcSessionConnectFuncPtr		IpcSession_Connect;
	IpcSession_DisconnectFuncPtr	IpcSession_Disconnect;
	IpcSession_WaitForStopFuncPtr	IpcSession_WaitForStop;
} IpcInterface;
#pragma pack(pop)

typedef esif_error_t (ESIF_CALLCONV *GetIpcInterfaceFuncPtr)(IpcInterface *);
