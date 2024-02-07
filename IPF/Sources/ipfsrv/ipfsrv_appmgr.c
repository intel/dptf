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

#include "esif_ccb_rc.h"
#include "esif_ccb_string.h"
#include "esif_ccb_time.h"
#include "esif_sdk_iface_app.h"
#include "esif_sdk_iface_esif.h"
#include "esif_sdk_event_guid.h"
#include "esif_sdk_event_map.h"
#include "esif_sdk_message.h"

#include "ipf_core_api.h"
#include "ipf_version.h"
#include "ipf_lifecycle.h"
#include "ipf_ibinary.h"
#include "ipf_ipc_codec.h"
#include "ipf_ipc_trxmgr.h"

#include "ipf_appinfo.h"
#include "ipf_handle.h"
#include "ipfsrv_appmgr.h"
#include "ipfsrv_authmgr.h"
#include "ipfsrv_ws_server.h"

#define IPFSRV_MAX_STRLEN	0x7ffffffe
#define IPFSRV_LIBNAME		"ipfsrv"	// Loadable Library Name

// IPF Session Manager Singleton Instance
AppSessionMgr	g_sessionMgr = { 0 };

#ifdef ESIF_ATTR_DEBUG
#define SESSION_EXPIRE_TIMEOUT_SECONDS	(15*60)	// Expire Dead Sessions after this many seconds (Default)
#else
#define SESSION_EXPIRE_TIMEOUT_SECONDS	45		// Expire Dead Sessions after this many seconds (Default)
#endif
#define SESSION_EXPIRE_TIMEOUT_NEVER	0x7fffffff	// Never Expire Dead Sessions

///////////////////////////////////////////////////////////////////////////////
// IPF Server App Session
///////////////////////////////////////////////////////////////////////////////

// Create a new AppSession
AppSession *AppSession_Create()
{
	AppSession *self = esif_ccb_malloc(sizeof(*self));
	if (self) {
		esif_ccb_lock_init(&self->lock);
		atomic_set(&self->refCount, 0);
		self->ipfHandle = self->esifHandle = self->appHandle = self->authHandle = ESIF_INVALID_HANDLE;
	}
	return self;
}

// Destroy an AppSession
void AppSession_Destroy(AppSession *self)
{
	esif_ccb_lock_uninit(&self->lock);
	esif_ccb_memset(self, 0, sizeof(*self));
	esif_ccb_free(self);
}

// Take a Reference
void AppSession_GetRef(AppSession *self)
{
	if (self) {
		atomic_inc(&self->refCount);
	}
}

// Release a Reference, Destroying object when Reference Count reaches 0
void AppSession_PutRef(AppSession *self)
{
	if (self) {
		if (atomic_dec(&self->refCount) == 0) {
			AppSession_Destroy(self);
		}
	}
}

// Is current Session Connected?
Bool AppSession_IsConnected(AppSession *self)
{
	Bool ret = ESIF_FALSE;
	if (self && self->ipfHandle != ESIF_INVALID_HANDLE && atomic_read(&self->connected)) {
		ret = WebServer_IsStarted(g_WebServer);
	}
	return ret;
}

// Disconnect Session from RPC Client Transport
void AppSession_Disconnect(AppSession *self)
{
	if (self) {
		esif_ccb_write_lock(&self->lock);
		self->esifHandle = ESIF_INVALID_HANDLE;
		self->appHandle = ESIF_INVALID_HANDLE;
		self->authHandle = ESIF_INVALID_HANDLE;
		esif_ccb_memset(&self->ifaceSet, 0, sizeof(self->ifaceSet));
		esif_ccb_write_unlock(&self->lock);
	}
}

///////////////////////////////////////////////////////////////////////////////
// IPF Server App Session Manager
///////////////////////////////////////////////////////////////////////////////

esif_error_t AppSessionMgr_Init(void)
{
	esif_error_t rc = ESIF_E_NOT_INITIALIZED;
	AppSessionMgr *self = &g_sessionMgr;
	if (self) {
		esif_ccb_lock_init(&self->lock);
		atomic_set(&self->sessionTimeout, SESSION_EXPIRE_TIMEOUT_SECONDS);
		atomic_set(&self->suspendTimeout, 0);
		AuthMgr_Init();
		rc = WebPlugin_Init();
	}
	return rc;
}
void AppSessionMgr_Exit(void)
{
	AppSessionMgr *self = &g_sessionMgr;
	if (self) {
		esif_ccb_read_lock(&self->lock);
		for (int j = 0; j < IPFSRV_MAX_SESSIONS; j++) {
			if (self->sessions[j] && self->sessions[j]->ipfHandle != ESIF_INVALID_HANDLE) {
				esif_handle_t ipfHandle = self->sessions[j]->ipfHandle;
				esif_ccb_read_unlock(&self->lock);
				IpfClient_Close(ipfHandle);
				esif_ccb_read_lock(&self->lock);
			}
		}
		esif_ccb_read_unlock(&self->lock);

		AuthMgr_Exit();
		WebPlugin_Exit();
		esif_ccb_lock_uninit(&self->lock);
	}
}

