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

#ifndef ESIF_WS_SERVER_H
#define ESIF_WS_SERVER_H

#include "esif.h"
#include "esif_ws_socket.h"

#ifdef MSG_NOSIGNAL
#define ESIF_WS_SEND_FLAGS MSG_NOSIGNAL
#else
#define ESIF_WS_SEND_FLAGS 0
#endif

#pragma pack(push, 1)

typedef enum SocketState_e {
	STATE_OPENING,
	STATE_NORMAL
}SocketState, *SocketStatePtr;

typedef struct ClientRecord_s {
	esif_ccb_socket_t socket;
	SocketState state;
	Protocol prot;
	u8 *frame_buffer;
	size_t frame_buf_len;
	u8 *frag_buffer;
	size_t frag_buf_len;
	FrameType frag_type;
} ClientRecord, *ClientRecordPtr;

#pragma pack(pop)



int  esif_ws_init(void);
void esif_ws_exit(esif_thread_t *threadPtr);
void esif_ws_server_set_ipaddr_port(const char *ipaddr, u32 port, Bool restricted);
void esif_ws_client_close_client(ClientRecordPtr clientPtr);
u32  esif_ws_buffer_resize(u32 size);
eEsifError esif_ws_broadcast_data_buffer(const u8 *bufferPtr, size_t bufferSize);

#endif /* ESIF_WS_SERVER_H */
