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


#include "esif_ccb_thread.h"
#include "ipf_common.h"
#include "ipf_dll.h"
#include "ipf_dll_ipc.h"
#include "ipf_dll_esif.h"
#include "ipf_dll_core.h"
#include "ipf_apploader.h"
#include "ipf_sdk_version.h"

#ifndef IPF_TRACE_AVAILABLE
#include "ipf_trace.h"
#else
#include "Tracing.h" // For EF support
#endif

#ifdef NO_IPFCORE_SUPPORT
// IpfCoreLib currently unsupported in Open Source distributions
esif_error_t ESIF_CALLCONV IpfCoreLib_GetInterface(IpfCoreLib* self)
{
	UNREFERENCED_PARAMETER(self);
	return ESIF_E_NOT_IMPLEMENTED;
}
#endif

#define	VOIDRESULT	((void)0)
#define APP_RESTART_DELAY_MSEC		1000			// Reconnect Timeout (ms)
#define APP_SUSPENDED_DELAY_MSEC	(60*60*1000)	// Signal Timeout (ms) for AppWorkerThread Suspended due to Unauthorized Pipe Access or Disabled App
#define APP_SUSPENDED_PAUSE_MSEC	5000			// Signal Timeout (ms) for AppWorkerThread Paused due to Unauthorized Pipe Access or Disabled App (Initial Failure Only)
#define APP_LOADLIB_RETRY_MSEC		1000			// LoadLibraries Retry Delay (ms)

typedef void (ESIF_CALLCONV* SetIpfLibraryFuncPtr)(const char* libname);
typedef void (ESIF_CALLCONV* SetAppLibraryFuncPtr)(const char* libname);

typedef enum {
	UNINIT = 0,
	INIT,
	RUNNING,
	STOPPED
} RunState_t;

typedef struct AppLoader_s {
	char applibpath[MAX_PATH];

	esif_lib_t appLibObj;
	IpfCoreLib coreObj;
	EsifAppLib esifObj;
	IpfIpcLib ipcObj;

	IpcSession_t client;
	IpfSessionInfo info;
	RunState_t runState;
	esif_wthread_t appMgrThread;
	esif_ccb_lock_t  lock;

	esif_error_t connectState;
	signal_t signal;
} AppLoader;

// Singleton Object used for Legacy Support
static AppLoader* g_singleton = NULL;

// Load IPF Core Library or ESIF App and IPF Libraries
static esif_error_t ESIF_CALLCONV LoadLibraries(AppLoader* self)
{
	esif_error_t rc = ESIF_OK;
	GetIpfInterfaceFuncPtr getIpfIfaceFP = NULL;
	GetAppInterfaceV2FuncPtr getEsifIfaceFP = NULL;
	
	if (NULL == self || self->applibpath[0] == 0) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Only load the App DLL if it's not already loaded to avoid resource leaks
	if (!esif_ccb_library_isloaded(self->appLibObj)) {
		IPF_TRACE_INFO("Try loading the app library: %s", self->applibpath);

		// Load the app dll using the full path
		self->appLibObj = esif_ccb_library_load(self->applibpath);

		if (!esif_ccb_library_isloaded(self->appLibObj)) {
			IPF_TRACE_ERROR("Error loading library (%d): %s\n", esif_ccb_library_error(self->appLibObj), esif_ccb_library_errormsg(self->appLibObj));
			rc = ESIF_E_NO_CREATE;
			goto exit;
		}

		// Try to get the IPF and ESIF interface functions
		getIpfIfaceFP = (GetIpfInterfaceFuncPtr)esif_ccb_library_get_func(self->appLibObj, GET_IPF_INTERFACE_FUNCTION);
		getEsifIfaceFP = (GetAppInterfaceV2FuncPtr)esif_ccb_library_get_func(self->appLibObj, GET_APPLICATION_INTERFACE_FUNCTION);

		if (NULL != getIpfIfaceFP) {
			// It's an IPF core app
			IPF_TRACE_INFO("IPF core app found : %s", self->applibpath);
			self->coreObj.libModule = self->appLibObj;
			rc = IpfCoreLib_GetInterface(&self->coreObj);
		}
		else if (NULL != getEsifIfaceFP) {
			// It's an ESIF app
			IPF_TRACE_INFO("ESIF core app found : %s", self->applibpath);
			self->esifObj.libModule = self->appLibObj;
			rc = EsifAppLib_GetInterface(&self->esifObj);
		}
		else {
			// No interface found
			rc = ESIF_E_NO_CREATE;
			IPF_TRACE_ERROR("No app interface found: %s", self->applibpath);
		}

		if (rc != ESIF_OK) {
			goto exit;
		}
	}

	// Load IPC Dynamic Library
	IPF_TRACE_INFO("Loading IPF IPC Library: %s", DEFAULT_IPCLIB);
	rc = IpfIpcLib_Load(&self->ipcObj, DEFAULT_IPCLIB);

exit:
	return (int)rc;
}

