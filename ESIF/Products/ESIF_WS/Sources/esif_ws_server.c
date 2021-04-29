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

#define ESIF_CCB_LINK_LIST_MAIN

#include "esif_ccb.h"
#include "esif_ccb_atomic.h"
#include "esif_ccb_lock.h"
#include "esif_ccb_string.h"
#include "esif_link_list.h"

#include "esif_ws_server.h"
#include "esif_ws_http.h"
#include "esif_ws_socket.h"

#ifdef ESIF_ATTR_OS_WINDOWS
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// Use Non-Blocking Socket I/O
#ifdef MSG_NOSIGNAL
#define WS_NONBLOCKING_FLAGS (MSG_NOSIGNAL|MSG_DONTWAIT)
#else
#define WS_NONBLOCKING_FLAGS 0
#endif
#define WS_SEND_FLAGS	0
#define WS_RECV_FLAGS	0

#define WS_NETWORK_BUFFER_LEN	65535	// Network Buffer Size for HTTP/Websocket Send and Receive Buffer
#define WS_SOCKET_TIMEOUT		2		// Socket activity timeout waiting on blocking select() [2 or greater]

#define WS_MAX_CLIENT_SENDBUF	(8*1024*1024)	// Max size of client send buffer (multiple messages)
#define WS_MAX_CLIENT_RECVBUF	(8*1024*1024)	// Max size of client receive buffer (multiple messages)

WebServerPtr g_WebServer = NULL;	// Global Web Server Singleton Intance

// Doorbell Opcodes
#define WS_OPCODE_NOOP			0x00	// No-Operation
#define WS_OPCODE_QUIT			0xFF	// Quit Web Server

//// TCP Doorbell Object Methods ////

// Initialize Doorbell Object
void TcpDoorbell_Init(TcpDoorbellPtr self)
{
	if (self) {
		int j = 0;
		self->isActive = ESIF_FALSE;
		for (j = 0; j < DOORBELL_SOCKETS; j++) {
			self->sockets[j] = INVALID_SOCKET;
		}
	}
}

// Create a Doorbell object with Paired Sockets for sending signals between threads
esif_error_t TcpDoorbell_Open(TcpDoorbellPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		if (esif_ccb_socketpair(ESIF_PF_LOCAL, SOCK_STREAM, IPPROTO_IP, self->sockets) == 0) {
			self->isActive = ESIF_TRUE;
			rc = ESIF_OK;
		}
		else {
			rc = ESIF_E_WS_INIT_FAILED;
		}
	}
	return rc;
}

// Close Doorbell object
void TcpDoorbell_Close(TcpDoorbellPtr self)
{
	if (self) {
		int j = 0;
		self->isActive = ESIF_FALSE;
		for (j = 0; j < DOORBELL_SOCKETS; j++) {
			if (self->sockets[j] != INVALID_SOCKET) {
				esif_ccb_socket_close(self->sockets[j]);
				self->sockets[j] = INVALID_SOCKET;
			}
		}
	}
}

// Stop Doorbell object and signal blocking select() to exit
void TcpDoorbell_Stop(TcpDoorbellPtr self)
{
	if (self && self->sockets[DOORBELL_BUTTON] != INVALID_SOCKET) {
		esif_ccb_socket_shutdown(self->sockets[DOORBELL_BUTTON], ESIF_SHUT_RDWR);
		self->isActive = ESIF_FALSE;
	}
}

// Send a signal using Doorbell object and signal blocking select() to exit
esif_error_t TcpDoorbell_Ring(TcpDoorbellPtr self, u8 opcode)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && self->sockets[DOORBELL_BUTTON] != INVALID_SOCKET) {
		if (self->isActive && send(self->sockets[DOORBELL_BUTTON], (const char *)&opcode, sizeof(opcode), 0) == sizeof(opcode)) {
			rc = ESIF_OK;
		}
		else {
			rc = ESIF_E_WS_SOCKET_ERROR;
		}
	}
	return rc;
}

