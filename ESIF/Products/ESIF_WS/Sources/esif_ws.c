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

#include <stdarg.h>
#include "esif_ccb_string.h"
#include "esif_ccb_library.h"
#include "esif_sdk_dcfg.h"

#include "esif_ws_server.h"
#include "esif_ws_version.h"

#include "esif_sdk_iface_app.h"
#include "esif_sdk_event_guid.h"
#include "esif_lib_istring.h"


//////////////////////////////////////////////////////////////////////////////
// ESIF/App Inferface Variables
//////////////////////////////////////////////////////////////////////////////

#define		WS_APP_NAME			"ipfui"
#define		WS_APP_DESCRIPTION	"Intel(R) Innovation Platform Framework Web Server"
#define		WS_APP_VERSION		ESIF_WS_VERSION
#define		WS_APP_BANNER		WS_APP_DESCRIPTION " v" WS_APP_VERSION " [" ESIF_ATTR_OS " " ESIF_PLATFORM_TYPE " " ESIF_BUILD_TYPE "]"
#define		WS_APP_HANDLE		0x3156626557667049	// "IpfWebV1"

// Alternate Configuration for DTT UI
#define		WS_APP_LEGACYNAME	WS_LIBRARY_NAME	// Legacy AppName
#define		WS_APP_ALTNAME		"dptfui"	// Alternate AppName
#define		WS_APP_ALTDESC		"Intel(R) Dynamic Tuning Technology Web Server"	// Alternate AppDesc
#define		WS_APP_ALTVERSION	"9.0." EXPAND_TOSTR(VER_HOTFIX.VER_BUILD) // Alternate AppVersion
#define		WS_APP_ALTPORT		8888		// Alternate Port if AppName (Loadable Library) is ALTNAME

typedef struct EsifAppConfig_s {
	char			appName[ESIF_NAME_LEN];			// AppName (Library Name)
	char			appDesc[ESIF_DESC_LEN];			// AppDescription
	char			appVersion[ESIF_VERSION_LEN];	// AppVersion
	char			ipAddr[ESIF_IPADDR_LEN];		// IP Address
	UInt16			port;							// Port Number
	esif_flags_t	flags;							// Flags
	Bool			isClient;						// Is IPF Client?
	char			docRoot[MAX_PATH];				// Document Root
} EsifAppConfig;

// ESIF/App Interface Instance
typedef struct EsifAppInstance_s {
	esif_handle_t	esifHandle;				// ESIF Handle (assigned by ESIF_UF)
	esif_handle_t	appHandle;				// App Handle (assigned by ESIF_WS)
	AppInterfaceSet	ifaceSet;				// ESIF/App Interface Set
	atomic_t		traceLevel;				// Current Trace Level
	EsifAppConfig	config;					// Web Server Configuration
	char			*restApiBufPtr;			// REST API Response Buffer
	u32				restApiBufLen;			// REST API Response Length
} EsifAppInstance;

static EsifAppInstance	g_esifApp;	// Defined below

//////////////////////////////////////////////////////////////////////////////
// Web Server Helper Functions called from Web Server modules
// ESIF App Interface Helper Functions
//////////////////////////////////////////////////////////////////////////////

const char *EsifWsDocRoot(void)
{
	const char *result = NULL;
	EsifAppInstance *self = &g_esifApp;

	if (self && self->config.docRoot[0]) {
		result = self->config.docRoot;
	}
	return result;
}

const char *EsifWsAppName(void)
{
	const char *result = NULL;
	EsifAppInstance *self = &g_esifApp;

	if (self && self->config.appName[0]) {
		result = self->config.appName;
	}
	return result;
}

Bool EsifWsShellEnabled(void)
{
	Bool rc = ESIF_FALSE;
	EsifAppInstance *self = &g_esifApp;

	if (self) {
		char command[] = "about shell";
		char *reply = EsifWsShellExec(command, sizeof(command), NULL, 0);
		if (reply) {
			rc = (esif_ccb_strstr(reply, "disabled") == NULL);
			esif_ccb_free(reply);
		}
	}
	return rc;
}

