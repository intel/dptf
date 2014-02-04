/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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


#include "esif.h"
#include "esif_uf_ccb_sock.h"
#include "esif_ws_algo.h"

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



enum frameType {
	EMPTY_FRAME      = 0xF0,
	ERROR_FRAME      = 0xF1,
	INCOMPLETE_FRAME = 0xF2,
	TEXT_FRAME       = 0x01,
	BINARY_FRAME     = 0x02,
	PING_FRAME       = 0x09,
	PONG_FRAME       = 0x0A,
	OPENING_FRAME    = 0xF3,
	CLOSING_FRAME    = 0x08,
	HTTP_FRAME       = 0xFF
};


enum socketState {
	STATE_OPENING,
	STATE_NORMAL,
	STATE_CLOSING
};


typedef struct _protocol {
	char  *hostField;
	char  *originField;
	char  *keyField;
	char  *webpage;
	char  *web_socket_field;
	enum frameType frame;
}protocol;


typedef struct _msgBuffer {
	char    *msgReceive;
	UInt32  rcvSize;
	char    *msgSend;
	UInt32  sendSize;
}msgBuffer;

enum frameType esif_ws_get_initial_frame_type(const UInt8*, size_t, protocol*);


eEsifError esif_ws_socket_build_response_header (const protocol*, UInt8*, size_t*);


void esif_ws_socket_build_payload(const UInt8*, size_t, UInt8*, size_t*, enum frameType);


enum frameType esif_ws_socket_get_subsequent_frame_type(UInt8*, size_t, UInt8 * *, size_t*);


void esif_ws_socket_initialize_frame (protocol*);

void esif_ws_socket_initialize_message_buffer (msgBuffer*);

void esif_ws_socket_delete_existing_field (char*);


#endif /* SOCKET_H */