// Receive a Doorbell signal from a Listener thread
esif_error_t TcpDoorbell_Receive(TcpDoorbellPtr self, u8 *opcodePtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && opcodePtr && self->isActive && self->sockets[DOORBELL_RINGER] != INVALID_SOCKET) {
		ssize_t messageLength = recv(self->sockets[DOORBELL_RINGER], (char *)opcodePtr, sizeof(*opcodePtr), WS_RECV_FLAGS);
		if (messageLength == 0 || messageLength == SOCKET_ERROR || messageLength < sizeof(*opcodePtr)) {
			*opcodePtr = WS_OPCODE_NOOP;
			rc = ESIF_E_WS_SOCKET_ERROR;
		}
		else {
			rc = ESIF_OK;
		}
	}
	return rc;
}

//// Listener Object Methods ////

// Initialize Listener Object
void WebListener_Init(WebListenerPtr self)
{
	if (self) {
		esif_ccb_memset(self, 0, sizeof(*self));
		self->socket = INVALID_SOCKET;
	}
}

// Configure a Web Sever Listener
esif_error_t WebListener_Config(WebListenerPtr self, char *ipAddr, short port, esif_flags_t flags)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		if (ipAddr == NULL) {
			self->ipAddr[0] = 0;
		}
		else {
			if (ipAddr[0] == 0) {
				ipAddr = WS_DEFAULT_IPADDR;
			}
			if (port == 0) {
				port = WS_DEFAULT_PORT;
			}
			esif_ccb_strcpy(self->ipAddr, ipAddr, sizeof(self->ipAddr));
		}
		self->port = port;
		self->flags = flags;
		rc = ESIF_OK;
	}
	return rc;
}

// Open a Web Listener Object and Listen on the configured IP:Port
esif_error_t WebListener_Open(WebListenerPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		int err = SOCKET_ERROR;
		int sockopt = 1; // Exclusive port use (Fail if already in use by another listener)
		struct sockaddr_in listener = { 0 };
		socklen_t listenerLen = sizeof(listener);

		if (self->port == 0) {
			return ESIF_OK;
		}
		if (self->socket != INVALID_SOCKET) {
			return ESIF_E_WS_ALREADY_STARTED;
		}

		// Setup Listner IP/Port Address
		listener.sin_family = AF_INET;
		listener.sin_port = 0;
		listener.sin_addr.s_addr = INADDR_ANY;

		// Validate valid IP Address and Port
		if (self->ipAddr[0]) {
			listener.sin_addr.s_addr = inet_addr(self->ipAddr);
			if (listener.sin_addr.s_addr == INADDR_NONE) {
				WS_TRACE_DEBUG("Invalid IP Address: %s\n", self->ipAddr);
				return ESIF_E_WS_INVALID_ADDR;
			}
		}
		if (self->port < 1) {
			WS_TRACE_DEBUG("Invalid Port Number: %s\n", self->port);
			return ESIF_E_WS_INVALID_ADDR;
		}
		listener.sin_port = esif_ccb_htons(self->port);

		if (// Create Listener socket on specified IP/Port, Failing if port is already in use
			((self->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) != INVALID_SOCKET) &&
			((err = setsockopt(self->socket, SOL_SOCKET, ESIF_SO_EXCLUSIVEADDRUSE, (const char *)&sockopt, sizeof(sockopt))) != SOCKET_ERROR) &&
			((err = bind(self->socket, (struct sockaddr *)&listener, listenerLen)) != SOCKET_ERROR) &&
			((err = listen(self->socket, WS_MAX_CLIENTS)) != SOCKET_ERROR)
			) {
			rc = ESIF_OK;
		}
		else {
			WS_TRACE_ERROR("Cannot Listen on IP [%s] Port %hd (Error %d)\n", self->ipAddr, self->port, esif_ccb_socket_error());
			rc = ESIF_E_WS_INIT_FAILED;
		}
	}
	return rc;
}

// Accept a Client Connection from a Listener Socket into a Socket Handle
esif_error_t WebListener_AcceptClient(WebListenerPtr self, esif_ccb_socket_t *socketPtr, char **ipAddrPtr)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && socketPtr) {
		struct sockaddr_in client = { 0 };
		socklen_t clientLen = sizeof(client);

		*socketPtr = accept(self->socket, (struct sockaddr *)&client, &clientLen);

		if (*socketPtr == INVALID_SOCKET) {
			rc = ESIF_E_WS_SOCKET_ERROR;
		}
		else {
			if (ipAddrPtr) {
				*ipAddrPtr = inet_ntoa(client.sin_addr);
			}
			rc = ESIF_OK;
		}
	}
	return rc;
}

