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

#include "esif_ccb_string.h"
#include "esif_sdk_message.h"

#include "ipf_ipc_codec.h"
#include "ipf_ipc_trxmgr.h"
#include "ipfsrv_appmgr.h"
#include "ipfsrv_ws_server.h"
#include "ipfsrv_ws_socket.h"


// WebSocket Frame Header consists of:
// 1. UInt16 header bits
// 2. Optional UInt16 or UInt64 Extended Payload, if hdr.payloadSize=126 or 127
// 3. Optional UInt32 Mask Key, if hdr.maskFlag=1
//
#pragma warning(disable:4214)
#pragma pack(push, 1)
typedef union WsFrameHeader_u {

	struct {
		UInt16 frameType : 4;	// 3:0  - Websocket Frame Type
		UInt16 reserved : 3;	// 7:4  - Reserved
		UInt16 fin : 1;			// 8    - Final Fragment Flag
		UInt16 payloadSize : 7;	// 14:9 - Payload Size: 0-125=hdr.payloadSize, 126=T2.payloadSize, 127=Use T3.payloadSize
		UInt16 maskFlag : 1;	// 15   - Mask Flag: Mandatory for Client to Server, Optional for Server to Client
	} hdr;

	struct {				// If hdr.payloadSize <= 125
		UInt16 header;		// Header Bits
	} T1;

	struct {				// If hdr.payloadSize <= 125
		UInt16 header;		// Header Bits
		UInt32 maskKey;		// If hdr.maskFlag=1
	} T1_WithMask;

	struct {
		UInt16 header;		// Header Bits
		UInt16 payloadSize;	// If hdr.payloadSize=126 [Big-Endian]
	} T2;

	struct {
		UInt16 header;		// Header Bits
		UInt16 payloadSize;	// If hdr.payloadSize=126 [Big-Endian]
		UInt32 maskKey;		// If hdr.maskFlag=1
	} T2_WithMask;

	struct {
		UInt16 header;		// Header Bits
		UInt64 payloadSize;	// If hdr.payloadSize=127 [Big-Endian]
	} T3;

	struct {
		UInt16 header;		// Header Bits
		UInt64 payloadSize;	// If hdr.payloadSize=127 [Big-Endian]
		UInt32 maskKey;		// If hdr.maskFlag=1
	} T3_WithMask;

} WsFrameHeader, *WsFrameHeaderPtr;
#pragma pack(pop)

// Normalized WebSocket Frame
typedef struct WsFrame_s {
	FrameType		frameType;		// Frame Type (or First Frame Type if Multi-Fragment message)
	WsFrameHeader	header;			// Binary Websocket Header
	size_t			headerSize;		// Binary Websocket Header Size
	u8 *			payload;		// Binary Payload
	size_t			payloadSize;	// Binary Payload Size
	size_t			frameSize;		// Total Frame Size (headerSize + payloadSize)
} WsFrame, *WsFramePtr;

#define WSFRAME_HEADER_TYPE1		125	// hdr.payloadSize <= 125
#define WSFRAME_HEADER_TYPE2		126	// hdr.payloadSize = 126, T2.payloadSize = UInt16 [Big-Endian]
#define WSFRAME_HEADER_TYPE3		127	// hdr.payloadSize = 127, T3.payloadSize = UInt64 [Big-Endian]

#define WSFRAME_HEADER_TYPE1_MAX	125			// header Type1 max length
#define WSFRAME_HEADER_TYPE2_MAX	0xFFFF		// header Type2 max length
#define WSFRAME_HEADER_TYPE3_MAX	0x7FFFFFFE	// header Type3 max length

#define MAX_WEBSOCKET_BUFFER		(8*1024*1024)	// Max size of any single inbound websocket message (all combined fragments)
#define MAX_WEBSOCKET_SENDBUF		(1*1024*1024)	// Max size of outbound websocket send buffer (multiple messages)
#define MAX_WEBSOCKET_FRAGMENT_SIZE	(64*1024)		// Max WebSocket Fragment Size for Multi-Fragment TEXT Frames REST API Responses
#define MAX_WEBSOCKET_MSGID_LEN		15				// Max WebSocket MessageId Length (space for "%u:" output)
#define MAX_WEBSOCKET_RESPONSE_LEN	0x7FFFFFFE		// Max WebSocket Response Length

