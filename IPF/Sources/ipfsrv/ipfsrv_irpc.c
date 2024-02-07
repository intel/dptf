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

#include "ipf_ipc_codec.h"
#include "ipf_ipc_trxmgr.h"
#include "ipfsrv_appmgr.h"
#include "ipfsrv_authmgr.h"

#pragma warning(disable:4204)

// Wait for RPC Transaction to complete and Close Conneciton if Request Failed
static esif_error_t IrpcTransaction_WaitForResponse(IrpcTransaction *self)
{
	esif_error_t rc = IrpcTransaction_Wait(self);
	
	if (rc == ESIF_E_SESSION_REQUEST_FAILED) {
		IpfClient_Stop(self->ipfHandle);
		rc = ESIF_E_SESSION_DISCONNECTED;
	}
	return rc;
}

// Shared AppGetName, AppGetVersion, AppGetDescription, AppGetIntro Handler
esif_error_t Irpc_Request_AppGetEsifData(
	eIrpcFunction funcId,
	EsifDataPtr data,
	esif_handle_t appHandle)
{
	AppSession *session = NULL;
	if (funcId == IrpcFunc_AppGetIntro) {
		session = AppSessionMgr_GetSessionByHandle(appHandle);
	}
	else if (data) {
		session = AppSessionMgr_GetSessionByName(data->buf_ptr);
	}
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx && funcId && data) {
		trx->ipfHandle = session->ipfHandle;
		rc = ESIF_E_INVALID_REQUEST_TYPE;

		switch (funcId) {
		case IrpcFunc_AppGetName:
		case IrpcFunc_AppGetDescription:
		case IrpcFunc_AppGetVersion:
		case IrpcFunc_AppGetIntro:
		{
			if (Irpc_Encode_AppGetStringFunc(
				trx,
				funcId,
				session->appHandle,
				data)) {
				rc = Irpc_SendMessage(trx);
			}

			// Wait for Response to be Receieved from Server
			if (rc == ESIF_OK) {
				rc = IrpcTransaction_WaitForResponse(trx);
			}

			if (rc == ESIF_OK) {
				Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);
				eIrpcFunction replyFuncId = (irpcMsg ? Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) : IrpcFunc_None);

				switch (Irpc_Uncast_eIrpcFunction(replyFuncId)) {
				case IrpcFunc_AppGetName:
				case IrpcFunc_AppGetDescription:
				case IrpcFunc_AppGetVersion:
				case IrpcFunc_AppGetIntro:
				{
					Encoded_AppGetStringFunction *ipcobj = (Encoded_AppGetStringFunction *)irpcMsg;
					if (ipcobj && ((rc = Irpc_Unmarshall_EsifDataPtr(data, &ipcobj->stringData, Irpc_OffsetFrom(*ipcobj, stringData))) == ESIF_OK || Irpc_Uncast_eEsifError(ipcobj->result) != ESIF_OK)) {
						rc = Irpc_Uncast_eEsifError(ipcobj->result);
					}
					break;
				}
				default:
					rc = ESIF_E_INVALID_REQUEST_TYPE;
					break;
				}
			}
			break;
		}
		default:
			break;
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

