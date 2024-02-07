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

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <assert.h>

#include "esif_sdk.h"
#include "esif_ccb_rc.h"
#include "esif_ccb_socket.h"
#include "esif_ccb_thread.h"
#include "esif_link_list.h"

#include "ipf_ipc_codec.h"
#include "ipf_ipc_trxmgr.h"
#include "ipf_ipc_iface.h"
#include "ipf_ipc_clisrv.h"

#define WS_HEADER_BUF_LEN	128		/* Buffer size for Websocket Header in REST responses */

#define WS_BUFFER_LENGTH (OUT_BUF_LEN + WS_HEADER_BUF_LEN)

#define WS_PROT_KEY_SIZE_MAX 1000
#define WS_FRAME_SIZE_TYPE_1	125 /* For payloads <= 125 bytes*/
#define WS_FRAME_SIZE_TYPE_2	126 /* For payload up to 64K - 1 in size */
#define WS_FRAME_SIZE_TYPE_3	127 /* For payloads > 64K - 1*/

#define WS_FRAME_SIZE_TYPE_1_MAX_PAYLOAD	125		/* Maximum payload size for a Type 1 frame size */
#define WS_FRAME_SIZE_TYPE_2_MAX_PAYLOAD	0xFFFF	/* Maximum payload size for a Type 2 frame size */

#define WS_FRAME_MAXIMUM_SIZE 0x7FFFFFFF	/* Maximum Supported Frame Size */

typedef enum FrameType_e {
	CONTINUATION_FRAME	= 0x00,
	TEXT_FRAME			= 0x01,
	BINARY_FRAME		= 0x02,
	CLOSING_FRAME		= 0x08,
	PING_FRAME			= 0x09,
	PONG_FRAME			= 0x0A,

	EMPTY_FRAME			= 0xF0,
	ERROR_FRAME			= 0xF1,
	INCOMPLETE_FRAME	= 0xF2,
	OPENING_FRAME		= 0xF3,
	HTTP_FRAME			= 0xFF
} FrameType, *FrameTypePtr;

#pragma pack(push, 1)

#pragma warning(disable:4214)
typedef union WsSocketFrameHeader_u {

	/* Header fields used by all frame types */
	struct {
		UInt16 opcode:4;	/* 3:0 - Frame type opcode */
		UInt16 rsvd1:3;		/* 7:4 */
		UInt16 fin:1;		/* 8 - Final fragment flag */
		UInt16 payLen:7;	/* 14:9 - Payload length/type */
		UInt16 maskFlag:1;  /* 15 - Mask flag - Always set for client to server */
	} s;

	UInt16  asU16;
} WsSocketFrameHeader, *WsSocketFrameHeaderPtr;

#pragma warning(disable:4200)
typedef struct WsSocketFrame_s {

	WsSocketFrameHeader header;

	union {
		struct {
			char data[0];
		} T1_NoMask;
		
		struct {
			UInt32 maskKey;
			char data[0];
		} T1;

		struct {
			UInt16 extPayLen;
			char data[0];
		} T2_NoMask;
		
		struct {
			UInt16 extPayLen;
			UInt32 maskKey;
			char data[0];
		} T2;

		struct {
			UInt64 extPayLen;
			char data[0];
		} T3_NoMask;

		struct {
			UInt64 extPayLen;
			UInt32 maskKey;
			char data[0];
		} T3;

	} u;
	
} WsSocketFrame, *WsSocketFramePtr;
#pragma pack(pop)

/*
** Public Functions
*/

typedef struct MessageQueue_s {
	esif_ccb_lock_t			lock;
	atomic_t				isActive;
	struct esif_link_list	*queue;
} MessageQueue, *MessageQueuePtr;

MessageQueuePtr MessageQueue_Create(void);
void MessageQueue_Destroy(MessageQueuePtr self);
esif_error_t MessageQueue_EnQueue(MessageQueuePtr self, IBinary *blob);
IBinary *MessageQueue_DeQueue(MessageQueuePtr self);
void MessageQueue_Deactivate(MessageQueuePtr self);