char *EsifWsShellExec(char *cmd, size_t cmd_len, char *prefix, size_t prefix_len)
{
	char *result = NULL;
	EsifAppInstance *self = &g_esifApp;

	if (self && self->esifHandle != ESIF_INVALID_HANDLE && self->ifaceSet.esifIface.fSendCommandFuncPtr) {
		const u32 WS_BUFSIZE = 4*1024; // Default Response Buffer Size
		const u32 WS_RESIZE_PADDING = 1024; // Padding to add to Resize Requests
		if (self->restApiBufPtr == NULL) {
			self->restApiBufPtr = (char *)esif_ccb_malloc(WS_BUFSIZE);
			self->restApiBufLen = (self->restApiBufPtr ? WS_BUFSIZE : 0);
		}
		char *response_buf = self->restApiBufPtr;
		u32 response_len = self->restApiBufLen;
		EsifData command  = { ESIF_DATA_STRING };
		EsifData response = { ESIF_DATA_STRING };
		command.buf_ptr = cmd;
		command.buf_len = command.data_len = (u32)cmd_len;
		response.buf_ptr = response_buf;
		response.buf_len = (response_buf ? response_len : 0);

		// Include formatting hint in response
		static char fmthint[] = "<XML>";
		if (response.buf_ptr && response.buf_len >= (u32)sizeof(fmthint)) {
			esif_ccb_strcpy(response.buf_ptr, fmthint, response.buf_len);
			response.data_len = (u32)sizeof(fmthint);
		}

		// Send command to ESIF_UF Command Shell
		esif_error_t rc = self->ifaceSet.esifIface.fSendCommandFuncPtr(
			self->esifHandle,
			1,
			&command,
			&response
		);

		// If response buffer too small, resize to required size plus padding to account for variable-length integer data element changes
		if (rc == ESIF_E_NEED_LARGER_BUFFER) {
			response_len = response.data_len + WS_RESIZE_PADDING;
			char *new_response_buf = esif_ccb_realloc(self->restApiBufPtr, response_len);
			if (new_response_buf) {
				self->restApiBufPtr = new_response_buf;
				self->restApiBufLen = response_len;
				response_buf = new_response_buf;
				response.buf_ptr = response_buf;
				response.buf_len = response_len;
				response.data_len = 0;

				// Resend formatting hint
				esif_ccb_strcpy(response.buf_ptr, fmthint, response.buf_len);
				response.data_len = (u32)sizeof(fmthint);

				// Resend command with larger buffer. Results not guaranteed to be the same.
				if (self->ifaceSet.esifIface.fSendCommandFuncPtr) {
					rc = self->ifaceSet.esifIface.fSendCommandFuncPtr(
						self->esifHandle,
						1,
						&command,
						&response
					);
				}
				else {
					rc = ESIF_E_CALLBACK_IS_NULL;
				}
			}
		}
		// Generate Response for Unauthorized Commands
		if (rc == ESIF_E_SESSION_PERMISSION_DENIED) {
			esif_ccb_strcpy(response.buf_ptr, "Unsupported Command", response.buf_len);
			response.data_len = (u32)esif_ccb_strlen(response.buf_ptr, response.buf_len) + 1;
			rc = ESIF_OK;
		}

		// Insert original request prefix into response and return to sender, who is responsible for freeing
		if (rc == ESIF_OK && response_buf && response.data_len + prefix_len < response_len) {
			if (prefix && prefix_len) {
				esif_ccb_memmove((char *)response_buf + prefix_len, response_buf, response.data_len);
				esif_ccb_memmove((char *)response_buf, prefix, prefix_len);
			}
			result = esif_ccb_strdup(response_buf);
		}
	}
	return result;
}

int EsifWsTraceLevel(void)
{
	int rc = TRACELEVEL_NONE;
	EsifAppInstance *self = &g_esifApp;

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
	int bytes = 0;
	EsifAppInstance *self = &g_esifApp;

	if (self && self->ifaceSet.esifIface.fWriteLogFuncPtr) {
		// Build Complete Trace Message
		va_list args;
		va_start(args, msg);
		IStringPtr tracemsg = IString_Create();
		IString_Sprintf(tracemsg, "[%s@%s#%d]: ", func, file, line);
		IString_VSprintfTo(tracemsg, (u32)IString_Strlen(tracemsg), (ZString)msg, args);
		IString_Strcat(tracemsg, "\n");
		va_end(args);

		EsifData message = { ESIF_DATA_STRING };
		message.buf_ptr = IString_GetString(tracemsg);
		message.buf_len = message.data_len = IString_DataLen(tracemsg);
		eLogType logType = (eLogType)level;

		// Log Message via ESIF Interface
		esif_error_t rc = self->ifaceSet.esifIface.fWriteLogFuncPtr(
			self->esifHandle,
			ESIF_HANDLE_PRIMARY_PARTICIPANT,
			ESIF_INVALID_HANDLE,
			&message,
			logType
		);
		if (rc == ESIF_OK) {
			bytes = IString_Strlen(tracemsg);
		}
		IString_Destroy(tracemsg);
	}
	return bytes;
}

