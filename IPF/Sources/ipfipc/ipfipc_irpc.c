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

#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"
#include "esif_sdk.h"
#include "esif_sdk_message.h"
#include "esif_sdk_event_type.h"
#include "esif_sdk_event_map.h"
#include "esif_sdk_capability_type.h"
#include "esif_sdk_logging_data.h"

#include "ipf_trace.h"
#include "ipf_ipc_codec.h"
#include "ipf_ipc_trxmgr.h"
#include "ipf_sdk_version_check.h"

#include "ipfipc_ws.h"
#include "ipfipc_trace.h"


#pragma warning(disable:4204)	// Irpc Initializers

// Wait for RPC Transaction to complete and Close Conneciton if Request Failed
static esif_error_t IrpcTransaction_WaitForResponse(IrpcTransaction *self)
{
	esif_error_t rc = IrpcTransaction_Wait(self);
	if (rc == ESIF_E_SESSION_REQUEST_FAILED) {
		// Return ESIF_OK to not auto-disconnect on timeout
		rc = ESIF_E_SESSION_DISCONNECTED;
	}
	return rc;
}

// Queue an Outgoing IRPC Request to be consumed by WebClient Thread
esif_error_t IrpcTransaction_QueueRequest(IrpcTransaction *self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IpcSession *session = NULL;
	if (self && (session = IpcSessionMgr_GetSessionByHandle(self->ipfHandle)) != NULL) {
		if (session->socket == INVALID_SOCKET) {
			rc = ESIF_E_SESSION_DISCONNECTED;
		}
		else {
			rc = IpfTrxMgr_AddTransaction(&session->trxMgr, self);
		}

		if (rc == ESIF_OK) {
			IBinary *request = self->request;
			self->request = NULL; // Destroyed by Websocket Thread
			rc = MessageQueue_EnQueue(session->sendQueue, request);
		}
		if (rc == ESIF_OK) {
			u8 opcode = WS_OPCODE_MESSAGE;
			if (send(session->doorbell[DOORBELL_BUTTON], (const char*)&opcode, sizeof(opcode), 0) != sizeof(opcode)) {
				rc = ESIF_E_WS_SOCKET_ERROR;
			}
		}
	}
	return rc;
}

// Set Response Header in an existing Blob
static ESIF_INLINE void Irpc_SetResponseHeader(IBinary* blob, EsifMsgHdr hdr)
{
	EsifMsgHdr* hdrPtr = (EsifMsgHdr*)IBinary_GetBuf(blob);
	if (hdrPtr && IBinary_GetBufLen(blob) >= sizeof(hdr)) {
		*hdrPtr = hdr;
	}
}

/*
** ESIF Interface Functions called from Loaded DLL
*/

// Esif Interface function to translate a Native ESIF API call to an RPC Call
eEsifError ESIF_CALLCONV Irpc_Request_EsifGetConfig(
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	EsifDataPtr elementValue
)
{
	IPFDEBUG("<<<< %s: %s %s\n", ESIF_FUNC, (esif_string)nameSpace->buf_ptr, (esif_string)elementPath->buf_ptr);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx) {
		IpcSession *session = IpcSessionMgr_GetSessionByEsifHandle(esifHandle);
		trx->ipfHandle = (session ? session->appSession.ipfHandle : trx->ipfHandle);

		if (Irpc_Encode_EsifGetConfigFunc(
			trx,
			esifHandle,
			nameSpace,
			elementPath,
			elementValue)) {

			EsifMsgHdr header = {
				.v1.signature = ESIFMSG_SIGNATURE,
				.v1.headersize = sizeof(header),
				.v1.version = ESIFMSG_VERSION,
				.v1.msgclass = ESIFMSG_CLASS_IRPC
			};
			header.v1.msglen = (UInt32)IBinary_GetLen(trx->request);

			if (IBinary_Insert(trx->request, &header, sizeof(header), 0) != NULL) {
				rc = IrpcTransaction_QueueRequest(trx);
			}
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);

			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_EsifGetConfig) {
				Encoded_EsifGetConfigFunction *ipcobj = (Encoded_EsifGetConfigFunction *)irpcMsg;
				if ((rc = Irpc_Unmarshall_EsifDataPtr(elementValue, &ipcobj->elementValue, Irpc_OffsetFrom(*ipcobj, elementValue))) == ESIF_OK || Irpc_Uncast_eEsifError(ipcobj->result) != ESIF_OK) {
					rc = Irpc_Uncast_eEsifError(ipcobj->result);
				}
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	IPFDEBUG(">>>>> %s: Result=%s (%d)\n", ESIF_FUNC, esif_rc_str(rc), rc);
	return rc;
}

eEsifError ESIF_CALLCONV Irpc_Request_EsifSetConfig(
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	const EsifDataPtr elementValue,
	const EsifFlags elementFlags
)
{
	IPFDEBUG("<<<< %s: %s %s 0x%02X\n", ESIF_FUNC, (esif_string)nameSpace->buf_ptr, (esif_string)elementPath->buf_ptr, elementFlags);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx) {
		IpcSession *session = IpcSessionMgr_GetSessionByEsifHandle(esifHandle);
		trx->ipfHandle = (session ? session->appSession.ipfHandle : trx->ipfHandle);

		if (Irpc_Encode_EsifSetConfigFunc(
			trx,
			esifHandle,
			nameSpace,
			elementPath,
			elementValue,
			elementFlags)) {

			EsifMsgHdr header = {
				.v1.signature = ESIFMSG_SIGNATURE,
				.v1.headersize = sizeof(header),
				.v1.version = ESIFMSG_VERSION,
				.v1.msgclass = ESIFMSG_CLASS_IRPC
			};
			header.v1.msglen = (UInt32)IBinary_GetLen(trx->request);

			if (IBinary_Insert(trx->request, &header, sizeof(header), 0) != NULL) {
				rc = IrpcTransaction_QueueRequest(trx);
			}
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);

			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_EsifSetConfig) {
				Encoded_EsifSetConfigFunction *ipcobj = (Encoded_EsifSetConfigFunction *)irpcMsg;
				rc = Irpc_Uncast_eEsifError(ipcobj->result);
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	IPFDEBUG(">>>>> %s: Result=%s (%d)\n", ESIF_FUNC, esif_rc_str(rc), rc);
	return rc;
}

