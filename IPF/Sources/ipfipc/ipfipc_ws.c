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

#define ESIF_CCB_LINK_LIST_MAIN

#include "esif_ccb_rc.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_file.h"
#include "esif_ccb_string.h"
#include "esif_ccb_random.h"
#include "esif_sdk_base64.h"

#include "ipf_ipc_clisrv.h"
#include "ipf_trace.h"
#include "ipf_handle.h"
#include "ipfipc_ws.h"

#define ESIF_ATTR_SHA1
#define ESIF_SDK_SHA_MAIN
#include "esif_sdk_sha.h"
#include "ipfipc_trace.h"

// OS-Specific Implementations

static ESIF_INLINE UInt64 htonll(UInt64 value)
{
	UInt32 hi = htonl((UInt32)(value >> 32));
	UInt32 lo = htonl((UInt32)value);
	return (((UInt64)lo) << 32) | hi;
}

#define RECV_BUF_SIZE		(1*1024*1024) /*8192*/	// Initial Receive Buffer Size
#define RECV_BUF_GROWBY		512		// Amount to Grow Receive Buffer by when Incomplete Frame

#define WSFRAME_HEADER_TYPE1		125	// hdr.payloadSize <= 125
#define WSFRAME_HEADER_TYPE2		126	// hdr.payloadSize = 126, T2.payloadSize = UInt16 [Big-Endian]
#define WSFRAME_HEADER_TYPE3		127	// hdr.payloadSize = 127, T3.payloadSize = UInt64 [Big-Endian]

#define WSFRAME_HEADER_TYPE1_MAX	125			// header Type1 max length
#define WSFRAME_HEADER_TYPE2_MAX	0xFFFF		// header Type2 max length
#define WSFRAME_HEADER_TYPE3_MAX	0x7FFFFFFE	// header Type3 max length

/*
** PRIVATE FUNCTIONS
*/

static Bool esif_ws_mask_enabled(const WsSocketFramePtr framePtr)
{
	return (Bool)framePtr->header.s.maskFlag;
}

static size_t esif_ws_get_header_size(
	const WsSocketFramePtr framePtr,
	size_t frameLen
	)
{
	size_t hdrLen = 0;

	if (framePtr && frameLen >= sizeof(framePtr->header)) {

		hdrLen += sizeof(framePtr->header);

		// Check if we have mask bits present or not
		if (esif_ws_mask_enabled(framePtr)) {
			hdrLen += sizeof(framePtr->u.T1.maskKey);
		}

		if (WS_FRAME_SIZE_TYPE_2 == framePtr->header.s.payLen) {
			hdrLen += sizeof(framePtr->u.T2_NoMask.extPayLen);
		}
		else if (WS_FRAME_SIZE_TYPE_3 == framePtr->header.s.payLen) {
			hdrLen += sizeof(framePtr->u.T3_NoMask.extPayLen);
		}

		if (frameLen < hdrLen) {
			hdrLen = 0;
		}
	}
	return hdrLen;
}

static size_t esif_ws_get_payload_size (
	const WsSocketFramePtr framePtr,
	size_t incomingFrameLen,
	FrameTypePtr frameTypePtr
	)
{
	size_t payLen = 0;
	size_t hdrLen = 0;

	hdrLen = esif_ws_get_header_size(framePtr, incomingFrameLen);
	if (0 == hdrLen) {
		*frameTypePtr  = INCOMPLETE_FRAME;
		return 0;
	}

	/* Get the base payload type/length */
	payLen = framePtr->header.s.payLen;

	if (payLen == WS_FRAME_SIZE_TYPE_2) {
		payLen = (size_t) htons(framePtr->u.T2.extPayLen);
	}
	else if (payLen == WS_FRAME_SIZE_TYPE_3) {
		payLen = (size_t) htonll(framePtr->u.T3.extPayLen);
		if (payLen > WS_FRAME_MAXIMUM_SIZE) {
			*frameTypePtr  = ERROR_FRAME;
			payLen = 0;
		}
	}
	return payLen;
}