#define	DOORBELL_BUTTON		0
#define DOORBELL_RINGER		1
#define WS_OPCODE_NOOP		0x00
#define WS_OPCODE_MESSAGE	0x01
#define WS_OPCODE_QUIT		0xFF

#ifdef ESIF_ATTR_DEBUG
#define DEBUGMSG(msg, ...)		(void)(0)
//#define DEBUGMSG(msg, ...)	do { printf(msg, ##__VA_ARGS__); fflush(stdout); } while (0)
#define PERRORMSG(msg)			do { perror(msg); } while (0)
#else
#define DEBUGMSG(msg, ...)		(void)(0)
#define PERRORMSG(msg)			(void)(0)
#endif

// AppState Values
#define APPSTATE_STOPPED	0	// AppCreate not called yet, currently in flight, or AppDestroy completed
#define APPSTATE_STOPPING	1	// AppDestroy currently in flight
#define APPSTATE_STARTED	2	// AppCreate completed successfully

typedef struct IpcSession_s {
	UInt32				objtype;		// ObjType_IpcSession
	char				serverAddr[MAX_PATH];
	esif_ccb_sockaddr_t	sockaddr;
	esif_ccb_socket_t	socket;
	esif_ccb_socket_t	doorbell[2];
	char				*recvBuf;
	size_t				recvBufLen;
	size_t				maxRecvBuf;
	size_t				bytesReceived;

	AppSession			appSession;		// IPC Client/Server AppSession
	atomic_t			appState;		// IPC Client State
	signal_t			appSignal;		// IPC Client Semaphore to signal waiting RPC AppDestroy

	MessageQueuePtr		recvQueue;		// RPC Request Queue  (Received from Remote Server)
	MessageQueuePtr		sendQueue;		// RPC Response Queue (Send to Remote Server)

	atomic_t			numThreads;		// Active Worker Threads
	esif_wthread_t		ioThread;		// I/O Worker Thread to handle all Network Socket I/O
	esif_wthread_t		rpcThread;		// RPC Worker Thread to Decode and Process incoming RPC Requests in recvQueue
	signal_t			rpcSignal;		// RPC Semaphore to signal RPC Worker Thread that there are pending messages

	IpfTrxMgr			trxMgr;			// Session Transaction Manager
} IpcSession;

/* Public Methods */

// Initialize
esif_error_t Ipc_Init(void);
void Ipc_Exit(void);

// Set App Interface Function Pointers for use by IPC Session
esif_error_t Ipc_SetAppIface(const AppInterfaceSet *ifaceSet);

// Create/Destroy Session with optional Configuration
IpcSession_t IpcSession_Create(const IpcSessionInfo *info);
void IpcSession_Destroy(IpcSession_t self);

// Connect/Disconnect (Start/Stop) Session on separate Threads
esif_error_t IpcSession_Connect(IpcSession_t self);
void IpcSession_Disconnect(IpcSession_t self);

// Wait for Session I/O Thread to Exit
void IpcSession_WaitForStop(IpcSession_t self);

/* Private Methods */

// Send/Receive an IRPC Message
esif_error_t IpcSession_SendMsg(IpcSession *self, const char *messageBuf, size_t messageLen);
esif_error_t IpcSession_ReceiveMsg(IpcSession *self, const char *messageBuf, size_t messageLen);

// Worker Thread Functions
void * ESIF_CALLCONV IpcSession_IoWorkerThread(void *ctx);
void * ESIF_CALLCONV IpcSession_RpcWorkerThread(void *ctx);

// IPC Session Manager Public Functions
IpcSession *IpcSessionMgr_GetSessionByHandle(IpcSession_t handle);
IpcSession *IpcSessionMgr_GetSessionByEsifHandle(esif_handle_t handle);
