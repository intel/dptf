/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#if defined(ESIF_ATTR_OS_LINUX) && defined(ESIF_ATTR_USER)

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef int esif_ccb_socket_t;

#define INVALID_SOCKET (~0)
#define SOCKET_ERROR (-1)

#define ESIF_SOCKERR_OK				0			// No Error
#define ESIF_SOCKERR_ENOENT			ENOENT		// File Not Found
#define ESIF_SOCKERR_EACCES			EACCES		// Access Denied
#define ESIF_SOCKERR_EWOULDBLOCK	EWOULDBLOCK	// Operation would block
#define ESIF_PF_LOCAL				PF_LOCAL
#define ESIF_SHUT_RD				SHUT_RD
#define ESIF_SHUT_WR				SHUT_WR
#define ESIF_SHUT_RDWR				SHUT_RDWR
#define ESIF_SO_EXCLUSIVEADDRUSE	SO_REUSEADDR

static int ESIF_INLINE esif_ccb_socket_init(void)
{
	return 0;
}

static void ESIF_INLINE esif_ccb_socket_exit(void)
{
}

static int ESIF_INLINE esif_ccb_socket_error(void)
{
	return errno;
}

static void ESIF_INLINE esif_ccb_socket_seterror(int err)
{
	errno = err;
}

// Call close only from the thread servicing the socket
static int ESIF_INLINE esif_ccb_socket_close(esif_ccb_socket_t socket)
{
	return close(socket);
}

// Call shutdown from another thread to signal a waiting select() to exit
static int ESIF_INLINE esif_ccb_socket_shutdown(esif_ccb_socket_t socket, int how)
{
	return shutdown(socket, how);
}

#define esif_ccb_socket_ioctl(socket, cmd, argp)	do { if (argp) UNREFERENCED_PARAMETER(*argp); } while (0)

#define esif_ccb_socketpair(af, typ, prot, sock)	socketpair(af, typ, prot, sock)

#endif /* LINUX USER */
