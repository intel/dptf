/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define MESSAGE_SUCCESS 0
#define MESSAGE_ERROR 1


/* Max number of Client connections. This cannot exceed FD_SETSIZE (Deafult: Windows=64, Linux=1024) */
#define MAX_CLIENTS		10

/*
 *******************************************************************************
 ** EXTERN
 *******************************************************************************
 */
extern int g_shell_enabled; 

/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */
#define WEBSOCKET_DEFAULT_IPADDR	"0.0.0.0"	// All Interfaces
#define WEBSOCKET_DEFAULT_PORT		"8888"		// Public Port
#define WEBSOCKET_RESTRICTED_IPADDR	"127.0.0.1"	// Localhost Only
#define WEBSOCKET_RESTRICTED_PORT	"888"		// System Port

static char *esif_ws_server_get_rest_response(size_t *msgLenPtr);
static char *esif_ws_server_get_rest_buffer(void);
static void esif_ws_server_initialize_clients(void);

static int esif_ws_server_create_inet_addr(
	void *addrPtr,
	socklen_t *addrlenPtr,
	char *hostPtr,
	char *portPtr,
	char *protPtr
	);

void esif_ws_server_execute_rest_cmd(
	const char *dataPtr,
	const size_t dataSize
	);

static void esif_ws_client_initialize_client(ClientRecordPtr);
static eEsifError esif_ws_client_process_request(ClientRecordPtr clientPtr);

static int esif_ws_client_write_to_socket(
	ClientRecordPtr clientPtr,
	const char *bufferPtr,
	size_t bufferSize
	);

static eEsifError esif_ws_client_open_client(
	ClientRecordPtr clientPtr,
	char *bufferPtr,
	size_t bufferSize,
	size_t messageLength
	);

static eEsifError esif_ws_client_process_active_client(
	ClientRecordPtr clientPtr,
	char *bufferPtr,
	size_t bufferSize,
	size_t messageLength
	);

static void esif_ws_protocol_initialize(ProtocolPtr protPtr);

static ClientRecordPtr g_clients = NULL; /* dynamically allocated array of MAX_CLIENTS */
static char *g_ws_http_buffer = NULL; /* dynamically allocated buffer of size OUT_BUF_LEN */
static char *g_rest_out = NULL;

static esif_ccb_mutex_t g_web_socket_lock;
static atomic_t g_ws_quit = 0;
atomic_t g_ws_threads = 0;

/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */
#define MAX_IPADDR 20
char g_ws_ipaddr[MAX_IPADDR]= WEBSOCKET_DEFAULT_IPADDR;
char g_ws_port[MAX_IPADDR] =  WEBSOCKET_DEFAULT_PORT;
Bool g_ws_restricted = ESIF_FALSE;

esif_ccb_socket_t g_listen = INVALID_SOCKET;