static FrameType esif_ws_get_frame_type(
	WsSocketFramePtr framePtr,
	size_t incomingFrameLen,
	char **dataPtr,
	size_t *dataLength,
	size_t *bytesRemaining
)
{
	size_t hdrLen = 0;
	size_t payloadLength = 0;
	UInt8 mode = 0;
	FrameType frameType = INCOMPLETE_FRAME;

	*bytesRemaining = 0;

	hdrLen = esif_ws_get_header_size(framePtr, incomingFrameLen);
	if (0 == hdrLen) {
		return INCOMPLETE_FRAME;
	}

	if (framePtr->header.s.rsvd1 != 0) {
		return ERROR_FRAME;
	}

	mode = (UInt8)framePtr->header.s.opcode;
	if (mode == TEXT_FRAME || mode == BINARY_FRAME || mode == CLOSING_FRAME || mode == PING_FRAME || mode == PONG_FRAME || mode == CONTINUATION_FRAME) {
		frameType = (FrameType)mode;

		payloadLength = esif_ws_get_payload_size(framePtr, incomingFrameLen, &frameType);
		if (payloadLength <= 0) {
			return frameType;
		}

		if (payloadLength > (incomingFrameLen - hdrLen)) {
			return INCOMPLETE_FRAME;
		}

		if (payloadLength < (incomingFrameLen - hdrLen)) {
			*bytesRemaining = incomingFrameLen - hdrLen - payloadLength;
		}

		if (payloadLength <= WSFRAME_HEADER_TYPE1_MAX) {
			*dataPtr = &(framePtr->u.T1_NoMask.data[0]);
		}
		else if (payloadLength <= WSFRAME_HEADER_TYPE2_MAX) {
			*dataPtr = &(framePtr->u.T2_NoMask.data[0]);
		}
		else {
			*dataPtr = &(framePtr->u.T3_NoMask.data[0]);
		}
		*dataLength = payloadLength;

		return frameType;
	}
	return ERROR_FRAME;
}

// Send a PONG Response for the given WebSocket PING request
static esif_error_t IpcSession_PongResponse(
	IpcSession *self,
	char *messageBuf,
	size_t messageSize
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Send Pong Response Frame with optional payload data which must be 125 bytes or less
	if (messageSize > WS_FRAME_SIZE_TYPE_1) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
	}
	else if (self && (messageBuf || messageSize == 0)) {
		char pongBuffer[sizeof(WsSocketFrameHeader) + WS_FRAME_SIZE_TYPE_1] = { 0 };
		WsSocketFrameHeaderPtr header = (WsSocketFrameHeaderPtr)pongBuffer;
		size_t pongLen = sizeof(*header);
		ssize_t bytesSent = 0;
		size_t bytesRemaining = 0;

		header->s.opcode = PONG_FRAME;
		header->s.fin = 1;
		header->s.payLen = (UInt16)messageSize;
		header->s.maskFlag = 0;

		if (messageBuf && messageSize) {
			esif_ccb_memcpy(pongBuffer + sizeof(*header), messageBuf, messageSize);
			pongLen += messageSize;
		}

		// Blocking Send
		bytesRemaining = pongLen;
		do {
			bytesRemaining -= bytesSent;
			ESIF_SERIAL_FENCE();
			bytesSent = send(self->socket, pongBuffer + bytesSent, (int)bytesRemaining, 0);
		} while (bytesSent > 0 && (size_t)bytesSent < bytesRemaining);

		if (bytesSent > 0) {
			rc = ESIF_OK;
		}
		else if (bytesSent == 0) {
			rc = ESIF_E_WS_DISC;
		}
		else {
			rc = ESIF_E_WS_SOCKET_ERROR;
		}
	}
	return rc;
}

//
// IPF Session Handle Manager (We only support one Session for now)
//
typedef struct IpcSessionMgr_s {
	IpcSession_t	handle;
	IpcSession		*session;
} IpcSessionMgr;

static IpcSessionMgr g_ipcSessionMgr = {
	.handle = IPC_INVALID_SESSION,
	.session = NULL
};

// Create a new IPC Session Handle for the given IPC Session Object
static IpcSession_t IpcSessionMgr_CreateHandle(IpcSession *session)
{
	IpcSession_t handle = IPC_INVALID_SESSION;
	if (session && g_ipcSessionMgr.handle == IPC_INVALID_SESSION) {
		const esif_handle_t handle_min = 0x0000000010000000; // Min Handle before Random Seed
		const esif_handle_t handle_max = 0x7ffffffff0000000; // Max Handle before Rollover
		const esif_handle_t handle_seed= 0x0000000000ffffff; // Handle Initial Seed Random Mask
		const esif_handle_t handle_inc = 0x000000000000ffff; // Handle Increment Random Mask

		handle = Ipf_GenerateHandle(handle_min, handle_max, handle_seed, handle_inc);

		g_ipcSessionMgr.handle = handle;
		g_ipcSessionMgr.session = session;
	}
	return handle;
}

// Remove the Handle for the given Session Handle
static void IpcSessionMgr_DeleteHandle(IpcSession_t handle)
{
	if (g_ipcSessionMgr.handle == handle || handle == IPC_INVALID_SESSION) {
		g_ipcSessionMgr.handle = IPC_INVALID_SESSION;
		g_ipcSessionMgr.session = NULL;
	}
}

// Convert an IPF Session Handle to an IPC Session Handle. We only support One Session for now.
static IpcSession *IpcSessionMgr_GetSessionByHandle(IpcSession_t handle)
{
	IpcSession *session = NULL;
	if (handle != IPC_INVALID_SESSION && handle == g_ipcSessionMgr.handle) {
		session = g_ipcSessionMgr.session;
	}
	return session;
}



/*
** Public Functions
*/

static AppInterfaceSet g_ipcAppIfaceSet = { 0 };