esif_error_t AppSessionMgr_Start(void)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	WebServerPtr server = g_WebServer;
	const AccessDef *listeners = AccessMgr_GetAccessDefs();

	// Configure and create Listeners
	if (listeners) {
		rc = ESIF_OK;
		for (u8 instance = 0; rc == ESIF_OK && instance < IPF_WS_MAX_LISTENERS && listeners[instance].serverAddr; instance++) {
			rc = WebServer_Config(server, instance, &listeners[instance]);
		}
	}

	// Start Server if all Listeners were successfully created
	if (rc == ESIF_OK) {
		rc = WebServer_Start(server);
	}
	return rc;
}

Bool AppSessionMgr_IsStarted()
{
	return WebServer_IsStarted(g_WebServer);
}

void AppSessionMgr_Stop(void)
{
	/*
	** We cannot call IpfClient_Stop here to trigger an "appstop client" for each connected client because
	** the current thread already may have a shell lock for the "appstop ipfsrv" command, causing a deadlock.
	** Therefore, our solution is to assume that ESIF has already stopped all child client applications before
	** calling IpfSrv_AppDestroy, thereby guaranteeing that we don't need to stop any clients here.
	*/
	WebServer_Stop(g_WebServer);
}

// Cleanup Dead Disconnected Sessions and Sessions that failed to AppCreate
void AppSessionMgr_SessionCleanup()
{
	AppSessionMgr *self = &g_sessionMgr;
	if (self) {
		esif_ccb_read_lock(&self->lock);
		esif_ccb_realtime_t now = esif_ccb_realtime_current();
		for (int j = 0; j < ESIF_ARRAY_LEN(self->sessions); j++) {
			AppSession *thisSession = self->sessions[j];
			if (thisSession) {
				if ((!atomic_read(&thisSession->connected)) ||
					(!IpfClient_IsStarted(thisSession->ipfHandle) && esif_ccb_realtime_diff_sec(thisSession->connectTime, now) >= atomic_read(&self->sessionTimeout))) {

					esif_ccb_read_unlock(&self->lock);
					IpfClient_Close(thisSession->ipfHandle);
					esif_ccb_read_lock(&self->lock);
				}
			}
		}
		esif_ccb_read_unlock(&self->lock);
	}
}

// Return the Minimum Timeout
double AppSessionMgr_GetMinTimeout()
{
	AppSessionMgr *self = &g_sessionMgr;
	double result = SESSION_EXPIRE_TIMEOUT_NEVER;
	if (self) {
		esif_ccb_realtime_t now = esif_ccb_realtime_current();
		esif_ccb_read_lock(&self->lock);
		for (size_t j = 0; j < ESIF_ARRAY_LEN(self->sessions); j++) {
			if (self->sessions[j]) {
				double timeout = (double)atomic_read(&self->sessionTimeout) - (esif_ccb_realtime_diff_msec(self->sessions[j]->connectTime, now) / 1000.0);
				if (timeout > 0 && timeout < result) {
					result = timeout;
				}
			}
		}
		esif_ccb_read_unlock(&self->lock);
	}
	return result;
}

// Set RPC Session Timeout
size_t AppSessionMgr_GetTimeout()
{
	AppSessionMgr *self = &g_sessionMgr;
	size_t timeout = 0;
	if (self) {
		timeout = atomic_read(&self->sessionTimeout);
	}
	return timeout;
}

// Set RPC Session Timeout and Transaction Timeout (0 = Use Default)
void AppSessionMgr_SetTimeout(size_t timeout)
{
	AppSessionMgr *self = &g_sessionMgr;
	if (self) {
		atomic_set(&self->sessionTimeout, (timeout ? timeout : SESSION_EXPIRE_TIMEOUT_SECONDS));
		IpfTrxMgr_SetTimeout(IpfTrxMgr_GetInstance(), timeout);
	}
}