// Compute Websocket Header Size based on Payload Size or 0 if invalid payload_size
static size_t WebSocket_HeaderSize(size_t payload_size, Bool mask_flag)
{
	size_t header_size = 0;
	WsFrameHeader header = { 0 };

	if (payload_size < WSFRAME_HEADER_TYPE1_MAX) {
		header_size = sizeof(header.T1);
	}
	else if (payload_size < WSFRAME_HEADER_TYPE2_MAX) {
		header_size = sizeof(header.T2);
	}
	else if (payload_size < WSFRAME_HEADER_TYPE3_MAX) {
		header_size = sizeof(header.T3);
	}

	if (mask_flag) {
		header_size += sizeof(header.T1_WithMask.maskKey);
	}
	return header_size;
}

// Get Frame Data from Websocket Request
static esif_error_t Websocket_GetFrame(
	WsFramePtr frame,
	u8 *buffer,
	size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	WsFrameHeaderPtr header = (WsFrameHeaderPtr)buffer;

	if (frame && buffer) {
		size_t header_size = sizeof(header->T1);
		size_t payload_size = 0;
		UInt32 mask_key = 0;
		size_t j = 0;

		esif_ccb_memset(frame, 0, sizeof(*frame));

		// Incomplete Header
		if (buf_len < sizeof(header->hdr)) {
			buf_len = 0;
		}
		// Mask Key is Required for Client Frames per RFC 6455
		else if (header->hdr.maskFlag == 0) {
			return ESIF_E_WS_SOCKET_ERROR;
		}
		// PayloadSize 0..125 in hdr.payloadSize
		else if (header->hdr.payloadSize <= WSFRAME_HEADER_TYPE1) {
			if (header->hdr.maskFlag) {
				header_size = sizeof(header->T1_WithMask);
			}

			if (buf_len >= header_size) {
				payload_size = (size_t)header->hdr.payloadSize;
				if (header->hdr.maskFlag) {
					mask_key = header->T1_WithMask.maskKey;
				}
			}
		}
		// PayloadSize is UInt16 in T2.payloadSize
		else if (header->hdr.payloadSize == WSFRAME_HEADER_TYPE2) {
			header_size = sizeof(header->T2);
			if (header->hdr.maskFlag) {
				header_size = sizeof(header->T2_WithMask);
			}

			if (buf_len >= header_size) {
				payload_size = (size_t)esif_ccb_htons(header->T2.payloadSize);
				if (header->hdr.maskFlag) {
					mask_key = header->T2_WithMask.maskKey;
				}
			}
		}
		// PayloadSize is UInt64 in T3.payloadSize
		else  if (header->hdr.payloadSize == WSFRAME_HEADER_TYPE3) {
			header_size = sizeof(header->T3);
			if (header->hdr.maskFlag) {
				header_size = sizeof(header->T3_WithMask);
			}

			if (buf_len >= header_size) {
				payload_size = (size_t)esif_ccb_htonll(header->T3.payloadSize);
				if (header->hdr.maskFlag) {
					mask_key = header->T3_WithMask.maskKey;
				}
			}
		}

		// Verify Full Header and Payload
		if (buf_len < header_size + payload_size) {
			rc = ESIF_E_WS_INCOMPLETE;
		}
		else if (header_size > sizeof(frame->header)) {
			rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		}
		else {
			frame->frameType = header->hdr.frameType;
			esif_ccb_memcpy(&frame->header, header, header_size);
			frame->headerSize = header_size;
			frame->payload = buffer + header_size;
			frame->payloadSize = payload_size;
			frame->frameSize = header_size + payload_size;

			// Unmask Data
			if (frame->header.hdr.maskFlag) {
				for (j = 0; j < frame->payloadSize; j++) {
					frame->payload[j] = frame->payload[j] ^ ((UInt8 *)&mask_key)[j % 4];
				}
			}
			rc = ESIF_OK;
		}
		IPF_TRACE_DEBUG(
			"Received WebSocket Frame: Type=%hd Fin=%hd Mask=%hd MaskKey=0x%x Size=%zd\n",
			frame->header.hdr.frameType,
			frame->header.hdr.fin,
			frame->header.hdr.maskFlag,
			mask_key,
			frame->payloadSize
		);
	}
	return rc;
}

