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

#ifndef _ESIF_UF_CCB_SOCK_H_
#define _ESIF_UF_CCB_SOCK_H_

#include "esif.h"

#ifdef ESIF_ATTR_OS_WINDOWS
typedef int ssize_t;
typedef int esif_uf_ccb_sock_len;
#define esif_uf_ccb_sock_close closesocket

static void ESIF_INLINE esif_uf_ccb_sock_init ()
{
	WSADATA wsaData;

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
	}
}


#endif

#ifdef ESIF_ATTR_OS_LINUX

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef socklen_t esif_uf_ccb_sock_len;
#define esif_uf_ccb_sock_close close

static void ESIF_INLINE esif_uf_ccb_sock_init () {}

#endif

#endif /* _ESIF_UF_CCB_SOCK_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