// Shared AppDestroy, AppSuspend, AppResume Handler
esif_error_t ESIF_CALLCONV Irpc_Request_AppHandle(
	const eIrpcFunction funcId,
	const esif_handle_t appHandle)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_INVALID_HANDLE;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx && session->appHandle != ESIF_INVALID_HANDLE) {
		trx->ipfHandle = session->ipfHandle;

		if (Irpc_Encode_AppHandleFunc(
			trx,
			funcId,
			session->appHandle)) {
			rc = Irpc_SendMessage(trx);
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);
			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == funcId) {
				Encoded_AppHandleFunction *ipcobj = (Encoded_AppHandleFunction *)irpcMsg;

				rc = Irpc_Uncast_eEsifError(ipcobj->result);

				if (rc == ESIF_OK) {

					IPFDEBUG("<<<<< %s(%d): esifHandle=0x%08llX, appHandle=0x%08llX\n", ESIF_FUNC, (int)funcId, session->esifHandle, session->appHandle);

					if (funcId == IrpcFunc_AppDestroy) {
						AppSession_Disconnect(session);
					}
				}
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

/*
** Irpc_Request_App* Functions initiate a blocking RPC Call on the current thread to the Remote Client using the given parameters
**
** Note that we depend on the App Interface functions being called from ESIF in a specific order with specific input so that
** we are able to lookup the correct IPF Client Session using its appHandle, appName, or ThreadID, depending on what is available.
**
** The appHandle passed into these functions and sent to the RPC Client is the Session's ipfHandle
** The esifHandle passed into these functions and sent down to ESIF is the esifHandle assigned by ESIF
*/

// 1. GetName called first with ESIF prefilling the buffer with ESIF's appName
esif_error_t ESIF_CALLCONV Irpc_Request_AppGetName(EsifDataPtr appNamePtr)
{
	return Irpc_Request_AppGetEsifData(IrpcFunc_AppGetName, appNamePtr, ESIF_INVALID_HANDLE);
}

// 2. GetDescription called next with ESIF prefilling the buffer with ESIF's appName
esif_error_t ESIF_CALLCONV Irpc_Request_AppGetDescription(EsifDataPtr appDescriptionPtr)
{
	return Irpc_Request_AppGetEsifData(IrpcFunc_AppGetDescription, appDescriptionPtr, ESIF_INVALID_HANDLE);
}

// 3. GetVersion called next with ESIF prefilling the buffer with ESIF's appName
esif_error_t ESIF_CALLCONV Irpc_Request_AppGetVersion(EsifDataPtr appVersionPtr)
{
	return Irpc_Request_AppGetEsifData(IrpcFunc_AppGetVersion, appVersionPtr, ESIF_INVALID_HANDLE);
}

// 4. AppCreate called next before appHandle has been assigned, so lookup using the same ThreadId that called GetName
//    Note that the RPC Client may start making ESIF Requests before the RPC Response is received
esif_error_t ESIF_CALLCONV Irpc_Request_AppCreate(
	AppInterfaceSetPtr ifaceSetPtr,
	const esif_handle_t esifHandle,
	esif_handle_t *appHandlePtr,
	const AppDataPtr appDataPtr,
	const eAppState appInitialState)
{
	AppSession *session = AppSessionMgr_GetSessionByThreadId(esif_ccb_thread_id_current());
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx && ifaceSetPtr && appHandlePtr && appDataPtr) {
		trx->ipfHandle = session->ipfHandle;
		*appHandlePtr = ESIF_INVALID_HANDLE;

		// Copy AppInterfaceSet and *appHandlePtr into Session Context
		AppInterfaceSet ifaceSetProxy = *ifaceSetPtr;
		esif_ccb_memset(&ifaceSetProxy.appIface, 0, sizeof(ifaceSetProxy.appIface));

		// Use IPF Client Session Handle for the RPC Client's esifHandle
		UNREFERENCED_PARAMETER(esifHandle);
		if (Irpc_Encode_AppCreateFunc(
			trx,
			&ifaceSetProxy,
			trx->ipfHandle,
			appHandlePtr,
			appDataPtr,
			appInitialState)) {
			rc = Irpc_SendMessage(trx);
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			IPFDEBUG(">>>>> %s: Sent AppCreate\n", ESIF_FUNC);
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);
			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_AppCreate) {
				Encoded_AppCreateFunction *ipcobj = (Encoded_AppCreateFunction *)irpcMsg;

				rc = Irpc_Uncast_eEsifError(ipcobj->result);

				if (rc == ESIF_OK) {
					Encoded_EsifHandle *encoded_appHandlePtr = (void *)(Irpc_OffsetFrom(*ipcobj, appHandlePtr) + Irpc_Uncast_UInt32(ipcobj->appHandlePtr.offset));
					*appHandlePtr = Irpc_Uncast_EsifHandle(*encoded_appHandlePtr);
				}
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

// 5. AppGetIntro is called after AppCreate completes so lookup using the appHandle
esif_error_t ESIF_CALLCONV Irpc_Request_AppGetIntro(
	const esif_handle_t appHandle,
	EsifDataPtr appIntroPtr)
{
	return Irpc_Request_AppGetEsifData(IrpcFunc_AppGetIntro, appIntroPtr, appHandle);
}

esif_error_t ESIF_CALLCONV Irpc_Request_AppDestroy(const esif_handle_t appHandle)
{
	return Irpc_Request_AppHandle(IrpcFunc_AppDestroy, appHandle);
}

esif_error_t ESIF_CALLCONV Irpc_Request_AppSuspend(const esif_handle_t appHandle)
{
	return Irpc_Request_AppHandle(IrpcFunc_AppSuspend, appHandle);
}

esif_error_t ESIF_CALLCONV Irpc_Request_AppResume(const esif_handle_t appHandle)
{
	return Irpc_Request_AppHandle(IrpcFunc_AppResume, appHandle);
}


esif_error_t ESIF_CALLCONV Irpc_Request_AppCommand(
	const esif_handle_t appHandle,
	const UInt32 argc,
	const EsifDataArray argv,	// EsifData[] array of length argc
	EsifDataPtr response)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx && session->appHandle != ESIF_INVALID_HANDLE && argc && argv && response) {
		trx->ipfHandle = session->ipfHandle;

		if (Irpc_Encode_AppCommandFunc(
			trx,
			session->appHandle,
			argc,
			argv,
			response)) {
			rc = Irpc_SendMessage(trx);
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);

			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_AppCommand) {
				Encoded_AppCommandFunction *ipcobj = (Encoded_AppCommandFunction *)irpcMsg;
				if ((rc = Irpc_Unmarshall_EsifDataPtr(response, &ipcobj->response, Irpc_OffsetFrom(*ipcobj, response))) == ESIF_OK || Irpc_Uncast_eEsifError(ipcobj->result) != ESIF_OK) {
					rc = Irpc_Uncast_eEsifError(ipcobj->result);
				}
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV Irpc_Request_AppGetStatus(
	const esif_handle_t appHandle,
	const eAppStatusCommand command,
	const UInt32 appStatusIn,
	EsifDataPtr appStatusOut)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx && session->appHandle != ESIF_INVALID_HANDLE && appStatusOut) {
		trx->ipfHandle = session->ipfHandle;

		if (Irpc_Encode_AppGetStatusFunc(
			trx,
			session->appHandle,
			command,
			appStatusIn,
			appStatusOut)) {
			rc = Irpc_SendMessage(trx);
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);

			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_AppGetStatus) {
				Encoded_AppGetStatusFunction *ipcobj = (Encoded_AppGetStatusFunction *)irpcMsg;
				if ((rc = Irpc_Unmarshall_EsifDataPtr(appStatusOut, &ipcobj->appStatusOut, Irpc_OffsetFrom(*ipcobj, appStatusOut))) == ESIF_OK || Irpc_Uncast_eEsifError(ipcobj->result) != ESIF_OK) {
					rc = Irpc_Uncast_eEsifError(ipcobj->result);
				}
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV Irpc_Request_AppParticipantCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const AppParticipantDataPtr participantData,
	const eParticipantState participantInitialState)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx && session->appHandle != ESIF_INVALID_HANDLE && participantData) {
		trx->ipfHandle = session->ipfHandle;

		if (Irpc_Encode_AppParticipantCreateFunc(
			trx,
			session->appHandle,
			participantHandle,
			participantData,
			participantInitialState)) {
			rc = Irpc_SendMessage(trx);
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			IPFDEBUG(">>>>> %s: Sent AppParticipantCreate\n", ESIF_FUNC);
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);
			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_AppParticipantCreate) {
				Encoded_AppParticipantCreateFunction *ipcobj = (Encoded_AppParticipantCreateFunction *)irpcMsg;

				rc = Irpc_Uncast_eEsifError(ipcobj->result);

				IPFDEBUG("<<<<< Received AppParticipantCreate: rc=%s (%d)\n", esif_rc_str(rc), rc);
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV Irpc_Request_AppParticipantDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx && session->appHandle != ESIF_INVALID_HANDLE) {
		trx->ipfHandle = session->ipfHandle;

		if (Irpc_Encode_AppParticipantDestroyFunc(
			trx,
			session->appHandle,
			participantHandle)) {
			rc = Irpc_SendMessage(trx);
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			IPFDEBUG(">>>>> %s: Sent AppParticipantDestroy\n", ESIF_FUNC);
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);
			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_AppParticipantDestroy) {
				Encoded_AppParticipantDestroyFunction *ipcobj = (Encoded_AppParticipantDestroyFunction *)irpcMsg;

				rc = Irpc_Uncast_eEsifError(ipcobj->result);

				IPFDEBUG("<<<<< Received AppParticipantDestroy: rc=%s (%d)\n", esif_rc_str(rc), rc);
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV Irpc_Request_AppDomainCreate(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const AppDomainDataPtr domainDataPtr,
	const eDomainState domainInitialState)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx && session->appHandle != ESIF_INVALID_HANDLE && domainDataPtr) {
		trx->ipfHandle = session->ipfHandle;

		if (Irpc_Encode_AppDomainCreateFunc(
			trx,
			session->appHandle,
			participantHandle,
			domainHandle,
			domainDataPtr,
			domainInitialState)) {
			rc = Irpc_SendMessage(trx);
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			IPFDEBUG(">>>>> %s: Sent AppDomainCreate\n", ESIF_FUNC);
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);
			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_AppDomainCreate) {
				Encoded_AppDomainCreateFunction *ipcobj = (Encoded_AppDomainCreateFunction *)irpcMsg;

				rc = Irpc_Uncast_eEsifError(ipcobj->result);

				IPFDEBUG("<<<<< Received AppDomainCreate: rc=%s (%d)\n", esif_rc_str(rc), rc);
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV Irpc_Request_AppDomainDestroy(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx && session->appHandle != ESIF_INVALID_HANDLE) {
		trx->ipfHandle = session->ipfHandle;

		if (Irpc_Encode_AppDomainDestroyFunc(
			trx,
			session->appHandle,
			participantHandle,
			domainHandle)) {
			rc = Irpc_SendMessage(trx);
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			IPFDEBUG(">>>>> %s: Sent AppDomainDestroy\n", ESIF_FUNC);
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);
			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_AppDomainDestroy) {
				Encoded_AppDomainDestroyFunction *ipcobj = (Encoded_AppDomainDestroyFunction *)irpcMsg;

				rc = Irpc_Uncast_eEsifError(ipcobj->result);

				IPFDEBUG("<<<<< Received AppDomainDestroy: rc=%s (%d)\n", esif_rc_str(rc), rc);
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

esif_error_t ESIF_CALLCONV Irpc_Request_AppEvent(
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventData,
	const EsifDataPtr eventGuid)
{
	AppSession *session = AppSessionMgr_GetSessionByHandle(appHandle);
	if (!AppSession_IsConnected(session)) {
		AppSession_PutRef(session);
		return ESIF_E_SESSION_DISCONNECTED;
	}
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	// Allow Events with Undefined session->appHandle since Events may be sent by ESIF before AppCreate completes.
	if (trx) {
		trx->ipfHandle = session->ipfHandle;

		if (Irpc_Encode_AppEventFunc(
			trx,
			session->appHandle,
			participantHandle,
			domainHandle,
			eventData,
			eventGuid)) {
			rc = Irpc_SendMessage(trx);
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			IPFDEBUG(">>>>> %s: Sent AppEvent\n", ESIF_FUNC);
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);
			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_AppEvent) {
				Encoded_AppEventFunction *ipcobj = (Encoded_AppEventFunction *)irpcMsg;

				rc = Irpc_Uncast_eEsifError(ipcobj->result);

				IPFDEBUG("<<<<< Received AppEvent: rc=%s (%d)\n", esif_rc_str(rc), rc);
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	AppSession_PutRef(session);
	return rc;
}

////////////////////////////////////////////////
// IRPC Server Responses


esif_error_t Irpc_Response_EsifGetConfig(IrpcTransaction *trx)
{
	esif_handle_t esifHandle = AppSessionMgr_GetEsifHandleByHandle(trx ? trx->ipfHandle : ESIF_INVALID_HANDLE);
	Encoded_IrpcMsg *ipcmsg = (trx ? (Encoded_IrpcMsg *)IBinary_GetBuf(trx->request) : NULL);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (esifHandle != ESIF_INVALID_HANDLE && ipcmsg && ipcmsg->funcId == IrpcFunc_EsifGetConfig && ((trx->response = IBinary_Create()) != NULL)) {
		Encoded_EsifGetConfigFunction *ipcbuf = (Encoded_EsifGetConfigFunction *)ipcmsg;
		EsifData decoded_nameSpace = { 0 };
		EsifData decoded_elementPath = { 0 };
		EsifData decoded_elementValue = { 0 };

		if ((Irpc_Deserialize_EsifDataPtr(&decoded_nameSpace, &ipcbuf->nameSpace, Irpc_OffsetFrom(*ipcbuf, nameSpace))) &&
			(Irpc_Deserialize_EsifDataPtr(&decoded_elementPath, &ipcbuf->elementPath, Irpc_OffsetFrom(*ipcbuf, elementPath))) &&
			(Irpc_Deserialize_EsifDataPtr(&decoded_elementValue, &ipcbuf->elementValue, Irpc_OffsetFrom(*ipcbuf, elementValue)))) {

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);

			// Verify Authorization and Call into native ESIF Interface
			if (trx->ipfHandle == Irpc_Uncast_EsifHandle(ipcbuf->esifHandle)) {
				trx->result = AuthMgr_EsifGetConfig(
					esifHandle,
					&decoded_nameSpace,
					&decoded_elementPath,
					&decoded_elementValue
				);
			}

			Encoded_EsifGetConfigFunction *response = Irpc_Encode_EsifGetConfigFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->esifHandle),
				NULL,
				NULL,
				&decoded_elementValue
			);

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				rc = ESIF_OK;
			}
		}
		esif_ccb_free(decoded_nameSpace.buf_ptr);
		esif_ccb_free(decoded_elementPath.buf_ptr);
		esif_ccb_free(decoded_elementValue.buf_ptr);
	}
	return rc;
}