// Build a WebSocket Frame using the given output buffer
static esif_error_t WebSocket_BuildFrame(
	WsFramePtr frame,		// Normalized WebSocket Frame
	void *out_buf,			// Frame Output Buffer (Header+Payload)
	size_t out_buf_len,		// Frame Output Buffer Length
	void *data_buf,			// Frame Payload Data
	size_t data_len,		// Frame Payload Data Length
	FrameType frameType,	// Frame Type
	FinType finType			// FIN Type
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (frame && (out_buf || out_buf_len == 0) && (data_buf || data_len == 0)) {
		size_t header_size = 0;
		esif_ccb_memset(frame, 0, sizeof(*frame));

		rc = ESIF_OK;
		frame->frameType = frameType;
		frame->header.hdr.frameType = frameType;
		frame->header.hdr.fin = finType;

		if (data_len <= WSFRAME_HEADER_TYPE1_MAX) {
			header_size = sizeof(frame->header.T1);
			frame->header.hdr.payloadSize = (UInt8)data_len;
		}
		else if (data_len <= WSFRAME_HEADER_TYPE2_MAX) {
			header_size = sizeof(frame->header.T2);
			frame->header.hdr.payloadSize = WSFRAME_HEADER_TYPE2;
			frame->header.T2.payloadSize = esif_ccb_htons((UInt16)data_len);
		}
		else if (data_len <= WSFRAME_HEADER_TYPE3_MAX) {
			header_size = sizeof(frame->header.T3);
			frame->header.hdr.payloadSize = WSFRAME_HEADER_TYPE3;
			frame->header.T3.payloadSize = esif_ccb_htonll((UInt64)data_len);
		}
		else {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		}

		if (rc == ESIF_OK) {
			if (out_buf && header_size + data_len > out_buf_len) {
				rc = ESIF_E_NEED_LARGER_BUFFER;
			}
			else if (out_buf) {
				esif_ccb_memcpy(out_buf, &frame->header, header_size);
				frame->headerSize = header_size;
				frame->payloadSize = data_len;
				frame->frameSize = header_size + data_len;
				if (data_buf && data_len > 0) {
					esif_ccb_memcpy((u8 *)out_buf + header_size, data_buf, data_len);
					frame->payload = (u8 *)out_buf + header_size;
				}
			}
		}
	}
	return rc;
}

void IpfSrv_Init(void)
{
	IpfTrxMgr_Init(IpfTrxMgr_GetInstance());
}

void IpfSrv_Uninit(void)
{
	IpfTrxMgr_Uninit(IpfTrxMgr_GetInstance());
}

// Process an incoming RPC Message by adding it to RPC Message Queue
// Synchronize with non-WebServer threads
esif_error_t WebServer_ReceiveRequest(WebServerPtr self, WebClientPtr client, IrpcTransaction *trx)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Queue Incoming RPC Message
	if (self && client && trx) {
		ClientRequestPtr object = esif_ccb_malloc(sizeof(ClientRequest));
		if (object == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else {
			object->ipfHandle = client->ipfHandle;
			object->trx = trx;

			WebWorkerPtr rpcWorker = client->rpcWorker;
			WebWorker_GetRef(rpcWorker);
			rc = WebServer_EnQueueRpc(self, rpcWorker, object);

			// Signal RPC Thread that there is work to do
			if (rc == ESIF_OK) {
				signal_post(&rpcWorker->rpcSignal);
			}
			else {
				if (rc == ESIF_E_WS_DISC) {
					WS_TRACE_WARNING("Connection Closed: Session=0x%llx Thread=0x%zx: Request Limit Exceeded (" ATOMIC_FMT ").\n", 
						client->ipfHandle,
						(size_t)rpcWorker->rpcThread,
						atomic_read(&self->rpcQueueMax)
					);
				}
				esif_ccb_free(object);
			}
			WebWorker_PutRef(rpcWorker);
		}
	}
	return rc;
}

