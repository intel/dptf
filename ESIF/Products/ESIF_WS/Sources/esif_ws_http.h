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

#ifndef ESIF_WS_HTTP_H
#define ESIF_WS_HTTP_H

#include "esif.h"
#include "esif_uf_ccb_sock.h"
#include "esif_ws_server.h"

#define BUFFER_LENGTH 0xFFFF

typedef struct s_extType {
	char  *fileExtension;
	char  *fileType;
} extType;

/*
 *******************************************************************************
 ** PUBLIC INTERFACE
 *******************************************************************************
 */

void esif_ws_http_copy_server_root(char*);
eEsifError esif_ws_http_process_reqs(clientRecord *, void*, ssize_t);


#endif /* ESIF_WS_HTTP_H*/
