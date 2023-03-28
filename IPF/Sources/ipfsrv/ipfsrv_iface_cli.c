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

#include "ipfsrv_appmgr.h"
#include "ipfsrv_authmgr.h"

/////////////////////////////////////////////////////////////////////////////////
// IPF Client App Interface Functions called by ESIF
// These are the App Interface functions exposed to ESIF for each IPF Client.
//
// In order for the appstart sequence (GetName, GetDescription, GetVersion, AppCreate)
// to complete, we make the following assumptions:
//  1. ESIF prefills the GetName, GetDescription, and GetVersion parameters with the ESIF appName
//  2. GetName is called before AppCreate on the same thread so AppCreate can lookup the Session by Thread ID
//  3. The appName passed to GetName is unique, otherwise the connection will be terminated.
//  3. The esifHandle passed to AppCreate is unique, otherwise the connection will be terminated.
//  4. The appHandle returned by AppCreate to ESIF is our unique IPF Client Session Handle (ipfHandle)
/////////////////////////////////////////////////////////////////////////////////

esif_error_t ESIF_CALLCONV IpfCli_AppGetName(EsifDataPtr appNamePtr)
{
	AppSession *session = AppSessionMgr_GetSessionByName(appNamePtr ? appNamePtr->buf_ptr : "");
	esif_error_t rc = ESIF_E_NOT_FOUND;

	if (session && appNamePtr) {
		session->threadId = ESIF_NULL_THREAD_ID;

		rc = Irpc_Request_AppGetName(appNamePtr);

		if (rc == ESIF_OK || rc == ESIF_E_NEED_LARGER_BUFFER) {
			appNamePtr->data_len = (UInt32)esif_ccb_strlen(appNamePtr->buf_ptr, appNamePtr->buf_len) + 1;
		}
		else {
			IpfClient_Close(session->ipfHandle);
		}

		if (rc == ESIF_OK) {
			esif_ccb_strcpy(session->appName, appNamePtr->buf_ptr, sizeof(session->appName));
			rc = IpfClient_Rename(session->ipfHandle, (esif_string)appNamePtr->buf_ptr);
		}
		if (rc == ESIF_OK) {
			esif_ccb_strcpy(session->esifName, appNamePtr->buf_ptr, sizeof(session->esifName));
			esif_ccb_strlwr(session->esifName, sizeof(session->esifName));
			if (session->appHandle == ESIF_INVALID_HANDLE) {
				session->threadId = esif_ccb_thread_id_current();
			}
		}
	}
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV IpfCli_AppGetDescription(EsifDataPtr appDescriptionPtr)
{
	AppSession *session = AppSessionMgr_GetSessionByName(appDescriptionPtr ? appDescriptionPtr->buf_ptr : "");
	esif_error_t rc = ESIF_E_NOT_FOUND;

	if (session && appDescriptionPtr) {
		rc = Irpc_Request_AppGetDescription(appDescriptionPtr);
		if (rc == ESIF_OK) {
			esif_ccb_strcpy(session->appDescription, appDescriptionPtr->buf_ptr, sizeof(session->appDescription));
		}
		if (rc == ESIF_OK || rc == ESIF_E_NEED_LARGER_BUFFER) {
			appDescriptionPtr->data_len = (UInt32)esif_ccb_strlen(appDescriptionPtr->buf_ptr, appDescriptionPtr->buf_len) + 1;
		}
		else {
			IpfClient_Close(session->ipfHandle);
		}
	}
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV IpfCli_AppGetVersion(EsifDataPtr appVersionPtr)
{
	AppSession *session = AppSessionMgr_GetSessionByName(appVersionPtr ? appVersionPtr->buf_ptr : "");
	esif_error_t rc = ESIF_E_NOT_FOUND;

	if (session && appVersionPtr) {
		rc = Irpc_Request_AppGetVersion(appVersionPtr);
		if (rc == ESIF_OK) {
			esif_ccb_strcpy(session->appVersion, appVersionPtr->buf_ptr, sizeof(session->appVersion));
		}
		if (rc == ESIF_OK || rc == ESIF_E_NEED_LARGER_BUFFER) {
			appVersionPtr->data_len = (UInt32)esif_ccb_strlen(appVersionPtr->buf_ptr, appVersionPtr->buf_len) + 1;
		}
		else {
			IpfClient_Close(session->ipfHandle);
		}
	}
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV IpfCli_AppGetIntro(
	const esif_handle_t appHandle,
	EsifDataPtr appIntroPtr
)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	esif_error_t rc = ESIF_E_INVALID_HANDLE;

	if (session) {
		rc = Irpc_Request_AppGetIntro(
			appHandle,
			appIntroPtr
		);
		if (rc == ESIF_OK) {
			esif_ccb_strcpy(session->appIntro, appIntroPtr->buf_ptr, sizeof(session->appIntro));
		}
	}
	AppSession_PutRef(session);
	return rc;

}

esif_error_t ESIF_CALLCONV IpfCli_AppCreate(
	AppInterfaceSetPtr ifaceSetPtr,
	const esif_handle_t esifHandle,
	esif_handle_t *appHandlePtr,
	const AppDataPtr appDataPtr,
	const eAppState appInitialState
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Verify ESIF Handle is unique, then Lookup Session by ThreadID. Assumes GetName already called on this thread
	if (ifaceSetPtr && appHandlePtr) {
		AppSession *session = AppSessionMgr_GetSessionByEsifHandle(esifHandle);
		if (session) {
			AppSession_PutRef(session);
			session = NULL;
		}
		else {
			session = AppSessionMgr_GetSessionByThreadId(esif_ccb_thread_id_current());
		}

		if (session == NULL) {
			rc = ESIF_E_INVALID_HANDLE;
		}
		else {
			esif_handle_t appHandleClient = ESIF_INVALID_HANDLE;

			// Set ESIF's appHandle immediately so it can be used during AppCreate to register events
			*appHandlePtr = session->ipfHandle;
			session->esifHandle = esifHandle;
			session->ifaceSet = *ifaceSetPtr;

			// Create App on Remote Client
			rc = Irpc_Request_AppCreate(
				ifaceSetPtr,
				esifHandle,
				&appHandleClient,
				appDataPtr,
				appInitialState
			);
			session->threadId = ESIF_NULL_THREAD_ID;

			if (rc == ESIF_OK) {
				session->appHandle = appHandleClient;
			}
			else {
				*appHandlePtr = ESIF_INVALID_HANDLE;
				IpfClient_Close(session->ipfHandle);
			}
		}
		AppSession_PutRef(session);
	}
	return rc;
}

esif_error_t ESIF_CALLCONV IpfCli_AppDestroy(esif_handle_t appHandle)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	if (session) {
		rc = Irpc_Request_AppDestroy(appHandle);
		IpfClient_Close(session->ipfHandle);
	}
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV IpfCli_AppSuspend(esif_handle_t appHandle)
{
	return Irpc_Request_AppSuspend(appHandle);
}

esif_error_t ESIF_CALLCONV IpfCli_AppResume(esif_handle_t appHandle)
{
	return Irpc_Request_AppResume(appHandle);
}

esif_error_t ESIF_CALLCONV IpfCli_AppCommand(
	const esif_handle_t appHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response)
{
	return Irpc_Request_AppCommand(
		appHandle,
		argc,
		argv,
		response
	);
}

esif_error_t ESIF_CALLCONV IpfCli_AppGetStatus(
	const esif_handle_t appHandle,
	const eAppStatusCommand command,
	const UInt32 appStatusIn,
	EsifDataPtr appStatusOut
)
{
	return Irpc_Request_AppGetStatus(
		appHandle,
		command,
		appStatusIn,
		appStatusOut
	);
}

esif_error_t ESIF_CALLCONV IpfCli_AppParticipantCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const AppParticipantDataPtr participantDataPtr,
	const eParticipantState initialParticipantState
)
{
	return Irpc_Request_AppParticipantCreate(
		appHandle,
		participantHandle,
		participantDataPtr,
		initialParticipantState
	);
}

esif_error_t ESIF_CALLCONV IpfCli_AppParticipantDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle
)
{
	return Irpc_Request_AppParticipantDestroy(
		appHandle,
		participantHandle
	);
}