esif_error_t WebServer_IrpcRequest(IrpcTransaction *trx)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (trx) {
		rc = IpfTrxMgr_AddTransaction(IpfTrxMgr_GetInstance(), trx);
		if (rc == ESIF_OK) {
			rc = WebServer_SendMsg(g_WebServer, trx->ipfHandle, IBinary_GetBuf(trx->request), IBinary_GetLen(trx->request));
		}
	}
	return rc;
}

// Process incoming IRPC Message and Generate Response
esif_error_t WebServer_IrpcProcess(WebServerPtr self, WebClientPtr client, IBinary *blob)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	Encoded_IrpcMsg *ipcmsg = (Encoded_IrpcMsg *)IBinary_GetBuf(blob);

	// Decode Message and signal waiting thread

	if (self && client && ipcmsg) {
		rc = ESIF_E_INVALID_REQUEST_TYPE;

		switch (Irpc_Uncast_eIrpcMsgType(ipcmsg->msgtype)) {
		// RPC Request from Client
		case IrpcMsg_ProcRequest:
		{
			// Queue this message and Signal the RPC Thread that there is a Message to process
			IrpcTransaction *trx = IrpcTransaction_Create(IrpcMsg_None);
			AppSession *session = AppSessionMgr_GetSessionByHandle(client->ipfHandle);
			if (trx && session) {
				trx->ipfHandle = client->ipfHandle;
				trx->request = blob;
				session->updateTime = esif_ccb_realtime_current();
				IrpcTransaction_GetRef(trx);

				rc = WebServer_ReceiveRequest(self, client, trx);

				if (rc != ESIF_OK) {
					trx->request = NULL;
					IrpcTransaction_PutRef(trx);
				}
			}
			IrpcTransaction_PutRef(trx);
			AppSession_PutRef(session);
			break;
		}
		// RPC Response from Client
		case IrpcMsg_ProcResponse:
		{
			IrpcTransaction *trx = NULL;
			UInt64 trxId = Irpc_Uncast_UInt64(ipcmsg->trxId);

			// Lookup Transaction
			trx = IpfTrxMgr_GetTransaction(IpfTrxMgr_GetInstance(), client->ipfHandle, trxId);

			// Signal Waiting Thread that the Response has been received
			if (trx) {
				AppSession *session = AppSessionMgr_GetSessionByHandle(trx->ipfHandle);
				if (session) {
					session->updateTime = esif_ccb_realtime_current();
					AppSession_PutRef(session);
				}
				trx->response = blob;
				IrpcTransaction_Signal(trx);
				rc = ESIF_OK;
			}
			else {
				// Ignore Expired Transactions
				IBinary_Destroy(blob);
				rc = ESIF_OK;
			}
			break;
		}
		default:
			break;
		}
	}
	return rc;
}

