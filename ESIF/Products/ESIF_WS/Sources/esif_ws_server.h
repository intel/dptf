/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "esif_ccb_lock.h"
#include "esif_ccb_socket.h"
#include "esif_ccb_thread.h"

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

// Web Server Listener Object
typedef struct WebListener_s {
	esif_ccb_socket_t	socket;					// Listener Socket or INVALID_SOCKET
	char				ipAddr[ESIF_IPADDR_LEN];// Listener IP Address
	short				port;					// Listner Port
	esif_flags_t		flags;					// Listener Flags
} WebListener, *WebListenerPtr;

// Web Server Client Object
typedef struct WebClient_s {
	ClientType			type;			// Client Type (Closed, Http, Websocket)
	esif_ccb_socket_t	socket;			// Client Socket Handle or INVALID_SOCKET
	char				*ipAddr;		// Client IP Address
	u8					*sendBuf;		// TCP/IP Send Buffer
	size_t				sendBufLen;		// TCP/IP Send Buffer Length
	u8					*recvBuf;		// TCP/IP Receive Buffer (Partial HTTP Request or Websocket Frame)
	size_t				recvBufLen;		// TCP/IP Receive Buffer Length
	
	int					httpStatus;		// HTTP Status Code
	char				*httpBuf;		// HTTP Request Buffer
	char				**httpRequest;	// HTTP Request Headers Array (NULL-terminated)
	
	FrameType			msgType;		// Websocket Message Type
	u8					*fragBuf;		// Websocket Multi-Fragment Buffer
	size_t				fragBufLen;		// Websocket Multi-Fragment Buffer Length
} WebClient, *WebClientPtr;

#define WS_MAX_LISTENERS	1	// Number of Web Server Listener Sockets
#define WS_MAX_CLIENTS		10	// 5 simultaneous UI websocket clients

// Max Active Sockets (per WebServer) cannot exceed FD_SETSIZE (Default: Windows=64, Linux=1024)
#if ((WS_MAX_LISTENERS + WS_MAX_CLIENTS + 1) > FD_SETSIZE)
# undef  WS_MAX_CLIENTS
# define WS_MAX_CLIENTS	(FD_SETSIZE - WS_MAX_LISTENERS - 1)
#endif
#define WS_MAX_SOCKETS	(WS_MAX_LISTENERS + WS_MAX_CLIENTS + 1)

// Default IP/ports
#define WS_REMOTE_IPADDR		"0.0.0.0"	// All Interfaces (localhost and remote connections)
#define WS_DEFAULT_IPADDR		"127.0.0.1"	// Loopback Interface only (localhost)
#define WS_DEFAULT_PORT			8086		// Standard Port
#define WS_DEFAULT_FLAGS		0			// Standard Flags

#define CRLF	"\r\n"

// Web Server Object (One per Worker Thread)
typedef struct WebServer_s {
	esif_ccb_lock_t		lock;						// Thread Lock

	TcpDoorbell			doorbell;					// Doorbell Intra-Process Signal object
	WebListener			listeners[WS_MAX_LISTENERS];// Web Server Listener Socket(s)
	WebClient			clients[WS_MAX_CLIENTS];	// Web Clients

	esif_thread_t		mainThread;					// Web Server Main Worker Thread
	atomic_t			isActive;					// Web Server Active Flag
	atomic_t			activeThreads;				// Active Thread Count

	u8					*netBuf;					// Network Send/Receive Buffer
	size_t				netBufLen;					// Network Send/Receive Buffer Length
} WebServer, *WebServerPtr;

extern WebServerPtr g_WebServer;

// Public Interfaces
esif_error_t WebPlugin_Init(void);
void WebPlugin_Exit(void);

esif_error_t WebServer_Start(WebServerPtr self);
void WebServer_Stop(WebServerPtr self);
Bool WebServer_IsStarted(WebServerPtr self);
esif_error_t WebServer_Config(WebServerPtr self, u8 instance, char *ipAddr, short port, esif_flags_t flags);

esif_error_t WebClient_Write(WebClientPtr self, void *buffer, size_t buf_len);
void WebClient_Close(WebClientPtr self);

// Trace Messaging
typedef enum esif_ws_tracelevel {
	TRACELEVEL_NONE = -1,
	TRACELEVEL_FATAL = 0,
	TRACELEVEL_ERROR = 1,
	TRACELEVEL_WARNING = 2,
	TRACELEVEL_INFO = 3,
	TRACELEVEL_DEBUG = 4,
} esif_ws_tracelevel_t;

#define WS_DOTRACE_IFACTIVE(level, msg, ...) \
	do { \
		if (EsifWsTraceLevel() >= level) { \
			EsifWsTraceMessageEx(level, ESIF_FUNC, __FILE__, __LINE__, msg, ##__VA_ARGS__); \
		} \
	} while ESIF_CONSTEXPR(ESIF_FALSE)

#define WS_TRACE_FATAL(msg, ...)	WS_DOTRACE_IFACTIVE(TRACELEVEL_FATAL, msg, ##__VA_ARGS__)
#define WS_TRACE_ERROR(msg, ...)	WS_DOTRACE_IFACTIVE(TRACELEVEL_ERROR, msg, ##__VA_ARGS__)
#define WS_TRACE_WARNING(msg, ...)	WS_DOTRACE_IFACTIVE(TRACELEVEL_WARNING, msg, ##__VA_ARGS__)
#define WS_TRACE_INFO(msg, ...)		WS_DOTRACE_IFACTIVE(TRACELEVEL_INFO, msg, ##__VA_ARGS__)
#define WS_TRACE_DEBUG(msg, ...)	WS_DOTRACE_IFACTIVE(TRACELEVEL_DEBUG, msg, ##__VA_ARGS__)

// ESIF_UF <-> ESIF_WS Interface Helpers
void EsifWsLock(void);
void EsifWsUnlock(void);
const char *EsifWsDocRoot(void);
Bool EsifWsShellEnabled(void);
char *EsifWsShellExec(char *cmd, size_t cmd_len, char *prefix, size_t prefix_len);
int EsifWsTraceLevel(void);
int EsifWsTraceMessageEx(int level, const char *func, const char *file, int line, const char *msg, ...);
int EsifWsConsoleMessageEx(const char *msg, ...);