esif_error_t Irpc_Response_EsifSetConfig(IrpcTransaction *trx)
{
	esif_handle_t esifHandle = AppSessionMgr_GetEsifHandleByHandle(trx ? trx->ipfHandle : ESIF_INVALID_HANDLE);
	Encoded_IrpcMsg *ipcmsg = (trx ? (Encoded_IrpcMsg *)IBinary_GetBuf(trx->request) : NULL);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (esifHandle != ESIF_INVALID_HANDLE && ipcmsg && ipcmsg->funcId == IrpcFunc_EsifSetConfig && ((trx->response = IBinary_Create()) != NULL)) {
		Encoded_EsifSetConfigFunction *ipcbuf = (Encoded_EsifSetConfigFunction *)ipcmsg;
		EsifData decoded_nameSpace = { 0 };
		EsifData decoded_elementPath = { 0 };
		EsifData decoded_elementValue = { 0 };

		if ((Irpc_Deserialize_EsifDataPtr(&decoded_nameSpace, &ipcbuf->nameSpace, Irpc_OffsetFrom(*ipcbuf, nameSpace))) &&
			(Irpc_Deserialize_EsifDataPtr(&decoded_elementPath, &ipcbuf->elementPath, Irpc_OffsetFrom(*ipcbuf, elementPath))) &&
			(Irpc_Deserialize_EsifDataPtr(&decoded_elementValue, &ipcbuf->elementValue, Irpc_OffsetFrom(*ipcbuf, elementValue)))) {

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);

			// Verify Authorization and Call into native ESIF Interface
			if (trx->ipfHandle == Irpc_Uncast_EsifHandle(ipcbuf->esifHandle)) {
				trx->result = AuthMgr_EsifSetConfig(
					esifHandle,
					&decoded_nameSpace,
					&decoded_elementPath,
					&decoded_elementValue,
					Irpc_Uncast_EsifFlags(ipcbuf->elementFlags)
				);
			}

			Encoded_EsifSetConfigFunction *response = Irpc_Encode_EsifSetConfigFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->esifHandle),
				NULL,
				NULL,
				NULL,
				Irpc_Uncast_EsifFlags(ipcbuf->elementFlags)
			);

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				rc = ESIF_OK;
			}
		}
		esif_ccb_free(decoded_nameSpace.buf_ptr);
		esif_ccb_free(decoded_elementPath.buf_ptr);
		esif_ccb_free(decoded_elementValue.buf_ptr);
	}
	return rc;
}

