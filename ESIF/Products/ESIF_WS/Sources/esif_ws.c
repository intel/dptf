/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "esif_sdk_iface_app.h"
#include "esif_sdk_event_guid.h"
#include "esif_lib_istring.h"

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

//////////////////////////////////////////////////////////////////////////////
// ESIF/App Inferface Variables
//////////////////////////////////////////////////////////////////////////////

#define		WS_APP_NAME			"esif_ws"
#define		WS_APP_DESCRIPTION	"Intel Platform Framework Web Server"
#define		WS_APP_VERSION		ESIF_WS_VERSION
#define		WS_APP_BANNER		WS_APP_DESCRIPTION " v" WS_APP_VERSION " [" ESIF_ATTR_OS " " ESIF_PLATFORM_TYPE " " ESIF_BUILD_TYPE "]"
#define		WS_APP_HANDLE		0x3156626557667049	// "IpfWebV1"

typedef struct EsifAppConfig_s {
	char			ipAddr[ESIF_IPADDR_LEN];	// IP Address
	UInt16			port;						// Port Number
	esif_flags_t	flags;						// Flags
	char			docRoot[MAX_PATH];			// Document Root
} EsifAppConfig;

// ESIF/App Interface Instance
typedef struct EsifAppInstance_s {
	esif_handle_t	esifHandle;				// ESIF Handle (assigned by ESIF_UF)
	esif_handle_t	appHandle;				// App Handle (assigned by ESIF_WS)
	atomic_t		traceLevel;				// Current Trace Level
	EsifAppConfig	config;					// Web Server Configuration
	AppInterfaceSet	ifaceSet;				// ESIF/App Interface Set
} EsifAppInstance;

static EsifAppInstance	g_esifApp;	// Defined below

//////////////////////////////////////////////////////////////////////////////
// Web Server Inferface Variables
//////////////////////////////////////////////////////////////////////////////

// Global ESIF_UF <-> ESIF_WS Interface
static EsifWsInterfacePtr g_ifaceWs = NULL;