// Close a Listener Object
void WebListener_Close(WebListenerPtr self)
{
	if (self->socket != INVALID_SOCKET) {
		esif_ccb_socket_close(self->socket);
		self->socket = INVALID_SOCKET;
	}
}

//// WebClient Object Methods ////

// Initialize a WebClient Object
void WebClient_Init(WebClientPtr self)
{
	if (self) {
		esif_ccb_memset(self, 0, sizeof(*self));
		self->type = ClientClosed;
		self->socket = INVALID_SOCKET;
		self->msgType = FRAME_NULL;
	}
}

// Close Web Client
void WebClient_Close(WebClientPtr self)
{
	if (self) {
		esif_ccb_free(self->ipAddr);
		esif_ccb_free(self->sendBuf);
		esif_ccb_free(self->recvBuf);
		esif_ccb_free(self->httpBuf);
		esif_ccb_free(self->httpRequest);
		esif_ccb_free(self->fragBuf);
		if (self->socket != INVALID_SOCKET) {
			esif_ccb_socket_close(self->socket);
		}
		WebClient_Init(self);
	}
}

// Write a Buffer to a WebClient
esif_error_t WebClient_Write(WebClientPtr self, void *buffer, size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && self->socket != INVALID_SOCKET) {
		int send_flags = (self->type == ClientWebsocket ? WS_NONBLOCKING_FLAGS : WS_SEND_FLAGS);
		ssize_t ret = 0;
		rc = ESIF_OK;

		// Debug HTTP Response
		if (buffer && esif_ccb_strncmp(buffer, "HTTP/", 5) == 0) {
			WS_TRACE_DEBUG("%.*s", (int)buf_len, (char *)buffer);
		}

		// Do Non-Blocking Send of any data already in send buffer first
		if (self->sendBuf != NULL && self->sendBufLen > 0) {
			ret = send(self->socket, (const char *)self->sendBuf, (int)self->sendBufLen, WS_NONBLOCKING_FLAGS);

			// Destroy send buffer if Complete send, otherwise remove sent data before appending new data
			if (ret == (int)self->sendBufLen) {
				esif_ccb_free(self->sendBuf);
				self->sendBuf = NULL;
				self->sendBufLen = 0;
				ret = 0;
			}
			else if (ret > 0) {
				esif_ccb_memmove(self->sendBuf, self->sendBuf + ret, self->sendBufLen - ret);
				self->sendBufLen -= ret;
				ret = 0;
			}
		}

		// Do Blocking or Non-Blocking send if send buffer is clear
		if (self->sendBuf == NULL && buffer != NULL && buf_len > 0) {
			ret = send(self->socket, (char*)buffer, (int)buf_len, send_flags);
		}

		// Close Socket on Failure; EWOULDBLOCK is not a failure it is expected if the operation would block
		if (ret == SOCKET_ERROR) {
			int errnum = 0;
			if ((errnum = esif_ccb_socket_error()) == ESIF_SOCKERR_EWOULDBLOCK) {
				ret = 0;
			}
			else {
				rc = ESIF_E_WS_SOCKET_ERROR;
			}
		}

		// Append any unsent data to send buffer
		if (rc == ESIF_OK && buffer != NULL && ret != (ssize_t)buf_len) {
			size_t newBufLen = self->sendBufLen + buf_len - ret;
			u8 *newBuffer = NULL;
			if (newBufLen <= WS_MAX_CLIENT_SENDBUF) {
				newBuffer = esif_ccb_realloc(self->sendBuf, newBufLen);
			}
			if (newBuffer == NULL) {
				rc = ESIF_E_NO_MEMORY;
			}
			else {
				esif_ccb_memmove(newBuffer + self->sendBufLen, (u8*)buffer + ret, buf_len - ret);
				self->sendBuf = newBuffer;
				self->sendBufLen = newBufLen;
			}
			WS_TRACE_DEBUG("WS SEND Buffering (%d): buffer=%zd sent=%d error=%d send_buf=%zd\n", (int)self->socket, buf_len, ret, esif_ccb_socket_error(), self->sendBufLen);
		}

		// Close Socket on Error
		if (rc != ESIF_OK) {
			WS_TRACE_DEBUG("WS SEND Failure (%d): buffer=%zd sent=%d error=%d send_buf=%zd [%zd total]\n", (int)self->socket, buf_len, ret, esif_ccb_socket_error(), self->sendBufLen, self->sendBufLen + buf_len);
			WebClient_Close(self);
		}
	}
	return rc;
}