esif_error_t ESIF_CALLCONV IpfCli_AppDomainCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const AppDomainDataPtr domainDataPtr,
	const eDomainState domainInitialState
)
{
	return Irpc_Request_AppDomainCreate(
		appHandle,
		participantHandle,
		domainHandle,
		domainDataPtr,
		domainInitialState
	);
}

esif_error_t ESIF_CALLCONV IpfCli_AppDomainDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle
)
{
	return Irpc_Request_AppDomainDestroy(
		appHandle,
		participantHandle,
		domainHandle
	);
}

esif_error_t ESIF_CALLCONV IpfCli_AppEvent(
	esif_handle_t appHandle,
	esif_handle_t participantHandle,
	esif_handle_t domainHandle,
	const EsifDataPtr eventDataPtr,
	const EsifDataPtr eventGuid
)
{
	return Irpc_Request_AppEvent(
		appHandle,
		participantHandle,
		domainHandle,
		eventDataPtr,
		eventGuid
	);
}

// App Interface Set used by IPFSRV Client Sessions
static AppInterfaceSet g_ifaceSetClient = {
	.hdr = {
		.fIfaceType = eIfaceTypeApplication,
		.fIfaceVersion = APP_INTERFACE_VERSION,
		.fIfaceSize = sizeof(AppInterfaceSet)
	}, 
	// App Interface exposed to ESIF from IPFSRV Client Sessions
	.appIface = {
		.fAppCreateFuncPtr = IpfCli_AppCreate,
		.fAppDestroyFuncPtr = IpfCli_AppDestroy,
		.fAppSuspendFuncPtr = IpfCli_AppSuspend,
		.fAppResumeFuncPtr = IpfCli_AppResume,
		.fAppGetVersionFuncPtr = IpfCli_AppGetVersion,
		.fAppCommandFuncPtr = IpfCli_AppCommand,
		.fAppGetIntroFuncPtr = IpfCli_AppGetIntro,
		.fAppGetDescriptionFuncPtr = IpfCli_AppGetDescription,
		.fAppGetNameFuncPtr = IpfCli_AppGetName,
		.fAppGetStatusFuncPtr = IpfCli_AppGetStatus,
		.fParticipantCreateFuncPtr = IpfCli_AppParticipantCreate,
		.fParticipantDestroyFuncPtr = IpfCli_AppParticipantDestroy,
		.fDomainCreateFuncPtr = IpfCli_AppDomainCreate,
		.fDomainDestroyFuncPtr = IpfCli_AppDomainDestroy,
		.fAppEventFuncPtr = IpfCli_AppEvent,
	},
	// ESIF Interface exposed to IPFSRV Client Sessions, filled in by ESIF on AppCreate
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

// Return IPF Client App Interface Set. May Contain NULLs
AppInterfaceSet IpfCli_GetInterface()
{
	return g_ifaceSetClient;
}