int EsifWsConsoleMessageEx(
	const char *msg,
	...)
{
	int rc = 0;
	EsifAppInstance *self = &g_esifApp;

	// No Console Messages when running as an IPF Client
	if (self && !self->config.isClient) {
		// Create Dynamic copy of console message
		va_list args;
		va_start(args, msg);
		IStringPtr command = IString_Create();
		IString_VSprintfTo(command, IString_Strlen(command), (ZString)msg, args);
		va_end(args);

		// Convert 'message' to 'echo ! "message"' and send to ESIF Shell
		char shellcmd[10] = "echo ! ";
		char *quote = (esif_ccb_strchr(IString_GetString(command), '\"') != NULL ? "\'" : "\"");
		esif_ccb_strcat(shellcmd, quote, sizeof(shellcmd));
		IString_Insert(command, shellcmd, 0);
		IString_Strcat(command, quote);

		char *reply = EsifWsShellExec(IString_GetString(command), IString_BufLen(command), NULL, 0);
		esif_ccb_free(reply);
		IString_Destroy(command);
	}
	return rc;
}

//////////////////////////////////////////////////////////////////////////////
// APP Interface Functions exposed to ESIF_UF via AppInterfaceSet
//////////////////////////////////////////////////////////////////////////////

// Prototypes
static esif_error_t ESIF_CALLCONV EsifWs_AppDestroy(const esif_handle_t appHandle);

// Load an EsifData string from a Null-Terminated string
static esif_error_t EsifWs_LoadString(EsifDataPtr response, const char *buf_ptr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	const size_t max_string = 4096;

	if (response && buf_ptr) {
		u32 buf_len = (u32)esif_ccb_strnlen(buf_ptr, max_string) + 1;
		if (response->type != ESIF_DATA_STRING) {
			rc = ESIF_E_INVALID_REQUEST_TYPE;
		}
		else if (response->buf_len < buf_len) {
			response->data_len = buf_len;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		else if (response->buf_ptr) {
			esif_ccb_strcpy(response->buf_ptr, buf_ptr, response->buf_len);
			response->data_len = buf_len;
			rc = ESIF_OK;
		}
	}
	return rc;
}

// AppGetName Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppGetName(EsifDataPtr appName)
{
	const char *libname = WS_APP_NAME;
	EsifAppInstance *self = &g_esifApp;

	if (self) {
		// Running as an IPF Client?
		static const char ipfclient[] = "ipfcli-";
		if (appName && appName->buf_ptr && esif_ccb_strncmp(appName->buf_ptr, ipfclient, sizeof(ipfclient) - 1) == 0) {
			self->config.isClient = ESIF_TRUE;
		}

		// Use Loadable Library Name for appName
		esif_lib_t lib = esif_ccb_library_load(NULL);
		if (lib) {
			char pathname[MAX_PATH] = {0};
			if (esif_ccb_library_getpath(lib, pathname, sizeof(pathname), (void *)EsifWs_AppGetName) == ESIF_OK) {
				char *slash = esif_ccb_strrchr(pathname, *ESIF_PATH_SEP);
				char *dot = (slash ? esif_ccb_strchr(slash, '.') : NULL);
				if (slash && dot) {
					*slash++ = 0;
					*dot = 0;
					size_t buf_left = sizeof(pathname) - ((size_t)slash - (size_t)pathname);
					if (esif_ccb_strlen(slash, buf_left) < sizeof(self->config.appName)) {
						esif_ccb_strcpy(self->config.appName, slash, sizeof(self->config.appName));
					}
					// Set docRoot for Windows IPF Clients
					if (self->config.isClient && pathname[0] != '/' && self->config.docRoot[0] == 0) {
						esif_ccb_sprintf(sizeof(self->config.docRoot), self->config.docRoot, "%s" ESIF_PATH_SEP "%s", pathname, "ui");
					}
					// Set Alternate Port & Description if using Alternate AppName 
					if (esif_ccb_stricmp(self->config.appName, WS_APP_ALTNAME) == 0 || esif_ccb_stricmp(self->config.appName, WS_APP_LEGACYNAME) == 0) {
						self->config.port = WS_APP_ALTPORT;
						esif_ccb_strcpy(self->config.appDesc, WS_APP_ALTDESC, sizeof(self->config.appDesc));
						esif_ccb_strcpy(self->config.appVersion, WS_APP_ALTVERSION, sizeof(self->config.appVersion));
					}
				}
			}
			esif_ccb_library_unload(lib);
		}
		libname = self->config.appName;
	}
	return EsifWs_LoadString(appName, libname);
}

// AppGetVersion Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppGetVersion(EsifDataPtr appVersion)
{
	char *version = WS_APP_VERSION;
	EsifAppInstance *self = &g_esifApp;

	if (self && self->config.appVersion[0]) {
		version = self->config.appVersion;
	}
	return EsifWs_LoadString(appVersion, version);
}

// AppGetDescription Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppGetDescription(EsifDataPtr appDescription)
{
	char *description = WS_APP_DESCRIPTION;
	EsifAppInstance *self = &g_esifApp;

	if (self && self->config.appDesc[0]) {
		description = self->config.appDesc;
	}
	return EsifWs_LoadString(appDescription, description);
}

// AppGetIntro Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppGetIntro(
	const esif_handle_t appHandle,
	EsifDataPtr appIntro
)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	EsifAppInstance *self = &g_esifApp;

	if (self && appHandle != ESIF_INVALID_HANDLE && appHandle == self->appHandle) {
		rc = EsifWs_LoadString(appIntro, WS_APP_BANNER);
	}
	return rc;
}

