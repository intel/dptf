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

#ifndef _ESIF_UF_IPC_
#define _ESIF_UF_IPC_

#include "esif_uf.h"
#include "esif_ipc.h"

//
// IPC
//
eEsifError ipc_connect();
eEsifError ipc_autoconnect(UInt32 max_retries); // 0 = Infinite
void ipc_disconnect();
int ipc_isconnected();

enum esif_rc ipc_execute(struct esif_ipc *ipc);

#endif /* _ESIF_UF_IPC_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