esif_error_t Ipc_Init(void)
{
	esif_error_t rc = (esif_ccb_socket_init() == 0 ? ESIF_OK : ESIF_E_WS_INIT_FAILED);

	// Intialize Trace Logging
	if (rc == ESIF_OK) {
		rc = IpfTrace_Init();
		if (rc == ESIF_OK) {
			IpfTrace_SetConfig("IPFIPC", IpfIpc_WriteLog, ESIF_FALSE);

			// Register as an ETW provider (if available)
			IpfRegisterEtwProvider();

			IPF_TRACE_DEBUG("Tracing started");

			// Initialize Transaction Manager
			rc = IpfTrxMgr_Init();

			if (rc != ESIF_OK) {
				// Unregister as an ETW provider (if available)
				IpfUnregisterEtwProvider();

				IpfTrace_Exit();
			}
		}
		if (rc != ESIF_OK) {
			esif_ccb_socket_exit();
		}
	}
	return rc;
}

void Ipc_Exit(void)
{
	IpfTrxMgr_Uninit();

	// Unregister as an ETW provider (if available)
	IpfUnregisterEtwProvider();

	IpfTrace_Exit();
	esif_ccb_socket_exit();
}

// Set ESIF App Interface Function Pointers for use by IPC Session
esif_error_t Ipc_SetAppIface(const AppInterfaceSet *ifaceSet)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	
	if (ifaceSet) {
		// Check EsifAppInterface
		if (ifaceSet->hdr.fIfaceType != eIfaceTypeApplication ||
			ifaceSet->hdr.fIfaceVersion != APP_INTERFACE_VERSION ||
			ifaceSet->hdr.fIfaceSize != (UInt16)sizeof(*ifaceSet)) {
			rc = ESIF_E_NOT_SUPPORTED;
		}
		// Check Required Functions Pointers
		else if (ifaceSet->appIface.fAppCreateFuncPtr == NULL ||
				 ifaceSet->appIface.fAppDestroyFuncPtr == NULL ||
				 ifaceSet->appIface.fAppGetNameFuncPtr == NULL ||
				 ifaceSet->appIface.fParticipantCreateFuncPtr == NULL ||
				 ifaceSet->appIface.fParticipantDestroyFuncPtr == NULL ||
				 ifaceSet->appIface.fDomainCreateFuncPtr == NULL ||
				 ifaceSet->appIface.fDomainDestroyFuncPtr == NULL) {
			rc = ESIF_E_CALLBACK_IS_NULL;
		}
		else {
			g_ipcAppIfaceSet = *ifaceSet;
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Initialize WebSocket Client using given IP-Address/Hostname and Port
IpcSession_t IpcSession_Create(const IpcSessionInfo *info)
{
	// Verify Optional Server Address before creating Session
	const char *serverAddr = DEFAULT_SERVERADDR;
	esif_ccb_sockaddr_t sockaddr = { 0 };
	if (info) {
		if (info->revision == IPF_SESSIONINFO_REVISION_V1) {
			if (info->v1.serverAddr[0]) {
				serverAddr = info->v1.serverAddr;
			}
		}
		else {
			serverAddr = NULL;
		}
	}
	if (serverAddr == NULL || IpfIpc_ParseServerAddress(serverAddr, &sockaddr) != ESIF_OK) {
		return IPC_INVALID_SESSION;
	}

	// Create Session and Session Handle
	IpcSession *self = esif_ccb_malloc(sizeof(*self));
	IpcSession_t handle = IpcSessionMgr_CreateHandle(self);
	if (handle == IPC_INVALID_SESSION) {
		esif_ccb_free(self);
		self = NULL;
	}
	if (self) {
		self->objtype = ObjType_IpcSession;
		esif_ccb_strcpy(self->serverAddr, serverAddr, sizeof(self->serverAddr));
		self->sockaddr = sockaddr;
		self->socket = INVALID_SOCKET;
		self->doorbell[0] = self->doorbell[1] = INVALID_SOCKET;
		signal_init(&self->rpcSignal);
		atomic_set(&self->numThreads, 0);
		esif_ccb_wthread_init(&self->ioThread);
		esif_ccb_wthread_init(&self->rpcThread);
	}
	return handle;
}

// Connect WebSocket Client to Server and Start Worker Threads
esif_error_t IpcSession_Connect(IpcSession_t handle)
{
	IpcSession *self = IpcSessionMgr_GetSessionByHandle(handle);
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// If Session is already Connected, return an error and leave Connection open
	if (self && self->appSession.ifaceSet.appIface.fAppCreateFuncPtr) {
		return ESIF_E_SESSION_ALREADY_STARTED;
	}

	if (self && g_ipcAppIfaceSet.appIface.fAppCreateFuncPtr) {
		self->appSession.ifaceSet = g_ipcAppIfaceSet;

		// Allocate Buffers
		self->recvQueue = MessageQueue_Create();
		self->sendQueue = MessageQueue_Create();
		self->recvBuf = esif_ccb_malloc(RECV_BUF_SIZE);
		if (self->recvQueue == NULL || self->sendQueue == NULL || self->recvBuf == NULL) {
			PERRORMSG("malloc");
			return ESIF_E_NO_MEMORY;
		}
		self->recvBufLen = 0;
		self->maxRecvBuf = RECV_BUF_SIZE;;

		// Connect to given IP/Port and create TCP Doorbell
		self->socket = socket(self->sockaddr.type, SOCK_STREAM, IPPROTO_IP);
		if (INVALID_SOCKET == self->socket || esif_ccb_socketpair(ESIF_PF_LOCAL, SOCK_STREAM, IPPROTO_IP, self->doorbell) != 0) {
			PERRORMSG("socket");
			rc = ESIF_E_WS_SOCKET_ERROR;
		}
		else {
			rc = ESIF_OK;
			char request_key[BASE64_ENCODED_LENGTH(sizeof(esif_guid_t))] = "dGhlIHNhbXBsZSBub25jZQ==";

			esif_guid_t raw_key = { 0 };
			if (esif_ccb_random(&raw_key, sizeof(raw_key)) == ESIF_OK) {
				esif_base64_encode(request_key, sizeof(request_key), raw_key, sizeof(raw_key));
			}

			/* TODO: Authentication Client before AppCreate
			** Use Sec-WebSocket-Key and Include BitFlag(s) and Metadata in Handshake
			*/
			int result = -1;
			const char upgradeHeader[] =
				"GET / HTTP/1.1\r\n"
				"User-Agent: IpfClient/1.0\r\n"
				"Host: %s\r\n"
				"Origin: http://%s\r\n"
				"Upgrade: websocket\r\n"
				"Sec-WebSocket-Key: %s\r\n"
				"Sec-WebSocket-Version: 13\r\n"
				"Connection: Upgrade\r\n"
				"\r\n";
			const char upgradeResponseHeader[] = "HTTP/1.1 101 Switching Protocols";

			DEBUGMSG("Connecting to %s\n", self->serverAddr);

			// Connect to server after verifying existence of Named Socket File for Unix Domain Sockets
			int sterr = 0;
			if (self->sockaddr.type != AF_UNIX || (sterr = esif_socket_file_stat(self->sockaddr.un_addr.sun_path)) == ESIF_SOCKERR_OK) {
				result = connect(self->socket, (struct sockaddr *)&self->sockaddr.un_addr, IpfIpc_SockaddrLen(self->sockaddr.type));
			}
			else {
				esif_ccb_socket_seterror(sterr);
			}

			// Convert OS error to esif_error_t
			if (-1 == result) {
				PERRORMSG("connect");
				int errnum = esif_ccb_socket_error();
				switch (errnum) {
				case ESIF_SOCKERR_EACCES:
					rc = ESIF_E_WS_UNAUTHORIZED;
					break;
				case ESIF_SOCKERR_ENOENT:
					rc = ESIF_E_WS_INVALID_ADDR;
					break;
				default:
					rc = ESIF_E_WS_SOCKET_ERROR;
					break;
				}
				return rc;
			}

			// Send websocket upgrade request, using Server Address (minus protocol) as the Host/Origin
			char scheme[] = IPC_PROTO_SCHEME;
			const char *srvhost = esif_ccb_strstr(self->serverAddr, scheme);
			ssize_t ret = 0;

			srvhost = (srvhost ? srvhost + sizeof(scheme) - 1 : self->serverAddr);
			esif_ccb_sprintf(self->maxRecvBuf, self->recvBuf,
				upgradeHeader,
				srvhost,
				srvhost,
				request_key
			);
			ret = send(self->socket, self->recvBuf, (int)esif_ccb_strlen(self->recvBuf, self->maxRecvBuf), 0);

			if (ret == SOCKET_ERROR) {
				DEBUGMSG("send error = %d\n", esif_ccb_socket_error());
			}

			esif_ccb_memset(self->recvBuf, 0, self->maxRecvBuf);

			// Generate expected Response Key
			esif_sha1_t sha_digest;
			char response_key[BASE64_ENCODED_LENGTH(sizeof(sha_digest.hash))] = { 0 };
			char guid_key[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

			esif_sha1_init(&sha_digest);
			esif_sha1_update(&sha_digest, request_key, esif_ccb_strlen(request_key, sizeof(request_key)));
			esif_sha1_update(&sha_digest, guid_key, sizeof(guid_key) - 1);
			esif_sha1_finish(&sha_digest);
			esif_base64_encode(response_key, sizeof(response_key), sha_digest.hash, sha_digest.hashsize);
			
			const char sec_websocket_accept[] = "Sec-WebSocket-Accept: ";
			char websocket_reply[sizeof(sec_websocket_accept) + sizeof(response_key) + 2] = { 0 };
			esif_ccb_sprintf(sizeof(websocket_reply), websocket_reply, "%s%s\r\n", sec_websocket_accept, response_key);
			
			// Verify the websocket upgrade response
			result = recv(self->socket, self->recvBuf, (int)self->maxRecvBuf, 0);
			if ((result > 0) &&
				(esif_ccb_strncmp(self->recvBuf, upgradeResponseHeader, sizeof(upgradeResponseHeader) - 1) == 0) &&
				(esif_ccb_strstr(self->recvBuf, websocket_reply) != NULL) &&
				(esif_ccb_strstr(self->recvBuf, "\r\n\r\n") != NULL)) {

				self->bytesReceived = 0;
				esif_ccb_memset(self->recvBuf, 0, self->maxRecvBuf);
				DEBUGMSG("Connected to Websocket Server\n");
				
				// TODO: Optional Challenge/Response Authentication using Binary WebSocket Frames
			}
			else {
				DEBUGMSG("Failed websocket handshake\n");
				rc = ESIF_E_WS_INIT_FAILED;
			}
		}
	}

	// Start I/O Worker Thread to handle all Network I/O and Send & Receive Websocket messages
	if (rc == ESIF_OK) {
		rc = esif_ccb_wthread_create(&self->ioThread, IpcSession_IoWorkerThread, self);
		if (rc == ESIF_OK) {
			atomic_inc(&self->numThreads);
		}
	}

	// Start RPC Worker Thread to process incoming RPC Requests and Responses from Remote Server
	if (rc == ESIF_OK) {
		rc = esif_ccb_wthread_create(&self->rpcThread, IpcSession_RpcWorkerThread, self);
		if (rc == ESIF_OK) {
			atomic_inc(&self->numThreads);
		}
	}
	return rc;
}

// Close WebSocket Client
void IpcSession_Disconnect(IpcSession_t handle)
{
	IpcSession *self = IpcSessionMgr_GetSessionByHandle(handle);
	if (self) {
		// Stop I/O Worker Thread
		if (atomic_read(&self->numThreads) && self->doorbell[DOORBELL_BUTTON] != INVALID_SOCKET) {
			u8 opcode = WS_OPCODE_QUIT;
			ssize_t ret = 0;

			ret = send(self->doorbell[DOORBELL_BUTTON], (const char *)&opcode, sizeof(opcode), 0);
			if (ret == SOCKET_ERROR) {
				DEBUGMSG("send error = %d\n", esif_ccb_socket_error());
			}
			esif_ccb_wthread_join(&self->ioThread);
			atomic_dec(&self->numThreads);
		}

		if (self->socket != INVALID_SOCKET) {
			esif_ccb_socket_close(self->socket);
			self->socket = INVALID_SOCKET;
		}
		if (self->doorbell[0] != INVALID_SOCKET) {
			esif_ccb_socket_close(self->doorbell[0]);
			self->doorbell[0] = INVALID_SOCKET;
		}
		if (self->doorbell[1] != INVALID_SOCKET) {
			esif_ccb_socket_close(self->doorbell[1]);
			self->doorbell[1] = INVALID_SOCKET;
		}
		MessageQueue_Deactivate(self->recvQueue);
		MessageQueue_Deactivate(self->sendQueue);
		esif_ccb_free(self->recvBuf);
		self->recvBuf = NULL;
		self->recvBufLen = 0;
		self->maxRecvBuf = 0;
		IpfTrxMgr_ExpireAll();

		// Stop RPC Worker Thread
		if (atomic_read(&self->numThreads)) {
			signal_post(&self->rpcSignal);
			esif_ccb_wthread_join(&self->rpcThread);
			atomic_dec(&self->numThreads);
		}
		esif_ccb_memset(&self->appSession.ifaceSet, 0, sizeof(self->appSession.ifaceSet));

		// Reset RPC Semaphore to clear any pending signals since we know no other threads are using it
		signal_uninit(&self->rpcSignal);
		signal_init(&self->rpcSignal);

		// Destroy Queues only after all threads complete to avoid race conditions
		MessageQueue_Destroy(self->sendQueue);
		MessageQueue_Destroy(self->recvQueue);
		self->sendQueue = NULL;
		self->recvQueue = NULL;
	}
}

// Destroy WebSocket Client
void IpcSession_Destroy(IpcSession_t handle)
{
	IpcSession *self = IpcSessionMgr_GetSessionByHandle(handle);
	if (self) {
		IpcSession_Disconnect(handle);
		signal_uninit(&self->rpcSignal);
		esif_ccb_wthread_uninit(&self->ioThread);
		esif_ccb_wthread_uninit(&self->rpcThread);
		esif_ccb_free(self);
	}
	IpcSessionMgr_DeleteHandle(handle);
}

// Process all IPC messages until Connection Closed (by Server or TCP doorbell)
void IpcSession_WaitForStop(IpcSession_t handle)
{
	IpcSession *self = IpcSessionMgr_GetSessionByHandle(handle);
	if (self) {
		esif_ccb_wthread_join(&self->ioThread);
	}
}

// I/O Worker Thread to handle all Socket I/O with Remote Server
void * ESIF_CALLCONV IpcSession_IoWorkerThread(void *ctx)
{
	IpcSession *self = (IpcSession *)ctx;
	IPFDEBUG("Started I/O Worker Thread (%p)\n", self);
	if (self && self->objtype == ObjType_IpcSession) {
		esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
		FrameType frameType;
		char *data = NULL;
		size_t dataSize = 0;
		size_t bytesRemaining = 0;
		FrameType fragmentType = INCOMPLETE_FRAME;
		size_t fragmentBufSize = 0;
		char *fragmentBuf = NULL;
		Bool multiFragment = ESIF_FALSE;
		char *messageBuf = NULL;
		size_t messageSize = 0;

		// select() parameters
		struct timeval tv = { 0 };	// Timeout
		fd_set readFDs = { 0 };		// Readable Sockets List
		fd_set writeFDs = { 0 };	// Writable Sockets List
		fd_set exceptFDs = { 0 };	// Exception Sockets List
		int selectResult = 0;		// select() result
		int maxfd = 0;				// Max file descriptor ID + 1
		int setsize = 0;			// Number of items in FD List

		// Receive bytes from TCP stream and buffer until a complete websocket message is available or disconnected
		do {
			if (self->recvBufLen >= self->maxRecvBuf) {
				char *newRecvBuf = esif_ccb_realloc(self->recvBuf, self->maxRecvBuf + RECV_BUF_GROWBY);
				if (newRecvBuf == NULL) {
					PERRORMSG("realloc");
					break;
				}
				esif_ccb_memset(newRecvBuf + self->maxRecvBuf, 0, RECV_BUF_GROWBY);
				self->recvBuf = newRecvBuf;
				self->maxRecvBuf += RECV_BUF_GROWBY;
			}
			esif_ccb_memset(self->recvBuf + self->recvBufLen, 0, self->maxRecvBuf - self->recvBufLen);

			// Wait for Activity on Client or Doorbell Sockets
			FD_ZERO(&readFDs);
			FD_ZERO(&writeFDs);
			FD_ZERO(&exceptFDs);
			maxfd = 0;
			setsize = 0;

			// Add Doorbell Ringer
			FD_SET(self->doorbell[DOORBELL_RINGER], &readFDs);
			FD_SET(self->doorbell[DOORBELL_RINGER], &exceptFDs);
			maxfd = (int)self->doorbell[DOORBELL_RINGER] + 1;
			setsize++;

			// Add Client Socket(s)
			FD_SET(self->socket, &readFDs);
			FD_SET(self->socket, &exceptFDs);
			maxfd = esif_ccb_max(maxfd, (int)self->socket + 1);
			setsize++;

			// Use Lowest Remaining Transaction Timeout for all Active Connections, if any
			double timeout = IpfTrxMgr_GetMinTimeout();
			tv.tv_sec = (long)timeout;
			tv.tv_usec = (long)((timeout - (Int64)timeout) * 1000000);
			if (tv.tv_sec == 0 && tv.tv_usec == 0) {
				tv.tv_sec = 30;
			}

			//// WAIT FOR SOCKET ACTIVITY ////

			selectResult = select(maxfd, &readFDs, &writeFDs, &exceptFDs, &tv);

			if (selectResult == SOCKET_ERROR) {
				self->bytesReceived = 0;
				rc = ESIF_E_WS_SOCKET_ERROR;
			}
			else if (selectResult == 0) { // Timeout
				IpfTrxMgr_ExpireInactive();
				continue;
			}

			//// PROCESS ALL ACTIVE SOCKETS ////

			// 1. Respond to Incoming Doorbell Socket signals
			if (FD_ISSET(self->doorbell[DOORBELL_RINGER], &readFDs)) {
				u8 opcode = WS_OPCODE_NOOP;
				ssize_t msglen = recv(self->doorbell[DOORBELL_RINGER], (char *)&opcode, sizeof(opcode), 0);
				if (msglen == 0 || msglen == SOCKET_ERROR || msglen < sizeof(opcode)) {
					opcode = WS_OPCODE_NOOP;
					rc = ESIF_E_WS_SOCKET_ERROR;
					break;
				}
				if (opcode == WS_OPCODE_QUIT) {
					rc = ESIF_OK;
					break;
				}
			}

			// 2. Process Pending Outgoing Messages
			IBinary *blob = NULL;
			while ((blob = MessageQueue_DeQueue(self->sendQueue)) != NULL) {
				rc = IpcSession_SendMsg(self, IBinary_GetBuf(blob), IBinary_GetLen(blob));
				IBinary_Destroy(blob);
				if (rc != ESIF_OK) {
					DEBUGMSG("Connection Closed\n");
					break;
				}
			}

			// 3. Process Active Client Requests
			// Close sockets with Exceptions
			if (FD_ISSET(self->socket, &exceptFDs)) {
				rc = ESIF_E_WS_DISC;
				break;
			}

			// Blocking Receive
			if (FD_ISSET(self->socket, &readFDs)) {
				ESIF_SERIAL_FENCE();
				self->bytesReceived = recv(self->socket, self->recvBuf + self->recvBufLen, (int)(self->maxRecvBuf - self->recvBufLen), 0);
				IPFDEBUG("Received: %zd\n", self->bytesReceived);
			}
			else {
				continue;
			}

			if (self->bytesReceived == 0) {
				DEBUGMSG("Connection Closed\n");
				break;
			}
			else if (self->bytesReceived == (size_t)SOCKET_ERROR) {
				PERRORMSG("recv");
				break;
			}

			// Process Received Data
			char *nextBuf = self->recvBuf;
			self->bytesReceived += (int)self->recvBufLen;
			do {
				WsSocketFrameHeaderPtr wsHeaderPtr = (WsSocketFrameHeaderPtr)nextBuf;
				self->recvBufLen = self->bytesReceived - (nextBuf - self->recvBuf);

				frameType = esif_ws_get_frame_type(
					(WsSocketFramePtr)nextBuf,
					self->recvBufLen,
					&data,
					&dataSize,
					&bytesRemaining
				);

				if (frameType == ERROR_FRAME) {
					DEBUGMSG("Error Parsing WebSocket Frame\n");
					self->bytesReceived = 0;
					break;
				}

				if (frameType == INCOMPLETE_FRAME) {
					esif_ccb_memmove(self->recvBuf, nextBuf, self->recvBufLen);
					break;
				}

				if (wsHeaderPtr->s.maskFlag) {
					DEBUGMSG("This sample app does not support masked payload, skip to next frame\n");
					goto next_frame;
				}

				// Support Multi-Fragment Messages
				multiFragment = ESIF_FALSE;
				messageBuf = data;
				messageSize = dataSize;
				if (wsHeaderPtr->s.fin == 0 || CONTINUATION_FRAME == frameType) {
					char *newFragmentBuf = esif_ccb_realloc(fragmentBuf, fragmentBufSize + dataSize);
					if (newFragmentBuf == NULL) {
						PERRORMSG("realloc");
						break;
					}
					esif_ccb_memcpy(newFragmentBuf + fragmentBufSize, data, dataSize);
					fragmentBuf = newFragmentBuf;
					fragmentBufSize += dataSize;
					if (wsHeaderPtr->s.fin) {
						frameType = fragmentType;
						messageBuf = fragmentBuf;
						messageSize = fragmentBufSize;
						multiFragment = ESIF_TRUE;
					}
					else {
						if (frameType != CONTINUATION_FRAME) {
							fragmentType = frameType;
						}
						goto next_frame;
					}
				}

				// Process Frame
				switch (frameType) {
				case BINARY_FRAME:			// Process a Complete WebSocket Message
					rc = IpcSession_ReceiveMsg(self, messageBuf, messageSize);
					break;
				case CLOSING_FRAME:			// Close Connection
					rc = ESIF_E_WS_DISC;
					break;
				case PING_FRAME:			// Pong Reponse
					rc = IpcSession_PongResponse(self, messageBuf, messageSize);
					break;
				case PONG_FRAME:			// Do Nothing
				case CONTINUATION_FRAME:	// Wait for next Fragment
				case TEXT_FRAME:			// Not Currently Supported
				default:					// Ignore all other FrameTypes
					rc = ESIF_OK;
					break;
				}

				// Close Connection on any Error
				if (rc != ESIF_OK) {
					self->bytesReceived = 0;
					break;
				}

			next_frame:
				self->recvBufLen = 0;
				if (bytesRemaining > 0) { // Move to the next frame
					nextBuf = data + dataSize;
					// Sanity check to nextBuf pointer
					if (nextBuf >= (self->recvBuf + self->bytesReceived)) {
						break;
					}
				}

				// Free Multi-Fragment Message Buffer
				if (multiFragment) {
					esif_ccb_free(fragmentBuf);
					fragmentBuf = NULL;
					fragmentBufSize = 0;
					fragmentType = INCOMPLETE_FRAME;
					multiFragment = ESIF_FALSE;
				}

			} while (bytesRemaining > 0);

			// 4. Expire Inactive Transactions
			IpfTrxMgr_ExpireInactive();
		} while (self->bytesReceived > 0 || selectResult == 0);

		if (self->socket != INVALID_SOCKET) {
			esif_ccb_socket_close(self->socket);
			self->socket = INVALID_SOCKET;
		}

		// Signal RPC Thread to Exit
		signal_post(&self->rpcSignal);
	}
	IPFDEBUG("Exiting I/O Worker Thread\n");
	return 0;
}

esif_error_t IpcSession_SendMsg(
	IpcSession *self,
	const char *messageBuf,
	size_t messageLen
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	UInt16 mask_flag = 1; // Masking of Client Frames Required per RFC 6455
	UInt32 mask_key = 0;

	if (messageBuf && messageLen && (!mask_flag || ((rc = esif_ccb_random(&mask_key, sizeof(mask_key))) == ESIF_OK))) {
		WsSocketFrame header = { 0 };
		size_t header_size = sizeof(header.header);
		
		rc = ESIF_OK;
		header.header.s.maskFlag = mask_flag;
		if (messageLen <= WSFRAME_HEADER_TYPE1_MAX) {
			header.header.s.payLen = (UInt8)messageLen;
			if (mask_flag) {
				header_size += sizeof(header.u.T1);
				header.u.T1.maskKey = mask_key;
			}
		}
		else if (messageLen <= WSFRAME_HEADER_TYPE2_MAX) {
			header.header.s.payLen = WSFRAME_HEADER_TYPE2;
			header_size += sizeof(header.u.T2_NoMask);
			header.u.T2_NoMask.extPayLen = esif_ccb_htons((UInt16)messageLen);
			if (mask_flag) {
				header_size += sizeof(header.u.T2) - sizeof(header.u.T2_NoMask);
				header.u.T2.maskKey = mask_key;
			}
		}
		else if (messageLen <= WSFRAME_HEADER_TYPE3_MAX) {
			header.header.s.payLen = WSFRAME_HEADER_TYPE3;
			header_size += sizeof(header.u.T3_NoMask);
			header.u.T3_NoMask.extPayLen = esif_ccb_htonll((UInt64)messageLen);
			if (mask_flag) {
				header_size += sizeof(header.u.T3) - sizeof(header.u.T3_NoMask);
				header.u.T3.maskKey = mask_key;
			}
		}
		else {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		}

		if (rc == ESIF_OK) {
			size_t frame_len = header_size + messageLen;
			UInt8 *frame_buf = (UInt8 *)esif_ccb_malloc(frame_len);
			if (frame_buf == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				int bytes_sent = 0;
				header.header.s.opcode = BINARY_FRAME;
				header.header.s.fin = 1;
				esif_ccb_memcpy(frame_buf, &header, header_size);
				esif_ccb_memcpy(frame_buf + header_size, messageBuf, messageLen);

				if (mask_flag) {
					for (size_t j = 0; j < messageLen; j++) {
						frame_buf[header_size + j] = frame_buf[header_size + j] ^ ((UInt8*)&mask_key)[j % 4];
					}
				}
				while ((bytes_sent = send(self->socket, (const char *)frame_buf + bytes_sent, (int)frame_len, 0)) >= 0 && bytes_sent < (int)frame_len) {
					frame_len -= bytes_sent;
				}
			}
			esif_ccb_free(frame_buf);
		}
	}
	return rc;
}

/*
** MessageQueue class
*/

// Create a new Queue
MessageQueuePtr MessageQueue_Create(void)
{
	MessageQueuePtr self = esif_ccb_malloc(sizeof(*self));
	if (self) {
		atomic_set(&self->isActive, 1);
		esif_ccb_lock_init(&self->lock);
		self->queue = esif_link_list_create();
		if (self->queue == NULL) {
			MessageQueue_Destroy(self);
			self = NULL;
		}
	}
	return self;
}

void MessageQueue_DestroyData(void *data)
{
	IBinary_Destroy((IBinary *)data);
	return;
}

// Destroy a Queue, freeing its contents
void MessageQueue_Destroy(MessageQueuePtr self)
{
	if (self) {
		esif_ccb_lock_uninit(&self->lock);
		if (self->queue) {
			esif_link_list_free_data_and_destroy(self->queue, MessageQueue_DestroyData);
			self->queue = NULL;
		}
		esif_ccb_free(self);
	}
}

// Add a message to the Queue. blob is a dynamic buffer now owned by queue
esif_error_t MessageQueue_EnQueue(MessageQueuePtr self, IBinary *blob)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && blob) {
		esif_ccb_write_lock(&self->lock);
		if (atomic_read(&self->isActive)) {
			rc = esif_link_list_add_at_back(self->queue, blob);
		}
		else {
			// Silently drop the message if Queue is inactive
			IPF_TRACE_DEBUG("Message queue inactive\n");
			IBinary_Destroy(blob);
			rc = ESIF_OK;
		}
		esif_ccb_write_unlock(&self->lock);
	}
	return rc;
}

// Remove the next Message from the Queue or NULL. Caller is responsible for freeing result
IBinary *MessageQueue_DeQueue(MessageQueuePtr self)
{
	IBinary *blob = NULL;
	if (self) {
		esif_ccb_write_lock(&self->lock);
		if (atomic_read(&self->isActive) && self->queue && self->queue->head_ptr) {
			blob = (IBinary *)self->queue->head_ptr->data_ptr;
			esif_link_list_node_remove(self->queue, self->queue->head_ptr);
		}
		esif_ccb_write_unlock(&self->lock);
	}
	return blob;
}

// Deactivate a Queue without actually destroying the queue object or its contents
void MessageQueue_Deactivate(MessageQueuePtr self)
{
	if (self) {
		atomic_set(&self->isActive, 0);
	}
}