int esif_ws_init(void)
{
	int index=0;
	int retVal=0;
	char *ipaddr = (char*)g_ws_ipaddr;
	char *portPtr = g_ws_port;

	struct sockaddr_in addrSrvr = {0};
	struct sockaddr_in addrClient = {0};
	socklen_t len_inet = 0;
	
	esif_ccb_socket_t client_socket = INVALID_SOCKET;
	
	int option = 1;
	eEsifError req_results = ESIF_OK;
	ClientRecordPtr clientPtr = NULL;

	int selRetVal = 0;
	int maxfd = 0;
	int setsize = 0;
	
	struct timeval tv={0}; 	/* Timeout value */
	fd_set workingSet = {0};

	esif_ccb_mutex_init(&g_web_socket_lock);
	atomic_inc(&g_ws_threads);
	atomic_set(&g_ws_quit, 0);

	CMD_OUT("Starting WebServer %s %s\n", ipaddr, portPtr);

	esif_ccb_socket_init();

	// Allocate pool of Client Records and HTTP input buffer
	g_clients = (ClientRecordPtr )esif_ccb_malloc(MAX_CLIENTS * sizeof(*g_clients));
	g_ws_http_buffer = (char *)esif_ccb_malloc(WS_BUFFER_LENGTH);
	if (NULL == g_clients || NULL == g_ws_http_buffer) {
		ESIF_TRACE_DEBUG("Out of memory");
		goto exit;
	}

	esif_ws_server_initialize_clients();

	len_inet = sizeof(addrSrvr);

	retVal   = esif_ws_server_create_inet_addr(&addrSrvr, &len_inet, ipaddr, portPtr, (char*)"top");
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
	
	retVal = listen(g_listen, MAX_CLIENTS);
	if (retVal < 0) {
		ESIF_TRACE_DEBUG("listen system call failed");
		goto exit;
	}

	/* Accept client requests and new connections until told to quit */
	while (!atomic_read(&g_ws_quit)) {
	
		/* Build file descriptor set of active sockets */
		maxfd = 0;
		setsize = 0;

		/* Clear the FD set we will check after each iteration */
		FD_ZERO(&workingSet);

		/* Add our listner to the FD set to check */
		if (g_listen != INVALID_SOCKET) {
			FD_SET((u_int)g_listen, &workingSet);
			maxfd = (int)g_listen + 1;
			setsize++;
		}

		/* Add our current clients to the FD set to check */
		for (index = 0; index <  MAX_CLIENTS && setsize < FD_SETSIZE; index++) {
			if (g_clients[index].socket != INVALID_SOCKET) {
				FD_SET((u_int)g_clients[index].socket, &workingSet);
				maxfd = esif_ccb_max(maxfd, (int)g_clients[index].socket + 1);
				setsize++;
			}
		}
		/* If we have nothing functional in te FD set to check; break */
		if (maxfd == 0) {
			break;
		}

		/*
		 *  timeout of N + 0.05 secs
		 */
		tv.tv_sec  = 2;
		tv.tv_usec = 50000;

		/* Check our FD set for sockets ready to be accepted (listener) or read (others already accepted) */
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

			/* Find the first empty client in our list */
			for (index = 0; index < MAX_CLIENTS && sockets < FD_SETSIZE; index++) {
				if (g_clients[index].socket == INVALID_SOCKET) {
					esif_ws_client_initialize_client(&g_clients[index]);
					g_clients[index].socket = client_socket;
					break;
				}
				sockets++;
			}

			/* If all clients are in use, close the new client */
			if (index >= MAX_CLIENTS || sockets >= FD_SETSIZE) {
				ESIF_TRACE_DEBUG("Connection Limit Exceeded (%d)", MAX_CLIENTS);
				esif_ccb_socket_close(client_socket);
				client_socket = INVALID_SOCKET;
				continue;
			}
		}

		/* Go through our client list and check if the FD set indicates any have activity */
		for (index = 0; index < MAX_CLIENTS; index++) {
			client_socket = g_clients[index].socket;
			if (client_socket == INVALID_SOCKET || client_socket == g_listen) {
				continue;
			}

			/* Process client if it is in the set of active file descriptors */
			if (FD_ISSET(client_socket, &workingSet)) {
				ESIF_TRACE_DEBUG("Client %d connected\n", client_socket);

				/******************** Process the client request ********************/
				clientPtr = &g_clients[index];
				req_results = esif_ws_client_process_request(clientPtr);

				if (req_results == ESIF_E_WS_DISC) {
					ESIF_TRACE_DEBUG("Client %d disconnected\n", client_socket);
					esif_ws_client_initialize_client(clientPtr); /* reset */
				}
				else if (req_results == ESIF_E_NO_MEMORY) {
					ESIF_TRACE_DEBUG("Out of memory\n");
					esif_ws_client_initialize_client(clientPtr); /* reset */
				}

				/* Clear everything after use */
				esif_ws_protocol_initialize(&clientPtr->prot);
			}
		}
	}

