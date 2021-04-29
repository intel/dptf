/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#ifdef ESIF_ATTR_USER

#include "esif_ccb.h"
#include "esif_ccb_atomic.h"
#include "esif_sdk_iface.h"

#define WS_LIBRARY_NAME				"esif_ws"			// Name of Loadable Library (.dll or .so)
#define WS_GET_INTERFACE_FUNCTION	"GetWsInterface"	// Interface Function Exported from Loadable Library
#define WS_IFACE_VERSION			4					// Interface Version
#define WS_MAX_REST_RESPONSE		0x7FFFFFFE			// Max REST API Response Length
#define WS_LISTENERS				1					// Max Number of Listener Ports
#define WS_FLAG_NOWHITELIST			0x01				// Do not enforce REST API Whitelist

// Interface Function Prototypes (WS -> ESIF) [Filled by ESIF]
typedef void	(ESIF_CALLCONV *EsifWsLockFunc)(void);
typedef void	(ESIF_CALLCONV *EsifWsUnlockFunc)(void);
typedef Bool	(ESIF_CALLCONV *EsifWsShellEnabledFunc)(void);
typedef char *	(ESIF_CALLCONV *EsifWsShellExecFunc)(char *cmd, size_t cmd_len, char *prefix, size_t prefix_len);
typedef int     (ESIF_CALLCONV *EsifWsTraceMessageFunc)(int level, const char *func, const char *file, int line, const char *msg, va_list arglist);
typedef int     (ESIF_CALLCONV *EsifWsConsoleMessageFunc)(const char *msg, va_list args);

// Interface Function Prototypes (ESIF -> WS) [Filled by Plugin]
typedef esif_error_t (ESIF_CALLCONV *EsifWsInitFunc)(void);
typedef esif_error_t (ESIF_CALLCONV *EsifWsExitFunc)(void);
typedef esif_error_t (ESIF_CALLCONV *EsifWsStartFunc)(void);
typedef esif_error_t (ESIF_CALLCONV *EsifWsStopFunc)(void);
typedef Bool		 (ESIF_CALLCONV *EsifWsIsStartedFunc)(void);
typedef void *       (ESIF_CALLCONV *EsifWsAllocFunc)(size_t buf_len);

#pragma pack(push, 8) // Use 8-byte alignment to satisfy clang 9.x when packing structs with atomic types

// Web Server Interface
typedef struct EsifWsInterface_s {
	EsifIfaceHdr hdr;

	// ESIF_WS -> ESIF_UF Interface Parameters and Functions [Filled by ESIF_UF]
	atomic_t					traceLevel;					// Current ESIF Trace Level
	char						docRoot[MAX_PATH];			// HTTP Document Root
	char						ipAddr[WS_LISTENERS][ESIF_IPADDR_LEN];	// Server IP Address
	u32							port[WS_LISTENERS];						// Server Port Number
	esif_flags_t				flags[WS_LISTENERS];					// Server Flags
	
	EsifWsLockFunc				tEsifWsLockFuncPtr;
	EsifWsUnlockFunc			tEsifWsUnlockFuncPtr;
	EsifWsAllocFunc				tEsifWsAllocFuncPtr;
	EsifWsShellEnabledFunc		tEsifWsShellEnabledFuncPtr;
	EsifWsShellExecFunc			tEsifWsShellExecFuncPtr;
	EsifWsTraceMessageFunc		tEsifWsTraceMessageFuncPtr;
	EsifWsConsoleMessageFunc	tEsifWsConsoleMessageFuncPtr;

	// ESIF_UF -> ESIF_WS Interface Parameters and Functions [Filled by ESIF_WS]
	char						wsVersion[ESIF_VERSION_LEN];

	EsifWsInitFunc				fEsifWsInitFuncPtr;
	EsifWsExitFunc				fEsifWsExitFuncPtr;
	EsifWsStartFunc				fEsifWsStartFuncPtr;
	EsifWsStopFunc				fEsifWsStopFuncPtr;
	EsifWsIsStartedFunc			fEsifWsIsStartedFuncPtr;
	EsifWsAllocFunc				fEsifWsAllocFuncPtr;

} EsifWsInterface, *EsifWsInterfacePtr;

#pragma pack(pop)

typedef esif_error_t (ESIF_CALLCONV *GetWsIfaceFuncPtr)(EsifWsInterfacePtr);

#endif