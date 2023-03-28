/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "esif_ccb_socket.h"


#include <sys/stat.h>
#include <sys/un.h>

// A Unix Domain Socket in Linux is always a Regular File.
static ESIF_INLINE int esif_socket_file_stat(const char* pathname)
{
	struct stat st = { 0 };
	int rc = stat(pathname, &st);
	if (rc == 0) {
		if ((st.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0) {
			rc = EACCES;
		}
	}
	else {
		rc = errno;
	}
	return rc;
}
#define AF_UNDEF	0		// Undefined Protocol Family

#define IPC_NAMEDPIPE_PATH		IpfIpc_NamedPipePath()

// Opaque Socket Address Union
typedef union esif_ccb_sockaddr_u {
	sa_family_t	type;			// Protocol Type: AF_UNDEF, AF_INET, AF_UNIX
	struct sockaddr_in in_addr;	// Internet Socket Address (IPv4 + Port)
	struct sockaddr_un un_addr;	// Unix Domain Socket Address (Full PathName)
} esif_ccb_sockaddr_t;

// Return sockaddr size for the given Protocol Family
static ESIF_INLINE socklen_t IpfIpc_SockaddrLen(sa_family_t type)
{
	return ((type) == AF_INET ? sizeof(struct sockaddr_in) : (type) == AF_UNIX ? sizeof(struct sockaddr_un) : 0);
}

// Return a String representation of a Socket Address (IP or Pipe Name) or NULL for an invalid address
char *IpfIpc_SockaddrAddr(
	esif_ccb_sockaddr_t sockaddr,
	char *buffer,
	size_t buf_len
);

// Parse Server Address into a Socket Address
esif_error_t IpfIpc_ParseServerAddress(
	const char *serverAddr,
	esif_ccb_sockaddr_t *sockaddrPtr
);

// Return IPC Logical Named Pipe (UDS File) Path
char *IpfIpc_NamedPipePath(void);