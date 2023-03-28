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
#include "esif_ccb_rc.h"
#include "esif_sdk_iface.h"
#include "esif_sdk_dcfg.h"

#include "ipf_ipc_codec.h"

// IPF Server App Session
AppSession *AppSession_Create();
void AppSession_Destroy(AppSession *self);
void AppSession_GetRef(AppSession *self);
void AppSession_PutRef(AppSession *self);
Bool AppSession_IsConnected(AppSession *self);
void AppSession_Disconnect(AppSession *self);

// IPF Server App Client (App Session identified by IPF Client Session Handle)
esif_error_t IpfClient_Start(esif_handle_t ipfHandle, esif_ccb_sockaddr_t sockaddr, esif_handle_t authHandle);
Bool IpfClient_IsStarted(esif_handle_t ipfHandle);
Bool IpfClient_Exists(esif_handle_t ipfHandle);
esif_error_t IpfClient_Stop(esif_handle_t ipfHandle);
esif_error_t IpfClient_Rename(esif_handle_t ipfHandle, esif_string newName);
esif_error_t IpfClient_Close(esif_handle_t ipfHandle);

// IRPC Transaction
esif_error_t Irpc_SendMessage(IrpcTransaction *trx);

/////////////////////////////////
// Authorization Manager
/////////////////////////////////

// IPF Server Authentication Manager
esif_error_t AuthMgr_Init(void);
void AuthMgr_Exit(void);

/////////////////////////////////
// App Session Manager
/////////////////////////////////

#define	IPFSRV_MAX_SESSIONS	ESIF_MAX_CLIENTS	// Max Sessions = Max Remote WebSocket Clients

// IPF Server Session Manager (Singleton Instance)
typedef struct AppSessionMgr_s {
	esif_ccb_lock_t		lock;
	AppSession			*sessions[IPFSRV_MAX_SESSIONS];
	atomic_t			sessionTimeout;
	atomic_t			suspendTimeout;
} AppSessionMgr;

extern AppSessionMgr g_sessionMgr;			// Global IPF Server Session manager Singleton Instance

// IPF Server App Session Manager
esif_error_t AppSessionMgr_Init(void);
void AppSessionMgr_Exit(void);

esif_error_t AppSessionMgr_Start(void);
void AppSessionMgr_Stop(void);
Bool AppSessionMgr_IsStarted(void);

void AppSessionMgr_SessionCleanup();
double AppSessionMgr_GetMinTimeout();
size_t AppSessionMgr_GetTimeout();
void AppSessionMgr_SetTimeout(size_t timeout);
void AppSessionMgr_SuspendTimeouts();
void AppSessionMgr_ResumeTimeouts();

esif_handle_t AppSessionMgr_GenerateHandle(void);

AppSession *AppSessionMgr_GetSessionById(size_t id);
AppSession *AppSessionMgr_GetSessionByHandle(esif_handle_t ipfHandle);
AppSession *AppSessionMgr_GetSessionByName(esif_string appName);
AppSession *AppSessionMgr_GetSessionByThreadId(esif_thread_id_t threadId);
AppSession *AppSessionMgr_GetSessionByEsifHandle(esif_handle_t esifHandle);

esif_handle_t AppSessionMgr_GetEsifHandleByHandle(esif_handle_t ipfHandle);
esif_handle_t AppSessionMgr_GetHandleByEsifHandle(esif_handle_t esifHandle);

AppSession *AppSessionMgr_CreateSession(esif_handle_t ipfHandle);
void AppSessionMgr_DeleteSession(esif_handle_t ipfHandle);

/////////////////////////////////
// IPF Server Application
/////////////////////////////////

DCfgOptions IpfSrv_GetDcfg();
void IpfSrv_SetDcfg(DCfgOptions opt);
Bool IpfSrv_IsAppInstalled(const char* appKey);
esif_handle_t IpfSrv_GetEsifHandle();
AppInterfaceSet IpfSrv_GetInterface();
void IpfSrv_SetInterface(const AppInterfaceSet *iface);

AppInterfaceSet IpfCli_GetInterface();

/////////////////////////////////
// Trace Debugging
/////////////////////////////////
#if 0
#define IPF_DEBUG(msg, ...)	fprintf(stderr, msg, ##__VA_ARGS__)
#else
#define IPF_DEBUG(msg, ...)	(void)(0)
#endif