void AppSessionMgr_SuspendTimeouts(void)
{
	AppSessionMgr *self = &g_sessionMgr;
	if (self && !atomic_read(&self->suspendTimeout) && atomic_read(&self->sessionTimeout)) {
		atomic_set(&self->suspendTimeout, atomic_read(&self->sessionTimeout));
		AppSessionMgr_SetTimeout(SESSION_EXPIRE_TIMEOUT_NEVER);
	}
}

void AppSessionMgr_ResumeTimeouts(void)
{
	AppSessionMgr *self = &g_sessionMgr;
	if (self && atomic_read(&self->suspendTimeout)) {
		IpfTrxMgr_ResetTimeouts(IpfTrxMgr_GetInstance());
		AppSessionMgr_SetTimeout(atomic_read(&self->suspendTimeout));
		atomic_set(&self->suspendTimeout, 0);
	}
}

// Generate a Unique IPF Client Session Handle (ipfHandle) or IPF Server Application Handle (ipfsrv appHandle)
esif_handle_t AppSessionMgr_GenerateHandle(void)
{
	const esif_handle_t handle_min = 0x0000000050000000; // Min Handle before Random Seed
	const esif_handle_t handle_max = 0x7ffffffff0000000; // Max Handle before Rollover
	const esif_handle_t handle_seed= 0x0000000000ffffff; // Handle Initial Seed Random Mask
	const esif_handle_t handle_inc = 0x000000000000ffff; // Handle Increment Random Mask

	esif_handle_t ipfHandle = ESIF_INVALID_HANDLE;
	AppSessionMgr *self = &g_sessionMgr;
	esif_ccb_read_lock(&self->lock);
	do {
		ipfHandle = Ipf_GenerateHandle(handle_min, handle_max, handle_seed, handle_inc);

		// Verify that no existing sessions are using the new handle
		for (int j = 0; j < ESIF_ARRAY_LEN(self->sessions); j++) {
			if (self->sessions[j] && self->sessions[j]->ipfHandle == ipfHandle) {
				ipfHandle = ESIF_INVALID_HANDLE;
				break;
			}
		}
	} while (ipfHandle == ESIF_INVALID_HANDLE);
	esif_ccb_read_unlock(&self->lock);
	return ipfHandle;
}

// Get a Session by Id and Take a Reference
AppSession *AppSessionMgr_GetSessionById(size_t id)
{
	AppSessionMgr *self = &g_sessionMgr;
	AppSession *result = NULL;
	if (self && id < ESIF_ARRAY_LEN(self->sessions)) {
		esif_ccb_read_lock(&self->lock);
		if (self->sessions[id]) {
			result = self->sessions[id];
			AppSession_GetRef(result);
		}
		esif_ccb_read_unlock(&self->lock);
	}
	return result;
}

// Get a Session by IPF Client Session Handle and Take a Reference
AppSession *AppSessionMgr_GetSessionByHandle(esif_handle_t ipfHandle)
{
	AppSessionMgr *self = &g_sessionMgr;
	AppSession *result = NULL;
	if (self && ipfHandle != ESIF_INVALID_HANDLE) {
		esif_ccb_read_lock(&self->lock);
		for (int j = 0; j < ESIF_ARRAY_LEN(self->sessions); j++) {
			if (self->sessions[j] && self->sessions[j]->ipfHandle == ipfHandle) {
				result = self->sessions[j];
				AppSession_GetRef(result);
				break;
			}
		}
		esif_ccb_read_unlock(&self->lock);
	}
	return result;
}

// Get a Session by appName exposed to ESIF (esifName) and Take a Reference
AppSession *AppSessionMgr_GetSessionByName(esif_string appName)
{
	AppSessionMgr *self = &g_sessionMgr;
	AppSession *result = NULL;
	if (self && appName && appName[0]) {
		esif_ccb_read_lock(&self->lock);
		for (int j = 0; j < ESIF_ARRAY_LEN(self->sessions); j++) {
			if (self->sessions[j] && esif_ccb_stricmp(self->sessions[j]->esifName, appName) == 0 && self->sessions[j]->ipfHandle != ESIF_INVALID_HANDLE) {
				result = self->sessions[j];
				AppSession_GetRef(result);
				break;
			}
		}
		esif_ccb_read_unlock(&self->lock);
	}
	return result;
}

