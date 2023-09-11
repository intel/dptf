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

#pragma once

#include "esif_sdk.h"
#include "esif_link_list.h"
#include "esif_ccb_lock.h"
#include "esif_ccb_socket.h"
#include "esif_ccb_thread.h"
#include "ipfsrv_authmgr.h"
#include "ipf_ipc_codec.h"
#include "ipf_ipc_clisrv.h"
#include "ipf_trace.h"

// Client Types
typedef enum ClientType_e {
	ClientClosed = 0,		// Closed Connection
	ClientHttp,				// HTTP Client
	ClientWebsocket,		// Websocket Client
} ClientType;

// WebSocket Frame Type
typedef enum FrameType_e {
	FRAME_NULL = 0xFF,
	FRAME_CONTINUATION = 0x00,
	FRAME_TEXT = 0x01,
	FRAME_BINARY = 0x02,
	FRAME_CLOSING = 0x08,
	FRAME_PING = 0x09,
	FRAME_PONG = 0x0A,
} FrameType, *FrameTypePtr;

// WebSocket FIN Flag Types
typedef enum FinType_e {
	FIN_FRAGMENT = 0,
	FIN_FINAL = 1
} FinType, *FinTypePtr;

// TCP Doorbell Object
#define DOORBELL_SOCKETS	2	// Pair of Sockets used to signal blocked select()
#define DOORBELL_RINGER		0	// Ringer   = Socket 0 [recv]
#define DOORBELL_BUTTON		1	// Doorbell = Socket 1 [send]

typedef struct TcpDoorbell_s {
	Bool				isActive;					// Paired Sockets are Active?
	esif_ccb_socket_t	sockets[DOORBELL_SOCKETS];	// Paired TCP Sockets used to signal blocked select()
} TcpDoorbell, *TcpDoorbellPtr;

// Binary Message Object Queue
typedef struct MessageQueue_s {
	esif_ccb_lock_t				lock;
	atomic_t					refcount;
	struct esif_link_list		*queue;
	link_list_data_destroy_func	destructor;
} MessageQueue, *MessageQueuePtr;

// Web Server Listener Object
typedef struct WebListener_s {
	char				serverAddr[MAX_PATH];	// Listener Address URL
	esif_ccb_sockaddr_t	sockaddr;				// Listener Socket Opaque Address
	esif_ccb_socket_t	socket;					// Listener Socket or INVALID_SOCKET
	esif_handle_t		authHandle;				// Authentication Role Handle
	const char			*accessControl;			// Optional OS-Specific Access Control List
} WebListener, *WebListenerPtr;

// Web Server RPC Worker Object
typedef struct WebWorker_s {
	atomic_t			exitFlag;		// Exit Thread Flag
	atomic_t			refCount;		// Reference Count
	atomic_t			queueSize;		// Incoming RPC Request Queue Size
	MessageQueuePtr		rpcQueue;		// Incoming RPC Request Queue
	esif_thread_t		rpcThread;		// RPC Service Thread
	signal_t			rpcSignal;		// RPC Thread Signal
} WebWorker, *WebWorkerPtr;

// Web Server Client Object
typedef struct WebClient_s {
	ClientType			type;			// Client Type (Closed, Http, Websocket)
	esif_ccb_socket_t	socket;			// Client Socket Handle or INVALID_SOCKET
	esif_ccb_sockaddr_t	sockaddr;		// Client Socket Address
	esif_handle_t		authHandle;		// Client Authentication Role Handle

	u8					*sendBuf;		// TCP/IP Send Buffer
	size_t				sendBufLen;		// TCP/IP Send Buffer Length
	u8					*recvBuf;		// TCP/IP Receive Buffer (Partial HTTP Request or Websocket Frame)
	size_t				recvBufLen;		// TCP/IP Receive Buffer Length
	
	char				*httpBuf;		// HTTP Request Buffer
	char				**httpRequest;	// HTTP Request Headers Array (NULL-terminated)
	
	FrameType			msgType;		// Websocket Message Type
	u8					*fragBuf;		// Websocket Multi-Fragment Buffer
	size_t				fragBufLen;		// Websocket Multi-Fragment Buffer Length

	esif_handle_t		ipfHandle;		// Unique IPF Client Session Handle
	WebWorkerPtr		rpcWorker;		// RPC Worker Object
} WebClient, *WebClientPtr;

#define IPF_WS_MAX_LISTENERS	12					// Max Number of Web Server Listeners
#define IPF_WS_MAX_CLIENTS		ESIF_MAX_CLIENTS	// Max simultaneous websocket clients

