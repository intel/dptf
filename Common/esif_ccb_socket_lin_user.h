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

#pragma once

#if defined(ESIF_ATTR_OS_LINUX) && defined(ESIF_ATTR_USER)

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef int esif_ccb_socket_t;

#define INVALID_SOCKET (~0)
#define SOCKET_ERROR (-1)

#define ESIF_SOCKERR_EWOULDBLOCK	EWOULDBLOCK	// Operation would block

// Use to avoid TIME_WAIT timeouts on dead listeners before a new listener can reuse a port
#ifndef SO_EXCLUSIVEADDRUSE
#define SO_EXCLUSIVEADDRUSE	SO_REUSEADDR
#endif

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

// Call close only from the thread servicing the socket
static int ESIF_INLINE esif_ccb_socket_close(esif_ccb_socket_t socket)
{
	return close(socket);
}

// Call shutdown from another thread to signal a waiting select() to exit
// For Linux, we signal select() by stopping all I/O on the socket
static int ESIF_INLINE esif_ccb_socket_shutdown(esif_ccb_socket_t socket)
{
	return shutdown(socket, SHUT_RDWR);
}

#define esif_ccb_socket_ioctl(socket, cmd, argp)	(0)

#endif /* LINUX USER */