// AppCreate Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppCreate(
	AppInterfaceSetPtr ifaceSetPtr,
	const esif_handle_t esifHandle,
	esif_handle_t *appHandlePtr,
	const AppDataPtr appDataPtr,
	const eAppState initialAppState
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifAppInstance *self = &g_esifApp;

	UNREFERENCED_PARAMETER(initialAppState);

	if (!self) {
		return rc;
	}
	else if (esifHandle == ESIF_INVALID_HANDLE) {
		rc = ESIF_E_INVALID_HANDLE;
	}
	else if (self->appHandle != ESIF_INVALID_HANDLE || self->esifHandle != ESIF_INVALID_HANDLE) {
		rc = ESIF_E_WS_ALREADY_STARTED;
	}
	else if (ifaceSetPtr && appHandlePtr && appDataPtr) {

		// Exact Interface Version Match Required
		if ((ifaceSetPtr->hdr.fIfaceType != self->ifaceSet.hdr.fIfaceType) ||
			(ifaceSetPtr->hdr.fIfaceVersion != self->ifaceSet.hdr.fIfaceVersion) ||
			(ifaceSetPtr->hdr.fIfaceSize != self->ifaceSet.hdr.fIfaceSize)) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else {
			self->esifHandle = esifHandle;
			self->appHandle = WS_APP_HANDLE;
			self->ifaceSet.esifIface = ifaceSetPtr->esifIface;
			*appHandlePtr = self->appHandle;

			rc = WebPlugin_Init();

			/* Check DCFG to verify that UI is enabled. Do not fail AppCreate on error, but fail if App blocked by DCFG.
			** This only needs to be checked when running in-process, otherwise the primitive will fail due to it not
			** being in the Application's Security Role, which causes the Server to log an Error in the Event Log.
			** If in-process support is ever removed, this can simply verify that the app is running out-of-process.
			*/
			if (rc == ESIF_OK && self->ifaceSet.esifIface.fPrimitiveFuncPtr && !self->config.isClient) {
				DCfgOptions dcfg = { 0 };
				EsifData request = { ESIF_DATA_VOID };
				EsifData response = { ESIF_DATA_UINT32 };
				response.buf_ptr = &dcfg.asU32;
				response.buf_len = (u32)sizeof(dcfg);
				esif_error_t dcfgrc = self->ifaceSet.esifIface.fPrimitiveFuncPtr(
					self->esifHandle,
					ESIF_HANDLE_PRIMARY_PARTICIPANT,
					ESIF_INVALID_HANDLE,
					&request,
					&response,
					GET_CONFIG_ACCESS_CONTROL_SUR,
					ESIF_INSTANCE_INVALID
				);
				if (dcfgrc == ESIF_OK && dcfg.opt.GenericUIAccessControl) {
					rc = ESIF_E_DISABLED;
				}
			}

			// Convert DPTF Path to UI Document Root Path, except for IPF Clients
			if (rc == ESIF_OK) {
				if (!self->config.docRoot[0]) {
					// Use OS-Specific UI Document Root Path instead of fPathHome when running In-Process
					#if   defined(ESIF_ATTR_OS_CHROME) 
						esif_ccb_strcpy(self->config.docRoot, "/usr/share/dptf/ui", sizeof(self->config.docRoot));
					#else
						esif_ccb_strcpy(self->config.docRoot, "/usr/share/dptf/ui", sizeof(self->config.docRoot));
					#endif
				}

				// Set Trace Level
				atomic_set(&self->traceLevel, appDataPtr->fLogLevel);
			}

			// Register for Trace Level Changed Event. Fail AppCreate on error.
			if (rc == ESIF_OK && self->ifaceSet.esifIface.fRegisterEventFuncPtr) {
				static UInt8 eventGuidBuf[ESIF_GUID_LEN] = LOG_VERBOSITY_CHANGED;
				EsifData eventGuid = { ESIF_DATA_GUID, eventGuidBuf, ESIF_GUID_LEN, ESIF_GUID_LEN };
				rc = self->ifaceSet.esifIface.fRegisterEventFuncPtr(
					self->esifHandle,
					ESIF_HANDLE_PRIMARY_PARTICIPANT,
					ESIF_INVALID_HANDLE,
					&eventGuid
				);
			}

			// Automatically Start Web Server on Startup. Do not fail AppCreate on error.
			if (rc == ESIF_OK) {
				WebServerPtr server = g_WebServer;


				// If in-process using legacy appname, lookup and use last known URL. Do Not Fail AppCreate on Error
				if (!self->config.isClient && esif_ccb_stricmp(self->config.appName, WS_APP_LEGACYNAME) == 0 && self->ifaceSet.esifIface.fSendCommandFuncPtr) {
					char cmd[MAX_PATH] = {0};
					char result_buf[MAX_PATH] = {0};
					esif_ccb_sprintf(sizeof(cmd), cmd, "config get @%s url", self->config.appName);
					EsifData request  = { ESIF_DATA_STRING };
					EsifData result = { ESIF_DATA_STRING };
					request.buf_ptr = cmd;
					request.buf_len = request.data_len = (u32)esif_ccb_strlen(cmd, sizeof(cmd)) + 1;
					result.buf_ptr = result_buf;
					result.buf_len = sizeof(result_buf);

					// Send command to ESIF_UF Command Shell
					esif_error_t urlrc = self->ifaceSet.esifIface.fSendCommandFuncPtr(
						self->esifHandle,
						1,
						&request,
						&result
					);

					// If successful response, parse last known URL and set server config parameters
					char prefix[] = "http://";
					if (urlrc == ESIF_OK && esif_ccb_strncmp(result_buf, prefix, sizeof(prefix) - 1) == 0) {
						char *sep = esif_ccb_strchr(result_buf + sizeof(prefix) - 1, ':');
						if (sep) {
							*sep++ = 0;
							self->config.port = (UInt16)atoi(sep);
						}
						esif_ccb_strcpy(self->config.ipAddr, result_buf + sizeof(prefix) - 1, sizeof(self->config.ipAddr));
					}
				}

				// Start Server
				esif_error_t startrc = WebServer_Config(
					server,
					0,
					self->config.ipAddr,
					self->config.port,
					self->config.flags
				);
				if (startrc == ESIF_OK) {
					startrc = WebServer_Start(server);
				}

				if (startrc == ESIF_OK) {
					EsifWsConsoleMessageEx("web server started\n");
				}
				else {
					EsifWsConsoleMessageEx("web server not started (%s)\n", esif_rc_str(startrc));
				}
			}
			else {
				EsifWs_AppDestroy(self->appHandle);
				*appHandlePtr = ESIF_INVALID_HANDLE;
			}
		}
	}
	return rc;
}