esif_error_t Irpc_Response_EsifPrimitive(IrpcTransaction *trx)
{
	esif_handle_t esifHandle = AppSessionMgr_GetEsifHandleByHandle(trx ? trx->ipfHandle : ESIF_INVALID_HANDLE);
	Encoded_IrpcMsg *ipcmsg = (trx ? (Encoded_IrpcMsg *)IBinary_GetBuf(trx->request) : NULL);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (esifHandle != ESIF_INVALID_HANDLE && ipcmsg && ipcmsg->funcId == IrpcFunc_EsifPrimitive && ((trx->response = IBinary_Create()) != NULL)) {
		Encoded_EsifPrimitiveFunction *ipcbuf = (Encoded_EsifPrimitiveFunction *)ipcmsg;
		EsifData decoded_request = { 0 };
		EsifData decoded_response = { 0 };

		if ((Irpc_Deserialize_EsifDataPtr(&decoded_request, &ipcbuf->request, Irpc_OffsetFrom(*ipcbuf, request))) &&
			(Irpc_Deserialize_EsifDataPtr(&decoded_response, &ipcbuf->response, Irpc_OffsetFrom(*ipcbuf, response)))) {

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);

			// Verify Authorization and Call into native ESIF Interface
			if (trx->ipfHandle == Irpc_Uncast_EsifHandle(ipcbuf->esifHandle)) {
				trx->result = AuthMgr_EsifPrimitive(
					esifHandle,
					Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
					Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
					&decoded_request,
					&decoded_response,
					Irpc_Uncast_ePrimitiveType(ipcbuf->primitive),
					Irpc_Uncast_UInt8(ipcbuf->instance)
				);
			}

			Encoded_EsifPrimitiveFunction *response = Irpc_Encode_EsifPrimitiveFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->esifHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
				NULL,
				&decoded_response,
				Irpc_Uncast_ePrimitiveType(ipcbuf->primitive),
				Irpc_Uncast_UInt8(ipcbuf->instance)
			);

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				rc = ESIF_OK;
			}
		}
		esif_ccb_free(decoded_request.buf_ptr);
		esif_ccb_free(decoded_response.buf_ptr);
	}
	return rc;
}

