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

#include "esif_ccb_library.h"
#include "esif_ccb_time.h"
#include "esif_sdk_event_map.h"

#include "ipf_appinfo.h"
#include "ipf_lifecycle.h"
#include "ipf_trace.h"
#include "ipf_sdk_version_check.h"

#include "ipfsrv_appmgr.h"
#include "ipfsrv_authmgr.h"
#include "ipfsrv_ws_server.h"


#define IPFSRV_COPYRIGHT	"Copyright (c) 2013-2023 Intel Corporation All Rights Reserved"

// IPF Server LifeCycle Initializiation Table
static LifecycleTableEntry g_lifecycleTableSrv[] = {
	// Init and Exit functions should all be before Start/Stop functions
	{ AppSessionMgr_Init,		AppSessionMgr_Exit,			LIFECYCLE_FLAG_NONE },

	// Start/Stop functions after Init/Exit functions
	{ AppSessionMgr_Start,		AppSessionMgr_Stop,			LIFECYCLE_FLAG_NONE },

	// Mark end of table with both entries NULL
	{ NULL,						NULL,						LIFECYCLE_FLAG_NONE },
};

// IPF Server Application Object
typedef struct IpfSrvApp_s {
	esif_handle_t	esifHandle;
	esif_handle_t	appHandle;
	char			*appPathHome;
	DCfgOptions		dcfg;
	Bool			isConjured;
} IpfSrvApp;

// Global IPF Server Singleton Instance
static IpfSrvApp g_ipfsrv = {
	.esifHandle = ESIF_INVALID_HANDLE,
	.appHandle = ESIF_INVALID_HANDLE,
	.appPathHome = NULL,
};

///////////////////////////////////////////////////////////////////////////////

esif_error_t Irpc_SendMessage(IrpcTransaction *trx)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (trx && trx->request) {
		AppSession *session = AppSessionMgr_GetSessionByHandle(trx->ipfHandle);
		if (!AppSession_IsConnected(session)) {
			rc = ESIF_E_SESSION_DISCONNECTED;
		}
		else if (IBinary_GetLen(trx->request)) {
			EsifMsgHdr header = {
				.v1.signature = ESIFMSG_SIGNATURE,
				.v1.headersize = sizeof(header),
				.v1.version = ESIFMSG_VERSION,
				.v1.msgclass = ESIFMSG_CLASS_IRPC
			};
			header.v1.msglen = (UInt32)IBinary_GetLen(trx->request);

			if (IBinary_Insert(trx->request, &header, sizeof(header), 0) != NULL) {
				rc = WebServer_IrpcRequest(trx);
				if (rc == ESIF_OK) {
					session->updateTime = esif_ccb_realtime_current();
				}
			}
		}
		AppSession_PutRef(session);
	}
	return rc;
}

////////////////////////////////////////////

/*
** IpfSrv Initialization and Trace Log functions
*/

static atomic_t g_initialized = ATOMIC_INIT(0);

static esif_error_t IpfSrv_TraceMessage(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr message,
	const eLogType logType
)
{
	esif_error_t rc = ESIF_E_NOT_INITIALIZED;

	UNREFERENCED_PARAMETER(esifHandle);

	if (g_ipfsrv.esifHandle != ESIF_INVALID_HANDLE && IpfSrv_GetInterface().esifIface.fWriteLogFuncPtr) {
		rc = IpfSrv_GetInterface().esifIface.fWriteLogFuncPtr(
			g_ipfsrv.esifHandle,
			participantHandle,
			domainHandle,
			message,
			logType
		);
	}
	return rc;
}

// Main Application Initialization function
esif_error_t IpfSrv_OnLoad()
{
	esif_error_t rc = ESIF_OK;

	if (atomic_inc(&g_initialized) == 1) {
		// Initialize Memory Leak Tracing for Debug Builds
		rc = esif_memtrace_init();
		esif_error_t memrc = rc;

		// Intialize Trace Logging
		if (rc == ESIF_OK) {
			rc = IpfTrace_Init();
			if (rc == ESIF_OK) {
				IpfTrace_SetConfig(g_ipfAppInfo.appName, IpfSrv_TraceMessage, ESIF_FALSE);
			}
		}
		if (rc != ESIF_OK) {
			if (memrc == ESIF_OK) {
				esif_memtrace_exit();
			}
			atomic_dec(&g_initialized);
		}
	}
	return rc;
}

// Main Application Unintialization Function
void IpfSrv_OnUnload()
{
	if (atomic_dec(&g_initialized) == 0) {
		// Unintialize Trace Logging
		IpfTrace_Exit();

		// Uninitialize Memory Leak Tracing
		esif_memtrace_exit();
	}
}

// Return IPF Server DCFG Options
DCfgOptions IpfSrv_GetDcfg()
{
	return g_ipfsrv.dcfg;
}

