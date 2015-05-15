/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#include "esif.h"
#include "esif_uf_ccb_sock.h"
#include "esif_ws_algo.h"

#define WS_BUFFER_LENGTH OUT_BUF_LEN

#define WS_PROT_KEY_SIZE_MAX 1000
#define WS_FRAME_SIZE_TYPE_1	125 /* For payloads <= 125 bytes*/
#define WS_FRAME_SIZE_TYPE_2	126 /* For payload up to 64K - 1 in size */
#define WS_FRAME_SIZE_TYPE_3	127 /* For payloads > 64K - 1*/

#define WS_FRAME_SIZE_TYPE_1_MAX_PAYLOAD	125		/* Maximum payload size for a Type 1 frame size */
#define WS_FRAME_SIZE_TYPE_2_MAX_PAYLOAD	0xFFFF	/* Maximum payload size for a Type 2 frame size */

#define MAXIMUM_SIZE 0x7F

#define HOST_FIELD                      "Host: "
#define ORIGIN_FIELD                    "Origin: "
#define WEB_SOCK_PROT_FIELD             "Sec-WebSocket-Protocol: "
#define WEB_SOCK_KEY_FIELD              "Sec-WebSocket-Key: "
#define WEB_SOCK_VERSION_FIELD          "Sec-WebSocket-Version: "

#define CONNECTION_FIELD                "Connection: "
#define UPGRADE_FIELD                   "upgrade"
#define ALT_UPGRADE_FIELD               "Upgrade: "
#define WEB_SOCK_FIELD                  "websocket"
#define VERSION_FIELD                   "13"
#define KEY                             "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


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

typedef struct Protocol_s {
	char  *hostField;
	char  *originField;
	char  *keyField;
	char  *webpage;
	char  *web_socket_field;
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



FrameType esif_ws_socket_get_initial_frame_type(
	const char *framePtr,
	size_t incomingFrameLen,
	Protocol *prot
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
	FrameType frameType
	);

FrameType esif_ws_socket_get_subsequent_frame_type(
	WsSocketFramePtr framePtr,
	size_t incomingFrameLen,
	char **dataPtr,
	size_t *dataLength,
	size_t *bytesRemaining
	);


#endif /* SOCKET_H */