//// WebServer Object Methods ////

// Initialize WebServer Object
void WebServer_Init(WebServerPtr self)
{
	int j = 0;
	if (self) {
		esif_ccb_lock_init(&self->lock);
		TcpDoorbell_Init(&self->doorbell);
		for (j = 0; j < WS_MAX_LISTENERS; j++) {
			WebListener_Init(&self->listeners[j]);
		}
		for (j = 0; j < WS_MAX_CLIENTS; j++) {
			WebClient_Init(&self->clients[j]);
		}
		atomic_set(&self->isActive, 0);
		atomic_set(&self->activeThreads, 0);
		self->netBuf = NULL;
		self->netBufLen = 0;
	}
}

// Close Web Server Objects
void WebServer_Close(WebServerPtr self)
{
	if (self) {
		int j = 0;
		TcpDoorbell_Close(&self->doorbell);
		for (j = 0; j < WS_MAX_LISTENERS; j++) {
			WebListener_Close(&self->listeners[j]);
		}
		for (j = 0; j < WS_MAX_CLIENTS; j++) {
			WebClient_Close(&self->clients[j]);
		}
		esif_ccb_free(self->netBuf);
		self->netBuf = NULL;
		self->netBufLen = 0;
		atomic_set(&self->isActive, 0);
	}
}

// Unitialize Web Server
void WebServer_Exit(WebServerPtr self)
{
	if (self) {
		WebServer_Stop(self);
		WebServer_Close(self);
		esif_ccb_lock_uninit(&self->lock);
	}
}

// Configure Web Server
esif_error_t WebServer_Config(WebServerPtr self, u8 instance, char *ipAddr, short port, esif_flags_t flags)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self && instance < WS_MAX_LISTENERS) {
		rc = WebListener_Config(&self->listeners[instance], ipAddr, port, flags);
	}
	return rc;
}

// Receive and Process a Client Request, Buffering message if necessary
esif_error_t WebServer_ProcessRequest(WebServerPtr self, WebClientPtr client)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && client && self->netBuf && self->netBufLen > 0) {
		u8 *buffer = self->netBuf;
		size_t buf_len = self->netBufLen;
		ssize_t messageLength = 0;
		u8 *messageBuffer = NULL;

		esif_ccb_memset(self->netBuf, 0, self->netBufLen);
		rc = ESIF_OK;

		// Read the next partial or complete message fragment from the client socket.
		messageLength = recv(client->socket, (char*)buffer, (int)buf_len, WS_RECV_FLAGS);
		if (messageLength == 0 || messageLength == SOCKET_ERROR) {
			int errnum = 0;
			if ((errnum = esif_ccb_socket_error()) != ESIF_SOCKERR_EWOULDBLOCK) {
				rc = ESIF_E_WS_DISC;
			}
		}
		if (rc == ESIF_OK) {
			WS_TRACE_DEBUG("Socket[%d] Received %d bytes\n", client->socket, (int)messageLength);

			// Combine this partial frame with the current connection's RECV Buffer, if any
			if (client->recvBuf != NULL && client->recvBufLen > 0) {
				size_t total_buffer_len = client->recvBufLen + messageLength;
				if (total_buffer_len <= WS_MAX_CLIENT_RECVBUF) {
					messageBuffer = esif_ccb_realloc(client->recvBuf, total_buffer_len);
				}
				if (messageBuffer == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					WS_TRACE_DEBUG("WS Frame Unbuffering: buflen=%zd msglen=%zd total=%zd net=%zd\n", client->recvBufLen, messageLength, total_buffer_len, self->netBufLen);
					esif_ccb_memcpy(messageBuffer + client->recvBufLen, self->netBuf, messageLength);
					buffer = messageBuffer;
					buf_len = total_buffer_len;
					messageLength = total_buffer_len;
					client->recvBuf = NULL;
					client->recvBufLen = 0;
				}
			}

			// Process Request for Connection current Protocol Type
			if (rc == ESIF_OK) {
				switch (client->type) {
				case ClientHttp:
					rc = WebServer_HttpRequest(self, client, buffer, messageLength);
					break;
				case ClientWebsocket:
					rc = WebServer_WebsocketRequest(self, client, buffer, messageLength);
					break;
				default:
					rc = ESIF_E_WS_DISC;
					break;
				}
			}

			// Process Incomplete Requests after more data is received
			if (rc == ESIF_E_WS_INCOMPLETE) {
				rc = ESIF_OK;
			}
		}
		esif_ccb_free(messageBuffer);
	}
	return rc;
}