eEsifError ESIF_CALLCONV Irpc_Request_EsifPrimitive(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr request,
	EsifDataPtr response,
	const ePrimitiveType primitive,
	const UInt8 instance
)
{
	IPFDEBUG("<<<< %s: %s (%d)\n", ESIF_FUNC, esif_primitive_str(primitive), primitive);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx) {
		IpcSession *session = IpcSessionMgr_GetSessionByEsifHandle(esifHandle);
		trx->ipfHandle = (session ? session->appSession.ipfHandle : trx->ipfHandle);

		if (Irpc_Encode_EsifPrimitiveFunc(
			trx,
			esifHandle,
			participantHandle,
			domainHandle,
			request,
			response,
			primitive,
			instance)) {

			EsifMsgHdr header = {
				.v1.signature = ESIFMSG_SIGNATURE,
				.v1.headersize = sizeof(header),
				.v1.version = ESIFMSG_VERSION,
				.v1.msgclass = ESIFMSG_CLASS_IRPC
			};
			header.v1.msglen = (UInt32)IBinary_GetLen(trx->request);

			if (IBinary_Insert(trx->request, &header, sizeof(header), 0) != NULL) {
				rc = IrpcTransaction_QueueRequest(trx);
			}
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);

			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_EsifPrimitive) {
				Encoded_EsifPrimitiveFunction *ipcobj = (Encoded_EsifPrimitiveFunction *)irpcMsg;
				if ((rc = Irpc_Unmarshall_EsifDataPtr(response, &ipcobj->response, Irpc_OffsetFrom(*ipcobj, response))) == ESIF_OK || Irpc_Uncast_eEsifError(ipcobj->result) != ESIF_OK) {
					rc = Irpc_Uncast_eEsifError(ipcobj->result);
				}
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	IPFDEBUG(">>>>> %s: Result=%s (%d)\n", ESIF_FUNC, esif_rc_str(rc), rc);
	return rc;
}

eEsifError ESIF_CALLCONV Irpc_Request_EsifWriteLog(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr message,
	const eLogType logType
)
{
	IPFDEBUG("<<<< %s: %s\n", ESIF_FUNC, (esif_string)message->buf_ptr);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx) {
		IpcSession *session = IpcSessionMgr_GetSessionByEsifHandle(esifHandle);
		trx->ipfHandle = (session ? session->appSession.ipfHandle : trx->ipfHandle);

		if (Irpc_Encode_EsifWriteLogFunc(
			trx,
			esifHandle,
			participantHandle,
			domainHandle,
			message,
			logType)) {

			EsifMsgHdr header = {
				.v1.signature = ESIFMSG_SIGNATURE,
				.v1.headersize = sizeof(header),
				.v1.version = ESIFMSG_VERSION,
				.v1.msgclass = ESIFMSG_CLASS_IRPC
			};
			header.v1.msglen = (UInt32)IBinary_GetLen(trx->request);

			if (IBinary_Insert(trx->request, &header, sizeof(header), 0) != NULL) {
				rc = IrpcTransaction_QueueRequest(trx);
			}
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);

			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_EsifWriteLog) {
				Encoded_EsifWriteLogFunction *ipcobj = (Encoded_EsifWriteLogFunction *)irpcMsg;
				rc = Irpc_Uncast_eEsifError(ipcobj->result);
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	IPFDEBUG(">>>>> %s: Result=%s (%d)\n", ESIF_FUNC, esif_rc_str(rc), rc);
	return rc;
}

static eEsifError ESIF_CALLCONV Irpc_Request_EsifEventAction(
	const eIrpcFunction funcId,
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuid
)
{
	IPFDEBUG("<<<< %s: %s\n", ESIF_FUNC, esif_data_type_str(eventGuid->type));
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx) {
		IpcSession *session = IpcSessionMgr_GetSessionByEsifHandle(esifHandle);
		trx->ipfHandle = (session ? session->appSession.ipfHandle : trx->ipfHandle);

		if (Irpc_Encode_EsifEventActionFunc(
			trx,
			funcId,
			esifHandle,
			participantHandle,
			domainHandle,
			eventGuid)) {

			EsifMsgHdr header = {
				.v1.signature = ESIFMSG_SIGNATURE,
				.v1.headersize = sizeof(header),
				.v1.version = ESIFMSG_VERSION,
				.v1.msgclass = ESIFMSG_CLASS_IRPC
			};
			header.v1.msglen = (UInt32)IBinary_GetLen(trx->request);

			if (IBinary_Insert(trx->request, &header, sizeof(header), 0) != NULL) {
				rc = IrpcTransaction_QueueRequest(trx);
			}
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);

			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == funcId) {
				Encoded_EsifEventActionFunction *ipcobj = (Encoded_EsifEventActionFunction *)irpcMsg;
				rc = Irpc_Uncast_eEsifError(ipcobj->result);
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	IPFDEBUG(">>>>> %s: Result=%s (%d)\n", ESIF_FUNC, esif_rc_str(rc), rc);
	return rc;
}

eEsifError ESIF_CALLCONV Irpc_Request_EsifEventRegister(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuid
)
{
	IPFDEBUG("<<<< %s: %s\n", ESIF_FUNC, esif_data_type_str(eventGuid->type));
	eEsifError rc = Irpc_Request_EsifEventAction(
		IrpcFunc_EsifEventRegister,
		esifHandle,
		participantHandle,
		domainHandle,
		eventGuid
	);
	IPFDEBUG(">>>>> %s: Result=%s (%d)\n", ESIF_FUNC, esif_rc_str(rc), rc);
	return rc;
}

eEsifError ESIF_CALLCONV Irpc_Request_EsifEventUnregister(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuid
)
{
	IPFDEBUG("<<<< %s: %s\n", ESIF_FUNC, esif_data_type_str(eventGuid->type));
	eEsifError rc = Irpc_Request_EsifEventAction(
		IrpcFunc_EsifEventUnregister,
		esifHandle,
		participantHandle,
		domainHandle,
		eventGuid
	);
	IPFDEBUG(">>>>> %s: Result=%s (%d)\n", ESIF_FUNC, esif_rc_str(rc), rc);
	return rc;
}

eEsifError ESIF_CALLCONV Irpc_Request_EsifSendEvent(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventData,
	const EsifDataPtr eventGuid
)
{
	IPFDEBUG("<<<< %s: %s\n", ESIF_FUNC, esif_data_type_str(eventGuid->type));
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);

	if (trx) {
		IpcSession *session = IpcSessionMgr_GetSessionByEsifHandle(esifHandle);
		trx->ipfHandle = (session ? session->appSession.ipfHandle : trx->ipfHandle);

		if (Irpc_Encode_EsifSendEventFunc(
			trx,
			esifHandle,
			participantHandle,
			domainHandle,
			eventData,
			eventGuid)) {

			EsifMsgHdr header = {
				.v1.signature = ESIFMSG_SIGNATURE,
				.v1.headersize = sizeof(header),
				.v1.version = ESIFMSG_VERSION,
				.v1.msgclass = ESIFMSG_CLASS_IRPC
			};
			header.v1.msglen = (UInt32)IBinary_GetLen(trx->request);

			if (IBinary_Insert(trx->request, &header, sizeof(header), 0) != NULL) {
				rc = IrpcTransaction_QueueRequest(trx);
			}
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);

			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_EsifSendEvent) {
				Encoded_EsifSendEventFunction *ipcobj = (Encoded_EsifSendEventFunction *)irpcMsg;
				rc = Irpc_Uncast_eEsifError(ipcobj->result);
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	IPFDEBUG(">>>>> %s: Result=%s (%d)\n", ESIF_FUNC, esif_rc_str(rc), rc);
	return rc;
}