// Worker Thread to load App libraries and maintain a persistent IPC connection for App
static void* ESIF_CALLCONV AppWorkerThread(void* ctx)
{
	esif_error_t rc = ESIF_OK;
	AppLoader* self = (AppLoader*)ctx;

	IPF_TRACE_DEBUG("AppWorkerThread Starting\n");

	if (self == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	self->connectState = ESIF_E_WS_DISC;

	// 1. CORE Mode Loading CORE App
	if (esif_ccb_library_isloaded(self->coreObj.libModule)) {
		IPF_TRACE_DEBUG("Connecting to %s via CORE ...\n", self->info.v1.serverAddr);

		//// Initialize CORE API
		IpfIface* coreIface = &self->coreObj.iface;

		rc = (coreIface->IpfCore_Init ? coreIface->IpfCore_Init() : ESIF_E_NOT_IMPLEMENTED);

		//// Create CORE Session
 		self->client = IPF_INVALID_SESSION;
		if (rc == ESIF_OK) {
			self->client = (coreIface->IpfCore_SessionCreate ? coreIface->IpfCore_SessionCreate(&self->info) : IPF_INVALID_SESSION);
		}

		if (self->client != IPF_INVALID_SESSION) {
			// Start IPC Session (Connect to Server)
			if (rc == ESIF_OK) {
				rc = (coreIface->IpfCore_SessionConnect ? coreIface->IpfCore_SessionConnect(self->client) : ESIF_E_NOT_IMPLEMENTED);
				self->connectState = rc;
			}

			// Process Messages until Connection Closed
			if (rc == ESIF_OK) {
				IPF_TRACE_DEBUG("Connected. SDK:%s Client:%s\n", self->info.v1.sdkVersion, IPF_SDK_VERSION);
				(coreIface->IpfCore_SessionWaitForStop ? coreIface->IpfCore_SessionWaitForStop(self->client) : VOIDRESULT);
				IPF_TRACE_DEBUG("Disconnected\n");
			}
			else {
				IPF_TRACE_DEBUG("%s\n", esif_rc_str(rc));
			}
			// Synchronize calls to IPF functions using a lock to avoid a known race condition on disconnect
			esif_ccb_write_lock(&self->lock);
			(coreIface->IpfCore_SessionDestroy ? coreIface->IpfCore_SessionDestroy(self->client) : VOIDRESULT);
			esif_ccb_write_unlock(&self->lock);
		}
		// Synchronize calls to IPF functions using a lock to avoid a known race condition on disconnect
		esif_ccb_write_lock(&self->lock);
		(coreIface->IpfCore_Exit ? coreIface->IpfCore_Exit() : VOIDRESULT); 
		esif_ccb_write_unlock(&self->lock);
	}
	// 2. IPC Mode Loading ESIF App
	else if (esif_ccb_library_isloaded(self->ipcObj.libModule) && esif_ccb_library_isloaded(self->esifObj.libModule)) {
		IPF_TRACE_INFO("Connecting to %s via IPC ...", self->info.v1.serverAddr);

		// Initialize IPC
		rc = (self->ipcObj.iface.Ipc_Init ? self->ipcObj.iface.Ipc_Init() : ESIF_E_NOT_IMPLEMENTED);

		// Set App Interface Function Pointers for use by IPC Session
		if (rc == ESIF_OK) {
			rc = (self->ipcObj.iface.Ipc_SetAppIface ? self->ipcObj.iface.Ipc_SetAppIface(&self->esifObj.ifaceSet) : ESIF_E_NOT_IMPLEMENTED);
		}

		// Create IPC Client Session
		self->client = IPC_INVALID_SESSION;
		if (rc == ESIF_OK) {
			self->client = (self->ipcObj.iface.IpcSession_Create ? self->ipcObj.iface.IpcSession_Create(&self->info) : IPC_INVALID_SESSION);
		}

		if (self->client != IPC_INVALID_SESSION) {

			// Start IPC Session (Connect to Server)
			if (rc == ESIF_OK) {
				rc = (self->ipcObj.iface.IpcSession_Connect ? self->ipcObj.iface.IpcSession_Connect(self->client) : ESIF_E_NOT_IMPLEMENTED);
				self->connectState = rc;
			}

			// Process Messages until Connection Closed
			if (rc == ESIF_OK) {
				IPF_TRACE_DEBUG("Connected\n");
				(self->ipcObj.iface.IpcSession_WaitForStop ? self->ipcObj.iface.IpcSession_WaitForStop(self->client) : VOIDRESULT);
				IPF_TRACE_DEBUG("Disconnected\n");
			}
			else {
				IPF_TRACE_DEBUG("%s\n", esif_rc_str(rc));
			}	
			// Synchronize calls to IPF functions using a lock to avoid a known race condition on disconnect
			esif_ccb_write_lock(&self->lock);
			(self->ipcObj.iface.IpcSession_Destroy ? self->ipcObj.iface.IpcSession_Destroy(self->client) : VOIDRESULT);
			esif_ccb_write_unlock(&self->lock);
		}
		// Synchronize calls to IPF functions using a lock to avoid a known race condition on disconnect
		esif_ccb_write_lock(&self->lock);
		(self->ipcObj.iface.Ipc_Exit ? self->ipcObj.iface.Ipc_Exit() : VOIDRESULT);
		esif_ccb_write_unlock(&self->lock);
	}
		
	// Suspend Thread if Not Authorized to connect to the given Pipe
	if (self->connectState == ESIF_E_WS_UNAUTHORIZED) {
		// To avoid Race condition with Server, only pause for a short time the first time we get an UNAUTHORIZED error.
		// After the second UNAUTHORIZED error, suspend for a much longer time.
		static u32 suspend_delay = APP_SUSPENDED_PAUSE_MSEC;
		signal_wait_timeout(&self->signal, suspend_delay);
		self->connectState = ESIF_E_WS_DISC;
		suspend_delay = APP_SUSPENDED_DELAY_MSEC;
	}

exit:
	IPF_TRACE_DEBUG("Exiting AppWorkerThread\n");
	return NULL;
}

// Main thread to maintain a peristent IPC connection until Service is Stopped
static void* ESIF_CALLCONV AppManagerThread(void* ctx)
{
	esif_error_t rc = ESIF_OK;
	AppLoader* self = (AppLoader*)ctx;
	RunState_t runstate = STOPPED;

	esif_wthread_t appThread;

	IPF_TRACE_DEBUG("AppManagerThread Starting\n");

	if (self == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Keep trying to start the application in a separate thread
	// while the service is running.  When the service is stopped, 
	// self->runState will be set to STOPPED.
	while (1) {
		esif_ccb_write_lock(&self->lock);
		runstate = self->runState;
		esif_ccb_write_unlock(&self->lock);

		if (RUNNING != runstate) {
			break;
		}

		// Try to Load Libraries and retry after a short nap if unsuccessful.
		// This is necessary since the IPF drivers may not be fully loaded and exposing the interface
		// used to lookup the DriverStore folder so we keep trying until the driver is fully loaded.
		if (!esif_ccb_library_isloaded(self->ipcObj.libModule)) {
			rc = LoadLibraries(self);
			IPF_TRACE_DEBUG("LoadLibraries = %s\n", esif_rc_str(rc));
			if (ESIF_OK != rc) {
				esif_ccb_sleep_msec(APP_LOADLIB_RETRY_MSEC);
				continue;
			}
		}
		
		esif_ccb_wthread_init(&appThread);
		rc = esif_ccb_wthread_create(&appThread, AppWorkerThread, self);

		if (ESIF_OK == rc) {
			esif_ccb_wthread_join(&appThread);
		}
		esif_ccb_wthread_uninit(&appThread);

		esif_ccb_write_lock(&self->lock);
		runstate = self->runState;
		esif_ccb_write_unlock(&self->lock);

		if (RUNNING == runstate) {
			esif_ccb_sleep_msec(APP_RESTART_DELAY_MSEC);
		}
	}
exit:
	IPF_TRACE_DEBUG("Exiting AppManagerThread\n");
	return 0;
}

// Legacy support for Singleton Object
AppLoader* ESIF_CALLCONV AppLoader_GetInstance()
{
	if (g_singleton == NULL) {
		g_singleton = AppLoader_Create();
	}
	return g_singleton;
}

// Create a new Uninitialized AppLoader object
AppLoader* ESIF_CALLCONV AppLoader_Create()
{
	AppLoader* obj = (AppLoader*)esif_ccb_malloc(sizeof(AppLoader));
	return obj;
}

// Destroy an Uninitialized AppLoader object
void ESIF_CALLCONV AppLoader_Destroy(AppLoader* self)
{
	esif_ccb_free(self);
}

// Initialize an AppLoader object
// This may be called multiple times before AppLoader_Start() to change path and server parameters
esif_error_t ESIF_CALLCONV AppLoader_Init(
	AppLoader* self,
	const char* libpath,
	const char* serveraddr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self) {
		// Initialize object once
		if (self->runState == UNINIT) {
			*self->applibpath = 0;
			self->info = (const IpfSessionInfo){ 0 };
			self->coreObj = (const IpfCoreLib){ 0 };
			self->esifObj = (const EsifAppLib){ 0 };
			self->ipcObj = (const IpfIpcLib){ 0 };
			self->client = IPC_INVALID_SESSION;

			// Synchronize calls to IPF functions using a lock to avoid a known race condition on disconnect
			esif_ccb_lock_init(&self->lock);

			// Connection Suspend Signal
			self->connectState = ESIF_E_WS_DISC;
			signal_init(&self->signal);

			self->runState = INIT;
		}

		// Initialize App Library Path and Session Parameters; May be called multiple times
		if (libpath) {
			esif_ccb_strcpy(self->applibpath, libpath, sizeof(self->applibpath));
		}
		if (serveraddr) {
			self->info.v1.revision = IPF_SESSIONINFO_REVISION;
			esif_ccb_strcpy(self->info.v1.serverAddr, serveraddr, sizeof(self->info.v1.serverAddr));
		}
		if (self->applibpath[0] && self->info.v1.serverAddr[0]) {
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Load Core or ESIF App Libraries and start App after establishing IPC connection
esif_error_t ESIF_CALLCONV AppLoader_Start(AppLoader* self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self) {
		self->runState = RUNNING;

		// Init and create the app manager thread that will start the application
		esif_ccb_wthread_init(&self->appMgrThread);
		rc = esif_ccb_wthread_create(&self->appMgrThread, AppManagerThread, self);
	}
	return rc;
}

// Stop App, Disconnect IPC, and wait for Worker Threads to stop
void ESIF_CALLCONV AppLoader_Stop(AppLoader* self)
{
	if (self) {
		// Wake up Suspended Worker Thread
		if (self->connectState == ESIF_E_WS_UNAUTHORIZED) {
			esif_ccb_write_lock(&self->lock);
			self->runState = STOPPED;
			esif_ccb_write_unlock(&self->lock);
			self->connectState = ESIF_E_WS_DISC;
			signal_post(&self->signal);
		}

		// Calling disconnect will cause the application to exit.
		// The application manager thread will exit when the runState is STOPPED
		if (esif_ccb_library_isloaded(self->coreObj.libModule)) {
			IpfIface* coreIface = &self->coreObj.iface;
			// Disconnect and Close Session
			// Synchronize calls to IPF functions using a lock to avoid a known race condition on disconnect
			esif_ccb_write_lock(&self->lock);
			self->runState = STOPPED;
			(coreIface->IpfCore_SessionDisconnect ? coreIface->IpfCore_SessionDisconnect(self->client) : VOIDRESULT);
			esif_ccb_write_unlock(&self->lock);

			// Wait for app manager thread to exit and uninit
			esif_ccb_wthread_join(&self->appMgrThread);
			esif_ccb_wthread_uninit(&self->appMgrThread);
		}
		else if (esif_ccb_library_isloaded(self->esifObj.libModule)) {
			// Disconnect and close session
			// Synchronize calls to IPF functions using a lock to avoid a known race condition on disconnect
			esif_ccb_write_lock(&self->lock);
			self->runState = STOPPED;
			if (esif_ccb_library_isloaded(self->ipcObj.libModule)) {
				(self->ipcObj.iface.IpcSession_Disconnect ? self->ipcObj.iface.IpcSession_Disconnect(self->client) : VOIDRESULT);
			}
			esif_ccb_write_unlock(&self->lock);

			// Wait for app manager thread to exit and uninit
			esif_ccb_wthread_join(&self->appMgrThread);
			esif_ccb_wthread_uninit(&self->appMgrThread);
		}
	}
	return;
}

// Uninitialize AppLoader Object after Stopping App, if necessary
void ESIF_CALLCONV AppLoader_Exit(AppLoader* self)
{
	if (self && self->runState != UNINIT) {
		if (self->runState != STOPPED) {
			AppLoader_Stop(self);
		}

		IpfIpcLib_Unload(&self->ipcObj);
		esif_ccb_library_unload(self->appLibObj);
		self->appLibObj = NULL;
		esif_ccb_lock_uninit(&self->lock);
		signal_uninit(&self->signal);
		*self = (const AppLoader){ 0 };
		self->runState = UNINIT;
		self->connectState = ESIF_E_WS_DISC;
	}

	// Destroy Singleton Object
	if (self && self == g_singleton) {
		AppLoader_Destroy(g_singleton);
		g_singleton = NULL;
	}
	return;
}

// Pause (Stop) App
esif_error_t ESIF_CALLCONV AppLoader_Pause(AppLoader* self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		AppLoader_Stop(self);
		rc = ESIF_OK;
	}
	return rc;
}

// Resume (Start) App
esif_error_t ESIF_CALLCONV AppLoader_Continue(AppLoader* self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		rc = AppLoader_Start(self);
	}
	return rc;
}