// Web Server Main Module (One per Thread)
static esif_error_t WebServer_Main(WebServerPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	if (self) {
		struct timeval tv = { 0 };	// Timeout
		fd_set readFDs = { 0 };		// Readable Sockets List
		fd_set writeFDs = { 0 };	// Writable Sockets List
		fd_set exceptFDs = { 0 };	// Exception Sockets List

		int selectResult = 0;		// select() result
		int maxfd = 0;				// Max file descriptor ID + 1
		int setsize = 0;			// Number of items in FD List
		int j = 0;

		atomic_inc(&self->activeThreads);
		rc = ESIF_OK;
	
		// Allocate Network Buffer
		self->netBufLen = WS_NETWORK_BUFFER_LEN;
		self->netBuf = esif_ccb_malloc(self->netBufLen);
		if (self->netBuf == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}

		// Create Doorbell Socket Pair
		if (rc == ESIF_OK) {
			rc = TcpDoorbell_Open(&self->doorbell);
		}

		// Create Listener Socket(s)
		for (j = 0; rc == ESIF_OK && j < WS_MAX_LISTENERS; j++) {
			if (self->listeners[j].ipAddr[0] && self->listeners[j].port > 0) {
				rc = WebListener_Open(&self->listeners[j]);
			}
		}

		//// MAIN LOOP ////
		
		// Process all active sockets and accept new connections until Quit signaled
		while (rc == ESIF_OK && atomic_read(&self->isActive)) {

			// Reset File Descriptor Lists after each iteration
			FD_ZERO(&readFDs);
			FD_ZERO(&writeFDs);
			FD_ZERO(&exceptFDs);
			maxfd = 0;
			setsize = 0;

			// Add Doorbell Ringer
			FD_SET(self->doorbell.sockets[DOORBELL_RINGER], &readFDs);
			FD_SET(self->doorbell.sockets[DOORBELL_RINGER], &exceptFDs);
			maxfd = (int)self->doorbell.sockets[DOORBELL_RINGER] + 1;
			setsize++;

			// Add Listener Socket(s)
			for (j = 0; j < WS_MAX_LISTENERS && setsize < WS_MAX_SOCKETS; j++) {
				if (self->listeners[j].socket != INVALID_SOCKET) {
					FD_SET(self->listeners[j].socket, &readFDs);
					FD_SET(self->listeners[j].socket, &exceptFDs);
					maxfd = esif_ccb_max(maxfd, (int)self->listeners[j].socket + 1);
					setsize++;
				}
			}

			// Add Client Socket(s)
			for (j = 0; j < WS_MAX_CLIENTS && setsize < WS_MAX_SOCKETS; j++) {
				if (self->clients[j].socket != INVALID_SOCKET) {
					FD_SET(self->clients[j].socket, &readFDs);
					FD_SET(self->clients[j].socket, &exceptFDs);
					maxfd = esif_ccb_max(maxfd, (int)self->clients[j].socket + 1);
					setsize++;

					// Wait for socket to become writable if pending send buffer
					if (self->clients[j].sendBuf) {
						FD_SET(self->clients[j].socket, &writeFDs);
					}
				}
			}

			// Use Timeout of N + 0.05 seconds where >= 2 (Since UI polls every second)
			tv.tv_sec = WS_SOCKET_TIMEOUT;
			tv.tv_usec = 50000;

			//// WAIT FOR SOCKET ACTIVITY ////

			selectResult = select(maxfd, &readFDs, &writeFDs, &exceptFDs, &tv);

			// Exit loop if select error or server stopping; continue loop if inactivity timeout
			if (selectResult == SOCKET_ERROR) {
				WS_TRACE_ERROR("SELECT Error (%d)\n", selectResult);
				rc = ESIF_E_WS_SOCKET_ERROR;
				break;
			}
			else if (!atomic_read(&self->isActive)) { // Exit if Server not Active
				break;
			}
			else if (selectResult == 0) { // Timeout
				continue;
			}

			//// PROCESS ALL ACTIVE SOCKETS ////

			// 1. Respond to Incoming Doorbell Socket signals
			if (FD_ISSET(self->doorbell.sockets[DOORBELL_RINGER], &readFDs)) {
				u8 opcode = WS_OPCODE_NOOP;

				// Exit if Doorbell socket(s) shutdown or a QUIT opcode received
				if (TcpDoorbell_Receive(&self->doorbell, &opcode) != ESIF_OK) {
					WS_TRACE_DEBUG("Doorbell Closed; Exiting");
					break;
				}
				WS_TRACE_DEBUG("Doorbell Received: 0x%02X", ((int)opcode & 0xFF));
				if (opcode == WS_OPCODE_QUIT) {
					break;
				}
			}

			// 2. Accept any new connections on the Listener Socket(s)
			int sockets = setsize;
			for (j = 0; j < WS_MAX_LISTENERS && atomic_read(&self->isActive); j++) {
				WebListenerPtr listener = &self->listeners[j];
				if (listener->socket != INVALID_SOCKET && FD_ISSET(listener->socket, &readFDs)) {
					esif_ccb_socket_t clientSocket = INVALID_SOCKET;
					WebClientPtr client = NULL;
					int k = 0;

					// Find first Unused Client
					for (k = 0; k < WS_MAX_CLIENTS && sockets < WS_MAX_SOCKETS; k++) {
						if (self->clients[k].type == ClientClosed) {
							client = &self->clients[k];
							break;
						}
					}

					// Accept Incoming Connection
					char *clientIpAddr = NULL;
					if ((rc = WebListener_AcceptClient(listener, &clientSocket, &clientIpAddr)) != ESIF_OK) {
						WS_TRACE_DEBUG("Listener[%d] Socket[%d]: Accept Error: %s (%d)\n", j, listener->socket, esif_rc_str(rc), rc);
					}
					else {
						// Close Client if Max Connections exceeded
						if (client == NULL) {
							WS_TRACE_WARNING("Connection Limit Exceeded (%d)\n", WS_MAX_CLIENTS);
							esif_ccb_socket_close(clientSocket);
							clientSocket = INVALID_SOCKET;
							continue;
						}

						// Client is now an HTTP Connection
						client->type = ClientHttp;
						client->socket = clientSocket;
						client->ipAddr = esif_ccb_strdup(clientIpAddr ? clientIpAddr : "NA");
						clientSocket = INVALID_SOCKET;
						sockets++;	
						WS_TRACE_DEBUG("Accepted Client[%d]: Socket[%d]\n", k, (int)client->socket);
					}
				}
			}

			// 3. Process Active Client Requests
			for (j = 0; j < WS_MAX_CLIENTS && atomic_read(&self->isActive); j++) {
				WebClientPtr client = &self->clients[j];
				if (client->socket != INVALID_SOCKET) {

					// Close sockets with Exceptions
					if (client->socket != INVALID_SOCKET && FD_ISSET(client->socket, &exceptFDs)) {
						WebClient_Close(client);
						WS_TRACE_DEBUG("Closing Client[%d]: (Exception) Socket[%d]\n", j, (int)client->socket);
						continue;
					}

					// Receive and Process Requests from Readable Clients
					if (client->socket != INVALID_SOCKET && FD_ISSET(client->socket, &readFDs)) {
						if ((rc = WebServer_ProcessRequest(self, client)) != ESIF_OK) {
							WebClient_Close(client);
							WS_TRACE_DEBUG("Client[%d] Disconnected: %s (%d)\n", j, esif_rc_str(rc), (int)rc);
						}
					}

					// Flush pending Send Buffer when socket becomes writable
					if (client->socket != INVALID_SOCKET && FD_ISSET(client->socket, &writeFDs)) {
						size_t sendBufLen = client->sendBufLen;
						UNREFERENCED_PARAMETER(sendBufLen);
						rc = WebClient_Write(client, NULL, 0);
						WS_TRACE_DEBUG("WS SEND Unbuffering (%d): before=%zd after=%zd\n", (int)client->socket, sendBufLen, client->sendBufLen);
					}
				}
			}
			rc = ESIF_OK;
		}

		// Cleanup
		WebServer_Close(self);
		atomic_dec(&self->activeThreads);
	}
	return rc;
}

