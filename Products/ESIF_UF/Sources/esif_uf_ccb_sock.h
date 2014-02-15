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
typedef size_t ssize_t;
typedef int socklen_t;
typedef SOCKET esif_ccb_socket_t;

#define esif_ccb_socket_close(s) closesocket(s)

static void ESIF_INLINE esif_ccb_socket_init(void)
{
	WSADATA wsaData={0};

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		ESIF_TRACE_ERROR("WSAStartup failed: %d\n", iResult);
	}
}

static void ESIF_INLINE esif_ccb_socket_exit(void)
{
	WSACleanup();
}

static int ESIF_INLINE esif_ccb_socket_error(void)
{
	return WSAGetLastError();
}

#endif

#ifdef ESIF_ATTR_OS_LINUX

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef int esif_ccb_socket_t;

#define INVALID_SOCKET (~0)
#define SOCKET_ERROR (-1)

#define esif_ccb_socket_close(s) close(s)

static void ESIF_INLINE esif_ccb_socket_init(void) {}

static void ESIF_INLINE esif_ccb_socket_exit(void) {}

static int ESIF_INLINE esif_ccb_socket_error(void)
{
	return errno;
}

#endif

#endif /* _ESIF_UF_CCB_SOCK_H_ */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
