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

#include "esif_ccb.h"
#include "esif_ccb_rc.h"
#include "esif_ccb_file.h"
#include "esif_ccb_string.h"
#include "esif_sdk.h"
#include "ipf_ipc_clisrv.h"
#include "ipf_core_api.h"

// TCP/IP Defaults
#define IPC_DEFAULT_PORT	18086	// Default TCP/IP Port

// Default Paths for Unix Domain Socket Files Folder
# define	DEFAULT_NAMEDPIPE_PATH		"/var/run/ipfsrv"

// Return a String representation of a Socket Address (IP or Pipe Name) or NULL for an invalid address
char *IpfIpc_SockaddrAddr(
	esif_ccb_sockaddr_t sockaddr,
	char *buffer,
	size_t buf_len
)
{
	char *result = NULL;
	if (buffer && buf_len) {
		switch (sockaddr.type) {
		case AF_INET:
		{
			char *ip = inet_ntoa(sockaddr.in_addr.sin_addr);
			if (ip) {
				esif_ccb_sprintf(buf_len, buffer, "%s%s%s:%hu", IPC_PROTO_WEBSOCKET, IPC_PROTO_SCHEME, ip, esif_ccb_htons(sockaddr.in_addr.sin_port));
				result = buffer;
			}
			break;
		}
		case AF_UNIX:
		{
			char *pipe = esif_ccb_strrchr(sockaddr.un_addr.sun_path, *ESIF_PATH_SEP);
			pipe = (pipe ? pipe + 1 : sockaddr.un_addr.sun_path);
			if (pipe[0]) {
				esif_ccb_sprintf(buf_len, buffer, "%s%s%s", IPC_PROTO_NAMEDPIPE, IPC_PROTO_SCHEME, pipe);
				result = buffer;
			}
			break;
		}
		default:
			break;
		}
	}
	return buffer;
}

// Parse a Server Address into an Opaque Socket Address
// Server Address may be one of multiple supported formats:
//   1. TCP/IP URL = [proto://]ipaddr[:port]
//		Examples:
//			ws://127.0.0.1:18086
//			127.0.0.1:18086
//			127.0.0.1
//   2. Named Pipe URL = [proto://]pipename
//		Examples:
//			pipe://ipfsrv.public
//			pipe://ipfsrv.elevated
//
esif_error_t IpfIpc_ParseServerAddress(
	const char *serverAddr,	// Server Address (URL) or NULL for Default
	esif_ccb_sockaddr_t *sockaddrPtr // Opaque Socket Address [out]
)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (sockaddrPtr) {
		// Set Defaults
		char proto[IPC_PROTO_LEN] = { 0 };
		esif_ccb_memset(sockaddrPtr, 0, sizeof(*sockaddrPtr));
		if (serverAddr == NULL || serverAddr[0] == 0) {
			serverAddr = DEFAULT_SERVERADDR;
		}

		// Parse [proto://][address][:port][/resource]
		const char *url = serverAddr;
		const char scheme[] = IPC_PROTO_SCHEME;
		const char *addr = strstr(url, scheme);
		if (addr) {
			esif_ccb_strmemcpy(proto, sizeof(proto) - 1, url, (ptrdiff_t)(addr - url));
			addr += sizeof(scheme) - 1;
		}
		else {
			addr = url;

			// If no [proto://], assume Named Pipe unless [:port] or [addr] starts with a digit
			if (esif_ccb_strchr(addr, ':') != NULL || isdigit(*addr)) {
				esif_ccb_strcpy(proto, IPC_PROTO_WEBSOCKET, sizeof(proto));
			}
			else {
				esif_ccb_strcpy(proto, IPC_PROTO_NAMEDPIPE, sizeof(proto));
			}
		}

		// Set Socket Address for Internet Addresses [IPv4 + Port]
		if (esif_ccb_stricmp(proto, IPC_PROTO_WEBSOCKET) == 0) {
			char host[ESIF_IPADDR_LEN] = { 0 };
			unsigned short port = IPC_DEFAULT_PORT;
			char *portsep = esif_ccb_strpbrk(addr, ":/?");
			if (portsep) {
				int portnum = (*portsep == ':' ? atoi(portsep + 1) : port);
				port = (unsigned short)(portnum > 0 && portnum <= 0xffff ? portnum : 0);
				esif_ccb_strmemcpy(host, sizeof(host) - 1, addr, (portsep - addr));
			}
			else {
				esif_ccb_strcpy(host, addr, sizeof(host));
			}
			in_addr_t in_addr = inet_addr(host);
			if (in_addr == INADDR_NONE || port == 0) {
				rc = ESIF_E_WS_INVALID_ADDR;
			}
			else {
				sockaddrPtr->in_addr.sin_family = AF_INET;
				sockaddrPtr->in_addr.sin_addr.s_addr = in_addr;
				sockaddrPtr->in_addr.sin_port = esif_ccb_htons(port);
				rc = ESIF_OK;
			}
		}
		// Set Socket Address for Logical Named Pipe (Unix Domain Socket or "Named Socket") [Full Pathname]
		else if (esif_ccb_stricmp(proto, IPC_PROTO_NAMEDPIPE) == 0) {
			size_t max_addr_len = sizeof(sockaddrPtr->un_addr.sun_path) - esif_ccb_strlen(IPC_NAMEDPIPE_PATH, sizeof(sockaddrPtr->un_addr.sun_path));
			if (addr[0] == 0 || esif_ccb_strpbrk(addr, ":/\\?") != NULL || esif_ccb_strncmp(addr, "..", 2) == 0 || esif_ccb_strlen(addr, MAX_PATH) > max_addr_len) {
				rc = ESIF_E_WS_INVALID_ADDR;
			}
			else {
				sockaddrPtr->un_addr.sun_family = AF_UNIX;
				esif_ccb_sprintf(sizeof(sockaddrPtr->un_addr.sun_path), sockaddrPtr->un_addr.sun_path, "%s" ESIF_PATH_SEP "%s", IPC_NAMEDPIPE_PATH, addr);
				rc = ESIF_OK;
			}
		}
		else {
			rc = ESIF_E_NOT_SUPPORTED;
		}
	}
	return rc;
}

// Return IPC Logical Named Pipe (UDS File) Path
char *IpfIpc_NamedPipePath(void)
{
#ifdef ALTERNATE_NAMEDPIPE_PATH
	static char *pipepath = NULL; // Set on first call to this function or IpfIpc_ParseServerAddress()
	if (pipepath == NULL) {
		// Use Default path under ServiceProfiles if it exists, otherwise use Public path [Required for Win10X Compatibility]
		if (esif_ccb_file_exists(CHECKFOR_NAMEDPIPE_PATH)) {
			pipepath = DEFAULT_NAMEDPIPE_PATH;
		}
		else {
			pipepath = ALTERNATE_NAMEDPIPE_PATH;
		}
	}
	return pipepath;
#else
	return DEFAULT_NAMEDPIPE_PATH;
#endif
}