// Web Server Main Worker Thread Wrapper
static void *ESIF_CALLCONV WebServer_WorkerThread(void *ctx)
{
	WebServerPtr self = (WebServerPtr)ctx;
	WS_TRACE_INFO("Web Server Worker Thread Starting");
	WebServer_Main(self);
	WS_TRACE_INFO("Web Server Worker Thread Exiting");
	return 0;
}

// Start Web Server
esif_error_t WebServer_Start(WebServerPtr self)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;;
	if (self) {
		if (atomic_read(&self->isActive) || atomic_read(&self->activeThreads)) {
			rc = ESIF_E_WS_ALREADY_STARTED;
		}
		else {
			int j = 0;
			atomic_set(&self->isActive, 1);
			for (j = 0; j < WS_MAX_LISTENERS; j++) {
				if (self->listeners[j].port) {
					EsifWsConsoleMessageEx("Starting web server: http://%s:%hd\n", self->listeners[j].ipAddr, self->listeners[j].port);
				}
			}
			rc = esif_ccb_thread_create(&self->mainThread, WebServer_WorkerThread, self);
		}
	}
	return rc;
}

// Stop Web Server and wait for Active Thread(s) to exit
void WebServer_Stop(WebServerPtr self)
{
	if (self && (atomic_read(&self->isActive) || atomic_read(&self->activeThreads))) {
		atomic_set(&self->isActive, 0);

		// Stop Doorbell to signal blocking select() to exit
		TcpDoorbell_Stop(&self->doorbell);

		// Wait for worker thread to exit
		esif_ccb_thread_join(&self->mainThread);
	}
}