// Max Active Sockets (per WebServer) cannot exceed FD_SETSIZE (Default: Windows=64, Linux=1024)
#if ((IPF_WS_MAX_LISTENERS + IPF_WS_MAX_CLIENTS + 1) > FD_SETSIZE)
# undef  IPF_WS_MAX_CLIENTS
# define IPF_WS_MAX_CLIENTS	(FD_SETSIZE - IPF_WS_MAX_LISTENERS - 1)
#endif
#define WS_MAX_SOCKETS	(IPF_WS_MAX_LISTENERS + IPF_WS_MAX_CLIENTS + 1)

#define CRLF	"\r\n"

// Web Server Object (One per Worker Thread)
typedef struct WebServer_s {
	esif_ccb_lock_t		lock;							// Thread Lock

	TcpDoorbell			doorbell;						// Doorbell Intra-Process Signal object
	WebListener			listeners[IPF_WS_MAX_LISTENERS];// Web Server Listener Socket(s)
	WebClient			clients[IPF_WS_MAX_CLIENTS];	// Web Clients

	esif_thread_t		mainThread;						// Web Server Main Worker Thread
	esif_thread_t		fileThread;						// FileWatcher Notify Thread
	signal_t			*fileDoorbell;					// FileWatcher Doorbell Signal
	atomic_t			isActive;						// Web Server Active Flag
	atomic_t			isPaused;						// Web Server Pause Flag
	atomic_t			isDiagnostic;					// Web Server Diagnostic Mode Flag
	atomic_t			activeThreads;					// Active Thread Count
	atomic_t			listenerMask;					// Active Listeners BitMask

	u8					*netBuf;						// Network Send/Receive Buffer
	size_t				netBufLen;						// Network Send/Receive Buffer Length

	MessageQueuePtr		msgQueue;						// Outgoing RPC Request/Response Queue
	atomic_t			rpcQueueMax;					// Max Incoming RPC Request Queue Size
} WebServer, *WebServerPtr;

extern WebServerPtr g_WebServer;

// Public Interfaces
esif_error_t WebPlugin_Init(void);
void WebPlugin_Exit(void);

esif_error_t WebServer_Config(WebServerPtr self, u8 instance, const AccessDef *listener);
esif_error_t WebServer_Start(WebServerPtr self);
void WebServer_Stop(WebServerPtr self);
Bool WebServer_IsStarted(WebServerPtr self);
void WebServer_Pause(WebServerPtr self);
void WebServer_Resume(WebServerPtr self);
Bool WebServer_IsPaused(WebServerPtr self);
Bool WebServer_IsDiagnostic(WebServerPtr self);
atomic_t WebServer_GetListenerMask(WebServerPtr self);
atomic_t WebServer_GetPipeMask(WebServerPtr self);
atomic_t WebServer_GetRpcQueueMax(WebServerPtr self);
void WebServer_SetRpcQueueMax(WebServerPtr self, size_t maxQueue);

esif_error_t WebServer_EnQueueRpc(WebServerPtr self, WebWorkerPtr rpcWorker, void *object);
esif_error_t WebServer_EnQueueMsg(WebServerPtr self, void *object);
esif_error_t WebServer_SendMsg(WebServerPtr self, const esif_handle_t ipfHandle, const void *buffer, size_t buf_len);
esif_error_t WebServer_IrpcRequest(IrpcTransaction *trx);

WebWorkerPtr WebWorker_Create();
void WebWorker_Destroy(WebWorkerPtr self);
void WebWorker_GetRef(WebWorkerPtr self);
void WebWorker_PutRef(WebWorkerPtr self);
esif_error_t WebWorker_Start(WebWorkerPtr self);
void WebWorker_Stop(WebWorkerPtr self);

esif_error_t WebClient_Write(WebClientPtr self, void *buffer, size_t buf_len);
void WebClient_Close(WebClientPtr self);
void WebServer_CloseOrphans(WebServerPtr self);

// Trace Messaging
#define WS_TRACE_FATAL(msg, ...)	IPF_TRACE_FATAL(msg, ##__VA_ARGS__)
#define WS_TRACE_ERROR(msg, ...)	IPF_TRACE_ERROR(msg, ##__VA_ARGS__)
#define WS_TRACE_WARNING(msg, ...)	IPF_TRACE_WARN(msg, ##__VA_ARGS__)
#define WS_TRACE_INFO(msg, ...)		IPF_TRACE_INFO(msg, ##__VA_ARGS__)
#define WS_TRACE_DEBUG(msg, ...)	IPF_TRACE_DEBUG(msg, ##__VA_ARGS__)

// Display Console Message
#define WS_CONSOLE_MSG(msg, ...)	printf(msg, ##__VA_ARGS__)

// Friends
void IpfSrv_Init(void);
void IpfSrv_Uninit(void);