// Get a Session by ThreadId and Take a Refererence. Requires AppGetName to be called on same thread prior to AppCreate.
AppSession *AppSessionMgr_GetSessionByThreadId(esif_thread_id_t threadId)
{
	AppSessionMgr *self = &g_sessionMgr;
	AppSession *result = NULL;
	if (self && threadId != ESIF_NULL_THREAD_ID) {
		esif_ccb_read_lock(&self->lock);
		for (int j = 0; j < ESIF_ARRAY_LEN(self->sessions); j++) {
			if (self->sessions[j] && self->sessions[j]->threadId == threadId && self->sessions[j]->ipfHandle != ESIF_INVALID_HANDLE) {
				result = self->sessions[j];
				AppSession_GetRef(result);
				break;
			}
		}
		esif_ccb_read_unlock(&self->lock);
	}
	return result;
}

// Get a Session by its ESIF Handle (which is unique) and Take a Reference
AppSession *AppSessionMgr_GetSessionByEsifHandle(esif_handle_t esifHandle)
{
	AppSessionMgr *self = &g_sessionMgr;
	AppSession *result = NULL;
	if (self && esifHandle != ESIF_INVALID_HANDLE) {
		esif_ccb_read_lock(&self->lock);
		for (int j = 0; j < ESIF_ARRAY_LEN(self->sessions); j++) {
			if (self->sessions[j] && self->sessions[j]->esifHandle == esifHandle) {
				result = self->sessions[j];
				AppSession_GetRef(result);
				break;
			}
		}
		esif_ccb_read_unlock(&self->lock);
	}
	return result;
}

// Create a new Session using the given IPF Client Session handle in the next available Session Manager slot, if any
AppSession *AppSessionMgr_CreateSession(esif_handle_t ipfHandle)
{
	AppSessionMgr *self = &g_sessionMgr;
	AppSession *result = NULL;
	if (self && ipfHandle != ESIF_INVALID_HANDLE) {
		int slot = -1;
		esif_ccb_write_lock(&self->lock);
		for (int j = 0; j < ESIF_ARRAY_LEN(self->sessions); j++) {
			if (self->sessions[j] == NULL) {
				if (slot == -1) {
					slot = j;
				}
			}
			else if (self->sessions[j]->ipfHandle == ipfHandle) {
				slot = -1;
				break;
			}
		}
		if (slot != -1) {
			result = AppSession_Create();
			if (result) {
				result->ipfHandle = ipfHandle;
				atomic_set(&result->connected, 1);
				result->connectTime = esif_ccb_realtime_current();
				result->updateTime = result->connectTime;
				AppSession_GetRef(result);
				self->sessions[slot] = result;
			}
		}
		esif_ccb_write_unlock(&self->lock);
	}
	return result;
}

void AppSessionMgr_DeleteSession(esif_handle_t ipfHandle)
{
	AppSessionMgr *self = &g_sessionMgr;
	if (self && ipfHandle != ESIF_INVALID_HANDLE) {
		esif_ccb_write_lock(&self->lock);
		for (int j = 0; j < ESIF_ARRAY_LEN(self->sessions); j++) {
			AppSession *thisSession = self->sessions[j];
			if (thisSession && thisSession->ipfHandle == ipfHandle) {
				self->sessions[j] = NULL;
				AppSession_PutRef(thisSession);
			}
		}
		esif_ccb_write_unlock(&self->lock);
	}
}

// Lookup a Session's ESIF Handle by its IPF Client Session Handle
esif_handle_t AppSessionMgr_GetEsifHandleByHandle(esif_handle_t ipfHandle)
{
	esif_handle_t esifHandle = ESIF_INVALID_HANDLE;
	AppSession *session = AppSessionMgr_GetSessionByHandle(ipfHandle);
	if (session) {
		esifHandle = session->esifHandle;
	}
	AppSession_PutRef(session);
	return esifHandle;
}

// Lookup a Session's IPF Client Session Handle by its ESIF Handle (which is unique)
esif_handle_t AppSessionMgr_GetHandleByEsifHandle(esif_handle_t esifHandle)
{
	esif_handle_t ipfHandle = ESIF_INVALID_HANDLE;
	AppSession *session = AppSessionMgr_GetSessionByEsifHandle(esifHandle);
	if (session) {
		ipfHandle = session->ipfHandle;
	}
	AppSession_PutRef(session);
	return ipfHandle;
}

////////////////////////////////////////////////////////////////////////////////
// IPF Server Client Object = App Session identified by IPF Client Server Handle
// The purpose of these functions are to manage both the App Session used by
// the IPF App Session Manager and the ESIF App used by the ESIF App Manager,
// which is controlled through ESIF App Interface Command function calls.
////////////////////////////////////////////////////////////////////////////////