// AppDestroy Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppDestroy(const esif_handle_t appHandle)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	EsifAppInstance *self = &g_esifApp;

	// Stop and Destroy Web Server App
	if (self && appHandle != ESIF_INVALID_HANDLE && appHandle == self->appHandle) {

		// Destroy App
		self->esifHandle = ESIF_INVALID_HANDLE;
		self->appHandle = ESIF_INVALID_HANDLE;
		self->ifaceSet.esifIface = (EsifInterface){0};
		self->config.docRoot[0] = 0;

		WebPlugin_Exit();

		// Destroy Response Buffer
		esif_ccb_free(self->restApiBufPtr);
		self->restApiBufPtr = NULL;
		self->restApiBufLen = 0;

		rc = ESIF_OK;
	}
	return rc;
}

// AppSuspend Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppSuspend(const esif_handle_t appHandle)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	EsifAppInstance *self = &g_esifApp;

	if (self && appHandle != ESIF_INVALID_HANDLE && appHandle == self->appHandle) {
		rc = ESIF_OK;
	}
	return rc;
}

// AppResume Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppResume(const esif_handle_t appHandle)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	EsifAppInstance *self = &g_esifApp;

	if (self && appHandle != ESIF_INVALID_HANDLE && appHandle == self->appHandle) {
		rc = ESIF_OK;
	}
	return rc;
}


