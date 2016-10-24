/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "esif_uf_ccb_sock.h"

#define WS_HEADER_BUF_LEN	128		/* Buffer size for Websocket Header in REST responses */
#define WS_BUFFER_LENGTH (OUT_BUF_LEN + WS_HEADER_BUF_LEN)

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

/*
 * Public Interface
 */

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