// Set IPF Server DCFG Options
void IpfSrv_SetDcfg(DCfgOptions opt)
{
	g_ipfsrv.dcfg = opt;
}

// Return IPF Server ESIF Handle (assigned by ESIF)
esif_handle_t IpfSrv_GetEsifHandle()
{
	return g_ipfsrv.esifHandle;
}

// Return IPF Server Conjured UF Option
Bool IpfSrv_GetConjured()
{
	return g_ipfsrv.isConjured;
}

// Return IPF Server Conjured UF Option
void IpfSrv_SetConjured(Bool opt)
{
	g_ipfsrv.isConjured = opt;
}


// Check whether the given App Key is Installed
Bool IpfSrv_IsAppInstalled(const char* appKey)
{
	// In Linux, check whether Apps are installed by checking whether the AppKey (library.so) exists in the same directory as ipfsrv.so
	Bool result = ESIF_TRUE;
	if (appKey) {
		esif_lib_t lib = esif_ccb_library_load(NULL);
		if (lib) {
			char pathname[MAX_PATH] = {0};
			esif_error_t rc = esif_ccb_library_getpath(lib, pathname, sizeof(pathname), (void *)IpfSrv_IsAppInstalled);
			char *slash = NULL;
			if (rc == ESIF_OK && ((slash = esif_ccb_strrchr(pathname, *ESIF_PATH_SEP)) != NULL)) {
				slash[1] = 0;
				esif_ccb_strcpy(slash + 1, appKey, sizeof(pathname) - esif_ccb_strlen(pathname, sizeof(pathname)));
				if (!esif_ccb_file_exists(pathname)) {
					result = ESIF_FALSE;
				}
			}
			esif_ccb_library_unload(lib);
		}
	}
	return result;
}

/*
** IpfSrv_App* functions are the App Interface functions exposed to ESIF
** by the Server Instance (I/O Thread) of the IPFSRV Shared Library.
** TODO - move to ipfsrv_iface.c
*/
esif_error_t ESIF_CALLCONV IpfSrv_AppDestroy(esif_handle_t appHandle);