// Is WebServer Started or Starting or Stopping?
Bool WebServer_IsStarted(WebServerPtr self)
{
	Bool rc = ESIF_FALSE;
	if (self && (atomic_read(&self->isActive) || atomic_read(&self->activeThreads))) {
		rc = ESIF_TRUE;
	}
	return rc;
}

// Initialize Plugin
esif_error_t WebPlugin_Init()
{
	esif_error_t rc = ESIF_OK;
	int ret = 0;

	if (((ret = esif_ccb_socket_init()) != 0) || ((ret = esif_link_list_init()) != 0)) {
		rc = ESIF_E_WS_INIT_FAILED;
	}
	else if ((g_WebServer = esif_ccb_malloc(sizeof(*g_WebServer))) == NULL) {
		rc = ESIF_E_NO_MEMORY;
	}

	if (rc == ESIF_OK) {
		WebServer_Init(g_WebServer);
	}
	else {
		WS_TRACE_ERROR("Socket Init Failure: %s (%d) [Error=%d]\n", esif_rc_str(rc), rc, ret);
	}
	return rc;
}

// Exit Plugin
void WebPlugin_Exit()
{
	if (g_WebServer) {
		WebServer_Exit(g_WebServer);
		esif_ccb_free(g_WebServer);
		g_WebServer = NULL;
	}
	esif_link_list_exit();
	esif_ccb_socket_exit();
}
