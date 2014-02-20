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

#ifndef ESIF_WS_SERVER_H
#define ESIF_WS_SERVER_H

#include "esif.h"
#include "esif_ws_socket.h"

typedef struct s_clientRecord {
	esif_ccb_socket_t socket;
	enum socketState  state;
	enum frameType    frameType;
	protocol   prot;
	msgBuffer  buf;
} clientRecord;

int  esif_ws_init(void);
void esif_ws_exit(esif_thread_t *web_thread);
void esif_ws_set_ipaddr_port(const char *ipaddr, u32 port);

#endif /* ESIF_WS_SERVER_H */