// AppCommand Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppCommand(
	const esif_handle_t appHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifAppInstance *self = &g_esifApp;

	if (!self) {
		return rc;
	}
	else if (appHandle == ESIF_INVALID_HANDLE || appHandle != self->appHandle) {
		rc = ESIF_E_INVALID_HANDLE;
	}
	else if (argc && argv && argv[0].buf_ptr && argv[0].data_len && response) {
		IStringPtr reply = IString_Create();
		UInt32 optarg = 0;
		const char *command = argv[optarg++].buf_ptr;
		int bytes = 0;
		rc = ESIF_E_NOT_IMPLEMENTED;

		// Process Web Server Commands
		WebServerPtr server = g_WebServer;

		// esif_ws {help | about}
		if (esif_ccb_stricmp(command, "help") == 0 || esif_ccb_stricmp(command, "about") == 0) {
			bytes = IString_Sprintf(reply,
				"%s - " WS_APP_BANNER "\n"
				"Copyright (c) 2013-2023 Intel Corporation All Rights Reserved\n"
				"Available Commands:\n"
				"  help\n"
				"  start [<options>] [<ip>] [<port>]\n"
				"  restart [<options>] [<ip>] [<port>]\n"
				"  stop\n"
				"  config\n"
				"  status\n"
				"  fetch [<filename>]\n"
				"\n"
				, self->config.appName
			);
			rc = ESIF_OK;
		}
		// esif_ws start [unrestricted] [<ip>] [<port>]
		// esif_ws restart [unrestricted] [<ip>] [<port>]
		else if (esif_ccb_stricmp(command, "start") == 0 || esif_ccb_stricmp(command, "restart") == 0) {
			Bool isRestart = (esif_ccb_stricmp(command, "restart") == 0);
			if (isRestart) {
				WebServer_Stop(server);
				esif_ccb_free(self->restApiBufPtr);
				self->restApiBufPtr = NULL;
				self->restApiBufLen = 0;
			}
			if (WebServer_IsStarted(server)) {
				IString_Strcpy(reply, "web server already started\n");
				bytes = IString_Strlen(reply);
				rc = ESIF_OK;
			}
			else {
				char *ip = self->config.ipAddr;
				UInt16 port = self->config.port;
				esif_flags_t flags = self->config.flags;

				// Parse optional Configuration
				while (optarg < argc) {
					// [unrestricted]
					if (esif_ccb_stricmp(argv[optarg].buf_ptr, "unrestricted") == 0) {
						flags |= WS_FLAG_NOWHITELIST;
					}
					// <ip>
					else if (esif_ccb_strchr((char*)argv[optarg].buf_ptr, '.') != NULL) {
						ip = argv[optarg].buf_ptr;
					}
					// <port>
					else if (isdigit(((char *)argv[optarg].buf_ptr)[0])) {
						UInt16 portnum = (UInt16)atoi(argv[optarg].buf_ptr);
						if (portnum) {
							port = portnum;
						}
					}
					optarg++;
				}
				esif_ccb_strcpy(self->config.ipAddr, ip, sizeof(self->config.ipAddr));
				self->config.port = port;
				self->config.flags = flags;

				// If in-process using legacy appname, save current URL
				if (!self->config.isClient && esif_ccb_stricmp(self->config.appName, WS_APP_LEGACYNAME) == 0 && (self->config.ipAddr[0] || self->config.port) && self->ifaceSet.esifIface.fSendCommandFuncPtr) {
					char cmd[MAX_PATH] = {0};
					char result_buf[MAX_PATH] = {0};
					esif_ccb_sprintf(sizeof(cmd), cmd,
						"config set @%s url http://%s:%hu nopersist", 
						self->config.appName,
						(self->config.ipAddr[0] ? self->config.ipAddr : WS_DEFAULT_IPADDR),
						(self->config.port ? self->config.port : WS_DEFAULT_PORT)
					);
					EsifData request  = { ESIF_DATA_STRING };
					EsifData result = { ESIF_DATA_STRING };
					request.buf_ptr = cmd;
					request.buf_len = request.data_len = (u32)esif_ccb_strlen(cmd, sizeof(cmd)) + 1;
					result.buf_ptr = result_buf;
					result.buf_len = sizeof(result_buf);

					// Send command to ESIF_UF Command Shell. Ignore Result.
					self->ifaceSet.esifIface.fSendCommandFuncPtr(
						self->esifHandle,
						1,
						&request,
						&result
					);
				}

				// Configure Listener
				rc = WebServer_Config(
					server,
					0,
					self->config.ipAddr,
					self->config.port,
					self->config.flags
				);

				if (rc == ESIF_OK) {
					rc = WebServer_Start(server);

					// thread synchronization delay for output
					if (rc == ESIF_OK) {
						int tries = 0;
						do {
							esif_ccb_sleep_msec(10);
						} while (++tries < 10 && !WebServer_IsStarted(server));
					}
				}
				bytes = IString_Sprintf(
					reply,
					"web server %s%sed\n",
					(WebServer_IsStarted(server) ? "" : "not "),
					command
				);
			}
		}
		// esif_ws stop
		else if (esif_ccb_stricmp(command, "stop") == 0) {
			if (WebServer_IsStarted(server)) {
				WebServer_Stop(server);
				esif_ccb_free(self->restApiBufPtr);
				self->restApiBufPtr = NULL;
				self->restApiBufLen = 0;
				IString_Strcpy(reply, "web server stopped\n");
			}
			else {
				IString_Strcpy(reply, "web server not started\n");
			}
			bytes = IString_Strlen(reply);
			rc = ESIF_OK;
		}
		// esif_ws config
		else if (esif_ccb_stricmp(command, "config") == 0) {
			bytes = IString_SprintfConcat(
				reply,
				"web config instance %d[%x]: IP=%s port=%hu [%s]\n",
				0,
				self->config.flags,
				self->config.ipAddr,
				self->config.port,
				(WebServer_IsStarted(server) ? "started" : "stopped")
			);
			rc = ESIF_OK;
		}
		// esif_ws status
		else if (esif_ccb_stricmp(command, "status") == 0) {
			bytes = IString_Sprintf(reply,
				"web server %s : http://%s:%hu (Version %s)\n"
				, (WebServer_IsStarted(server) ? "started" : "stopped")
				, server->listeners[0].ipAddr
				, server->listeners[0].port
				, WS_APP_VERSION
			);
			rc = ESIF_OK;
		}
		// esif_ws fetch <filename>
		else if (esif_ccb_stricmp(command, "fetch") == 0) {
			char *content = NULL;
			if (optarg < argc) {
				char cmd[MAX_PATH] = {0};
				esif_ccb_sprintf(sizeof(cmd), cmd, "%s %s %s", WS_APPNAME_LOCALSERVER, command, (char *)argv[optarg++].buf_ptr);

				content = WebServer_ExecRestCmdInternal(server, cmd, NULL);

				if (content) {
					bytes = IString_Sprintf(reply, "%s", content);
					rc = ESIF_OK;
				}
				else {
					rc = ESIF_E_NOT_FOUND;
				}
				esif_ccb_free(content);
			}
			else {
				rc = ESIF_E_PARAMETER_IS_NULL;
			}
		}


		// Return Reply
		if (rc == ESIF_OK) {
			if (bytes < 0) {
				rc = ESIF_E_OVERFLOWED_RESULT_TYPE;
			}
			else if ((u32)bytes >= response->buf_len || response->buf_ptr == NULL) {
				response->data_len = (u32)bytes + 1;
				rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			else {
				response->data_len = (u32)bytes + 1;
				esif_ccb_strcpy(response->buf_ptr, IString_GetString(reply), (size_t)response->buf_len);
			}
		}
		IString_Destroy(reply);
	}
	return rc;
}