eEsifError ESIF_CALLCONV Irpc_Request_EsifSendCommand(
	const esif_handle_t esifHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response
)
{
	IPFDEBUG("<<<< %s: argc=%d cmd=%s\n", ESIF_FUNC, argc, (argc && argv ? (esif_string)argv[0].buf_ptr : ""));
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Intercept Command: "sdk-version <client-version>" and perform optional Client/Server SDK Version Check
	if (argc > 0 && argv[0].buf_ptr && argv[0].data_len && esif_ccb_stricmp(argv[0].buf_ptr, "sdk-version") == 0 && response) {
		char serverSdkVersion[] = IPF_SDK_VERSION;
		char *clientSdkVersion = NULL;
		rc = ESIF_OK;

		// sdk-version <client-version>
		if (argc > 1 && argv[1].buf_ptr && argv[1].data_len) {
			clientSdkVersion = (char *)argv[1].buf_ptr;
			rc = IpfSdk_VersionCheck(serverSdkVersion, clientSdkVersion);
		}

		if (rc == ESIF_OK) {
			u32 data_len = (u32)sizeof(serverSdkVersion);
			if (response->buf_len < data_len) {
				response->data_len = data_len;
				rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			else {
				esif_ccb_strcpy(response->buf_ptr, serverSdkVersion, response->buf_len);
				response->data_len = data_len;
			}
		}
		IPFDEBUG(">>>>> %s: Result=%s (%d) SDK:%s Client:%s\n", ESIF_FUNC, esif_rc_str(rc), rc, serverSdkVersion, clientSdkVersion);
		return rc;
	}

	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcRequest);
	if (trx) {
		IpcSession *session = IpcSessionMgr_GetSessionByEsifHandle(esifHandle);
		trx->ipfHandle = (session ? session->appSession.ipfHandle : trx->ipfHandle);

		if (Irpc_Encode_EsifSendCommandFunc(
			trx,
			esifHandle,
			argc,
			argv,
			response)) {

			EsifMsgHdr header = {
				.v1.signature = ESIFMSG_SIGNATURE,
				.v1.headersize = sizeof(header),
				.v1.version = ESIFMSG_VERSION,
				.v1.msgclass = ESIFMSG_CLASS_IRPC
			};
			header.v1.msglen = (UInt32)IBinary_GetLen(trx->request);

			if (IBinary_Insert(trx->request, &header, sizeof(header), 0) != NULL) {
				rc = IrpcTransaction_QueueRequest(trx);
			}
		}

		// Wait for Response to be Receieved from Server
		if (rc == ESIF_OK) {
			rc = IrpcTransaction_WaitForResponse(trx);
		}

		if (rc == ESIF_OK) {
			Encoded_IrpcMsg *irpcMsg = (Encoded_IrpcMsg *)IBinary_GetBuf(trx->response);

			if (irpcMsg && Irpc_Uncast_eIrpcFunction(irpcMsg->funcId) == IrpcFunc_EsifSendCommand) {
				Encoded_EsifSendCommandFunction *ipcobj = (Encoded_EsifSendCommandFunction *)irpcMsg;
				if ((rc = Irpc_Unmarshall_EsifDataPtr(response, &ipcobj->response, Irpc_OffsetFrom(*ipcobj, response))) == ESIF_OK || Irpc_Uncast_eEsifError(ipcobj->result) != ESIF_OK) {
					rc = Irpc_Uncast_eEsifError(ipcobj->result);
				}
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	IPFDEBUG(">>>>> %s: Result=%s (%d)\n", ESIF_FUNC, esif_rc_str(rc), rc);
	return rc;
}

/* 
** IRPC Responses
*/

IBinary *Irpc_Response_AppGetString(IpcSession *client, Encoded_IrpcMsg *msg)
{
	Encoded_AppGetStringFunction *ipcbuf = (Encoded_AppGetStringFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);
	esif_handle_t appHandle = ESIF_INVALID_HANDLE;

	if (client && ipcbuf && trx) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);
			EsifData *stringDataPtr = Irpc_Deserialize_EsifDataPtr(NULL, &ipcbuf->stringData, Irpc_OffsetFrom(*ipcbuf, stringData));

			if (stringDataPtr && stringDataPtr->buf_ptr) {
				AppInterface *appIface = &client->appSession.ifaceSet.appIface;

				trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
				trx->result = ESIF_E_NOT_IMPLEMENTED;

				// Call into Loaded AppInterface
				switch (Irpc_Uncast_eIrpcFunction(msg->funcId)) {
				case IrpcFunc_AppGetName:
					if (appIface->fAppGetNameFuncPtr) {
						trx->result = appIface->fAppGetNameFuncPtr(stringDataPtr);
					}
					break;
				case IrpcFunc_AppGetDescription:
					if (appIface->fAppGetDescriptionFuncPtr) {
						trx->result = appIface->fAppGetDescriptionFuncPtr(stringDataPtr);
					}
					break;
				case IrpcFunc_AppGetVersion:
					if (appIface->fAppGetVersionFuncPtr) {
						trx->result = appIface->fAppGetVersionFuncPtr(stringDataPtr);
					}
					break;
				case IrpcFunc_AppGetIntro:
					if (client->appSession.appHandle != Irpc_Uncast_EsifHandle(ipcbuf->appHandle)) {
						trx->result = ESIF_E_INVALID_HANDLE;
					}
					else if (appIface->fAppGetIntroFuncPtr) {
						appHandle = client->appSession.appHandle;
						trx->result = appIface->fAppGetIntroFuncPtr(appHandle, stringDataPtr);
					}
					break;
				default:
					trx->result = ESIF_E_INVALID_REQUEST_TYPE;
					break;
				}

				Encoded_AppGetStringFunction *response = Irpc_Encode_AppGetStringFunc(
					trx,
					Irpc_Uncast_eIrpcFunction(ipcbuf->irpcHdr.funcId),
					appHandle,
					stringDataPtr
				);

				if (response) {
					response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
					response->result = Irpc_Cast_eEsifError(trx->result);
					responseHdr.v1.msglen = (UInt32)(IBinary_GetLen(trx->response) - msgstart);
					Irpc_SetResponseHeader(trx->response, responseHdr);
					result = trx->response;
					trx->response = NULL;
				}
				esif_ccb_free(stringDataPtr->buf_ptr);
			}
			esif_ccb_free(stringDataPtr);
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

IBinary *Irpc_Response_AppCommand(IpcSession *client, Encoded_IrpcMsg *msg)
{
	Encoded_AppCommandFunction *ipcbuf = (Encoded_AppCommandFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);

	if (client && ipcbuf && trx) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);
			UInt32 argc = Irpc_Uncast_UInt32(ipcbuf->argc);
			EsifData *decoded_argv = Irpc_Deserialize_EsifDataArray(NULL, argc, &ipcbuf->argv, Irpc_OffsetFrom(*ipcbuf, argv));
			EsifData *decoded_response = Irpc_Deserialize_EsifDataPtr(NULL, &ipcbuf->response, Irpc_OffsetFrom(*ipcbuf, response));

			if (decoded_argv && decoded_response && decoded_response->buf_ptr) {
				AppInterface *appIface = &client->appSession.ifaceSet.appIface;

				trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
				trx->result = ESIF_E_NOT_IMPLEMENTED;

				// Call into Loaded AppInterface
				if (client->appSession.appHandle != Irpc_Uncast_EsifHandle(ipcbuf->appHandle)) {
					trx->result = ESIF_E_INVALID_HANDLE;
				}
				// Intercept "timeout" commands and use to Get or Set Client-side RPC Transaction Timeout, bypassing App Interface
				else if (Irpc_Uncast_UInt32(ipcbuf->argc) > 0 && decoded_argv[0].type == ESIF_DATA_STRING && esif_ccb_stricmp(decoded_argv[0].buf_ptr, "timeout") == 0) {
					size_t timeout = 0;
					if (Irpc_Uncast_UInt32(ipcbuf->argc) > 1 && decoded_argv[1].type == ESIF_DATA_STRING && isdigit(((char *)decoded_argv[1].buf_ptr)[0])) {
						timeout = (size_t)atoi((char *)decoded_argv[1].buf_ptr);
						IpfTrxMgr_SetTimeout(&client->trxMgr, timeout);
					}
					decoded_response->data_len = (u32)esif_ccb_sprintf(decoded_response->buf_len, decoded_response->buf_ptr, "%zd\n", IpfTrxMgr_GetTimeout(&client->trxMgr)) + 1;
					trx->result = ESIF_OK;
				}
				else if (appIface->fAppCommandFuncPtr) {
					trx->result = appIface->fAppCommandFuncPtr(
						Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
						Irpc_Uncast_UInt32(ipcbuf->argc),
						decoded_argv,
						decoded_response
					);
				}

				Encoded_AppCommandFunction *response = Irpc_Encode_AppCommandFunc(
					trx,
					Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
					Irpc_Uncast_UInt32(0),
					NULL,
					decoded_response
				);

				if (response) {
					response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
					response->result = Irpc_Cast_eEsifError(trx->result);
					responseHdr.v1.msglen = (UInt32)(IBinary_GetLen(trx->response) - msgstart);
					Irpc_SetResponseHeader(trx->response, responseHdr);
					result = trx->response;
					trx->response = NULL;
				}
#ifdef ESIF_ATTR_DEBUG
				// Timeout Debugging: Do not RPC Response to "dropit" command
				if (result && argc && decoded_argv[0].buf_ptr && esif_ccb_stricmp(decoded_argv[0].buf_ptr, "dropit") == 0) {
					IPF_TRACE_DEBUG("%s: Dropping RPC Response", ESIF_FUNC);
					IBinary_Destroy(result);
					result = NULL;
				}
#endif
				for (UInt32 j = 0; decoded_argv && j < argc; j++) {
					esif_ccb_free(decoded_argv[j].buf_ptr);
				}
				esif_ccb_free(decoded_response->buf_ptr);
			}
			esif_ccb_free(decoded_argv);
			esif_ccb_free(decoded_response);
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

IBinary *Irpc_Response_AppCreate(IpcSession *client, Encoded_IrpcMsg *msg)
{
	Encoded_AppCreateFunction *ipcbuf = (Encoded_AppCreateFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (client && ipcbuf && trx) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);
			Encoded_AppInterfaceSet *encoded_ifaceSetPtr = (Encoded_AppInterfaceSet *)(Irpc_OffsetFrom(*ipcbuf, ifaceSetPtr) + Irpc_Uncast_UInt32(ipcbuf->ifaceSetPtr.offset));
			EsifIfaceHdr decoded_hdr = Irpc_Uncast_EsifIfaceHdr(encoded_ifaceSetPtr->hdr);

			if (decoded_hdr.fIfaceType != eIfaceTypeApplication || decoded_hdr.fIfaceVersion != APP_INTERFACE_VERSION) {
				rc = ESIF_E_NOT_SUPPORTED;
			}
			else {
				AppInterfaceSet *appIfaceSet = &client->appSession.ifaceSet;

				// Irpc_Request_Esif* functions translate a Native EsifInterface function call to an IRPC Request
				EsifInterface esifIface = {
					.fGetConfigFuncPtr = Irpc_Request_EsifGetConfig,
					.fSetConfigFuncPtr = Irpc_Request_EsifSetConfig,
					.fPrimitiveFuncPtr = Irpc_Request_EsifPrimitive,
					.fWriteLogFuncPtr = Irpc_Request_EsifWriteLog,
					.fRegisterEventFuncPtr = Irpc_Request_EsifEventRegister,
					.fUnregisterEventFuncPtr = Irpc_Request_EsifEventUnregister,
					.fSendEventFuncPtr = Irpc_Request_EsifSendEvent,
					.fSendCommandFuncPtr = Irpc_Request_EsifSendCommand,
				};
				appIfaceSet->hdr = decoded_hdr;
				appIfaceSet->esifIface = esifIface;

				// For x86/x64 compatibility between processes
				appIfaceSet->hdr.fIfaceSize = sizeof(*appIfaceSet);

				// Save Session Data
				client->appSession.esifHandle = Irpc_Uncast_EsifHandle(ipcbuf->esifHandle);
				client->appSession.appHandle = ESIF_INVALID_HANDLE;

				trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
				trx->result = ESIF_E_NOT_IMPLEMENTED;

				// Call into Loaded AppInterface
				if (client->appSession.ifaceSet.appIface.fAppCreateFuncPtr) {
					Encoded_AppData *encoded_appData = (Encoded_AppData *)(Irpc_OffsetFrom(*ipcbuf, appData) + Irpc_Uncast_UInt32(ipcbuf->appData.offset));
					AppData decoded_appData = {
						.fPathHome = Irpc_Uncast_EsifData(&encoded_appData->fPathHome, NULL),
						.fLogLevel = Irpc_Uncast_eLogType(encoded_appData->fLogLevel),
					};
					decoded_appData.fPathHome.buf_ptr = Irpc_Deserialize_EsifData(NULL, &encoded_appData->fPathHome, Irpc_OffsetFrom(*encoded_appData, fPathHome));

					// Intercept Verbosity Changed Events to set Trace Level
					IpfTrace_SetTraceLevel((int)decoded_appData.fLogLevel);

					if (decoded_appData.fPathHome.buf_ptr == NULL) {
						trx->result = ESIF_E_NO_MEMORY;
					}
					else {
						// Translate Paths so Dptf.so on Linux can work with ESIF on Windows, or Dptf.dll on Windows can work with ESIF on Linux
						char *localPaths = NULL;
						u32 localLen = 0;
						char *buf_ptr = NULL;
#if   defined(ESIF_ATTR_OS_CHROME)
						if (isalpha(((char *)decoded_appData.fPathHome.buf_ptr)[0]) && ((char *)decoded_appData.fPathHome.buf_ptr)[1] == ':') {
							localPaths = "/var/log/dptf";
						}
#else
						if (isalpha(((char *)decoded_appData.fPathHome.buf_ptr)[0]) && ((char *)decoded_appData.fPathHome.buf_ptr)[1] == ':') {
							localPaths = "/usr/share/dptf/log";
						}
#endif
						if (localPaths && ((localLen = (u32)esif_ccb_strlen(localPaths, MAX_PATH) + 1) > 1) && ((buf_ptr = esif_ccb_malloc(localLen)) != NULL)) {
							esif_ccb_free(decoded_appData.fPathHome.buf_ptr);
							decoded_appData.fPathHome.buf_ptr = buf_ptr;
							decoded_appData.fPathHome.buf_len = decoded_appData.fPathHome.data_len = localLen;
							esif_ccb_strcpy(decoded_appData.fPathHome.buf_ptr, localPaths, localLen);
						}

						// Call AppCreate RPC Function
						trx->result = client->appSession.ifaceSet.appIface.fAppCreateFuncPtr(
							appIfaceSet,
							Irpc_Uncast_EsifHandle(ipcbuf->esifHandle),
							&client->appSession.appHandle,
							&decoded_appData,
							Irpc_Uncast_eAppState(ipcbuf->initialAppState)
						);
						esif_ccb_free(decoded_appData.fPathHome.buf_ptr);
					}
				}

				Encoded_AppCreateFunction *response = Irpc_Encode_AppCreateFunc(
					trx,
					appIfaceSet,
					Irpc_Uncast_EsifHandle(ipcbuf->esifHandle),
					&client->appSession.appHandle,
					NULL,
					Irpc_Uncast_eAppState(ipcbuf->initialAppState)
				);

				if (response) {
					response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
					response->result = Irpc_Cast_eEsifError(trx->result);
					responseHdr.v1.msglen = (UInt32)(IBinary_GetLen(trx->response) - msgstart);
					Irpc_SetResponseHeader(trx->response, responseHdr);
					result = trx->response;
					trx->response = NULL;
				}

				if (result && trx->result == ESIF_OK) {
					DEBUGMSG("App Started (%s)\n", esif_rc_str(trx->result));
				}
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

IBinary *Irpc_Response_AppHandle(IpcSession *client, eIrpcFunction funcId, Encoded_IrpcMsg *msg)
{
	Encoded_AppHandleFunction *ipcbuf = (Encoded_AppHandleFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);

	if (client && ipcbuf && trx) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
			trx->result = ESIF_E_NOT_IMPLEMENTED;

			// Call into Loaded AppInterface
			if (client->appSession.appHandle != Irpc_Uncast_EsifHandle(ipcbuf->appHandle)) {
				trx->result = ESIF_E_INVALID_HANDLE;
			}
			else {
				switch (funcId) {
				case IrpcFunc_AppDestroy:
					if (client->appSession.ifaceSet.appIface.fAppDestroyFuncPtr) {
						trx->result = client->appSession.ifaceSet.appIface.fAppDestroyFuncPtr(client->appSession.appHandle);
					}
					break;
				case IrpcFunc_AppSuspend:
					if (client->appSession.ifaceSet.appIface.fAppSuspendFuncPtr) {
						trx->result = client->appSession.ifaceSet.appIface.fAppSuspendFuncPtr(client->appSession.appHandle);
					}
					break;
				case IrpcFunc_AppResume:
					if (client->appSession.ifaceSet.appIface.fAppResumeFuncPtr) {
						trx->result = client->appSession.ifaceSet.appIface.fAppResumeFuncPtr(client->appSession.appHandle);
					}
					break;
				default:
					trx->result = ESIF_E_INVALID_REQUEST_TYPE;
					break;
				}
			}

			Encoded_AppHandleFunction *response = Irpc_Encode_AppHandleFunc(
				trx,
				funcId,
				client->appSession.appHandle
			);

			// Destroy Session Data
			if (funcId == IrpcFunc_AppDestroy) {
				client->appSession.esifHandle = ESIF_INVALID_HANDLE;
				client->appSession.appHandle = ESIF_INVALID_HANDLE;
			}

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				responseHdr.v1.msglen = Irpc_Cast_UInt32((UInt32)(IBinary_GetLen(trx->response) - msgstart));
				Irpc_SetResponseHeader(trx->response, responseHdr);
				result = trx->response;
				trx->response = NULL;
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

IBinary *Irpc_Response_AppGetStatus(IpcSession *client, Encoded_IrpcMsg *msg)
{
	Encoded_AppGetStatusFunction *ipcbuf = (Encoded_AppGetStatusFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);

	if (client && ipcbuf && trx) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);
			EsifData *decoded_appStatusOut = Irpc_Deserialize_EsifDataPtr(NULL, &ipcbuf->appStatusOut, Irpc_OffsetFrom(*ipcbuf, appStatusOut));

			if (decoded_appStatusOut && decoded_appStatusOut->buf_ptr) {
				AppInterface *appIface = &client->appSession.ifaceSet.appIface;

				trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
				trx->result = ESIF_E_NOT_IMPLEMENTED;

				// Call into Loaded AppInterface
				if (client->appSession.appHandle != Irpc_Uncast_EsifHandle(ipcbuf->appHandle)) {
					trx->result = ESIF_E_INVALID_HANDLE;
				}
				else if (appIface->fAppGetStatusFuncPtr) {
					trx->result = appIface->fAppGetStatusFuncPtr(
						Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
						Irpc_Uncast_eAppStatusCommand(ipcbuf->command),
						Irpc_Uncast_UInt32(ipcbuf->appStatusIn),
						decoded_appStatusOut
					);
				}

				Encoded_AppGetStatusFunction *response = Irpc_Encode_AppGetStatusFunc(
					trx,
					Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
					Irpc_Uncast_eAppStatusCommand(ipcbuf->command),
					Irpc_Uncast_UInt32(ipcbuf->appStatusIn),
					decoded_appStatusOut
				);

				if (response) {
					response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
					response->result = Irpc_Cast_eEsifError(trx->result);
					responseHdr.v1.msglen = (UInt32)(IBinary_GetLen(trx->response) - msgstart);
					Irpc_SetResponseHeader(trx->response, responseHdr);
					result = trx->response;
					trx->response = NULL;
				}
				esif_ccb_free(decoded_appStatusOut->buf_ptr);
			}
			esif_ccb_free(decoded_appStatusOut);
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

IBinary *Irpc_Response_AppParticipantCreate(IpcSession *client, Encoded_IrpcMsg *msg)
{
	Encoded_AppParticipantCreateFunction *ipcbuf = (Encoded_AppParticipantCreateFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (client && ipcbuf && trx) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
			trx->result = ESIF_E_NOT_IMPLEMENTED;

			// Call into Loaded AppInterface
			if (client->appSession.appHandle != Irpc_Uncast_EsifHandle(ipcbuf->appHandle)) {
				trx->result = ESIF_E_INVALID_HANDLE;
			}
			else if (client->appSession.ifaceSet.appIface.fParticipantCreateFuncPtr) {
				Encoded_AppParticipantData *encoded_participantData = (Encoded_AppParticipantData *)(Irpc_OffsetFrom(*ipcbuf, participantData) + Irpc_Uncast_UInt32(ipcbuf->participantData.offset));

				AppParticipantData participantData = { 0 };
				AppParticipantData *decoded_participantData = Irpc_Decode_AppParticipantData(&participantData, encoded_participantData);

				if (decoded_participantData) {
					trx->result = client->appSession.ifaceSet.appIface.fParticipantCreateFuncPtr(
						Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
						Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
						decoded_participantData,
						Irpc_Uncast_eParticipantState(ipcbuf->participantInitialState)
					);
					IPFDEBUG(">>>> %s: Received ParticipantCreate Name=%s\n", ESIF_FUNC, (esif_string)decoded_participantData->fName.buf_ptr);

					esif_ccb_free(decoded_participantData->fDriverType.buf_ptr);
					esif_ccb_free(decoded_participantData->fDeviceType.buf_ptr);
					esif_ccb_free(decoded_participantData->fName.buf_ptr);
					esif_ccb_free(decoded_participantData->fDesc.buf_ptr);
					esif_ccb_free(decoded_participantData->fDriverName.buf_ptr);
					esif_ccb_free(decoded_participantData->fDeviceName.buf_ptr);
					esif_ccb_free(decoded_participantData->fDevicePath.buf_ptr);

					esif_ccb_free(decoded_participantData->fAcpiDevice.buf_ptr);
					esif_ccb_free(decoded_participantData->fAcpiScope.buf_ptr);
					esif_ccb_free(decoded_participantData->fAcpiUID.buf_ptr);
				}
			}

			Encoded_AppParticipantCreateFunction *response = Irpc_Encode_AppParticipantCreateFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
				NULL,
				Irpc_Uncast_eParticipantState(ipcbuf->participantInitialState)
			);

			if (response) {
				response->irpcHdr.msgtype = IrpcMsg_ProcResponse;
				response->result = Irpc_Cast_eEsifError(trx->result);
				responseHdr.v1.msglen = (UInt32)(IBinary_GetLen(trx->response) - msgstart);
				Irpc_SetResponseHeader(trx->response, responseHdr);
				result = trx->response;
				trx->response = NULL;
			}
			rc = trx->result;
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

IBinary *Irpc_Response_AppParticipantDestroy(IpcSession *client, Encoded_IrpcMsg *msg)
{
	Encoded_AppParticipantDestroyFunction *ipcbuf = (Encoded_AppParticipantDestroyFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);

	if (client && ipcbuf && trx) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
			trx->result = ESIF_E_NOT_IMPLEMENTED;

			// Call into Loaded AppInterface
			if (client->appSession.appHandle != Irpc_Uncast_EsifHandle(ipcbuf->appHandle)) {
				trx->result = ESIF_E_INVALID_HANDLE;
			}
			else if (client->appSession.ifaceSet.appIface.fParticipantDestroyFuncPtr) {
				trx->result = client->appSession.ifaceSet.appIface.fParticipantDestroyFuncPtr(
					Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
					Irpc_Uncast_EsifHandle(ipcbuf->participantHandle)
				);
			}

			Encoded_AppParticipantDestroyFunction *response = Irpc_Encode_AppParticipantDestroyFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->participantHandle)
			);

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				responseHdr.v1.msglen = Irpc_Cast_UInt32((UInt32)(IBinary_GetLen(trx->response) - msgstart));
				Irpc_SetResponseHeader(trx->response, responseHdr);
				result = trx->response;
				trx->response = NULL;
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

IBinary *Irpc_Response_AppDomainCreate(IpcSession *client, Encoded_IrpcMsg *msg)
{
	Encoded_AppDomainCreateFunction *ipcbuf = (Encoded_AppDomainCreateFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (client && ipcbuf && trx) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
			trx->result = ESIF_E_NOT_IMPLEMENTED;

			// Call into Loaded AppInterface
			if (client->appSession.appHandle != Irpc_Uncast_EsifHandle(ipcbuf->appHandle)) {
				trx->result = ESIF_E_INVALID_HANDLE;
			}
			else if (client->appSession.ifaceSet.appIface.fDomainCreateFuncPtr) {
				Encoded_AppDomainData *encoded_domainDataPtr = (Encoded_AppDomainData *)(Irpc_OffsetFrom(*ipcbuf, domainDataPtr) + Irpc_Uncast_UInt32(ipcbuf->domainDataPtr.offset));

				AppDomainData domainData = { 0 };
				AppDomainData *decoded_domainDataPtr = Irpc_Decode_AppDomainData(&domainData, encoded_domainDataPtr);

				if (decoded_domainDataPtr) {
					trx->result = client->appSession.ifaceSet.appIface.fDomainCreateFuncPtr(
						Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
						Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
						Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
						decoded_domainDataPtr,
						Irpc_Uncast_eDomainState(ipcbuf->domainInitialState)
					);
					IPFDEBUG(">>>> %s: Received DomainCreate Name=%s\n", ESIF_FUNC, (esif_string)decoded_domainDataPtr->fName.buf_ptr);

					esif_ccb_free(decoded_domainDataPtr->fName.buf_ptr);
					esif_ccb_free(decoded_domainDataPtr->fDescription.buf_ptr);
					esif_ccb_free(decoded_domainDataPtr->fGuid.buf_ptr);
				}
			}

			Encoded_AppDomainCreateFunction *response = Irpc_Encode_AppDomainCreateFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
				NULL,
				Irpc_Uncast_eDomainState(ipcbuf->domainInitialState)
			);

			if (response) {
				response->irpcHdr.msgtype = IrpcMsg_ProcResponse;
				response->result = Irpc_Cast_eEsifError(trx->result);
				responseHdr.v1.msglen = (UInt32)(IBinary_GetLen(trx->response) - msgstart);
				Irpc_SetResponseHeader(trx->response, responseHdr);
				result = trx->response;
				trx->response = NULL;
			}
			rc = trx->result;
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

IBinary *Irpc_Response_AppDomainDestroy(IpcSession *client, Encoded_IrpcMsg *msg)
{
	Encoded_AppDomainDestroyFunction *ipcbuf = (Encoded_AppDomainDestroyFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);

	if (client && ipcbuf && trx) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
			trx->result = ESIF_E_NOT_IMPLEMENTED;

			// Call into Loaded AppInterface
			if (client->appSession.appHandle != Irpc_Uncast_EsifHandle(ipcbuf->appHandle)) {
				trx->result = ESIF_E_INVALID_HANDLE;
			}
			else if (client->appSession.ifaceSet.appIface.fDomainDestroyFuncPtr) {
				trx->result = client->appSession.ifaceSet.appIface.fDomainDestroyFuncPtr(
					Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
					Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
					Irpc_Uncast_EsifHandle(ipcbuf->domainHandle)
				);
			}

			Encoded_AppDomainDestroyFunction *response = Irpc_Encode_AppDomainDestroyFunc(
				trx,
				Irpc_Uncast_EsifHandle(ipcbuf->appHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->domainHandle)
			);

			if (response) {
				response->irpcHdr.msgtype = Irpc_Cast_eIrpcMsgType(IrpcMsg_ProcResponse);
				response->result = Irpc_Cast_eEsifError(trx->result);
				responseHdr.v1.msglen = Irpc_Cast_UInt32((UInt32)(IBinary_GetLen(trx->response) - msgstart));
				Irpc_SetResponseHeader(trx->response, responseHdr);
				result = trx->response;
				trx->response = NULL;
			}
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

IBinary *Irpc_Response_AppEvent(IpcSession *client, Encoded_IrpcMsg *msg)
{
	Encoded_AppEventFunction *ipcbuf = (Encoded_AppEventFunction *)msg;
	IBinary *result = NULL;
	IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_ProcResponse);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	esif_handle_t theAppHandle = (client ? client->appSession.appHandle : ESIF_INVALID_HANDLE);

	if (client && ipcbuf && trx && theAppHandle != ESIF_INVALID_HANDLE) {
		trx->ipfHandle = client->appSession.ipfHandle;

		EsifMsgHdr responseHdr = {
			.v1.signature = ESIFMSG_SIGNATURE,
			.v1.headersize = sizeof(EsifMsgHdr),
			.v1.version = ESIFMSG_VERSION,
			.v1.msgclass = ESIFMSG_CLASS_IRPC,
			.v1.msglen = 0
		};

		if (IBinary_Append(trx->response, &responseHdr, sizeof(responseHdr)) != NULL) {
			size_t msgstart = IBinary_GetLen(trx->response);

			trx->trxId = Irpc_Uncast_UInt64(ipcbuf->irpcHdr.trxId);
			trx->result = ESIF_E_NOT_IMPLEMENTED;

			// Call into Loaded AppInterface
			// Allow Events with Undefined ipcbuf->appHandle since Events may be sent by ESIF before AppCreate completes.
			if (client->appSession.appHandle != Irpc_Uncast_EsifHandle(ipcbuf->appHandle) && Irpc_Uncast_EsifHandle(ipcbuf->appHandle) != ESIF_INVALID_HANDLE) {
				trx->result = ESIF_E_INVALID_HANDLE;
			}
			else if (client->appSession.ifaceSet.appIface.fAppEventFuncPtr) {
				EsifData *decoded_eventData = Irpc_Deserialize_EsifDataPtr(NULL, &ipcbuf->eventData, Irpc_OffsetFrom(*ipcbuf, eventData));
				EsifData *decoded_eventGuid = Irpc_Deserialize_EsifDataPtr(NULL, &ipcbuf->eventGuid, Irpc_OffsetFrom(*ipcbuf, eventGuid));

				trx->result = client->appSession.ifaceSet.appIface.fAppEventFuncPtr(
					theAppHandle,
					Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
					Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
					decoded_eventData,
					decoded_eventGuid
				);

				u8 *guid = (decoded_eventGuid ? decoded_eventGuid->buf_ptr : NULL);
				esif_event_type_t eventType = (esif_event_type_t)(-1);
				char guidbuf[(2 * ESIF_GUID_LEN) + 5] = "NA";
				if (guid) {
					esif_ccb_sprintf(sizeof(guidbuf), guidbuf,
						"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
						guid[0], guid[1], guid[2], guid[3],
						guid[4], guid[5],
						guid[6], guid[7],
						guid[8], guid[9],
						guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]
					);
					esif_event_map_guid2type(decoded_eventGuid->buf_ptr, &eventType);
				}
				IPFDEBUG(">>>> %s: Received Event Guid=%s (%s)\n", ESIF_FUNC, guidbuf, esif_event_type_str(eventType));

				// Intercept Verbosity Changed Events to set Trace Level
				if (eventType == ESIF_EVENT_LOG_VERBOSITY_CHANGED && decoded_eventData && decoded_eventData->buf_ptr && (decoded_eventData->buf_len == sizeof(int))) {
					int traceLevel = *(int *)(decoded_eventData->buf_ptr);
					IpfTrace_SetTraceLevel(traceLevel);
					IPF_TRACE_INFO("Trace Level Verbosity Changed to %d\n", traceLevel);
				}

				if (decoded_eventData) {
					esif_ccb_free(decoded_eventData->buf_ptr);
					esif_ccb_free(decoded_eventData);
				}
				if (decoded_eventGuid) {
					esif_ccb_free(decoded_eventGuid->buf_ptr);
					esif_ccb_free(decoded_eventGuid);
				}
			}

			Encoded_AppEventFunction *response = Irpc_Encode_AppEventFunc(
				trx,
				theAppHandle,
				Irpc_Uncast_EsifHandle(ipcbuf->participantHandle),
				Irpc_Uncast_EsifHandle(ipcbuf->domainHandle),
				NULL,
				NULL
			);

			if (response) {
				response->irpcHdr.msgtype = IrpcMsg_ProcResponse;
				response->result = Irpc_Cast_eEsifError(trx->result);
				responseHdr.v1.msglen = (UInt32)(IBinary_GetLen(trx->response) - msgstart);
				Irpc_SetResponseHeader(trx->response, responseHdr);
				result = trx->response;
				trx->response = NULL;
			}
			rc = trx->result;
		}
	}
	IrpcTransaction_PutRef(trx);
	return result;
}

///////////////////////////////////////////////////////////////////////////////////////

// Process RPC Request on a Thread other than Websocket Thread
esif_error_t Irpc_ProcessRequest(
	IpcSession *self,
	Encoded_IrpcMsg *msg,
	size_t payload_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Send AppDestroy up App Interface to Client App for Broken Connections
	if (msg == NULL) {
		if (self->appSession.appHandle && self->appSession.appHandle != ESIF_INVALID_HANDLE) {
			if (self->appSession.ifaceSet.appIface.fAppDestroyFuncPtr) {
				rc = self->appSession.ifaceSet.appIface.fAppDestroyFuncPtr(self->appSession.appHandle);
			}
		}
	}
	// Otherwise Process RPC Message and send response to Server
	else if (msg && payload_len) {
		IBinary *response = NULL;
		eIrpcFunction funcId = Irpc_Uncast_eIrpcFunction(msg->funcId);
		
		rc = ESIF_E_INVALID_ARGUMENT_COUNT;

		switch (funcId) {
		case IrpcFunc_AppGetName:
			response = Irpc_Response_AppGetString(self, msg);
			break;
		case IrpcFunc_AppGetDescription:
			response = Irpc_Response_AppGetString(self, msg);
			break;
		case IrpcFunc_AppGetVersion:
			response = Irpc_Response_AppGetString(self, msg);
			break;
		case IrpcFunc_AppCommand:
			response = Irpc_Response_AppCommand(self, msg);
			break;
		case IrpcFunc_AppGetIntro:
			response = Irpc_Response_AppGetString(self, msg);
			break;
		case IrpcFunc_AppCreate:
			response = Irpc_Response_AppCreate(self, msg);
			break;
		case IrpcFunc_AppDestroy:
		case IrpcFunc_AppSuspend:
		case IrpcFunc_AppResume:
			response = Irpc_Response_AppHandle(self, funcId, msg);
			break;
		case IrpcFunc_AppGetStatus:
			response = Irpc_Response_AppGetStatus(self, msg);
			break;
		case IrpcFunc_AppParticipantCreate:
			response = Irpc_Response_AppParticipantCreate(self, msg);
			break;
		case IrpcFunc_AppParticipantDestroy:
			response = Irpc_Response_AppParticipantDestroy(self, msg);
			break;
		case IrpcFunc_AppDomainCreate:
			response = Irpc_Response_AppDomainCreate(self, msg);
			break;
		case IrpcFunc_AppDomainDestroy:
			response = Irpc_Response_AppDomainDestroy(self, msg);
			break;
		case IrpcFunc_AppEvent:
			response = Irpc_Response_AppEvent(self, msg);
			break;
		default:
			break;
		}

		if (response) {
			IPFDEBUG("<<<<   IRPC RESPONSE: Bytes=%zd\n", IBinary_GetLen(response));
			// Enqueue Request to be sent by Websocket Thread
			rc = MessageQueue_EnQueue(self->sendQueue, response);
			if (rc == ESIF_OK) {
				response = NULL; // Destroyed by Websocket Thread when DeQueued
				u8 opcode = WS_OPCODE_MESSAGE;
				if (send(self->doorbell[DOORBELL_BUTTON], (const char *)&opcode, sizeof(opcode), 0) != sizeof(opcode)) {
					rc = ESIF_E_WS_SOCKET_ERROR;
				}
			}
			IBinary_Destroy(response);
		}
	}
	return rc;
}

// Callback Function called to Process a Complete ESIF Message
esif_error_t IpcSession_ReceiveMsg(
	IpcSession *self,
	const char *messageBuf,
	size_t messageLen)
{
	esif_error_t rc = ESIF_OK;
	EsifMsgHdrPtr msgHdrPtr = (EsifMsgHdrPtr)messageBuf;
	UInt32 msgclass = 0;
	void *payload_buf = NULL;
	size_t payload_len = 0;

	// Run some sanity check before processing the event data
	if (self && EsifMsgFrame_GetPayload(msgHdrPtr, messageLen, &msgclass, &payload_buf, &payload_len) == ESIF_OK) {

		switch (msgclass) {

		case ESIFMSG_CLASS_IRPC:
		{
			// TODO: This whole function except for this section should run on another thread
			// so that when AppCreate calls back into EsifInterface, the rest of this function can process the response
			Encoded_IrpcMsg *msg = (Encoded_IrpcMsg *)(msgHdrPtr + 1);

			IPFDEBUG(">>>> IRPC MESSAGE: Bytes=%zd Type=%d Func=0x%02X ****\n",
				payload_len,
				Irpc_Uncast_eIrpcMsgType(msg->msgtype),
				Irpc_Uncast_eIrpcFunction(msg->funcId)
			);

			switch (Irpc_Uncast_eIrpcMsgType(msg->msgtype)) {
			case IrpcMsg_ProcRequest:
			{
				IBinary *blob = IBinary_Create();
				if (payload_len && blob) {
					if (IBinary_Clone(blob, msg, payload_len) != NULL) {
						if (MessageQueue_EnQueue(self->recvQueue, blob) == ESIF_OK) {
							blob = NULL; // Destroyed by RPC Thread or Queue Destroyer
							signal_post(&self->rpcSignal);
						}
					}
				}
				IBinary_Destroy(blob);
				break;
			}
			case IrpcMsg_ProcResponse:
			{
				// Process ESIF Response and wakeup calling thread
				IrpcTransaction *trx = NULL;
				UInt64 trxId = Irpc_Uncast_UInt64(msg->trxId);

				// Lookup Session
				trx = IpfTrxMgr_GetTransaction(&self->trxMgr, ESIF_INVALID_HANDLE, trxId);

				// Signal Waiting RPC Thread that the Response has been received
				if (trx) {
					IBinary *blob = IBinary_Create();
					if (IBinary_Clone(blob, msg, payload_len) != NULL) {
						trx->response = blob;
						blob = NULL; // Destroyed by RPC Thread
						IrpcTransaction_Signal(trx);
						rc = ESIF_OK;
					}
					IBinary_Destroy(blob);
				}
				break;
			}
			default:
				rc = ESIF_E_INVALID_REQUEST_TYPE;
				break;
			}
			break;
		}
		default:
			IPFDEBUG("Error parsing ESIF Message Header. Unsupported Message Class '%4.4s'\n", (char *)&msgclass);
			return ESIF_E_SESSION_DISCONNECTED;
			break;
		}
	}
	return rc;
}

// RPC Worker Thread to handle incoming RPC Requests from Remote Server
void * ESIF_CALLCONV IpcSession_RpcWorkerThread(void *ctx)
{
	IpcSession *self = (IpcSession *)ctx;
	IPFDEBUG("Started RPC Worker Thread (%p)\n", self);
	if (self && self->objtype == ObjType_IpcSession) {

		esif_error_t rc = ESIF_OK;
		Bool exitThread = ESIF_FALSE;

		while (!exitThread) {
			signal_wait(&self->rpcSignal);
			IBinary *blob = MessageQueue_DeQueue(self->recvQueue);
			if (blob && IBinary_GetBuf(blob)) {
				// Process RPC Request
				IPFDEBUG("**** RPC Request Len=%zd\n", IBinary_GetLen(blob));
				rc = Irpc_ProcessRequest(self, IBinary_GetBuf(blob), IBinary_GetLen(blob));
				IPFDEBUG("**** RPC Request rc=%s (%d)\n", esif_rc_str(rc), rc);
			}
			else {
				exitThread = ESIF_TRUE;
			}
			IBinary_Destroy(blob);
		}
		
		// Expire All Active Transactions
		IpfTrxMgr_ExpireAll(&self->trxMgr);

		// Generate AppDestroy for Broken Connections
		Irpc_ProcessRequest(self, NULL, 0);
	}

	IPFDEBUG("Exiting RPC Worker Thread\n");
	return 0;
}

// Exported DLL Interface Function
ESIF_EXPORT esif_error_t GetIpcInterface(IpcInterface *theIface)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (theIface) {
		if ((theIface->hdr.fIfaceType != eIfaceTypeIpfIpc) ||
			(theIface->hdr.fIfaceVersion != IPC_INTERFACE_VERSION) ||
			(theIface->hdr.fIfaceSize != (UInt16)sizeof(*theIface))) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		else {
			esif_ccb_strcpy(theIface->Ipc_Version, IPF_APP_VERSION, sizeof(theIface->Ipc_Version));
			esif_ccb_strcpy(theIface->Ipc_SdkVersion, IPF_SDK_VERSION, sizeof(theIface->Ipc_SdkVersion));
			rc = IpfSdk_VersionCheck(theIface->Ipc_SdkVersion, theIface->Ipc_ClientSdkVersion);
		}
		if (rc == ESIF_OK) {
			theIface->Ipc_Init = Ipc_Init;
			theIface->Ipc_Exit = Ipc_Exit;
			theIface->Ipc_SetAppIface = Ipc_SetAppIface;
			theIface->IpcSession_Create = IpcSession_Create;
			theIface->IpcSession_Destroy = IpcSession_Destroy;
			theIface->IpcSession_Connect = IpcSession_Connect;
			theIface->IpcSession_Disconnect = IpcSession_Disconnect;
			theIface->IpcSession_WaitForStop = IpcSession_WaitForStop;
		}
	}
	return rc;
}
