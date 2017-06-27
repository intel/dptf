/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#ifdef _WIN32
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
	return esif_ws_init();
}

static esif_error_t ESIF_CALLCONV EsifWsExit(void)
{
	esif_ws_exit();
	return ESIF_OK;
}

static esif_error_t ESIF_CALLCONV EsifWsStart(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	esif_ws_server_set_options(self->ipAddr, self->port, self->isRestricted);
	return esif_ws_start();
}

static esif_error_t ESIF_CALLCONV EsifWsStop(void)
{
	esif_ws_stop();
	return ESIF_OK;
}

static esif_error_t EsifWsBroadcast(const u8 *buffer, size_t buf_len)
{
	return esif_ws_broadcast(buffer, buf_len);
}

static Bool EsifWsIsStarted(void)
{
	extern atomic_t g_ws_threads;
	return (atomic_read(&g_ws_threads) > 0);
}

// ESIF_WS -> ESIF_UF Interface Helper Functions

const char *EsifWsDocRoot(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	const char *result = NULL;
	if (self) {
		result = self->docRoot;
	}
	return result;
}

const char *EsifWsLogRoot(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	const char *result = NULL;
	if (self) {
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

void EsifWsShellLock(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	if (self && self->tEsifWsShellLockFuncPtr) {
		self->tEsifWsShellLockFuncPtr();
	}
}

void EsifWsShellUnlock(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	if (self && self->tEsifWsShellUnlockFuncPtr) {
		self->tEsifWsShellUnlockFuncPtr();
	}
}

char *EsifWsShellExec(char *cmd, size_t data_len)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	char *result = NULL;
	if (self && self->tEsifWsShellExecFuncPtr) {
		result = self->tEsifWsShellExecFuncPtr(cmd, data_len);
	}
	return result;
}

size_t EsifWsShellBufLen(void)
{
	EsifWsInterfacePtr self = g_ifaceWs;
	size_t result = 0;
	if (self && self->tEsifWsShellBufLenFuncPtr) {
		result = self->tEsifWsShellBufLenFuncPtr();
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
			g_ifaceWs = ifacePtr;
			rc = ESIF_OK;
		}
	}
	return rc;
}