// AppGetStatus Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppGetStatus(
	const esif_handle_t appHandle,
	const eAppStatusCommand command,
	const UInt32 appStatusIn,
	EsifDataPtr appStatusOut

)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifAppInstance *self = &g_esifApp;

	UNREFERENCED_PARAMETER(command);
	UNREFERENCED_PARAMETER(appStatusIn);

	if (!self) {
		return rc;
	}
	else if (appHandle == ESIF_INVALID_HANDLE || appHandle != self->appHandle) {
		rc = ESIF_E_INVALID_HANDLE;
	}
	else if (appStatusOut->buf_ptr && appStatusOut->buf_len) {
		appStatusOut->data_len = 0;
		rc = ESIF_OK;
	}
	return rc;
}

// AppParticipantCreate Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppParticipantCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const AppParticipantDataPtr participantDataPtr,
	const eParticipantState initialParticipantState
)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	EsifAppInstance *self = &g_esifApp;

	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(participantDataPtr);
	UNREFERENCED_PARAMETER(initialParticipantState);

	if (self && appHandle != ESIF_INVALID_HANDLE && appHandle == self->appHandle) {
		rc = ESIF_OK;
	}
	return rc;
}

// AppParticipantDestroy Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppParticipantDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle
)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	EsifAppInstance *self = &g_esifApp;

	UNREFERENCED_PARAMETER(participantHandle);

	if (self && appHandle != ESIF_INVALID_HANDLE && appHandle == self->appHandle) {
		rc = ESIF_OK;
	}
	return rc;
}

// AppDomainCreate Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppDomainCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const AppDomainDataPtr domainDataPtr,
	const eDomainState domainInitialState
)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	EsifAppInstance *self = &g_esifApp;

	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(domainHandle);
	UNREFERENCED_PARAMETER(domainDataPtr);
	UNREFERENCED_PARAMETER(domainInitialState);

	if (self && appHandle != ESIF_INVALID_HANDLE && appHandle == self->appHandle) {
		rc = ESIF_OK;
	}
	return rc;
}

