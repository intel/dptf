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
#define ESIF_TRACE_ID ESIF_TRACEMODULE_WEBSERVER
#include <ctype.h>
#include "esif_ws_socket.h"
#include "esif_ws_http.h"
#include "esif_ws_server.h"
#include "esif_ccb_atomic.h"

#include "esif_uf_ccb_file.h"
#include "esif_uf_shell.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define BUFFER_LENGTH 0xFFFF

#define MESSAGE_SUCCESS 0
#define MESSAGE_ERROR 1

char *g_rest_out = NULL;

/* Max number of Client connections. This cannot exceed FD_SETSIZE (Deafult: Windows=64, Linux=1024) */
#define MAX_CLIENTS		10

/*
 *******************************************************************************
 ** EXTERN
 *******************************************************************************
 */
void esif_ws_close_socket(clientRecord *connection);
extern int g_shell_enabled; 

/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */
static int esif_ws_server_create_inet_addr (void*, socklen_t*, char*, char*, char*);
static eEsifError esif_ws_server_process_request (clientRecord *);
static int esif_ws_server_write_to_socket (clientRecord *, const UInt8*, size_t);
static void esif_ws_server_setup_buffer (size_t*, UInt8*);
static void esif_ws_server_setup_frame (clientRecord*, size_t*, UInt8*);

static void esif_ws_server_set_rest_api (clientRecord *, const char*, const size_t);
static char*esif_ws_server_get_rest_api (void);

static int esif_ws_server_get_websock_msg (clientRecord *);

static void esif_ws_server_initialize_connection(clientRecord *connection);
static void esif_ws_server_initialize_clients (void);

static clientRecord *g_client = NULL; /* dynamically allocated array of MAX_CLIENTS */
static UInt8 *g_ws_http_buffer = NULL; /* dynamically allocated buffer of size BUFFER_LENGTH */
static esif_ccb_mutex_t g_web_socket_lock;
static atomic_t g_ws_quit = 0;
atomic_t g_ws_threads = 0;

/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */
#define MAX_IPADDR 20
char g_ws_ipaddr[MAX_IPADDR]= "0.0.0.0";
char g_ws_port[MAX_IPADDR] = "8888";

esif_ccb_socket_t g_listen = INVALID_SOCKET;