//////////////////////////////////////////////////////////////////////////////
// Web Server Interface Functions exposed to ESIF_UF via EsifWsInterface
// ESIF_UF -> ESIF_WS Interface Callback Functions
//////////////////////////////////////////////////////////////////////////////

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
	if (self && server) {
		for (u8 instance = 0; instance < esif_ccb_min(WS_LISTENERS, WS_MAX_LISTENERS); instance++) {
			char* ipAddr = NULL;
			short port = 0;
			esif_flags_t flags = 0;
			if (instance == 0 || self->ipAddr[instance][0] || self->port[instance]) {
				ipAddr = self->ipAddr[instance];
				port = (short)self->port[instance];
				flags = self->flags[instance];
			}
			rc = WebServer_Config(server, instance, ipAddr, port, flags);
			
			// Fill in Interface structure with the actual IP/Port used for each instance.
			if (rc == ESIF_OK) {
				if (ipAddr == NULL || *ipAddr == 0) {
					esif_ccb_strcpy(self->ipAddr[instance], server->listeners[instance].ipAddr, sizeof(self->ipAddr[instance]));
				}
				if (port == 0) {
					self->port[instance] = server->listeners[instance].port;
				}
				if (flags == 0) {
					self->flags[instance] = server->listeners[instance].flags;
				}
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

static Bool ESIF_CALLCONV EsifWsIsStarted(void)
{
	return WebServer_IsStarted(g_WebServer);
}

static void * ESIF_CALLCONV EsifWsAlloc(size_t buf_len)
{
	return esif_ccb_malloc(buf_len);
}

//////////////////////////////////////////////////////////////////////////////
// Web Server Helper Functions called from Web Server modules
// ESIF_WS -> ESIF_UF Interface Helper Functions
//////////////////////////////////////////////////////////////////////////////

const char *EsifWsDocRoot(void)
{
	const char *result = NULL;

	// Web Interface
	if (g_ifaceWs) {
		EsifWsInterfacePtr self = g_ifaceWs;
		if (self->docRoot[0]) {
			result = self->docRoot;
		}
	}
	// ESIF/App Interface
	else {
		EsifAppInstance *self = &g_esifApp;
		if (self->config.docRoot[0]) {
			result = self->config.docRoot;
		}
	}
	return result;
}

Bool EsifWsShellEnabled(void)
{
	Bool rc = ESIF_FALSE;

	// Web Interface
	if (g_ifaceWs) {
		EsifWsInterfacePtr self = g_ifaceWs;
		if (self->tEsifWsShellEnabledFuncPtr) {
			rc = self ->tEsifWsShellEnabledFuncPtr();
		}
	}
	// ESIF/App Interface
	else {
		EsifAppInstance *self = &g_esifApp;
		if (self) {
			char command[] = "shell";
			char *reply = EsifWsShellExec(command, sizeof(command), NULL, 0);
			if (reply) {
				rc = (esif_ccb_strstr(reply, "disabled") == NULL);
				esif_ccb_free(reply);
			}
		}
	}
	return rc;
}

char *EsifWsShellExec(char *cmd, size_t cmd_len, char *prefix, size_t prefix_len)
{
	char *result = NULL;

	// Web Interface
	if (g_ifaceWs) {
		EsifWsInterfacePtr self = g_ifaceWs;
		if (self->tEsifWsShellExecFuncPtr) {
			// Calls into ESIF_UF, which calls back into EsifWsAlloc()
			result = self->tEsifWsShellExecFuncPtr(cmd, cmd_len, prefix, prefix_len);
		}
	}
	// ESIF/App Interface
	else {
		EsifAppInstance *self = &g_esifApp;
		if (self->esifHandle != ESIF_INVALID_HANDLE && self->ifaceSet.esifIface.fSendCommandFuncPtr) {
			const u32 WS_BUFSIZE = 4*1024; // Default Response Size
			const u32 WS_RESIZE_PADDING = 1024; // Padding to add to Resize Requests
			u32 response_len = WS_BUFSIZE;
			char *response_buf = (char *)esif_ccb_malloc(response_len);
			EsifData command  = { ESIF_DATA_STRING };
			EsifData response = { ESIF_DATA_STRING };
			command.buf_ptr = cmd;
			command.buf_len = command.data_len = (u32)cmd_len;
			response.buf_ptr = response_buf;
			response.buf_len = (response_buf ? response_len : 0);
			
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
				char *new_response_buf = esif_ccb_realloc(response_buf, response_len);
				if (new_response_buf) {
					response_buf = new_response_buf;
					response.buf_ptr = response_buf;
					response.buf_len = response_len;
					response.data_len = 0;

					// Resend command with larger buffer. Results not guaranteed to be the same.
					rc = self->ifaceSet.esifIface.fSendCommandFuncPtr(
						self->esifHandle,
						1,
						&command,
						&response
					);
				}
			}

			// Insert original request prefix into response and return to sender, who is responsible for freeing
			if (rc == ESIF_OK && response_buf && response.data_len + prefix_len < response_len) {
				if (prefix && prefix_len) {
					esif_ccb_memmove((char *)response_buf + prefix_len, response_buf, response.data_len);
					esif_ccb_memmove((char *)response_buf, prefix, prefix_len);
				}
				result = response_buf;
			}
			else {
				esif_ccb_free(response_buf);
			}
		}
	}
	return result;
}

int EsifWsTraceLevel(void)
{
	int rc = TRACELEVEL_NONE;

	// Web Interface
	if (g_ifaceWs) {
		EsifWsInterfacePtr self = g_ifaceWs;
		rc = (int)atomic_read(&self->traceLevel);
	}
	// ESIF/App Interface
	else {
		EsifAppInstance *self = &g_esifApp;
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

	// Web Interface
	if (g_ifaceWs) {
		EsifWsInterfacePtr self = g_ifaceWs;
		va_list args;
		va_start(args, msg);
		if (self->tEsifWsTraceMessageFuncPtr) {
			bytes = self->tEsifWsTraceMessageFuncPtr(level, func, file, line, msg, args);
		}
		va_end(args);
	}
	// ESIF/App Interface
	else {
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
			eLogType logType = (eLogType)atomic_read(&self->traceLevel);

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
	}
	return bytes;
}

int EsifWsConsoleMessageEx(
	const char *msg, 
	...)
{
	int rc = 0;

	// Web Interface
	if (g_ifaceWs) {
		EsifWsInterfacePtr self = g_ifaceWs;
		va_list args;
		va_start(args, msg);
		if (self->tEsifWsConsoleMessageFuncPtr) {
			rc = self->tEsifWsConsoleMessageFuncPtr(msg, args);
		}
		va_end(args);
	}
	// ESIF/App Interface
	else {
		EsifAppInstance *self = &g_esifApp;
		if (self) {
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
	}
	return rc;
}

//////////////////////////////////////////////////////////////////////////////
// APP Interface Functions exposed to ESIF_UF via AppInterfaceSet
//////////////////////////////////////////////////////////////////////////////

// Load an EsifData string from a Null-Terminated string
static esif_error_t EsifWs_LoadString(EsifDataPtr response, const char *buf_ptr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	const size_t max_string = 4096;

	if (response && buf_ptr) {
		u32 buf_len = (u32)esif_ccb_strlen(buf_ptr, max_string) + 1;
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
	return EsifWs_LoadString(appName, WS_APP_NAME);
}

// AppGetVersion Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppGetVersion(EsifDataPtr appVersion)
{
	return EsifWs_LoadString(appVersion, WS_APP_VERSION);
}

// AppGetDescription Interface Function
static esif_error_t ESIF_CALLCONV EsifWs_AppGetDescription(EsifDataPtr appDescription)
{
	return EsifWs_LoadString(appDescription, WS_APP_DESCRIPTION);
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
	else if (self->appHandle != ESIF_INVALID_HANDLE || self->esifHandle != ESIF_INVALID_HANDLE || g_ifaceWs) {
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

			// Convert DPTF Path to UI Path
			esif_ccb_strcpy(self->config.docRoot, appDataPtr->fPathHome.buf_ptr, sizeof(self->config.docRoot));
			char *sep = esif_ccb_strchr(self->config.docRoot, '|');
			if (sep) {
				*sep = 0;
				// Convert Linux /xxx/xxx/dptf or /xxx/xxx/dptf/xxx to /xxx/xxx/dptf/ui
				if (self->config.docRoot[0] == '/') {
					sep = esif_ccb_strrchr(self->config.docRoot, '/');
					if (sep) {
						if (esif_ccb_stricmp(sep + 1, "dptf") != 0) {
							*sep = 0;
						}
						esif_ccb_strcat(self->config.docRoot, "/ui", sizeof(self->config.docRoot));
					}
				}
				// No conversion needed for Windows
			}

			// Set Trace Level
			atomic_set(&self->traceLevel, appDataPtr->fLogLevel);

			// Register for Trace Level Changed Event
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
				esif_error_t startrc = WebServer_Config(
					server,
					0,
					WS_DEFAULT_IPADDR,
					WS_DEFAULT_PORT,
					WS_DEFAULT_FLAGS
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
				WS_APP_BANNER "\n"
				"Copyright (c) 2013-2020 Intel Corporation All Rights Reserved\n"
				"Usage:\n"
				"  " WS_APP_NAME " help\n"
				"  " WS_APP_NAME " start [<options>] [<ip>] [<port>]\n"
				"  " WS_APP_NAME " restart [<options>] [<ip>] [<port>]\n"
				"  " WS_APP_NAME " stop\n"
				"  " WS_APP_NAME " config\n"
				"  " WS_APP_NAME " status\n"
				"\n"
			);
			rc = ESIF_OK;
		}
		// esif_ws start [unrestricted] [<ip>] [<port>]
		// esif_ws restart [unrestricted] [<ip>] [<port>]
		else if (esif_ccb_stricmp(command, "start") == 0 || esif_ccb_stricmp(command, "restart") == 0) {
			Bool isRestart = (esif_ccb_stricmp(command, "restart") == 0);
			if (isRestart) {
				WebServer_Stop(server);
			}
			if (WebServer_IsStarted(server)) {
				IString_Strcpy(reply, "web server already started\n");
				bytes = IString_Strlen(reply);
				rc = ESIF_OK;
			}
			else {
				char *ip = (isRestart ? self->config.ipAddr : WS_DEFAULT_IPADDR);
				UInt16 port = (isRestart ? self->config.port : WS_DEFAULT_PORT);
				esif_flags_t flags = (isRestart ? self->config.flags : WS_DEFAULT_FLAGS);

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
		.ipAddr = WS_DEFAULT_IPADDR,
		.port = WS_DEFAULT_PORT,
		.flags = WS_DEFAULT_FLAGS,
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

// Exported WebServer Interface Function
ESIF_EXPORT esif_error_t ESIF_CALLCONV GetWsInterface(EsifWsInterfacePtr ifacePtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (ifacePtr) {
		// Must be an exact Interface match
		if ((ifacePtr->hdr.fIfaceType != eIfaceTypeWeb) ||
			(ifacePtr->hdr.fIfaceVersion != WS_IFACE_VERSION) ||
			(ifacePtr->hdr.fIfaceSize != sizeof(*ifacePtr))) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else if (g_ifaceWs || g_esifApp.esifHandle != ESIF_INVALID_HANDLE || g_esifApp.appHandle != ESIF_INVALID_HANDLE) {
			rc = ESIF_E_WS_ALREADY_STARTED;
		}
		else {
			// Fill in ESIF_WS side of Interface
			esif_ccb_strcpy(ifacePtr->wsVersion, ESIF_WS_VERSION, sizeof(ifacePtr->wsVersion));
			ifacePtr->fEsifWsInitFuncPtr = EsifWsInit;
			ifacePtr->fEsifWsExitFuncPtr = EsifWsExit;
			ifacePtr->fEsifWsStartFuncPtr = EsifWsStart;
			ifacePtr->fEsifWsStopFuncPtr = EsifWsStop;
			ifacePtr->fEsifWsIsStartedFuncPtr = EsifWsIsStarted;
			ifacePtr->fEsifWsAllocFuncPtr = EsifWsAlloc;
			g_ifaceWs = ifacePtr;
			rc = ESIF_OK;
		}
	}
	return rc;
}

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