esif_error_t Irpc_Response_EsifWriteLog(IrpcTransaction *trx)
{
	esif_handle_t esifHandle = AppSessionMgr_GetEsifHandleByHandle(trx ? trx->ipfHandle : ESIF_INVALID_HANDLE);
	Encoded_IrpcMsg *ipcmsg = (trx ? (Encoded_IrpcMsg *)IBinary_GetBuf(trx->request) : NULL);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (esifHandle != ESIF_INVALID_HANDLE && ipcmsg && Irpc_Uncast_eIrpcFunction(ipcmsg->funcId) == IrpcFunc_EsifWriteLog && ((trx->response = IBinary_Create()) != NULL)) {
		Encoded_EsifWriteLogFunction *ipcbuf = (Encoded_EsifWriteLogFunction *)ipcmsg;
		EsifData decoded_message = { 0 };

		if (Irpc_Deserialize_EsifDataPtr(&decoded_message, &ipcbuf->message, Irpc_OffsetFrom(*ipcbuf, message))) {

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);

			// Verify Authorization and Call into native ESIF Interface
			if (trx->ipfHandle == Irpc_Uncast_EsifHandle(ipcbuf->esifHandle)) {
				trx->result = AuthMgr_EsifWriteLog(
					esifHandle,
					Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
					Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
					&decoded_message,
					Irpc_Uncast_eLogType(ipcbuf->logType)
				);
			}

			Encoded_EsifWriteLogFunction *response = Irpc_Encode_EsifWriteLogFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->esifHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
				NULL,
				Irpc_Uncast_eLogType(ipcbuf->logType)
			);

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				rc = ESIF_OK;
			}
		}
		esif_ccb_free(decoded_message.buf_ptr);
	}
	return rc;
}