// Process a Websocket Request and send a Response
esif_error_t WebServer_WebsocketResponse(
	WebServerPtr self,
	WebClientPtr client,
	WsFramePtr request)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && client && request) {
		WsFrame outFrame = { 0 };

		switch (request->frameType) {

		case FRAME_CONTINUATION:
			// Ignore Continuation Frames; They will be processed when a FIN is received on the final Frame
			rc = ESIF_OK;
			break;

		case FRAME_TEXT:
			// Ignore Text Frames
			esif_ccb_free(client->fragBuf);
			client->fragBuf = NULL;
			client->fragBufLen = 0;
			client->msgType = FRAME_NULL;
			rc = ESIF_OK;
			break;

		case FRAME_BINARY:
			// Binary Frames start with a valid EsifMsgHdr
			esif_ccb_free(client->fragBuf);
			client->fragBuf = NULL;
			client->fragBufLen = 0;
			client->msgType = FRAME_NULL;
			rc = ESIF_OK;

			EsifMsgHdrPtr message = (EsifMsgHdrPtr)request->payload;
			if (request->payloadSize >= sizeof(EsifMsgHdr) && message->v1.signature == ESIFMSG_SIGNATURE) {
				switch (message->v1.msgclass) {
				case ESIFMSG_CLASS_IRPC: {
					IBinary *blob = IBinary_Create();
					if (IBinary_Clone(blob, (message + 1), message->v1.msglen) != NULL) {
						rc = WebServer_IrpcProcess(self, client, blob);
					}
					if (rc != ESIF_OK) {
						IBinary_Destroy(blob);
					}
					break;
				}
				default:
					rc = ESIF_E_INVALID_REQUEST_TYPE;
					break;
				}
			}
			break;

		case FRAME_CLOSING:
			// Send Closing Frame to Client and Disconnect
			rc = WebSocket_BuildFrame(
				&outFrame,
				self->netBuf, self->netBufLen,
				NULL, 0,
				FRAME_CLOSING,
				FIN_FINAL);

			if (rc == ESIF_OK) {
				rc = WebClient_Write(client, self->netBuf, outFrame.frameSize);
			}
			if (rc == ESIF_OK) {
				rc = ESIF_E_WS_DISC;
			}
			break;

		case FRAME_PING:
			// Respond to PING Frames with PONG Frame containing Ping's Data
			rc = WebSocket_BuildFrame(
				&outFrame,
				self->netBuf, self->netBufLen,
				request->payload, request->payloadSize,
				FRAME_PONG,
				FIN_FINAL);

			if (rc == ESIF_OK) {
				rc = WebClient_Write(client, self->netBuf, outFrame.frameSize);
			}
			break;

		case FRAME_PONG:
			// Handle unsolicited PONG (keepalive) messages from Internet Explorer 10 (Not required per RFC 6455 but allowed)
			rc = WebSocket_BuildFrame(
				&outFrame,
				self->netBuf, self->netBufLen,
				NULL, 0,
				FRAME_TEXT,
				FIN_FINAL);

			if (rc == ESIF_OK) {
				rc = WebClient_Write(client, self->netBuf, outFrame.frameSize);
			}
			break;
		default:
			break;
		}
	}
	return rc;
}