// Get Server Name. May be called before AppCreate
esif_error_t ESIF_CALLCONV IpfSrv_AppGetName(EsifDataPtr appNamePtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (appNamePtr && appNamePtr->buf_ptr) {
		const char *appName = (g_ipfAppInfo.appName ? g_ipfAppInfo.appName : "NA");
		u32 appNameLen = (u32)esif_ccb_strlen(appName, ESIF_NAME_LEN) + 1;
		if (appNamePtr->buf_len < appNameLen) {
			appNamePtr->data_len = appNameLen;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		else {
			esif_ccb_strcpy((esif_string)appNamePtr->buf_ptr, appName, appNamePtr->buf_len);
			appNamePtr->data_len = (UInt32)esif_ccb_strlen((const char*)appNamePtr->buf_ptr, appNamePtr->buf_len) + 1;
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Get Server Version. May be called before AppCreate
esif_error_t ESIF_CALLCONV IpfSrv_AppGetVersion(EsifDataPtr appVersionPtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (appVersionPtr && appVersionPtr->buf_ptr) {
		const char *appVersion = (g_ipfAppInfo.appVersion ? g_ipfAppInfo.appVersion : "NA");
		u32 appVersionLen = (u32)esif_ccb_strlen(appVersion, ESIF_VERSION_LEN) + 1;
		if (appVersionPtr->buf_len < appVersionLen) {
			appVersionPtr->data_len = appVersionLen;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		else {
			esif_ccb_strcpy((esif_string)appVersionPtr->buf_ptr, appVersion, appVersionPtr->buf_len);
			appVersionPtr->data_len = (UInt32)esif_ccb_strlen((const char*)appVersionPtr->buf_ptr, appVersionPtr->buf_len) + 1;
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Get Server Description. May be called before AppCreate
esif_error_t ESIF_CALLCONV IpfSrv_AppGetDescription(EsifDataPtr appDescriptionPtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (appDescriptionPtr && appDescriptionPtr->buf_ptr) {
		const char *appDescription = (g_ipfAppInfo.appDescription ? g_ipfAppInfo.appDescription : "NA");
		u32 appDescriptionLen = (u32)esif_ccb_strlen(appDescription, ESIF_DESC_LEN) + 1;
		if (appDescriptionPtr->buf_len < appDescriptionLen) {
			appDescriptionPtr->data_len = appDescriptionLen;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		else {
			esif_ccb_strcpy((esif_string)appDescriptionPtr->buf_ptr, appDescription, appDescriptionPtr->buf_len);
			appDescriptionPtr->data_len = (UInt32)esif_ccb_strlen((const char*)appDescriptionPtr->buf_ptr, appDescriptionPtr->buf_len) + 1;
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Get Server Intro Banner. May be called before or after AppCreate
esif_error_t ESIF_CALLCONV IpfSrv_AppGetIntro(
	const esif_handle_t appHandle,
	EsifDataPtr appBannerPtr
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	UNREFERENCED_PARAMETER(appHandle);

	if (appBannerPtr && appBannerPtr->buf_ptr) {
		const char *appBanner = (g_ipfAppInfo.appBanner ? g_ipfAppInfo.appBanner : "");
		u32 appBannerLen = (u32)esif_ccb_strlen(appBanner, 0xFFFF) + 1;

		if (appBannerPtr->buf_len < appBannerLen) {
			appBannerPtr->data_len = appBannerLen;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		else {
			esif_ccb_strcpy((esif_string)appBannerPtr->buf_ptr, appBanner, appBannerPtr->buf_len);
			appBannerPtr->data_len = (UInt32)esif_ccb_strlen((const char*)appBannerPtr->buf_ptr, (size_t)appBannerPtr->buf_len) + 1;
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Get Server Application Status
esif_error_t ESIF_CALLCONV IpfSrv_AppGetStatus(
	const esif_handle_t appHandle,
	const eAppStatusCommand command,
	const UInt32 appStatusIn,
	EsifDataPtr appStatusOut
)
{
	// Ignore
	esif_error_t rc = ESIF_OK;
	UNREFERENCED_PARAMETER(appHandle);
	UNREFERENCED_PARAMETER(command);
	UNREFERENCED_PARAMETER(appStatusIn);
	UNREFERENCED_PARAMETER(appStatusOut);
	return rc;
}


// Server Application Create Instance
esif_error_t ESIF_CALLCONV IpfSrv_AppCreate(
	AppInterfaceSetPtr ifaceSetPtr,
	const esif_handle_t esifHandle,
	esif_handle_t *appHandlePtr,
	const AppDataPtr appDataPtr,
	const eAppState appInitialState
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	
	UNREFERENCED_PARAMETER(appInitialState);

	if (ifaceSetPtr && appHandlePtr && ((rc = IpfSrv_OnLoad()) == ESIF_OK)) {
		g_ipfsrv.esifHandle = esifHandle;
		g_ipfsrv.appHandle = AppSessionMgr_GenerateHandle();
		*appHandlePtr = g_ipfsrv.appHandle;

		if (appDataPtr) {
			IpfTrace_SetTraceLevel(appDataPtr->fLogLevel);
			if (appDataPtr->fPathHome.buf_ptr) {
				esif_ccb_free(g_ipfsrv.appPathHome);
				g_ipfsrv.appPathHome = esif_ccb_strdup((const char *)appDataPtr->fPathHome.buf_ptr);
			}
		}

		// Perform ESIF Services initialization, which requires an ESIF Interface Set
		if ((ifaceSetPtr->hdr.fIfaceType != eIfaceTypeApplication) ||
			(ifaceSetPtr->hdr.fIfaceVersion != APP_INTERFACE_VERSION) ||
			(ifaceSetPtr->hdr.fIfaceSize != (UInt16)sizeof(AppInterfaceSet))) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else {
			AppInterfaceSet ifaceSet = IpfSrv_GetInterface();
			ifaceSet.esifIface = ifaceSetPtr->esifIface;
			IpfSrv_SetInterface(&ifaceSet);
			rc = ESIF_OK;
		}

		// Get System DCFG Setting. Do not fail AppCreate on error
		if (rc == ESIF_OK && IpfSrv_GetInterface().esifIface.fPrimitiveFuncPtr) {
			DCfgOptions dcfg = { 0 };
			EsifData request = { ESIF_DATA_VOID };
			EsifData response = { ESIF_DATA_UINT32 };
			response.buf_ptr = &dcfg.asU32;
			response.buf_len = (u32)sizeof(dcfg);
			esif_error_t dcfgrc = IpfSrv_GetInterface().esifIface.fPrimitiveFuncPtr(
				g_ipfsrv.esifHandle,
				ESIF_HANDLE_PRIMARY_PARTICIPANT,
				ESIF_INVALID_HANDLE,
				&request,
				&response,
				GET_CONFIG_ACCESS_CONTROL_SUR,
				ESIF_INSTANCE_INVALID
			);

			if (dcfgrc == ESIF_OK) {
				IpfSrv_SetDcfg(dcfg);
			}
			if (dcfgrc == ESIF_E_NO_LOWER_FRAMEWORK) {
				IpfSrv_SetConjured(ESIF_TRUE);
			}
		}

		// Register for Server Events
		if (rc == ESIF_OK && IpfSrv_GetInterface().esifIface.fRegisterEventFuncPtr) {
			UInt8 serverEvents[][ESIF_GUID_LEN] = {
				LOG_VERBOSITY_CHANGED,
#ifdef IPF_FEAT_OPT_DISABLE_STANDBY_TIMEOUTS
				WINDOWS_LOW_POWER_MODE_ENTRY,
				WINDOWS_LOW_POWER_MODE_EXIT,
				DISPLAY_ON,
				DISPLAY_OFF,
#endif
			};
			for (int j = 0; j < ESIF_ARRAY_LEN(serverEvents); j++) {
				EsifData eventGuid = { ESIF_DATA_GUID, NULL, ESIF_GUID_LEN, ESIF_GUID_LEN };
				eventGuid.buf_ptr = serverEvents[j];
				rc = IpfSrv_GetInterface().esifIface.fRegisterEventFuncPtr(
					g_ipfsrv.esifHandle,
					ESIF_HANDLE_PRIMARY_PARTICIPANT,
					ESIF_INVALID_HANDLE,
					&eventGuid
				);
			}
		}

		// Perform Lifecycle Initialization functions
		if (rc == ESIF_OK) {
			rc = LifecycleMgr_Init(g_lifecycleTableSrv);
		}

		// Cleanup on Error since AppDestroy will not be called
		if (rc != ESIF_OK) {
			IpfSrv_AppDestroy(ESIF_INVALID_HANDLE);
		}
	}
	return rc;
}

// Server Application Destroy Instance
esif_error_t ESIF_CALLCONV IpfSrv_AppDestroy(esif_handle_t appHandle)
{
	esif_error_t rc = ESIF_OK;

	UNREFERENCED_PARAMETER(appHandle);

	// Execute de-initialization functions
	LifecycleMgr_Exit(g_lifecycleTableSrv);

	esif_ccb_free(g_ipfsrv.appPathHome);
	g_ipfsrv.appPathHome = NULL;
	g_ipfsrv.esifHandle = ESIF_INVALID_HANDLE;

	AppInterfaceSet ifaceSet = IpfSrv_GetInterface();
	esif_ccb_memset(&ifaceSet.esifIface, 0, sizeof(ifaceSet.esifIface));
	IpfSrv_SetInterface(&ifaceSet);

	IpfSrv_OnUnload();
	return rc;
}

// Server Application Command
esif_error_t ESIF_CALLCONV IpfSrv_AppCommand(
	const esif_handle_t appHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr responsePtr
)
{
	esif_error_t rc = ESIF_E_NOT_IMPLEMENTED;

	UNREFERENCED_PARAMETER(appHandle);

	// Process IPF Server Commands. Pass all others to Client
	if (argc > 0 && argv[0].buf_ptr && argv[0].type == ESIF_DATA_STRING && responsePtr && responsePtr->buf_ptr && responsePtr->buf_len) {
		esif_string opcode = argv[0].buf_ptr;
		esif_string optarg = (argc > 1 && argv[1].buf_ptr && argv[1].type == ESIF_DATA_STRING ? argv[1].buf_ptr : NULL);

		// ipfsrv app-started <appname>
		if (esif_ccb_stricmp(opcode, "app-started") == 0) {
			responsePtr->data_len = (u32)esif_ccb_sprintf(responsePtr->buf_len, responsePtr->buf_ptr, "%s\n", esif_rc_str(rc)) + 1;
			rc = ESIF_OK;
		}
		// ipfsrv <about|help>
		else if (esif_ccb_stricmp(opcode, "about") == 0 || esif_ccb_stricmp(opcode, "help") == 0) {
			responsePtr->data_len = (u32)esif_ccb_sprintf(responsePtr->buf_len, responsePtr->buf_ptr,
				"%s (SDK %s)\n"
				"%s\n"
				"\n"
				"Available Commands:\n"
				"  about              Display Help Text\n"
				"  help               Display Help Text\n"
				"  sdk [version]      Display or Verify IPF SDK Version\n"
				"  apps [appspec]     Display Connected Client App(s)\n"
				"  status [appspec]   Display Details for matching Client App(s) [or --server]\n"
				"  stop [appname|id]  Stop and Close a Client App by Name, 0xHandle, or @ID\n"
				"  close [appname|id] Close a Client App by Name, 0xHandle, or @ID [No AppStop]\n"
				"  pause              Pause accepting new Client connections\n"
				"  resume             Resume accepting new Client connections\n"
				"  queue [limit]      Get or Set Request Queue Limit\n"
				"  timeout [seconds]  Get or Set Server RPC Session and Transaction Timeout\n"
				"\n"
				, g_ipfAppInfo.appBanner
				, IPF_SDK_VERSION
				, IPFSRV_COPYRIGHT
			) + 1;
			rc = ESIF_OK;
		}
		// ipfsrv sdk [<version>]
		// ipfsrv sdk-version [<version>]
		else if (esif_ccb_stricmp(opcode, "sdk-version") == 0 || esif_ccb_stricmp(opcode, "sdk") == 0) {
			char serverSdkVersion[] = IPF_SDK_VERSION;
			char *clientSdkVersion = NULL;
			rc = ESIF_OK;

			// sdk-version <client-version>
			if (optarg) {
				clientSdkVersion = optarg;
				rc = IpfSdk_VersionCheck(serverSdkVersion, clientSdkVersion);
			}

			if (rc == ESIF_OK) {
				u32 newline = (esif_ccb_stricmp(opcode, "sdk") == 0 ? 1 : 0);
				u32 data_len = (u32)sizeof(serverSdkVersion) + newline;
				if (responsePtr->buf_len < data_len) {
					responsePtr->data_len = data_len;
					rc = ESIF_E_NEED_LARGER_BUFFER;
				}
				else {
					esif_ccb_strcpy(responsePtr->buf_ptr, serverSdkVersion, responsePtr->buf_len);
					if (newline) {
						esif_ccb_strcat(responsePtr->buf_ptr, "\n", responsePtr->buf_len);
					}
					responsePtr->data_len = data_len;
				}
			}
		}
		// ipfsrv <apps|clients> [<appspec>]
		else if (esif_ccb_stricmp(opcode, "apps") == 0 || esif_ccb_stricmp(opcode, "clients") == 0) {
			int data_len = esif_ccb_sprintf(responsePtr->buf_len, responsePtr->buf_ptr,
				"\n"
				"ID Name         Description                                Version          State   Connected    Idle\n"
				"-- ------------ ------------------------------------------ ---------------- ------- ------------ -----------------\n"
			);

			esif_ccb_read_lock(&g_sessionMgr.lock);
			esif_ccb_realtime_t now = esif_ccb_realtime_current();
			for (int j = 0; j < IPFSRV_MAX_SESSIONS; j++) {
				AppSession *session = g_sessionMgr.sessions[j];
				if (session && (optarg == NULL || esif_ccb_strmatch(session->appName, optarg))) {
					Bool isConnected = AppSession_IsConnected(session);
					Int64 connectTime = (isConnected ? esif_ccb_realtime_diff_sec(session->connectTime, now) : 0);
					double idleTime = (isConnected ? esif_ccb_realtime_diff_msec(session->updateTime, now) / 1000 : 0);
					data_len += esif_ccb_sprintf_concat(responsePtr->buf_len, responsePtr->buf_ptr,
						"%02d %-12.12s %-42.42s %-16.16s %-7.7s %03lld:%02lld:%02lld:%02lld %03d:%02d:%02d:%07.4lf\n"
						, j
						, session->esifName
						, session->appDescription
						, session->appVersion
						, (IpfClient_IsStarted(session->ipfHandle) ? "Started" : "Stopped")
						, connectTime / 86400, connectTime / 3600 % 3600, connectTime / 60 % 60, connectTime % 60
						, (int)(idleTime / 86400), (int)(idleTime / 3600) % 3600, (int)(idleTime / 60) % 60, (double)((double)(idleTime / 60) - (Int64)(idleTime / 60)) * 60
					);
				}
			}
			data_len += esif_ccb_sprintf_concat(responsePtr->buf_len, responsePtr->buf_ptr, "\n");
			esif_ccb_read_unlock(&g_sessionMgr.lock);

			responsePtr->data_len = (u32)data_len + 1;
			rc = ESIF_OK;
		}
		// ipfsrv status [<appspec|--server>]
		else if (esif_ccb_stricmp(opcode, "status") == 0 || esif_ccb_stricmp(opcode, "stats") == 0) {
			int data_len = esif_ccb_sprintf(responsePtr->buf_len, responsePtr->buf_ptr, "\n");
			if (optarg == NULL || esif_ccb_stricmp(optarg, "--server") == 0) {
				data_len += esif_ccb_sprintf_concat(responsePtr->buf_len, responsePtr->buf_ptr,
					"Server       : %s\n"
					"  Name       : %s\n"
					"  Description: %s\n"
					"  Platform   : %s %s %s\n"
					"  Version    : %s\n"
					"  SDK Version: %s\n"
					"  ESIF Handle: 0x%llx\n"
					"  App Handle : 0x%llx\n"
					, (AppSessionMgr_IsStarted() ? (WebServer_IsPaused(g_WebServer) ? "Paused" : "Started") : "Stopped")
					, g_ipfAppInfo.appName
					, g_ipfAppInfo.appDescription
					, ESIF_ATTR_OS
					, ESIF_PLATFORM_TYPE
					, ESIF_BUILD_TYPE
					, g_ipfAppInfo.appVersion
					, IPF_SDK_VERSION
					, g_ipfsrv.esifHandle
					, g_ipfsrv.appHandle
				);

				// Display Active Listeners
				const AccessDef* listeners = AccessMgr_GetAccessDefs();
				if (listeners) {
					atomic_t activeListeners = WebServer_GetListenerMask(g_WebServer);
					atomic_t activePipes = WebServer_GetPipeMask(g_WebServer);
					for (size_t bit = 0; bit < IPF_WS_MAX_LISTENERS && listeners[bit].serverAddr; bit++) {
						if (activeListeners & ((size_t)1 << bit)) {
							size_t pad = 24;
							size_t len = esif_ccb_strlen(listeners[bit].serverAddr, pad);
							esif_string authname = AuthMgr_GetAuthNameByAuthHandle(listeners[bit].authHandle);
							data_len += esif_ccb_sprintf_concat(responsePtr->buf_len, responsePtr->buf_ptr,
								"  Address[%02zd]: %s%*.*s[%s]"
								, bit
								, listeners[bit].serverAddr
								, (int)(pad - len), (int)(pad - len), ""
								, authname
							);
							if (listeners[bit].accessControl) {
								pad = 12;
								len = esif_ccb_strlen(authname, pad);
								data_len += esif_ccb_sprintf_concat(responsePtr->buf_len, responsePtr->buf_ptr,
									"%*.*s(%s)"
									, (int)(pad - len), (int)(pad - len), ""
									, (activePipes & ((size_t)1 << bit) ? listeners[bit].accessControl : IPFAUTH_ACL_NOBODY)
								);
							}
							data_len += esif_ccb_sprintf_concat(responsePtr->buf_len, responsePtr->buf_ptr, "\n");
						}
					}
				}
				data_len += esif_ccb_sprintf_concat(responsePtr->buf_len, responsePtr->buf_ptr, "\n");
			}

			esif_ccb_read_lock(&g_sessionMgr.lock);
			esif_ccb_realtime_t now = esif_ccb_realtime_current();
			for (int j = 0; j < IPFSRV_MAX_SESSIONS; j++) {
				AppSession *session = g_sessionMgr.sessions[j];
				if (session && (optarg == NULL || esif_ccb_strmatch(session->appName, optarg))) {
					char clientAddr[MAX_PATH] = { 0 };
					IpfIpc_SockaddrAddr(session->sockaddr, clientAddr, sizeof(clientAddr));
					Bool isConnected = AppSession_IsConnected(session);
					Int64 connectTime = (isConnected ? esif_ccb_realtime_diff_sec(session->connectTime, now) : 0);
					double idleTime = (isConnected ? esif_ccb_realtime_diff_msec(session->updateTime, now) / 1000 : 0);
					data_len += esif_ccb_sprintf_concat(responsePtr->buf_len, responsePtr->buf_ptr,
						"Client[%02d]   : %s\n"
						"  App State  : %s\n"
						"  Address    : %s\n"
						"  Connected  : %03lld:%02lld:%02lld:%02lld\n"
						"  Idle       : %03d:%02d:%02d:%07.4lf\n"
						"  IPF Handle : 0x%llx\n"
						"  ESIF Handle: 0x%llx\n"
						"  App Handle : 0x%llx\n"
						"  Auth Handle: 0x%llx\n"
						"  Auth Role  : %s\n"
						"  ESIF Name  : %s\n"
						"  App Name   : %s\n"
						"  Description: %s\n"
						"  Version    : %s\n"
						"  Intro      : %s\n"
						"\n"
						, j
						, (isConnected ? "Connected" : "Disconnected")
						, (IpfClient_IsStarted(session->ipfHandle) ? "Started" : "Stopped")
						, clientAddr
						, connectTime / 86400, connectTime / 3600 % 3600, connectTime / 60 % 60, connectTime % 60
						, (int)(idleTime / 86400), (int)(idleTime / 3600) % 3600, (int)(idleTime / 60) % 60, (double)((double)(idleTime / 60) - (Int64)(idleTime / 60)) * 60
						, session->ipfHandle
						, session->esifHandle
						, session->appHandle
						, session->authHandle
						, AuthMgr_GetAuthNameByEsifHandle(session->esifHandle)
						, session->esifName
						, session->appName
						, session->appDescription
						, session->appVersion
						, session->appIntro
					);
				}
			}
			esif_ccb_read_unlock(&g_sessionMgr.lock);

			responsePtr->data_len = (u32)data_len + 1;
			rc = ESIF_OK;
		}
		// iprsrv <stop|close> <appname|handle|@id>
		else if (esif_ccb_stricmp(opcode, "stop") == 0 || esif_ccb_stricmp(opcode, "close") == 0) {
			esif_handle_t handle = ESIF_INVALID_HANDLE;

			if (optarg == NULL) {
				rc = ESIF_E_INVALID_ARGUMENT_COUNT;
			}
			else if (esif_ccb_strncmp(optarg, "0x", 2) == 0) {
				esif_ccb_sscanf(optarg + 2, "%llx", &handle);
			}
			else if (*optarg == '@') {
				size_t id = 0;
				esif_ccb_sscanf(optarg + 1, "%zd", &id);
				AppSession *session = AppSessionMgr_GetSessionById(id);
				if (session) {
					handle = session->ipfHandle;
				}
				AppSession_PutRef(session);
			}
			else {
				AppSession *session = AppSessionMgr_GetSessionByName(optarg);
				if (session) {
					handle = session->ipfHandle;
				}
				AppSession_PutRef(session);
			}
			if (handle && handle != ESIF_INVALID_HANDLE) {
				if (esif_ccb_stricmp(opcode, "stop") == 0) {
					rc = IpfClient_Stop(handle);
				}
				else {
					rc = IpfClient_Close(handle);
				}
			}
			else {
				rc = ESIF_E_NOT_FOUND;
			}
			responsePtr->data_len = (u32)esif_ccb_sprintf(responsePtr->buf_len, responsePtr->buf_ptr, "%s\n", esif_rc_str(rc)) + 1;
		}
		// ipfsrv <pause|resume>
		else if (esif_ccb_stricmp(opcode, "pause") == 0 || esif_ccb_stricmp(opcode, "resume") == 0) {
			if (esif_ccb_stricmp(opcode, "pause") == 0) {
				WebServer_Pause(g_WebServer);
			}
			else {
				WebServer_Resume(g_WebServer);
			}
			responsePtr->data_len = (u32)esif_ccb_sprintf(responsePtr->buf_len, responsePtr->buf_ptr, "Server %s\n", (AppSessionMgr_IsStarted() ? (WebServer_IsPaused(g_WebServer) ? "Paused" : "Running") : "Stopped")) + 1;
			rc = ESIF_OK;
		}
		// ipfsrv queue [<limit>]
		else if (esif_ccb_stricmp(opcode, "queue") == 0) {
			// ipfsrv queue <limit>
			if (optarg) {
				size_t maxQueue = (size_t)atoi(optarg);
				if (maxQueue) {
					WebServer_SetRpcQueueMax(g_WebServer, maxQueue);
				}
			}
			responsePtr->data_len = (u32)esif_ccb_sprintf(responsePtr->buf_len, responsePtr->buf_ptr, ATOMIC_FMT "\n", WebServer_GetRpcQueueMax(g_WebServer)) + 1;
			rc = ESIF_OK;
		}
		// ipfsrv timeout [<seconds>]
		else if (esif_ccb_stricmp(opcode, "timeout") == 0) {
			// ipfsrv timeout <seconds>
			if (optarg) {
				size_t timeout = (size_t)atoi(optarg);
				AppSessionMgr_SetTimeout(timeout);
			}
			responsePtr->data_len = (u32)esif_ccb_sprintf(responsePtr->buf_len, responsePtr->buf_ptr, "%zd\n", AppSessionMgr_GetTimeout()) + 1;
			rc = ESIF_OK;
		}
#ifdef ESIF_ATTR_DEBUG
		// ipfsrv leak
		else if (esif_ccb_stricmp(opcode, "leak") == 0) {
			// Intentionally Create a Memory Leak for Debug Builds
			void *leak = esif_ccb_malloc(42);
			UNREFERENCED_PARAMETER(leak);
			responsePtr->data_len = (u32)esif_ccb_sprintf(responsePtr->buf_len, responsePtr->buf_ptr, "Memory Leak Created\n") + 1;
			rc = ESIF_OK;
		}
#endif
	}
	return rc;
}

// Server Application Event
esif_error_t ESIF_CALLCONV IpfSrv_AppEvent(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventDataPtr,
	const EsifDataPtr eventGuid
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	esif_event_type_t eventType = (esif_event_type_t)(-1);

	UNREFERENCED_PARAMETER(appHandle);
	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(domainHandle);

	if (eventGuid && eventGuid->buf_ptr) {
		if (eventGuid->buf_len < sizeof(esif_guid_t)) {
			rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		}
		else if (!esif_event_map_guid2type((esif_guid_t*)eventGuid->buf_ptr, (esif_event_type_t*)&eventType)) {
			rc = ESIF_E_EVENT_NOT_FOUND;
		}
		else {
			switch (eventType) {
			case ESIF_EVENT_LOG_VERBOSITY_CHANGED:
				if (eventDataPtr && eventDataPtr->buf_ptr && (eventDataPtr->buf_len >= sizeof(int))) {
					int level = *(int*)(eventDataPtr->buf_ptr);
					IpfTrace_SetTraceLevel(level);
					IPF_TRACE_INFO("Setting Trace Level in Server to %d\n", level);
				}
				break;
			case ESIF_EVENT_DISPLAY_OFF:
			case ESIF_EVENT_WINDOWS_LOW_POWER_MODE_ENTRY:
				AppSessionMgr_SuspendTimeouts();
				break;
			case ESIF_EVENT_DISPLAY_ON:
			case ESIF_EVENT_WINDOWS_LOW_POWER_MODE_EXIT:
				AppSessionMgr_ResumeTimeouts();
				break;
			default:
				break;
			}
		}
	}
	return rc;
}

// Server Application Suspend
esif_error_t ESIF_CALLCONV IpfSrv_AppSuspend(const esif_handle_t appHandle)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	if (appHandle == g_ipfsrv.appHandle) {
		WebServer_Pause(g_WebServer);
		rc = ESIF_OK;
	}
	return rc;
}

// Server Application Resume
esif_error_t ESIF_CALLCONV IpfSrv_AppResume(const esif_handle_t appHandle)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	if (appHandle == g_ipfsrv.appHandle) {
		WebServer_Resume(g_WebServer);
		rc = ESIF_OK;
	}
	return rc;
}

// Server Participant Create
esif_error_t ESIF_CALLCONV IpfSrv_AppParticipantCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const AppParticipantDataPtr participantDataPtr,
	const eParticipantState initialParticipantState
)
{
	// Ignore
	esif_error_t rc = ESIF_OK;
	UNREFERENCED_PARAMETER(appHandle);
	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(participantDataPtr);
	UNREFERENCED_PARAMETER(initialParticipantState);
	return rc;
}

// Server Participant Destroy
esif_error_t ESIF_CALLCONV IpfSrv_AppParticipantDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle
)
{
	// Ignore
	esif_error_t rc = ESIF_OK;
	UNREFERENCED_PARAMETER(appHandle);
	UNREFERENCED_PARAMETER(participantHandle);
	return rc;
}

// Server Domain Create
esif_error_t ESIF_CALLCONV IpfSrv_AppDomainCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const AppDomainDataPtr domainDataPtr,
	const eDomainState domainInitialState
)
{
	// Ignore
	esif_error_t rc = ESIF_OK;
	UNREFERENCED_PARAMETER(appHandle);
	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(domainHandle);
	UNREFERENCED_PARAMETER(domainDataPtr);
	UNREFERENCED_PARAMETER(domainInitialState);
	return rc;
}

esif_error_t ESIF_CALLCONV IpfSrv_AppDomainDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle
)
{
	// Ignore
	esif_error_t rc = ESIF_OK;
	UNREFERENCED_PARAMETER(appHandle);
	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(domainHandle);
	return rc;
}


/////////////////////////////////////////////////////////////////////////////////

// App Interface Set used by IPFSRV Server Application
static AppInterfaceSet g_ifaceSetServer = {
	.hdr = {
		.fIfaceType = eIfaceTypeApplication,
		.fIfaceVersion = APP_INTERFACE_VERSION,
		.fIfaceSize = sizeof(AppInterfaceSet)
	}, 
	// App Interface exposed to ESIF from IPFSRV Server Application
	.appIface = {
		.fAppCreateFuncPtr = IpfSrv_AppCreate,
		.fAppDestroyFuncPtr = IpfSrv_AppDestroy,
		.fAppSuspendFuncPtr = IpfSrv_AppSuspend,
		.fAppResumeFuncPtr = IpfSrv_AppResume,
		.fAppGetVersionFuncPtr = IpfSrv_AppGetVersion,
		.fAppCommandFuncPtr = IpfSrv_AppCommand,
		.fAppGetIntroFuncPtr = IpfSrv_AppGetIntro,
		.fAppGetDescriptionFuncPtr = IpfSrv_AppGetDescription,
		.fAppGetNameFuncPtr = IpfSrv_AppGetName,
		.fAppGetStatusFuncPtr = IpfSrv_AppGetStatus,
		.fParticipantCreateFuncPtr = IpfSrv_AppParticipantCreate,
		.fParticipantDestroyFuncPtr = IpfSrv_AppParticipantDestroy,
		.fDomainCreateFuncPtr = IpfSrv_AppDomainCreate,
		.fDomainDestroyFuncPtr = IpfSrv_AppDomainDestroy,
		.fAppEventFuncPtr = IpfSrv_AppEvent,
	},
	// ESIF Interface exposed to IPFSRV Server Application, filled in by ESIF on AppCreate
	.esifIface = {
		.fGetConfigFuncPtr = NULL,
		.fSetConfigFuncPtr = NULL,
		.fPrimitiveFuncPtr = NULL,
		.fWriteLogFuncPtr = NULL,
		.fRegisterEventFuncPtr = NULL,
		.fUnregisterEventFuncPtr = NULL,
		.fSendEventFuncPtr = NULL,
		.fSendCommandFuncPtr = NULL,
	}
};

// Return IPF Server App Interface Set. May Contain NULLs
AppInterfaceSet *IpfSrv_GetInterfaceSet()
{
	return &g_ifaceSetServer;
}

AppInterfaceSet IpfSrv_GetInterface()
{
	return g_ifaceSetServer;
}

void IpfSrv_SetInterface(const AppInterfaceSet *iface)
{
	if (iface) {
		g_ifaceSetServer = *iface;
	}
}