// EsifEventRegister and EsifEventUnregister use the same prototype
esif_error_t Irpc_Response_EsifEventAction(IrpcTransaction *trx, eIrpcFunction funcId)
{
	esif_handle_t esifHandle = AppSessionMgr_GetEsifHandleByHandle(trx ? trx->ipfHandle : ESIF_INVALID_HANDLE);
	Encoded_IrpcMsg *ipcmsg = (trx ? (Encoded_IrpcMsg *)IBinary_GetBuf(trx->request) : NULL);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (esifHandle != ESIF_INVALID_HANDLE && ipcmsg && Irpc_Uncast_eIrpcFunction(ipcmsg->funcId) == funcId && ((trx->response = IBinary_Create()) != NULL)) {
		Encoded_EsifEventActionFunction *ipcbuf = (Encoded_EsifEventActionFunction *)ipcmsg;
		EsifData decoded_eventGuid = { 0 };

		if (Irpc_Deserialize_EsifDataPtr(&decoded_eventGuid, &ipcbuf->eventGuid, Irpc_OffsetFrom(*ipcbuf, eventGuid))) {

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);

			// Verify Authorization and Call into native ESIF Interface
			if (trx->ipfHandle == Irpc_Uncast_EsifHandle(ipcbuf->esifHandle)) {
				switch (funcId) {
				case IrpcFunc_EsifEventRegister:
					trx->result = AuthMgr_EsifEventRegister(
						esifHandle,
						Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
						Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
						&decoded_eventGuid
					);
					break;
				case IrpcFunc_EsifEventUnregister:
					trx->result = AuthMgr_EsifEventUnregister(
						esifHandle,
						Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
						Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
						&decoded_eventGuid
					);
					break;
				default:
					trx->result = ESIF_E_INVALID_REQUEST_TYPE;
					break;
				}
			}

			Encoded_EsifEventActionFunction *response = Irpc_Encode_EsifEventActionFunc(
				trx,
				funcId,
				esifHandle,
				Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
				NULL
			);

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				rc = ESIF_OK;
			}
		}
		esif_ccb_free(decoded_eventGuid.buf_ptr);
	}
	return rc;
}