// Process a WebSocket Request, if a complete Message is available
esif_error_t WebServer_WebsocketRequest(
	WebServerPtr self,
	WebClientPtr client,
	u8 *buffer,
	size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && client && buffer && buf_len > 0) {
		size_t bytesRemaining = 0;
		char *bufferRemaining = NULL;

		do {
			WsFrame request = { 0 };

			// If more frames remaining, copy them into buffer and reparse
			if (bufferRemaining != NULL && bytesRemaining > 0) {
				esif_ccb_memcpy(buffer, bufferRemaining, bytesRemaining);
				buf_len = bytesRemaining;
				bytesRemaining = 0;
			}

			// Parse WebSocket Header into a Request Frame
			rc = Websocket_GetFrame(&request, buffer, buf_len);

			if (rc == ESIF_OK && buf_len > request.frameSize) {
				bytesRemaining = buf_len - request.frameSize;
			}

			// Append this partial frame to the current connection's Receive Buffer, if any
			if (rc == ESIF_E_WS_INCOMPLETE) {
				size_t oldSize = client->recvBufLen;
				size_t newSize = oldSize + buf_len;
				u8 *newBuffer = NULL;
				if (newSize <= MAX_WEBSOCKET_BUFFER) {
					newBuffer = esif_ccb_realloc(client->recvBuf, newSize);
				}
				if (newBuffer == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					WS_TRACE_DEBUG("WS Frame Buffering: oldlen=%zd reqlen=%zd total=%zd\n", oldSize, buf_len, newSize);
					esif_ccb_memcpy(newBuffer + oldSize, buffer, buf_len);
					client->recvBuf = newBuffer;
					client->recvBufLen = newSize;
				}
				break;
			}

			// Append this message fragment to the current connection's Fragment Buffer, if any
			if (rc == ESIF_OK && request.header.hdr.fin == FIN_FRAGMENT) {
				size_t oldSize = client->fragBufLen;
				size_t newSize = oldSize + request.payloadSize;
				u8 *newBuffer = NULL;
				if (newSize <= MAX_WEBSOCKET_BUFFER) {
					newBuffer = esif_ccb_realloc(client->fragBuf, newSize);
				}
				if (newBuffer == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					WS_TRACE_DEBUG("WS Fragment Buffering: buflen=%zd msglen=%zd total=%zd\n", oldSize, request.payloadSize, newSize);
					esif_ccb_memcpy(newBuffer + oldSize, request.payload, request.payloadSize);
					client->fragBuf = newBuffer;
					client->fragBufLen = newSize;
				}
				// Multi-Fragment messages put real opcode in 1st Fragment only; All others are Continuation Frames, including FIN
				if (request.frameType != FRAME_CONTINUATION) {
					client->msgType = request.frameType;
				}
			}

			// Use First Fragment's Frame Type if FIN is set on (final) CONTINUATION frame
			if (request.frameType == FRAME_CONTINUATION && request.header.hdr.fin == FIN_FINAL) {
				request.frameType = client->msgType;
			}

			// Save remaining frames for reparsing if more than one frame received
			if (bytesRemaining > 0) {
				if (bufferRemaining == NULL) {
					bufferRemaining = (char *)esif_ccb_malloc(bytesRemaining);
					if (NULL == bufferRemaining) {
						rc = ESIF_E_NO_MEMORY;
					}
				}
				if (bufferRemaining) {
					esif_ccb_memcpy(bufferRemaining, request.payload + request.payloadSize, bytesRemaining);
				}
			}

			// Close Connection on Error
			if (rc != ESIF_OK) {
				WS_TRACE_DEBUG("Invalid Frame; Closing socket: rc=%s (%d) Type=%02hX FIN=%hd Len=%hd (%zd) Mask=%hd\n", esif_rc_str(rc), rc, request.header.hdr.frameType, request.header.hdr.fin, request.header.hdr.payloadSize, request.payloadSize, request.header.hdr.maskFlag);

				// Send Closing Frame to Client
				WsFrame outFrame = { 0 };
				rc = WebSocket_BuildFrame(
					&outFrame,
					self->netBuf, self->netBufLen,
					NULL, 0,
					FRAME_CLOSING,
					FIN_FINAL);

				if (rc == ESIF_OK) {
					rc = WebClient_Write(client, self->netBuf, outFrame.payloadSize);
				}
				if (rc == ESIF_OK) {
					rc = ESIF_E_WS_DISC;
				}
				break;
			}

			// Process a Complete Websocket Single or Multi-Fragment Message
			if (rc == ESIF_OK && request.header.hdr.fin == FIN_FINAL) {
				rc = WebServer_WebsocketResponse(self, client, &request);
			}

		} while (bytesRemaining > 0);

		esif_ccb_free(bufferRemaining);
	}
	return rc;
}

// Deliver a buffer to an Active WebSocket Client connection
esif_error_t WebServer_WebsocketDeliver(
	WebServerPtr self,
	WebClientPtr client,
	u8 *buffer,
	size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && client && buffer && buf_len > 0) {
		WsFrame outFrame = { 0 };

		rc = WebSocket_BuildFrame(
			&outFrame,
			self->netBuf, self->netBufLen,
			buffer, buf_len,
			FRAME_BINARY,
			FIN_FINAL);

		if (rc == ESIF_OK) {
			rc = WebClient_Write(client, self->netBuf, outFrame.frameSize);
		}
	}
	return rc;
}