// Create a new Session using the given IPF Client Session Handle and start new ESIF Client App
esif_error_t IpfClient_Start(esif_handle_t ipfHandle, esif_ccb_sockaddr_t sockaddr, esif_handle_t authHandle)
{
	AppSession *self = AppSessionMgr_CreateSession(ipfHandle);
	esif_error_t rc = ESIF_E_NO_CREATE;

	// Call appstart on a separate thread
	if (self) {
		self->sockaddr = sockaddr;
		self->authHandle = authHandle;
		if (IpfSrv_GetInterface().esifIface.fSendCommandFuncPtr && IpfSrv_GetEsifHandle() != ESIF_INVALID_HANDLE) {
			char replybuf[MAX_PATH] = { 0 };
			char command[MAX_PATH] = { 0 };
			int argc = 1;
			EsifData argv[] = { { ESIF_DATA_STRING } };
			EsifData response = { ESIF_DATA_STRING };
			esif_ccb_sprintf(sizeof(self->esifName), self->esifName, "ipfcli-%llx", ipfHandle); // ipfcli-<handle>
			esif_ccb_sprintf(sizeof(command), command, "start appstart @%s=%s", self->esifName, g_ipfAppInfo.appName);
			argv[0].buf_ptr = command;
			argv[0].buf_len = argv[0].data_len = (u32)esif_ccb_strlen(command, sizeof(command));
			response.buf_ptr = replybuf;
			response.buf_len = sizeof(replybuf);

			// This works around a timing issue that mostly affects Linux
			esif_ccb_sleep_msec(100);

			// Start IPF Client Session via ESIF
			rc = IpfSrv_GetInterface().esifIface.fSendCommandFuncPtr(
				IpfSrv_GetEsifHandle(),
				argc,
				argv,
				&response
			);
		}
	}
	return rc;
}

// Has AppCreate completed for the given IPF Client Session Handle?
Bool IpfClient_IsStarted(esif_handle_t ipfHandle)
{
	Bool rc = ESIF_FALSE;
	AppSession *self = AppSessionMgr_GetSessionByHandle(ipfHandle);
	if (self) {
		rc = (Bool)(self->appHandle != ESIF_INVALID_HANDLE);
	}
	AppSession_PutRef(self);
	return rc;
}

// Does the given IPF Client Session Handle currently exist in the App Session Manager as an Active Connection?
Bool IpfClient_Exists(esif_handle_t ipfHandle)
{
	Bool rc = ESIF_FALSE;
	AppSession *self = AppSessionMgr_GetSessionByHandle(ipfHandle);
	if (self) {
		rc = (Bool)atomic_read(&self->connected);
	}
	AppSession_PutRef(self);
	return rc;
}

// Stop the given Session and associated ESIF Client App. Assumes Client Connection has been closed.
esif_error_t IpfClient_Stop(esif_handle_t ipfHandle)
{
	AppSession *self = AppSessionMgr_GetSessionByHandle(ipfHandle);
	esif_error_t rc = ESIF_E_INVALID_HANDLE;

	// Disconnect Client and stop ESIF app
	if (self) {
		Bool isStarted = IpfClient_IsStarted(ipfHandle);

		// Disconnect Client to prevent any more IPC calls
		atomic_set(&self->connected, 0);

		// Close Active Tranactions
		IrpcTransaction *trx = NULL;
		while ((trx = IpfTrxMgr_GetTransaction(IpfTrxMgr_GetInstance(), ipfHandle, IPFTRX_MATCHANY)) != NULL) {
			trx->result = ESIF_E_SESSION_DISCONNECTED;
			signal_post(&trx->sync);
		}

		// Call appstop on a separate thread
		if (IpfSrv_GetInterface().esifIface.fSendCommandFuncPtr && IpfSrv_GetEsifHandle() != ESIF_INVALID_HANDLE && isStarted) {
			char replybuf[MAX_PATH] = { 0 };
			char command[MAX_PATH] = { 0 };
			int argc = 1;
			EsifData argv[] = { { ESIF_DATA_STRING } };
			EsifData response = { ESIF_DATA_STRING };
			esif_ccb_sprintf(sizeof(command), command, "start appstop '%s'", self->esifName); // ipfcli-<handle> or appname
			argv[0].buf_ptr = command;
			argv[0].buf_len = argv[0].data_len = (u32)esif_ccb_strlen(command, sizeof(command)) + 1;
			response.buf_ptr = replybuf;
			response.buf_len = sizeof(replybuf);

			// Stop IPF Client Session via ESIF
			rc = IpfSrv_GetInterface().esifIface.fSendCommandFuncPtr(
				IpfSrv_GetEsifHandle(),
				argc,
				argv,
				&response
			);
		}
		else {
			WebServer_CloseOrphans(g_WebServer);
			esif_ccb_memset(self->appName, 0, sizeof(self->appName));
		}
	}
	AppSession_PutRef(self);
	return rc;
}

