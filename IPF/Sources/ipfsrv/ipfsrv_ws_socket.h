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

#include "ipfsrv_ws_server.h"

// IRPC Message Queue Node Object
typedef struct ClientRequest_s {
	esif_handle_t	ipfHandle;	// Unique IPF Client Handle
	IrpcTransaction	*trx;		// IRPC Transaction
} ClientRequest, *ClientRequestPtr;

// WebSocket Server Public Interface
esif_error_t WebServer_WebsocketRequest(WebServerPtr self, WebClientPtr client, u8 *buffer, size_t buf_len);
esif_error_t WebServer_WebsocketDeliver(WebServerPtr self, WebClientPtr client, u8 *buffer, size_t buf_len);