int esif_ws_init(void)
{
	int index=0;
	int retVal=0;
	char *ipaddr = (char*)g_ws_ipaddr;
	char *port = g_ws_port;

	struct sockaddr_in addrSrvr = {0};
	struct sockaddr_in addrClient = {0};
	socklen_t len_inet = 0;
	
	esif_ccb_socket_t client_socket = INVALID_SOCKET;
	
	int option = 1;
	eEsifError req_results = ESIF_OK;
	clientRecord *connection = NULL;

	int selRetVal = 0;
	int maxfd = 0;
	int setsize = 0;
	
	struct timeval tv={0}; 	/* Timeout value */
	fd_set workingSet = {0};

	esif_ccb_mutex_init(&g_web_socket_lock);
	atomic_inc(&g_ws_threads);
	atomic_set(&g_ws_quit, 0);

	CMD_OUT("Starting WebServer %s %s\n", ipaddr, port);

	esif_ccb_socket_init();

	// Allocate pool of Client Records and HTTP input buffer
	g_client = (clientRecord *)esif_ccb_malloc(MAX_CLIENTS * sizeof(clientRecord));
	g_ws_http_buffer = (UInt8 *)esif_ccb_malloc(BUFFER_LENGTH);
	if (NULL == g_client || NULL == g_ws_http_buffer) {
		ESIF_TRACE_DEBUG("Out of memory");
		goto exit;
	}

	esif_ws_server_initialize_clients();

	len_inet = sizeof(addrSrvr);
	retVal   = esif_ws_server_create_inet_addr(&addrSrvr, &len_inet, ipaddr, port, (char*)"top");

	if (retVal < 0 && !addrSrvr.sin_port) {
		ESIF_TRACE_DEBUG("Invalid server address/port number");
		goto exit;
	}

	g_listen = socket(PF_INET, SOCK_STREAM, 0);

	if (g_listen == SOCKET_ERROR) {
		ESIF_TRACE_DEBUG("open socket error");
		goto exit;
	}
	
	retVal = setsockopt(g_listen, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

	if (retVal < 0) {
		esif_ccb_socket_close(g_listen);
		g_listen = INVALID_SOCKET;
		ESIF_TRACE_DEBUG("setsockopt failed");
		goto exit;
	}

	retVal = bind(g_listen, (struct sockaddr*)&addrSrvr, len_inet);

	if (retVal < -1) {
		esif_ccb_socket_close(g_listen);
		g_listen = INVALID_SOCKET;
		ESIF_TRACE_DEBUG("bind sysem call failed");
		goto exit;
	}
	
	retVal = listen(g_listen, 10);

	if (retVal < 0) {
		ESIF_TRACE_DEBUG("listen system call failed");
		goto exit;
	}

	/* Accept client requests and new connections until told to quit */
	while (!atomic_read(&g_ws_quit)) {
	
		/* Build file descriptor set of active sockets */
		maxfd = 0;
		setsize = 0;
		FD_ZERO(&workingSet);
		if (g_listen != INVALID_SOCKET) {
			FD_SET((u_int)g_listen, &workingSet);
			maxfd = (int)g_listen + 1;
			setsize++;
		}
		for (index = 0; index <  MAX_CLIENTS && setsize < FD_SETSIZE; index++) {
			if (g_client[index].socket != INVALID_SOCKET) {
				FD_SET((u_int)g_client[index].socket, &workingSet);
				maxfd = esif_ccb_max(maxfd, (int)g_client[index].socket + 1);
				setsize++;
			}
		}
		if (maxfd == 0) {
			break;
		}

		/*
		 *  timeout of N + 0.05 secs
		 */
		tv.tv_sec  = 2;
		tv.tv_usec = 50000;
		selRetVal  = select(maxfd, &workingSet, NULL, NULL, &tv);

		if (selRetVal == SOCKET_ERROR) {
			break;
		} else if (!selRetVal) {
			continue;
		}

		/* Accept any new connections on the listening socket */
		if (FD_ISSET(g_listen, &workingSet)) {
			int sockets = (g_listen == INVALID_SOCKET ? 0 : 1);
			len_inet = sizeof addrClient;

			client_socket = (int)accept(g_listen, (struct sockaddr*)&addrClient, &len_inet);

			if (client_socket == SOCKET_ERROR) {
				ESIF_TRACE_DEBUG("accept(2)");
				goto exit;
			}

			/* find the first available connection */
			for (index = 0; index < MAX_CLIENTS && sockets < FD_SETSIZE; index++) {
				if (g_client[index].socket == INVALID_SOCKET) {
					esif_ws_server_initialize_connection(&g_client[index]);
					g_client[index].socket = client_socket;
					break;
				}
				sockets++;
			}

			/* No more connections available */
			if (index >= MAX_CLIENTS || sockets >= FD_SETSIZE) {
				ESIF_TRACE_DEBUG("Connection Limit Exceeded (%d)", MAX_CLIENTS);
				esif_ccb_socket_close(client_socket);
				client_socket = INVALID_SOCKET;
				continue;
			}
		}

		/* process all active client requests */
		for (index = 0; index < MAX_CLIENTS; index++) {
			client_socket = g_client[index].socket;
			if (client_socket == INVALID_SOCKET || client_socket == g_listen) {
				continue;
			}
			connection = &g_client[index];

			/*
			* Process connection if client_socket is in the set of active file descriptors
			*/
			if (FD_ISSET(client_socket, &workingSet)) {
				ESIF_TRACE_DEBUG("Client %d connected\n", client_socket);

				req_results = esif_ws_server_process_request(connection);

				if (req_results == ESIF_E_WS_DISC) {
					ESIF_TRACE_DEBUG("Client %d disconnected\n", client_socket);
					esif_ws_server_initialize_connection(connection); /* reset */
				}
				else if (req_results == ESIF_E_NO_MEMORY) {
					ESIF_TRACE_DEBUG("Out of memory\n");
					esif_ws_server_initialize_connection(connection); /* reset */
				}
				esif_ws_socket_initialize_frame(&connection->prot);
				esif_ws_socket_initialize_message_buffer(&connection->buf);
			}
		}
	}

exit:
	/* cleanup */
	if (g_listen != INVALID_SOCKET) {
		esif_ccb_socket_close(g_listen);
		g_listen = INVALID_SOCKET;
	}
	if (g_client) {
		for (index = 0; index < MAX_CLIENTS; index++) {
			esif_ws_close_socket(&g_client[index]);
		}
		esif_ccb_free(g_client);
		g_client = NULL;
	}
	esif_ccb_free(g_rest_out);
	esif_ccb_free(g_ws_http_buffer);
	g_rest_out = NULL;
	g_ws_http_buffer = NULL;
	esif_ccb_socket_exit();
	atomic_dec(&g_ws_threads);
	return 0;
}

/* stop web server and wait for worker threads to exit */
void esif_ws_exit (esif_thread_t *web_thread)
{
	CMD_OUT("Stopping WebServer...\n");
	atomic_set(&g_ws_quit, 1);
	esif_ccb_thread_join(web_thread);  /* join to close child thread, clean up handle */
	// Wait for worker thread to finish
	while (atomic_read(&g_ws_threads) > 0) {
		esif_ccb_sleep(1);
	}
	esif_ccb_mutex_uninit(&g_web_socket_lock);
	atomic_set(&g_ws_quit, 0);
	CMD_OUT("WebServer Stopped\n");
}

void esif_ws_set_ipaddr_port(const char *ipaddr, u32 port)
{
	if (ipaddr) {
		esif_ccb_strcpy(g_ws_ipaddr, ipaddr, sizeof(g_ws_ipaddr));
	}
	if (port) {
		esif_ccb_sprintf(sizeof(g_ws_port), g_ws_port, "%d", port);
	}
}


int esif_ws_server_get_websock_msg (clientRecord *connection)
{
	char *mes = esif_ws_server_get_rest_api();
	ESIF_TRACE_DEBUG("Message Received: %s \n",mes);
	if (mes == NULL)
	{
		return MESSAGE_ERROR;
	}
	else if (esif_ccb_strlen(mes, BUFFER_LENGTH) == 0)
	{
		return MESSAGE_ERROR;
	}
	connection->buf.msgSend = (char*)esif_ccb_malloc(esif_ccb_strlen(mes, BUFFER_LENGTH) + 1);
	esif_ccb_memcpy(connection->buf.msgSend, mes, esif_ccb_strlen(mes, BUFFER_LENGTH));
	connection->buf.sendSize = (UInt32)esif_ccb_strlen(mes, BUFFER_LENGTH);
	return MESSAGE_SUCCESS;
}

void esif_ws_server_set_rest_api (
	clientRecord *connection,
	const char *esif_ws_server_rest_api,
	const size_t dataSize
	)
{
	char *recv_buf    = NULL;
	char *command_buf = NULL;

	if (atomic_read(&g_ws_quit))
		return;

	connection->buf.msgReceive = (char*)esif_ccb_malloc(dataSize + 1);
	if (NULL == connection->buf.msgReceive) {
		return;
	}
	esif_ccb_memcpy(connection->buf.msgReceive, esif_ws_server_rest_api, dataSize);
	connection->buf.rcvSize    = (UInt32)dataSize;
	connection->buf.msgReceive[dataSize] = 0;

	/* Grab Message ID */
	recv_buf    = connection->buf.msgReceive;

	command_buf = strchr(recv_buf, ':');
	if (NULL != command_buf) {
		u32 msg_id = atoi(recv_buf);
		*command_buf = 0;
		command_buf++;

		// Ad-Hoc UI Shell commands begin with "0:", so verify ESIF shell is enabled and command is valid
		if (msg_id == 0) {
			char *response = NULL;
			if (!g_shell_enabled) {
				response = "0:Shell Disabled";
			}
			else {
				static char *unsupported[] = {"shell", "web", "exit", "quit", NULL};
				int j=0;
				for (j = 0; unsupported[j] != NULL; j++) {
					if (esif_ccb_strnicmp(command_buf, unsupported[j], esif_ccb_strlen(unsupported[j], MAX_PATH)) == 0) {
						response = "0:Unsupported Command";
						break;
					}
				}
			}
			// Exit if shell or command disabled
			if (response) {
				esif_ccb_free(g_rest_out);
				g_rest_out = esif_ccb_strdup(response);
				goto exit;
			}
		}

		// Lock Shell so we can capture output before another thread executes another command
#ifdef ESIF_ATTR_SHELL_LOCK
		esif_ccb_mutex_lock(&g_shellLock);
#endif
		if (!atomic_read(&g_ws_quit)) {
			EsifString cmd_results = esif_shell_exec_command(command_buf, dataSize, ESIF_TRUE);
			if (NULL != cmd_results) {
				size_t out_len = esif_ccb_strlen(cmd_results, OUT_BUF_LEN) + 12;
				esif_ccb_free(g_rest_out);
				g_rest_out = (EsifString) esif_ccb_malloc(out_len);
				if (g_rest_out) {
					esif_ccb_sprintf(out_len, g_rest_out, "%u:%s", msg_id, cmd_results);
				}
			}
		}
#ifdef ESIF_ATTR_SHELL_LOCK
		esif_ccb_mutex_unlock(&g_shellLock);
#endif
	}

exit:
	esif_ccb_free(connection->buf.msgReceive);
	connection->buf.msgReceive = NULL;
	connection->buf.rcvSize = 0;
}


char*esif_ws_server_get_rest_api (void)
{
	return g_rest_out;
}


static int esif_ws_server_create_inet_addr (
	void *addr,
	socklen_t *addrlen,
	char *host,
	char *port,
	char *protocol
	)
{
	struct sockaddr_in *sockaddr_inPtr = (struct sockaddr_in*)addr;
	struct hostent *hostenPtr  = NULL;
	struct servent *serventPtr = NULL;

	char *endStr;
	long longVal;

	if (!host) {
		host = (char*)"*";
	}

	if (!port) {
		port = (char*)"*";
	}

	if (!protocol) {
		protocol = (char*)"tcp";
	}


	esif_ccb_memset(sockaddr_inPtr, 0, *addrlen);
	sockaddr_inPtr->sin_family = AF_INET;
	sockaddr_inPtr->sin_port   = 0;
	sockaddr_inPtr->sin_addr.s_addr = INADDR_ANY;


	if (esif_ccb_strcmp(host, "*") == 0) {
		;
	} else if (isdigit(*host)) {
		sockaddr_inPtr->sin_addr.s_addr = inet_addr(host);

		if (sockaddr_inPtr->sin_addr.s_addr == INADDR_NONE) {
			return -1;
		}
	} else {
		hostenPtr = gethostbyname(host);

		if (!hostenPtr) {
			return -1;
		}

		if (hostenPtr->h_addrtype != AF_INET) {
			return -1;
		}

		sockaddr_inPtr->sin_addr = *(struct in_addr*)

			hostenPtr->h_addr_list[0];
	}

	if (!esif_ccb_strcmp(port, "*")) {
		;
	} else if (isdigit(*port)) {
		longVal = strtol(port, &endStr, 10);
		if (endStr != NULL && *endStr) {
			return -2;
		}

		if (longVal < 0L || longVal >= 32768) {
			return -2;
		}

		sockaddr_inPtr->sin_port = htons((short)longVal);
	} else {
		serventPtr = getservbyname(port, protocol);
		if (!serventPtr) {
			return -2;
		}

		sockaddr_inPtr->sin_port = (short)serventPtr->s_port;
	}


	*addrlen = sizeof *sockaddr_inPtr;

	return 0;
}


static int esif_ws_server_write_to_socket (
	clientRecord *connection,
	const UInt8 *buffer,
	size_t bufferSize
	)
{
	ssize_t ret=0;
	
	ret = send(connection->socket, (char*)buffer, (int)bufferSize, 0);
	if (ret == -1 || ret != (ssize_t)bufferSize) {
		esif_ccb_socket_close(connection->socket);
		connection->socket = INVALID_SOCKET;
		ESIF_TRACE_DEBUG("%s", (ret == -1 ? "error in sending packets\n" : "incomplete data\n"));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/* This function processes requests for either websocket connections or
 * http connections.
 */
static eEsifError esif_ws_server_process_request (clientRecord *connection)
{

	size_t numBytesRcvd    = 0;
	size_t frameSize       = BUFFER_LENGTH;
	UInt8 *data 		   = NULL;
	size_t dataSize        = 0;
	size_t bytesRemaining  = 0;
	UInt8 *bufferRemaining = NULL;
	UInt8 *received_string = NULL;
	ssize_t messageLength  = 0;
	eEsifError result = ESIF_OK;
	
	esif_ccb_memset(g_ws_http_buffer, 0, BUFFER_LENGTH);

	if (connection->frameType == INCOMPLETE_FRAME) {
		/*Pull messages from the client socket store length lenght of recieved message */
		messageLength = recv(connection->socket, (char*)g_ws_http_buffer + numBytesRcvd, BUFFER_LENGTH - (int)numBytesRcvd, 0);
		if (messageLength == 0 || messageLength == SOCKET_ERROR) {
			ESIF_TRACE_DEBUG("no messages received from the socket\n");
			result =  ESIF_E_WS_DISC;
		} else {
			ESIF_TRACE_DEBUG("%d bytes received\n", (int)messageLength);
		}


		/*Add the length of the message of the number of characters received */
		numBytesRcvd += messageLength;

		/*Is the socket in its opening state? */
		if (connection->state == STATE_OPENING) {
			ESIF_TRACE_DEBUG("socket in its opening state\n");
			/*Determine the initial frame type:  http frame type or websocket frame type */
			connection->frameType = esif_ws_get_initial_frame_type(g_ws_http_buffer, numBytesRcvd, &connection->prot);

			/*Is the frame type http frame type? */
			if (connection->frameType == HTTP_FRAME) {				
				result = esif_ws_http_process_reqs(connection, g_ws_http_buffer, (ssize_t)numBytesRcvd);
			}
		}
		else {
			/*
			 * The socket is not in its opening state nor does it have an http frame type
			 * Therefore, determine the frame type
			 */

more_data:
			connection->frameType = esif_ws_socket_get_subsequent_frame_type(g_ws_http_buffer, numBytesRcvd, &data, &dataSize, &bytesRemaining);
			ESIF_TRACE_DEBUG("frameType: %d\n", connection->frameType);

			/* Save remaining frames for reparsing if more than one frame received */
			if (bytesRemaining > 0) {
				if (bufferRemaining == NULL) {
					bufferRemaining = (UInt8 *)esif_ccb_malloc(bytesRemaining);
					if (NULL == bufferRemaining) {
						result = ESIF_E_NO_MEMORY;
						goto exit;
					}
				}
				esif_ccb_memcpy(bufferRemaining, data+dataSize, bytesRemaining);
			}
			else {
				esif_ccb_free(bufferRemaining);
				bufferRemaining = NULL;
			}

			/* Handle unsolicited PONG (keepalive) messages from Internet Explorer 10 */
			if (messageLength >= 6 && connection->frameType == PONG_FRAME) {
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
				esif_ws_socket_build_payload((UInt8*)"", 0, g_ws_http_buffer, &frameSize, TEXT_FRAME);
				esif_ws_server_write_to_socket(connection, g_ws_http_buffer, frameSize);
				esif_ws_server_setup_frame(connection, &frameSize, g_ws_http_buffer);
			}
		}

		/*Now, if the frame type is an incomplete type or if it is an error type of frame */
		if ((connection->frameType == INCOMPLETE_FRAME && numBytesRcvd == BUFFER_LENGTH) ||
			connection->frameType == ERROR_FRAME) {
			/*Is the frame type an incomplete frame type */
			if (connection->frameType == INCOMPLETE_FRAME) {
				ESIF_TRACE_DEBUG("websocket: incomplete frame received\n");
			} else {
				ESIF_TRACE_DEBUG("webscocket: improper format for frame\n");
			}

			/*
			 * If the socket frame type is in error or is incomplete and happens to
			 * be in its opening state, send a message to the client that the request is bad
			 */
			if (connection->state == STATE_OPENING) {
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
				frameSize = esif_ccb_sprintf(BUFFER_LENGTH, (char*)g_ws_http_buffer,
											 "HTTP/1.1 400 Bad Request\r\n%s%s\r\n\r\n", "Sec-WebSocket-Version: ", "13");

				esif_ws_server_write_to_socket(connection, g_ws_http_buffer, frameSize);
				ESIF_TRACE_DEBUG("websocket: error writing to socket line %d\n", __LINE__);
				result =  ESIF_E_WS_DISC;
				goto exit;
			}
			else {
				/*
				 * If the socket is not in its opening state while its frame type is in error or is incomplete
				 * setup a g_ws_http_buffer to store the payload to send to the client
				 */
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
				esif_ws_socket_build_payload(NULL, 0, g_ws_http_buffer, &frameSize, CLOSING_FRAME);
				if (esif_ws_server_write_to_socket(connection, g_ws_http_buffer, frameSize) == EXIT_FAILURE) {
					ESIF_TRACE_DEBUG("websocket: error writing to socket line %d\n", __LINE__);
					result =   ESIF_E_WS_DISC;
					return result;
				}

				/*
				 * Force the socket state into its closing state
				 */
				connection->state = STATE_CLOSING;
				esif_ws_server_setup_frame(connection, &frameSize, g_ws_http_buffer);
			}
		}

		if (connection->state == STATE_OPENING) {
			if (connection->frameType == OPENING_FRAME) {
				/*Validate the resource */
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
				if (esif_ws_socket_build_response_header(&connection->prot, g_ws_http_buffer, &frameSize) != 0)	{
					ESIF_TRACE_DEBUG("websocket: nable to build response header line %d\n", __LINE__);
					result =   ESIF_E_WS_DISC;
					goto exit;
				}

				if (esif_ws_server_write_to_socket(connection, g_ws_http_buffer, frameSize) == EXIT_FAILURE) {
					ESIF_TRACE_DEBUG("websocket: error writing to socket line %d\n", __LINE__);
					result =   ESIF_E_WS_DISC;
					goto exit;
				}

				/*This is a websocket connection*/
				connection->state = STATE_NORMAL;
				esif_ws_server_setup_frame(connection, &frameSize, g_ws_http_buffer);
			}
		}
		else {
			/*Falls though here when requesting to disconnect*/
			if (connection->frameType == CLOSING_FRAME) {
				if (connection->state == STATE_CLOSING) {
					ESIF_TRACE_DEBUG("websocket: receive fails line %d\n", __LINE__);
				} else {
					esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
					esif_ws_socket_build_payload(NULL, 0, g_ws_http_buffer, &frameSize, CLOSING_FRAME);
					esif_ws_server_write_to_socket(connection, g_ws_http_buffer, frameSize);
				}

				esif_ccb_free(connection->prot.keyField);
				connection->prot.keyField = NULL;
				result =   ESIF_E_WS_DISC;
				goto exit;

			} else if (connection->frameType == TEXT_FRAME) {
				received_string = (UInt8*)esif_ccb_malloc(dataSize + 1);
				if (NULL == received_string) {
					result = ESIF_E_NO_MEMORY;
					goto exit;
				}
				esif_ccb_memcpy(received_string, data, dataSize);
				received_string[dataSize] = 0;

				/* Save incoming socket message if needed */

				esif_ws_server_set_rest_api(connection, (const char*)received_string, esif_ccb_strlen((const char*)received_string, BUFFER_LENGTH));
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);

				/* Get message to send*/

				if (MESSAGE_SUCCESS == esif_ws_server_get_websock_msg(connection))
				{
					esif_ws_socket_build_payload((UInt8*)connection->buf.msgSend, connection->buf.sendSize, g_ws_http_buffer, &frameSize, TEXT_FRAME);
					esif_ccb_free(connection->buf.msgSend);
					connection->buf.msgSend = NULL;
					connection->buf.sendSize = 0;
					
					if (esif_ws_server_write_to_socket(connection, g_ws_http_buffer, frameSize) == EXIT_FAILURE) {
						ESIF_TRACE_DEBUG("websocket: error writing to socket line %d\n", __LINE__);
	
						result =   ESIF_E_WS_DISC;
						goto exit;
					}
				}

				esif_ws_server_setup_frame(connection, &frameSize, g_ws_http_buffer);
				esif_ccb_free(received_string);
				received_string = NULL;

				/* If more frames remaining, copy them into buffer and reparse */
				if (bufferRemaining != NULL) {
					esif_ccb_memcpy(g_ws_http_buffer, bufferRemaining, bytesRemaining);
					numBytesRcvd = messageLength = bytesRemaining;
					bytesRemaining = 0;
					goto more_data;
				}
			}
		}
	}
	else if (connection->frameType != PONG_FRAME) {
		ESIF_TRACE_DEBUG("unhandled frame type %d \n", connection->frameType);
		result =   ESIF_E_WS_DISC;
		goto exit;
	}

exit:
	esif_ccb_free(received_string);
	esif_ccb_free(bufferRemaining);
	received_string = NULL;
	bufferRemaining = NULL;

	return result;
}


static void esif_ws_server_setup_buffer (
	size_t *size,
	UInt8 *buf
	)
{
	*size = BUFFER_LENGTH;
	esif_ccb_memset(buf, 0, BUFFER_LENGTH);
}


static void esif_ws_server_setup_frame (
	clientRecord *connection,
	size_t *numRcv,
	UInt8 *buf
	)
{
	connection->frameType = INCOMPLETE_FRAME;
	
	*numRcv = 0;
	esif_ccb_memset(buf, 0, BUFFER_LENGTH);
}


void esif_ws_server_initialize_connection(clientRecord *connection)
{
	if (connection) {
		connection->state     = STATE_OPENING;
		connection->frameType = INCOMPLETE_FRAME;
		esif_ws_socket_initialize_frame(&connection->prot);
		esif_ws_socket_initialize_message_buffer(&connection->buf);
		if (connection->socket != INVALID_SOCKET) {
			esif_ccb_socket_close(connection->socket);
		}
		connection->socket = INVALID_SOCKET;
	}
}

void esif_ws_server_initialize_clients (void)
{
	int index=0;
	for (index = 0; index < MAX_CLIENTS; ++index) {
		g_client[index].socket = INVALID_SOCKET;
		esif_ws_server_initialize_connection(&g_client[index]);
	}
}

void esif_ws_close_socket(clientRecord *connection)
{
	esif_ws_socket_initialize_frame(&connection->prot);
	esif_ws_socket_initialize_message_buffer(&connection->buf);
	connection->state = STATE_CLOSING;
	connection->frameType = INCOMPLETE_FRAME;

	if (connection->socket != INVALID_SOCKET) {
		esif_ccb_socket_close(connection->socket);
		connection->socket = INVALID_SOCKET;
	}
}