// Rename the appname exposed to ESIF
esif_error_t IpfClient_Rename(esif_handle_t ipfHandle, esif_string newName)
{
	AppSession *self = AppSessionMgr_GetSessionByHandle(ipfHandle);
	esif_error_t rc = ESIF_E_INVALID_HANDLE;

	// Call app rename on this thread
	if (self && newName && *newName) {
		if (IpfSrv_GetInterface().esifIface.fSendCommandFuncPtr && IpfSrv_GetEsifHandle() != ESIF_INVALID_HANDLE) {
			char replybuf[MAX_PATH] = { 0 };
			char command[MAX_PATH] = { 0 };
			int argc = 1;
			EsifData argv[] = { { ESIF_DATA_STRING } };
			EsifData response = { ESIF_DATA_STRING };

			char appName[ESIF_NAME_LEN] = { 0 };
			esif_ccb_strcpy(appName, newName, sizeof(appName));
			esif_ccb_strlwr(appName, sizeof(appName));
			esif_ccb_sprintf(sizeof(command), command, "app rename %s '%s'", self->esifName, appName); // ipfcli-<handle> => appname

			argv[0].buf_ptr = command;
			argv[0].buf_len = argv[0].data_len = (u32)esif_ccb_strlen(command, sizeof(command)) + 1;
			response.buf_ptr = replybuf;
			response.buf_len = sizeof(replybuf);

			// Rename IPF Client Session via ESIF
			rc = IpfSrv_GetInterface().esifIface.fSendCommandFuncPtr(
				IpfSrv_GetEsifHandle(),
				argc,
				argv,
				&response
			);

			// If Rename was not successful, get error code and stop Client App
			if (rc == ESIF_OK && esif_ccb_strstr((esif_string)response.buf_ptr, "ESIF_OK") == NULL) {
				rc = ESIF_E_UNSPECIFIED;
				esif_string rcStr = esif_ccb_strchr((esif_string)response.buf_ptr, '(');
				if (rcStr) {
					rc = (esif_error_t)atoi(++rcStr);
					if (rc == ESIF_E_APP_ALREADY_STARTED) {
						rc = ESIF_E_SESSION_ALREADY_STARTED;
					}
				}
				IpfClient_Close(ipfHandle);
			}
		}
	}
	AppSession_PutRef(self);
	return rc;
}

// Close an Active Client Session, Terminating Connection if necessary
esif_error_t IpfClient_Close(esif_handle_t ipfHandle)
{
	AppSession *self = AppSessionMgr_GetSessionByHandle(ipfHandle);
	esif_error_t rc = ESIF_E_INVALID_HANDLE;

	if (self) {
		// Close Active Tranactions
		IrpcTransaction *trx = NULL;
		while ((trx = IpfTrxMgr_GetTransaction(IpfTrxMgr_GetInstance(), self->ipfHandle, IPFTRX_MATCHANY)) != NULL) {
			trx->result = ESIF_E_SESSION_DISCONNECTED;
			signal_post(&trx->sync);
		}

		// Clear IPF Session
		atomic_set(&self->connected, 0);
		self->connectTime = esif_ccb_realtime_null();
		self->updateTime = esif_ccb_realtime_null();
		self->esifHandle = ESIF_INVALID_HANDLE;
		self->appHandle = ESIF_INVALID_HANDLE;
		esif_ccb_memset(self->esifName, 0, sizeof(self->esifName));
		esif_ccb_memset(self->appName, 0, sizeof(self->appName));
		esif_ccb_memset(self->appDescription, 0, sizeof(self->appDescription));
		esif_ccb_memset(self->appVersion, 0, sizeof(self->appVersion));
		esif_ccb_memset(self->appIntro, 0, sizeof(self->appIntro));
		rc = ESIF_OK;
	}
	AppSession_PutRef(self);
	AppSessionMgr_DeleteSession(ipfHandle);

	// Close Orphaned Client Connections
	WebServer_CloseOrphans(g_WebServer);
	return rc;
}
