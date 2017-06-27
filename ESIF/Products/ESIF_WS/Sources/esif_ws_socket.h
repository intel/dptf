/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#ifndef ESIF_WS_SOCKET_H
#define ESIF_WS_SOCKET_H

#ifdef ESIF_ATTR_OS_WINDOWS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include "esif_ccb.h"
#include "esif_ccb_rc.h"
#include "esif_ccb_socket.h"
#include "esif_sdk_iface_ws.h"

// ESIF_UF <-> ESIF_WS Interface Helpers
void EsifWsLock(void);
void EsifWsUnlock(void);
const char *EsifWsDocRoot(void);
const char *EsifWsLogRoot(void);
Bool EsifWsShellEnabled(void);
void EsifWsShellLock(void);
void EsifWsShellUnlock(void);
char *EsifWsShellExec(char *cmd, size_t data_len);
size_t EsifWsShellBufLen(void);
int EsifWsTraceLevel(void);
int EsifWsTraceMessageEx(int level, const char *func, const char *file, int line, const char *msg, ...);
int EsifWsConsoleMessageEx(const char *msg, ...);

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

#define WS_HEADER_BUF_LEN	128		/* Buffer size for Websocket Header in REST responses */
#define WS_BUFFER_LENGTH (OUT_BUF_LEN + WS_HEADER_BUF_LEN)

typedef enum FrameType_e {
	CONTINUATION_FRAME	= 0x00,
	TEXT_FRAME			= 0x01,
	BINARY_FRAME		= 0x02,
	CLOSING_FRAME		= 0x08,
	PING_FRAME			= 0x09,
	PONG_FRAME			= 0x0A,

	/* Pseudo Frame Types used internally. Per RFC6455:
	 * %x3-7 are reserved for further non-control frames
	 * %xB-F are reserved for futher control frames
	 */
	ERROR_FRAME			= 0xF3,
	INCOMPLETE_FRAME	= 0xF4,
	OPENING_FRAME		= 0xF5,
	HTTP_FRAME			= 0xF6,
	FRAGMENT_FRAME		= 0xF7,

	EMPTY_FRAME			= 0xFF,
} FrameType, *FrameTypePtr;

typedef enum FinType_e {
	FRAME_FRAGMENT = 0,
	FRAME_FINAL = 1
} FinType, *FinTypePtr;

#pragma pack(push, 1)

typedef struct Protocol_s {
	char  *hostField;
	char  *originField;
	char  *keyField;
	char  *webpage;
	char  *web_socket_field;
	char  *user_agent;
	FrameType frameType;
} Protocol, *ProtocolPtr;

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

typedef struct WsSocketFrame_s {

	WsSocketFrameHeader header;

	union {
		struct {
			char data[1];
		} T1_NoMask;
		
		struct {
			UInt32 maskKey;
			char data[1];
		} T1;

		struct {
			UInt16 extPayLen;
			char data[1];
		} T2_NoMask;
		
		struct {
			UInt16 extPayLen;
			UInt32 maskKey;
			char data[1];
		} T2;

		struct {
			UInt64 extPayLen;
			char data[1];
		} T3_NoMask;

		struct {
			UInt64 extPayLen;
			UInt32 maskKey;
			char data[1];
		} T3;

	} u;
	
} WsSocketFrame, *WsSocketFramePtr;

#pragma pack(pop)

/*
 * Public Interface
 */

FrameType esif_ws_socket_get_initial_frame_type(
	const char *framePtr,
	size_t incomingFrameLen,
	ProtocolPtr prot
	);

eEsifError esif_ws_socket_build_protocol_change_response(
	const ProtocolPtr protPtr,
	char *outgoingFrame,
	size_t outgoingFrameBufferSize,
	size_t *outgoingFrameSizePtr
	);

void esif_ws_socket_build_payload(
	const char *data,
	size_t dataLength,
	WsSocketFramePtr framePtr,
	size_t bufferSize,
	size_t *outgoingFrameSizePtr,
	FrameType frameType,
	FinType finType
	);

FrameType esif_ws_socket_get_subsequent_frame_type(
	WsSocketFramePtr framePtr,
	size_t incomingFrameLen,
	char **dataPtr,
	size_t *dataLength,
	size_t *bytesRemaining
	);


#endif /* SOCKET_H */