esif_error_t Irpc_Response_EsifEventRegister(IrpcTransaction *trx)
{
	return Irpc_Response_EsifEventAction(trx, IrpcFunc_EsifEventRegister);
}

esif_error_t Irpc_Response_EsifEventUnregister(IrpcTransaction *trx)
{
	return Irpc_Response_EsifEventAction(trx, IrpcFunc_EsifEventUnregister);
}

esif_error_t Irpc_Response_EsifSendEvent(IrpcTransaction *trx)
{
	esif_handle_t esifHandle = AppSessionMgr_GetEsifHandleByHandle(trx ? trx->ipfHandle : ESIF_INVALID_HANDLE);
	Encoded_IrpcMsg *ipcmsg = (trx ? (Encoded_IrpcMsg *)IBinary_GetBuf(trx->request) : NULL);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (esifHandle != ESIF_INVALID_HANDLE && ipcmsg && Irpc_Uncast_eIrpcFunction(ipcmsg->funcId) == IrpcFunc_EsifSendEvent && ((trx->response = IBinary_Create()) != NULL)) {
		Encoded_EsifSendEventFunction *ipcbuf = (Encoded_EsifSendEventFunction *)ipcmsg;
		EsifData decoded_eventData = { 0 };
		EsifData decoded_eventGuid = { 0 };

		if ((Irpc_Deserialize_EsifDataPtr(&decoded_eventData, &ipcbuf->eventData, Irpc_OffsetFrom(*ipcbuf, eventData))) &&
			(Irpc_Deserialize_EsifDataPtr(&decoded_eventGuid, &ipcbuf->eventGuid, Irpc_OffsetFrom(*ipcbuf, eventGuid)))) {

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);

			// Verify Authorization and Call into native ESIF Interface
			if (trx->ipfHandle == Irpc_Uncast_EsifHandle(ipcbuf->esifHandle)) {
				trx->result = AuthMgr_EsifSendEvent(
					esifHandle,
					Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
					Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
					&decoded_eventData,
					&decoded_eventGuid
				);
			}

			Encoded_EsifSendEventFunction *response = Irpc_Encode_EsifSendEventFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->esifHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
				NULL,
				NULL
			);

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				rc = ESIF_OK;
			}
		}
		esif_ccb_free(decoded_eventData.buf_ptr);
		esif_ccb_free(decoded_eventGuid.buf_ptr);
	}
	return rc;
}