exit:
	/* cleanup */
	if (g_listen != INVALID_SOCKET) {
		esif_ccb_socket_close(g_listen);
		g_listen = INVALID_SOCKET;
	}
	if (g_clients) {
		for (index = 0; index < MAX_CLIENTS; index++) {
			esif_ws_client_close_client(&g_clients[index]);
		}
		esif_ccb_free(g_clients);
		g_clients = NULL;
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
void esif_ws_exit(esif_thread_t *threadPtr)
{
	CMD_OUT("Stopping WebServer...\n");
	atomic_set(&g_ws_quit, 1);
	esif_ccb_thread_join(threadPtr);  /* join to close child thread, clean up handle */
	// Wait for worker thread to finish
	while (atomic_read(&g_ws_threads) > 0) {
		esif_ccb_sleep(1);
	}
	esif_ccb_mutex_uninit(&g_web_socket_lock);
	atomic_set(&g_ws_quit, 0);
	CMD_OUT("WebServer Stopped\n");
}

void esif_ws_server_set_ipaddr_port(const char *ipaddr, u32 portPtr, Bool restricted)
{
	if (ipaddr == NULL) {
		ipaddr = (restricted ? WEBSOCKET_RESTRICTED_IPADDR : WEBSOCKET_DEFAULT_IPADDR);
	}
	if (portPtr == 0) {
		portPtr = atoi(restricted ? WEBSOCKET_RESTRICTED_PORT : WEBSOCKET_DEFAULT_PORT);
	}
	esif_ccb_strcpy(g_ws_ipaddr, ipaddr, sizeof(g_ws_ipaddr));
	esif_ccb_sprintf(sizeof(g_ws_port), g_ws_port, "%d", portPtr);
	g_ws_restricted = restricted;
}


char *esif_ws_server_get_rest_response(size_t *msgLenPtr)
{
	char *msgPtr = g_rest_out;
	size_t msgLen = 0;

	ESIF_TRACE_DEBUG("Message Received: %s \n", msgPtr);

	if (msgPtr == NULL) {
		goto exit;
	}

	msgLen = esif_ccb_strlen((char *)msgPtr, WS_BUFFER_LENGTH);
	if (msgLen == 0) {
		goto exit;
	}

	*msgLenPtr = msgLen;
exit:
	return msgPtr;
}


void esif_ws_server_execute_rest_cmd(
	const char *dataPtr,
	const size_t dataSize
	)
{
	char *command_buf = NULL;

	if (atomic_read(&g_ws_quit))
		return;

	command_buf = strchr(dataPtr, ':');
	if (NULL != command_buf) {
		u32 msg_id = atoi(dataPtr);
		*command_buf = 0;
		command_buf++;

		// Ad-Hoc UI Shell commands begin with "0:", so verify ESIF shell is enabled and command is valid
		if (msg_id == 0 || g_ws_restricted) {
			char *response = NULL;
			if (msg_id == 0 && !g_shell_enabled) {
				response = "Shell Disabled";
			}
			else {
				static char *whitelist[] = { "status", "participants", NULL };
				static char *blacklist[] = { "shell", "web", "exit", "quit", NULL };
				Bool blocked = ESIF_FALSE;
				int j = 0;
				if (g_ws_restricted) {
					for (j = 0; whitelist[j] != NULL; j++) {
						if (esif_ccb_strnicmp(command_buf, whitelist[j], esif_ccb_strlen(whitelist[j], MAX_PATH)) == 0) {
							break;
						}
					}
					if (whitelist[j] == NULL) {
						blocked = ESIF_TRUE;
					}
				}
				else {
					for (j = 0; blacklist[j] != NULL; j++) {
						if (esif_ccb_strnicmp(command_buf, blacklist[j], esif_ccb_strlen(blacklist[j], MAX_PATH)) == 0) {
							blocked = ESIF_TRUE;
							break;
						}
					}
				}
				if (blocked) {
					response = "Unsupported Command";
				}
			}
			// Exit if shell or command unavailable
			if (response) {
				char buffer[MAX_PATH] = { 0 };
				esif_ccb_sprintf(sizeof(buffer), buffer, "%d:%s", msg_id, response);
				esif_ccb_free(g_rest_out);
				g_rest_out = esif_ccb_strdup(buffer);
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
	return;
}


static int esif_ws_server_create_inet_addr(
	void *addrPtr,
	socklen_t *addrlenPtr,
	char *hostPtr,
	char *portPtr,
	char *protPtr
	)
{
	struct sockaddr_in *sockaddr_inPtr = (struct sockaddr_in*)addrPtr;
	struct hostent *hostenPtr  = NULL;
	struct servent *serventPtr = NULL;

	char *endStr;
	long longVal;

	if (!hostPtr) {
		hostPtr = (char*)"*";
	}

	if (!portPtr) {
		portPtr = (char*)"*";
	}

	if (!protPtr) {
		protPtr = (char*)"tcp";
	}


	esif_ccb_memset(sockaddr_inPtr, 0, *addrlenPtr);
	sockaddr_inPtr->sin_family = AF_INET;
	sockaddr_inPtr->sin_port   = 0;
	sockaddr_inPtr->sin_addr.s_addr = INADDR_ANY;


	if (esif_ccb_strcmp(hostPtr, "*") == 0) {
		;
	} else if (isdigit(*hostPtr)) {
		sockaddr_inPtr->sin_addr.s_addr = inet_addr(hostPtr);

		if (sockaddr_inPtr->sin_addr.s_addr == INADDR_NONE) {
			return -1;
		}
	} else {
		hostenPtr = gethostbyname(hostPtr);

		if (!hostenPtr) {
			return -1;
		}

		if (hostenPtr->h_addrtype != AF_INET) {
			return -1;
		}

		sockaddr_inPtr->sin_addr = *(struct in_addr*)

			hostenPtr->h_addr_list[0];
	}

	if (!esif_ccb_strcmp(portPtr, "*")) {
		;
	} else if (isdigit(*portPtr)) {
		longVal = strtol(portPtr, &endStr, 10);
		if (endStr != NULL && *endStr) {
			return -2;
		}

		if (longVal < 0L || longVal >= 32768) {
			return -2;
		}

		sockaddr_inPtr->sin_port = htons((short)longVal);
	} else {
		serventPtr = getservbyname(portPtr, protPtr);
		if (!serventPtr) {
			return -2;
		}

		sockaddr_inPtr->sin_port = (short)serventPtr->s_port;
	}

	*addrlenPtr = sizeof *sockaddr_inPtr;

	return 0;
}


static int esif_ws_client_write_to_socket(
	ClientRecordPtr clientPtr,
	const char *bufferPtr,
	size_t bufferSize
	)
{
	ssize_t ret=0;
	
	ret = send(clientPtr->socket, (char*)bufferPtr, (int)bufferSize, ESIF_WS_SEND_FLAGS);
	if (ret == -1 || ret != (ssize_t)bufferSize) {
		esif_ccb_socket_close(clientPtr->socket);
		clientPtr->socket = INVALID_SOCKET;
		ESIF_TRACE_DEBUG("Error writing to socket: %s", (ret == -1 ? "Error in sending packets\n" : "Incomplete data\n"));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


/*
 * This function processes requests for either websocket connections or
 * http connections.
 */
static eEsifError esif_ws_client_process_request(ClientRecordPtr clientPtr)
{
	eEsifError result = ESIF_OK;
	size_t messageLength  = 0;
	
	esif_ccb_memset(g_ws_http_buffer, 0, WS_BUFFER_LENGTH);

	/*Pull the next message from the client socket */
	messageLength = (size_t)recv(clientPtr->socket, (char*)g_ws_http_buffer, WS_BUFFER_LENGTH, 0);
	if (messageLength == 0 || messageLength == SOCKET_ERROR) {
		ESIF_TRACE_DEBUG("no messages received from the socket\n");
		result =  ESIF_E_WS_DISC;
		goto exit;
	} else {
		ESIF_TRACE_DEBUG("%d bytes received\n", (int)messageLength);
	}

	if (clientPtr->state == STATE_NORMAL) {
		result = esif_ws_client_process_active_client(clientPtr, (char *)g_ws_http_buffer, WS_BUFFER_LENGTH, messageLength);
		goto exit;
	}

	if (clientPtr->state == STATE_OPENING) {
		result = esif_ws_client_open_client(clientPtr, (char *)g_ws_http_buffer, WS_BUFFER_LENGTH, messageLength);
		goto exit;
	}

	result =  ESIF_E_WS_DISC;
exit:
	return result;
}


/*
 * This function processes the socket when it is in the "opening" state
 */
static eEsifError esif_ws_client_open_client(
	ClientRecordPtr clientPtr,
	char *bufferPtr,
	size_t bufferSize,
	size_t messageLength
	)
{
	eEsifError result = ESIF_OK;
	FrameType frameType;
	size_t frameSize = 0;

	ESIF_ASSERT(clientPtr->state == STATE_OPENING);
	ESIF_ASSERT(messageLength > 0);

	ESIF_TRACE_DEBUG("Socket in its opening state\n");
	/*Determine the initial frame type:  http frame type or websocket frame type */
	frameType = esif_ws_socket_get_initial_frame_type(bufferPtr, messageLength, &clientPtr->prot);

	if ((INCOMPLETE_FRAME == frameType) ||  (ERROR_FRAME == frameType)) {
		if (INCOMPLETE_FRAME == frameType) {
			ESIF_TRACE_DEBUG("Incomplete frame received\n");
		} else {
			ESIF_TRACE_DEBUG("Improper format for frame\n");
		}

		/*
		 * If the socket frame type is in error or is incomplete and happens to
		 * be in its opening state, send a message to the client that the request is bad
		 */
		frameSize = esif_ccb_sprintf(bufferSize,
			(char*)bufferPtr,
			"HTTP/1.1 400 Bad Request\r\n"
			"Content-Type: text/html\r\n\r\n"
			"<html>"
			"<head></head>"
			"  <body>"
			"    ERROR: HTTP/1.1 400 Bad Request"
			"  </body>"
			"</html>");
		
		esif_ws_client_write_to_socket(clientPtr, bufferPtr, frameSize);
		result =  ESIF_E_WS_DISC;
		goto exit;
	}

	if (OPENING_FRAME == frameType) {
		if (esif_ws_socket_build_protocol_change_response(&clientPtr->prot, bufferPtr, bufferSize, &frameSize) != 0)	{
			ESIF_TRACE_DEBUG("Unable to build response header\n");
			result =   ESIF_E_WS_DISC;
			goto exit;
		}

		if (esif_ws_client_write_to_socket(clientPtr, bufferPtr, frameSize) == EXIT_FAILURE) {
			result =   ESIF_E_WS_DISC;
			goto exit;
		}

		/**************************** This is a now a websocket connection ****************************/
		clientPtr->state = STATE_NORMAL;
	}

	if (HTTP_FRAME == frameType) {				
		result = esif_ws_http_process_reqs(clientPtr, g_ws_http_buffer, messageLength);
	}
exit:
	return result;
}


/*
 * This function processes requests for clients already opened.
 */
static eEsifError esif_ws_client_process_active_client(
	ClientRecordPtr clientPtr,
	char *bufferPtr,
	size_t bufferSize,
	size_t messageLength
	)
{
	eEsifError result = ESIF_OK;
	FrameType frameType;
	size_t frameSize       = 0;
	char *data 		   = NULL;
	size_t dataSize        = 0;
	char *restRespPtr = NULL;
	size_t restRespSize = 0;
	size_t bytesRemaining  = 0;
	char *bufferRemaining = NULL;
	char *textStrPtr = NULL;
	
	do {
		/* If more frames remaining, copy them into buffer and reparse */
		if (bytesRemaining != 0) {
			esif_ccb_memcpy(bufferPtr, bufferRemaining, bytesRemaining);
			messageLength = bytesRemaining;
			bytesRemaining = 0;
		}

		frameType = esif_ws_socket_get_subsequent_frame_type((WsSocketFramePtr)bufferPtr, messageLength, &data, &dataSize, &bytesRemaining);
		ESIF_TRACE_DEBUG("FrameType: %d\n", frameType);

		/* Save remaining frames for reparsing if more than one frame received */
		if (bytesRemaining > 0) {
			if (bufferRemaining == NULL) {
				bufferRemaining = (char *)esif_ccb_malloc(bytesRemaining);
				if (NULL == bufferRemaining) {
					result = ESIF_E_NO_MEMORY;
					goto exit;
				}
			}
			esif_ccb_memcpy(bufferRemaining, data + dataSize, bytesRemaining);
		}
		else {
			esif_ccb_free(bufferRemaining);
			bufferRemaining = NULL;
		}

		/*Now, if the frame type is an incomplete type or if it is an error type of frame */
		if ((INCOMPLETE_FRAME == frameType) ||  (ERROR_FRAME == frameType)) {
			if (INCOMPLETE_FRAME == frameType) {
				ESIF_TRACE_DEBUG("Incomplete frame received; closing socket\n");
			} else {
				ESIF_TRACE_DEBUG("Improper format for frame; closing socket\n");
			}

			/*
			 * If the socket is not in its opening state while its frame type is in error or is incomplete
			 * setup to store the payload to send to the client
			 */
			esif_ws_socket_build_payload(NULL, 0, (WsSocketFramePtr)bufferPtr, bufferSize, &frameSize, CLOSING_FRAME);
			esif_ws_client_write_to_socket(clientPtr, bufferPtr, frameSize);

			/*
			 * Force the socket state into its closing state
			 */
			result =   ESIF_E_WS_DISC;
			goto exit;

		}

		if (CLOSING_FRAME == frameType) {
			ESIF_TRACE_DEBUG("Close frame received; closing socket\n");
			esif_ws_socket_build_payload(NULL, 0, (WsSocketFramePtr)bufferPtr, bufferSize, &frameSize, CLOSING_FRAME);
			esif_ws_client_write_to_socket(clientPtr, bufferPtr, frameSize);

			result =   ESIF_E_WS_DISC;
			goto exit;
		}
	
		if (TEXT_FRAME == frameType) {

			/* Use a copy of the frame text to send to the rest API */
			textStrPtr = (char*)esif_ccb_malloc(dataSize + 1);
			if (NULL == textStrPtr) {
				result = ESIF_E_NO_MEMORY;
				goto exit;
			}
			esif_ccb_memcpy(textStrPtr, data, dataSize);
			textStrPtr[dataSize] = 0;

			esif_ws_server_execute_rest_cmd((const char*)textStrPtr,
				esif_ccb_strlen((const char*)textStrPtr, bufferSize));

			/* Get message to send*/
			restRespPtr = esif_ws_server_get_rest_response(&restRespSize);
			if (restRespPtr != NULL) {
				esif_ws_socket_build_payload(restRespPtr, restRespSize, (WsSocketFramePtr)bufferPtr, bufferSize, &frameSize, TEXT_FRAME);
					
				if (esif_ws_client_write_to_socket(clientPtr, bufferPtr, frameSize) == EXIT_FAILURE) {
					result =  ESIF_E_WS_DISC;
					goto exit;
				}
			}

			esif_ccb_free(textStrPtr);
			textStrPtr = NULL;
		}

		/* Handle unsolicited PONG (keepalive) messages from Internet Explorer 10 */
		if (PONG_FRAME == frameType) {
			esif_ws_socket_build_payload("", 0, (WsSocketFramePtr)bufferPtr, bufferSize, &frameSize, TEXT_FRAME);
			esif_ws_client_write_to_socket(clientPtr, bufferPtr, frameSize);
		}

	} while (bytesRemaining != 0);
exit:
	esif_ccb_free(textStrPtr);
	esif_ccb_free(bufferRemaining);
	textStrPtr = NULL;
	bufferRemaining = NULL;

	return result;
}


void esif_ws_server_initialize_clients(void)
{
	int index=0;
	for (index = 0; index < MAX_CLIENTS; ++index) {
		g_clients[index].socket = INVALID_SOCKET;
		esif_ws_client_initialize_client(&g_clients[index]);
	}
}

void esif_ws_client_initialize_client(ClientRecordPtr clientPtr)
{
	if (NULL == clientPtr) {
		return;
	}

	clientPtr->state     = STATE_OPENING;
	esif_ws_protocol_initialize(&clientPtr->prot);
	if (clientPtr->socket != INVALID_SOCKET) {
		esif_ccb_socket_close(clientPtr->socket);
	}
	clientPtr->socket = INVALID_SOCKET;
}


void esif_ws_client_close_client(ClientRecordPtr clientPtr)
{
	if (NULL == clientPtr) {
		return;
	}

	esif_ws_protocol_initialize(&clientPtr->prot);
	clientPtr->state = STATE_OPENING;

	if (clientPtr->socket != INVALID_SOCKET) {
		esif_ccb_socket_close(clientPtr->socket);
		clientPtr->socket = INVALID_SOCKET;
	}
}


void esif_ws_protocol_initialize(ProtocolPtr protPtr)
{
	esif_ccb_free(protPtr->hostField);
	esif_ccb_free(protPtr->originField);
	esif_ccb_free(protPtr->webpage);
	esif_ccb_free(protPtr->keyField);
	esif_ccb_free(protPtr->web_socket_field);
	protPtr->hostField   = NULL;
	protPtr->originField = NULL;
	protPtr->webpage     = NULL;
	protPtr->keyField    = NULL;
	protPtr->web_socket_field = NULL;
	protPtr->frameType = EMPTY_FRAME;
}