// AppDomainDestroy Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppDomainDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle
)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	EsifAppInstance *self = &g_esifApp;

	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(domainHandle);

	if (self && appHandle != ESIF_INVALID_HANDLE && appHandle == self->appHandle) {
		rc = ESIF_OK;
	}
	return rc;
}

// AppEvent Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppEvent(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventDataPtr,
	const EsifDataPtr eventGuid
)
{
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	EsifAppInstance *self = &g_esifApp;

	UNREFERENCED_PARAMETER(participantHandle);
	UNREFERENCED_PARAMETER(domainHandle);

	if (self && appHandle != ESIF_INVALID_HANDLE && appHandle == self->appHandle) {
		rc = ESIF_E_PARAMETER_IS_NULL;

		// Set Trace Level
		if (eventGuid && eventGuid->buf_ptr && eventGuid->buf_len == sizeof(esif_guid_t)) {
			esif_guid_t eventVerbosityChanged = LOG_VERBOSITY_CHANGED;
			if (memcmp(eventGuid->buf_ptr, eventVerbosityChanged, sizeof(eventVerbosityChanged)) == 0) {
				if (eventDataPtr && eventDataPtr->buf_ptr && (eventDataPtr->buf_len == sizeof(UInt32))) {
					UInt32 level = *(UInt32*)(eventDataPtr->buf_ptr);
					atomic_set(&self->traceLevel, level);
					rc = ESIF_OK;
				}
			}
		}
	}
	return rc;
}

// ESIF/App Application Instance
static EsifAppInstance	g_esifApp = {
	.esifHandle = ESIF_INVALID_HANDLE,
	.appHandle  = ESIF_INVALID_HANDLE,
	.traceLevel = TRACELEVEL_ERROR,
	.config = {
		.appName = WS_APP_NAME,
		.appDesc = WS_APP_DESCRIPTION,
		.appVersion = WS_APP_VERSION,
		.ipAddr = WS_DEFAULT_IPADDR,
		.port = WS_DEFAULT_PORT,
		.flags = WS_DEFAULT_FLAGS,
		.isClient = ESIF_FALSE,
	},
	.ifaceSet = {
		.hdr = {
			.fIfaceType = eIfaceTypeApplication,
			.fIfaceVersion = APP_INTERFACE_VERSION,
			.fIfaceSize = sizeof(AppInterfaceSet),
		},
		.appIface = {
			.fAppGetNameFuncPtr = EsifWs_AppGetName,
			.fAppGetDescriptionFuncPtr = EsifWs_AppGetDescription,
			.fAppGetVersionFuncPtr = EsifWs_AppGetVersion,
			.fAppGetIntroFuncPtr = EsifWs_AppGetIntro,
			.fAppCreateFuncPtr = EsifWs_AppCreate,
			.fAppDestroyFuncPtr = EsifWs_AppDestroy,
			.fAppSuspendFuncPtr = EsifWs_AppSuspend,
			.fAppResumeFuncPtr = EsifWs_AppResume,
			.fAppCommandFuncPtr = EsifWs_AppCommand,
			.fAppGetStatusFuncPtr = EsifWs_AppGetStatus,
			.fParticipantCreateFuncPtr = EsifWs_AppParticipantCreate,
			.fParticipantDestroyFuncPtr = EsifWs_AppParticipantDestroy,
			.fDomainCreateFuncPtr = EsifWs_AppDomainCreate,
			.fDomainDestroyFuncPtr = EsifWs_AppDomainDestroy,
			.fAppEventFuncPtr = EsifWs_AppEvent,
		},
		// .esifIface populated by ESIF_UF during AppCreate
	},
};

//////////////////////////////////////////////////////////////////////////////
// Exported Interface Functions
//////////////////////////////////////////////////////////////////////////////

// Exported ESIF/App Interface Function
ESIF_EXPORT esif_error_t GetApplicationInterfaceV2(AppInterfaceSetPtr ifacePtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	EsifAppInstance *self = &g_esifApp;

	if (self && ifacePtr) {
		// Must be an exact Interface Match
		if ((ifacePtr->hdr.fIfaceType != self->ifaceSet.hdr.fIfaceType) ||
			(ifacePtr->hdr.fIfaceVersion != self->ifaceSet.hdr.fIfaceVersion) ||
			(ifacePtr->hdr.fIfaceSize != self->ifaceSet.hdr.fIfaceSize)) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else {
			// Fill in App Interface
			ifacePtr->appIface = self->ifaceSet.appIface;
			rc = ESIF_OK;
		}
	}
	return rc;
}