esif_error_t Irpc_Response_EsifSendCommand(IrpcTransaction *trx)
{
	esif_handle_t esifHandle = AppSessionMgr_GetEsifHandleByHandle(trx ? trx->ipfHandle : ESIF_INVALID_HANDLE);
	Encoded_IrpcMsg *ipcmsg = (trx ? (Encoded_IrpcMsg *)IBinary_GetBuf(trx->request) : NULL);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (esifHandle != ESIF_INVALID_HANDLE && ipcmsg && ipcmsg->funcId == IrpcFunc_EsifSendCommand && ((trx->response = IBinary_Create()) != NULL)) {
		Encoded_EsifSendCommandFunction *ipcbuf = (Encoded_EsifSendCommandFunction *)ipcmsg;
		UInt32 argc = Irpc_Uncast_UInt32(ipcbuf->argc);
		EsifData *decoded_argv = Irpc_Deserialize_EsifDataArray(NULL, argc, &ipcbuf->argv, Irpc_OffsetFrom(*ipcbuf, argv));
		EsifData *decoded_response = Irpc_Deserialize_EsifDataPtr(NULL, &ipcbuf->response, Irpc_OffsetFrom(*ipcbuf, response));

		if (decoded_argv && decoded_response) {

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);

			// Verify Authorization and Call into native ESIF Interface
			if (trx->ipfHandle == Irpc_Uncast_EsifHandle(ipcbuf->esifHandle)) {
				trx->result = AuthMgr_EsifSendCommand(
					esifHandle,
					argc,
					decoded_argv,
					decoded_response
				);
			}

			Encoded_EsifSendCommandFunction *response = Irpc_Encode_EsifSendCommandFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->esifHandle),
				argc,
				NULL,
				decoded_response
			);

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				rc = ESIF_OK;
			}
			for (UInt32 j = 0; decoded_argv && j < argc; j++) {
				esif_ccb_free(decoded_argv[j].buf_ptr);
			}
			esif_ccb_free(decoded_response->buf_ptr);
		}
		esif_ccb_free(decoded_argv);
		esif_ccb_free(decoded_response);
	}
	return rc;
}

// Process an Incoming ESIF RPC Request
esif_error_t IrpcTransaction_EsifRequest(IrpcTransaction *trx)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (trx && trx->request && trx->response == NULL) {
		Encoded_IrpcMsg *ipcmsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->request);

		rc = ESIF_OK;
		trx->result = ESIF_E_INVALID_HANDLE;

		switch (Irpc_Uncast_eIrpcFunction(ipcmsg->funcId)) {
		case IrpcFunc_EsifGetConfig:
			rc = Irpc_Response_EsifGetConfig(trx);
			break;
		case IrpcFunc_EsifSetConfig:
			rc = Irpc_Response_EsifSetConfig(trx);
			break;
		case IrpcFunc_EsifPrimitive:
			rc = Irpc_Response_EsifPrimitive(trx);
			break;
		case IrpcFunc_EsifWriteLog:
			rc = Irpc_Response_EsifWriteLog(trx);
			break;
		case IrpcFunc_EsifEventRegister:
			rc = Irpc_Response_EsifEventRegister(trx);
			break;
		case IrpcFunc_EsifEventUnregister:
			rc = Irpc_Response_EsifEventUnregister(trx);
			break;
		case IrpcFunc_EsifSendEvent:
			rc = Irpc_Response_EsifSendEvent(trx);
			break;
		case IrpcFunc_EsifSendCommand:
			rc = Irpc_Response_EsifSendCommand(trx);
			break;
		default:
			trx->result = rc = ESIF_E_NOT_IMPLEMENTED;
			break;
		}
		if (rc == ESIF_OK && trx->response == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
	}
	return rc;
}
