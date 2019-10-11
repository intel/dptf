/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include <stdarg.h>
#include "esif_ccb_string.h"

#include "esif_sdk_iface_ws.h"
#include "esif_ws_server.h"
#include "esif_ws_version.h"

#ifdef ESIF_ATTR_OS_WINDOWS
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(ul_reason_for_call);
	UNREFERENCED_PARAMETER(lpReserved);
	return TRUE;
}
#endif

// Global ESIF_UF <-> ESIF_WS Interface
static EsifWsInterfacePtr g_ifaceWs = NULL;

// ESIF_UF -> ESIF_WS Interface Callback Functions

static esif_error_t ESIF_CALLCONV EsifWsInit(void)
{
	return WebPlugin_Init();
}

static esif_error_t ESIF_CALLCONV EsifWsExit(void)
{
	WebPlugin_Exit();
	return ESIF_OK;
}

static esif_error_t ESIF_CALLCONV EsifWsStart(void)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifWsInterfacePtr self = g_ifaceWs;
	WebServerPtr server = g_WebServer;
	if (server) {
		u8 instance = 0;
		for (instance = 0; instance < WS_LISTENERS && instance < WS_MAX_LISTENERS; instance++) {
			if (instance == 0 || self->ipAddr[instance][0] || self->port[instance] || self->isRestricted[instance]) {
				rc = WebServer_Config(server, instance, self->ipAddr[instance], (short)self->port[instance], (self->isRestricted[instance] ? ServerRestricted : ServerNormal));
			}
			else {
				WebServer_Config(server, instance, NULL, 0, ServerNormal);
			}
		}
		if (rc == ESIF_OK) {
			rc = WebServer_Start(server);
		}
	}
	return rc;
}

static esif_error_t ESIF_CALLCONV EsifWsStop(void)
{
	WebServer_Stop(g_WebServer);
	return ESIF_OK;
}

static esif_error_t ESIF_CALLCONV EsifWsBroadcast(const u8 *buffer, size_t buf_len)
{
	return WebServer_Broadcast(g_WebServer, buffer, buf_len);
}

static Bool ESIF_CALLCONV EsifWsIsStarted(void)
{
	return WebServer_IsStarted(g_WebServer);
}

static void * ESIF_CALLCONV EsifWsAlloc(size_t buf_len)
{
	return esif_ccb_malloc(buf_len);
}

// ESIF_WS -> ESIF_UF Interface Helper Functions

const char *EsifWsDocRoot(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	const char *result = NULL;
	if (self && self->docRoot[0]) {
		result = self->docRoot;
	}
	return result;
}

const char *EsifWsLogRoot(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	const char *result = NULL;
	if (self && self->logRoot[0]) {
		result = self->logRoot;
	}
	return result;
}

void EsifWsLock(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	if (self && self->tEsifWsLockFuncPtr) {
		self->tEsifWsLockFuncPtr();
	}
}

void EsifWsUnlock(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	if (self && self->tEsifWsUnlockFuncPtr) {
		self->tEsifWsUnlockFuncPtr();
	}
}

Bool EsifWsShellEnabled(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	Bool rc = ESIF_FALSE;
	if (self && self->tEsifWsShellEnabledFuncPtr) {
		rc = self ->tEsifWsShellEnabledFuncPtr();
	}
	return rc;
}

char *EsifWsShellExec(char *cmd, size_t cmd_len, char *prefix, size_t prefix_len)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	char *result = NULL;
	if (self && self->tEsifWsShellExecFuncPtr) {
		result = self->tEsifWsShellExecFuncPtr(cmd, cmd_len, prefix, prefix_len);
	}
	return result;
}

int EsifWsTraceLevel(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	int rc = TRACELEVEL_NONE;
	if (self) {
		rc = (int)atomic_read(&self->traceLevel);
	}
	return rc;
}

int EsifWsTraceMessageEx(
	int level,
	const char *func,
	const char *file,
	int line,
	const char *msg,
	...)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	int rc = 0;
	if (self && self->tEsifWsTraceMessageFuncPtr) {
		va_list args;
		va_start(args, msg);
		rc = self->tEsifWsTraceMessageFuncPtr(level, func, file, line, msg, args);
		va_end(args);
	}
	return rc;
}

int EsifWsConsoleMessageEx(
	const char *msg, 
	...)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	int rc = 0;
	if (self && self->tEsifWsConsoleMessageFuncPtr) {
		va_list args;
		va_start(args, msg);
		rc = self->tEsifWsConsoleMessageFuncPtr(msg, args);
		va_end(args);
	}
	return rc;
}

// Exported Interface Function
ESIF_EXPORT esif_error_t ESIF_CALLCONV GetWsInterface(EsifWsInterfacePtr ifacePtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (ifacePtr) {
		// Must be an exact interface match
		if ((ifacePtr->hdr.fIfaceType != eIfaceTypeWeb) ||
			(ifacePtr->hdr.fIfaceVersion != WS_IFACE_VERSION) ||
			(ifacePtr->hdr.fIfaceSize != sizeof(*ifacePtr))) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else {
			// Fill in ESIF_WS side of Interface
			esif_ccb_strcpy(ifacePtr->wsVersion, ESIF_WS_VERSION, sizeof(ifacePtr->wsVersion));
			ifacePtr->fEsifWsInitFuncPtr = EsifWsInit;
			ifacePtr->fEsifWsExitFuncPtr = EsifWsExit;
			ifacePtr->fEsifWsStartFuncPtr = EsifWsStart;
			ifacePtr->fEsifWsStopFuncPtr = EsifWsStop;
			ifacePtr->fEsifWsBroadcastFuncPtr = EsifWsBroadcast;
			ifacePtr->fEsifWsIsStartedFuncPtr = EsifWsIsStarted;
			ifacePtr->fEsifWsAllocFuncPtr = EsifWsAlloc;
			g_ifaceWs = ifacePtr;
			rc = ESIF_OK;
		}
	}
	return rc;
}
