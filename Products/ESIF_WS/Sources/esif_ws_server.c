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

#define BUFFER_LENGTH 0xFFFF

#define MESSAGE_SUCCESS 0
#define MESSAGE_ERROR 1

#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif

#define ESIF_UF_SHELL


#ifndef SOCKET_ERROR
# define SOCKET_ERROR (-1)
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/*
 *******************************************************************************
 ** EXTERN
 *******************************************************************************
 */
void esif_ws_server_initialize_clients (void);

void esif_ws_close_socket(int client_socket, int force);

/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */
/* Forward Decleration */
static void esif_ws_server_report_error (const char*);
static int esif_ws_server_create_inet_addr (void*, int*, char*, char*);
static eEsifError esif_ws_server_process_websok_or_http_request (int);
static int esif_ws_server_write_to_socket (int, const UInt8*, size_t);
static void esif_ws_server_setup_buffer (size_t*, UInt8*);
static void esif_ws_server_setup_frame (int clientSocket, size_t*, UInt8*);
static void esif_ws_server_destroy_ptr(char **ptr);


static eEsifError esif_ws_server_handle_keep_alive_start ();
static void esif_ws_server_handle_keep_alive_stop ();

int esif_ws_push_xml_data (char *xml_data);


char esif_ws_server_rest_api[256];
void esif_ws_server_set_rest_api (const char*, const size_t, int);
char*esif_ws_server_get_rest_api (void);

int getOutgoingWebsockMessage (int);

clientRecord *g_client = NULL; /* dynamically allocated array of NUM_CLIENTS */

static int g_websocket_state = closed;
static int g_websocket_fd    = 0;
static esif_thread_t g_handle_keep_alive_thread;

esif_ccb_mutex_t g_web_socket_lock;

static u32 g_ws_keepalive_sleeptime = 0; /* 0=no keepalive thread, otherwise keepalive thread sleep time in seconds */

static atomic_t g_ws_quit = 0;
static atomic_t g_ws_threads = 0;

/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */
static fd_set readSet = {0};
static fd_set workingSet = {0};
int esif_ws_init (char *directory)
{
	int index=0;
	int retVal=0;
	char *defSrvrAddr = (char*)"0.0.0.0:8888";

	struct sockaddr_in addrSrvr = {0};
	struct sockaddr_in addrClient = {0};

	int len_inet;
	int sockfd = -1;
	int client_socket = -1;
	int option = 1;
	eEsifError req_results = ESIF_OK;

	int selRetVal = 0;
	int maxfd = 0;

	struct timeval tv;	/* Timeout value */

	esif_ccb_mutex_init(&g_web_socket_lock);
	atomic_set(&g_ws_quit, 0);
	atomic_inc(&g_ws_threads);

	// Allocate pool of Client Records
	g_client = (clientRecord *)esif_ccb_malloc(NUM_CLIENTS * sizeof(clientRecord));
	if (NULL == g_client) {
		atomic_dec(&g_ws_threads);
		return EOF;
	}

#if defined(ESIF_ATTR_OS_LINUX) || defined(ESIF_ATTR_OS_ANDROID) || \
	defined(ESIF_ATTR_OS_CHROME)
	directory = "/usr/share/dptf/";
#endif

	esif_ws_http_copy_server_root(directory);

	esif_uf_ccb_sock_init();
	CMD_OUT("Starting WebServer %s\n", defSrvrAddr);

	esif_ws_server_initialize_clients();


	len_inet = sizeof addrSrvr;

	retVal   = esif_ws_server_create_inet_addr(&addrSrvr, &len_inet, defSrvrAddr, (char*)"top");

	if (retVal < 0 && !addrSrvr.sin_port) {
		esif_ws_server_report_error("Invalid server address/port number");
	}


	sockfd = (int)socket(PF_INET, SOCK_STREAM, 0);

	if (sockfd < -1) {
		esif_ws_server_report_error("socket(2)");
	}
	
	retVal = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

	if (retVal < 0) {
		esif_uf_ccb_sock_close(sockfd);
		esif_ws_server_report_error("setsockopt failed");
	}

	retVal = bind(sockfd, (struct sockaddr*)&addrSrvr, len_inet);

	if (retVal < -1) {
		esif_uf_ccb_sock_close(sockfd);
		esif_ws_server_report_error("bind sysem call failed");
	}


	
	retVal = listen(sockfd, 10);


	if (retVal < 0) {
		esif_ws_server_report_error("listen system call failed");
	}

	FD_ZERO(&readSet);
	FD_SET((u_int)sockfd, &readSet);
	maxfd = sockfd + 1;

	while (!atomic_read(&g_ws_quit)) {
		FD_ZERO(&workingSet);
		for (index = 0; index < maxfd; ++index)
			if (FD_ISSET((u_int)index, &readSet)) {
				FD_SET((u_int)index, &workingSet);
			}

		/*
		 *  timeout of 2.05 secs:
		 */
		tv.tv_sec  = 2;
		tv.tv_usec = 50000;

		selRetVal  = select(maxfd, &workingSet, NULL, NULL, &tv);

		if (selRetVal == SOCKET_ERROR) {
			break;
		} else if (!selRetVal) {
			continue;
		}

		if (FD_ISSET(sockfd, &workingSet)) {
			len_inet = sizeof addrClient;

			client_socket = (int)accept(sockfd, (struct sockaddr*)&addrClient, (esif_uf_ccb_sock_len*)&len_inet);

			if (client_socket == -1) {
				esif_ws_server_report_error("accept(2)");
			}


			if (client_socket >= NUM_CLIENTS) {
				esif_uf_ccb_sock_close(client_socket);
				continue;
			}


			if (client_socket + 1 > maxfd) {
				maxfd = client_socket + 1;
			}

			FD_SET((u_int)client_socket, &readSet);
		}


		for (client_socket = 0; client_socket < maxfd; ++client_socket) {
			if (client_socket == sockfd) {
				continue;
			}


			/*
			* Check to see if the file descriptor client_socket 
			* is set in the set of file descritptors
			*/
			if (FD_ISSET(client_socket, &workingSet)) {
				ESIF_TRACE_DEBUG("Client %d connected\n", client_socket);

				/*
				 * File descriptor client_socket is set in the
				 * set of file descriptors. Therefore, process
				 * it.
				 */
				 
				 req_results = esif_ws_server_process_websok_or_http_request(client_socket);

				if (req_results == ESIF_E_WS_DISC) {
				
					ESIF_TRACE_DEBUG("Client %d disconnected \n", client_socket);
						/* reset */
					 esif_ws_socket_initialize_frame(&g_client[client_socket].prot);
					 g_client[client_socket].frameType = INCOMPLETE_FRAME;
					 g_client[client_socket].state = STATE_OPENING;
					 
					FD_CLR((u_int)client_socket, &readSet);
				}
				else if (req_results == OUT_OF_MEMORY) {
					ESIF_TRACE_DEBUG("Out of memory \n");
					FD_CLR((u_int)client_socket, &readSet);
				}
							
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.hostField);
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.keyField);
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.originField);
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.webpage);
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.web_socket_field);
				esif_ws_server_destroy_ptr(&g_client[client_socket].buf.msgSend);
				esif_ws_server_destroy_ptr(&g_client[client_socket].buf.msgReceive);
				g_client[client_socket].buf.rcvSize = g_client[client_socket].buf.sendSize = 0;
			}
		}


		for ((client_socket = maxfd - 1); client_socket >= 0 && !FD_ISSET(client_socket, &readSet); client_socket = maxfd - 1)
			maxfd = client_socket;

	}

	/* cleanup */
	esif_uf_ccb_sock_close(sockfd);
	for (index = 0; index < NUM_CLIENTS; index++) {
		esif_ws_close_socket(index, 0);
	}
	atomic_dec(&g_ws_threads);
	return 0;
}

/* stop web server and wait for worker threads to exit */
void esif_ws_exit ()
{
	atomic_set(&g_ws_quit, 1);
	g_websocket_state = closed;

	CMD_OUT("Stopping WebServer...\n");
	while (atomic_read(&g_ws_threads) > 0) {
		esif_ccb_sleep(1);
	}
	CMD_OUT("WebServer Stopped\n");
	esif_ccb_free(g_client);
	g_client = NULL;
	esif_ccb_mutex_uninit(&g_web_socket_lock);
	atomic_set(&g_ws_quit, 0);
}

int getOutgoingWebsockMessage (int clientSocket)
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
		g_client[clientSocket].buf.msgSend = (char*)esif_ccb_malloc(esif_ccb_strlen(mes, BUFFER_LENGTH) + 1);
	
		esif_ccb_memcpy(g_client[clientSocket].buf.msgSend, mes, esif_ccb_strlen(mes, BUFFER_LENGTH));
		g_client[clientSocket].buf.sendSize = (UInt32)esif_ccb_strlen(mes, BUFFER_LENGTH);
	

	return MESSAGE_SUCCESS;
}

UInt8 g_xml_data_buffer[BUFFER_LENGTH];
int esif_ws_push_xml_data (char *xml_data)
{
	size_t frameSize = BUFFER_LENGTH;

	if (!g_websocket_state) {
		// printf("Websocket is not available\n");
		return 0;
	} else {
		
		g_client[g_websocket_fd].frameType = TEXT_FRAME;
		g_client[g_websocket_fd].state     = STATE_NORMAL;
	

		esif_ws_server_setup_buffer(&frameSize, g_xml_data_buffer);
		esif_ws_socket_build_payload((UInt8*)xml_data, esif_ccb_strlen(xml_data, BUFFER_LENGTH), g_xml_data_buffer, &frameSize, TEXT_FRAME);
		if (esif_ws_server_write_to_socket(g_websocket_fd, g_xml_data_buffer, frameSize) == EXIT_FAILURE) {
			ESIF_TRACE_DEBUG("recv failed @ line %d\n", __LINE__);
		}
		esif_ws_server_setup_frame(g_websocket_fd, &frameSize, g_xml_data_buffer);
	}

	return 1;
}


#ifndef ESIF_UF_SHELL
void esif_ws_server_set_rest_api (
	const char *esif_ws_server_rest_api,
	const size_t dataSize,
	int clientSocket
	)
{
	client[clientSocket].buf.msgReceive = (char*)esif_ccb_malloc(dataSize + 1);
	esif_ccb_memcpy(client[clientSocket].buf.msgReceive, esif_ws_server_rest_api, dataSize);
	client[clientSocket].buf.rcvSize    = (UInt32)dataSize;
}


char*esif_ws_server_get_rest_api (void)
{
	return "esif_ws_server_rest_api";
}


#else

#include "esif_uf_shell.h"
char *g_rest_out = NULL;
u32 msg_id = 0;
void esif_ws_server_set_rest_api (
	const char *esif_ws_server_rest_api,
	const size_t dataSize,
	int clientSocket
	)
{
	char *recv_buf    = NULL;
	char *command_buf = NULL;

	if (atomic_read(&g_ws_quit))
		return;

	g_client[clientSocket].buf.msgReceive = (char*)esif_ccb_malloc(dataSize + 1);
	esif_ccb_memcpy(g_client[clientSocket].buf.msgReceive, esif_ws_server_rest_api, dataSize);
	g_client[clientSocket].buf.rcvSize    = (UInt32)dataSize;
	g_client[clientSocket].buf.msgReceive[dataSize] = 0;

	/* Grab Message ID */
	recv_buf    = g_client[clientSocket].buf.msgReceive;

	command_buf = strchr(recv_buf, ':');
	if (NULL != command_buf) {
		*command_buf = 0;
		command_buf++;
		msg_id = atoi(recv_buf);

		g_rest_out = NULL;
		if (!atomic_read(&g_ws_quit))
			g_rest_out = parse_cmd(command_buf, TRUE);

		if (NULL != g_rest_out) {
			EsifString temp = esif_ccb_strdup(g_rest_out);
			if (temp) {
				esif_ccb_sprintf(OUT_BUF_LEN, g_rest_out, "%u:%s", msg_id, temp);
				esif_ccb_free(temp);
			}
		}
	}
	esif_ccb_free(g_client[clientSocket].buf.msgReceive);
	g_client[clientSocket].buf.msgReceive = NULL;
	g_client[clientSocket].buf.rcvSize = 0;
}


char*esif_ws_server_get_rest_api (void)
{
	return g_rest_out;
}


#endif


static void esif_ws_server_report_error (const char *message)
{
	ESIF_TRACE_DEBUG("WS Error: %s\n", message);
}


static int esif_ws_server_create_inet_addr (
	void *addr,
	int *addrlen,
	char *addStr,
	char *protocol
	)
{
	char *context = 0;
	char *inpAddr = esif_ccb_strdup(addStr);
	char *host    = NULL;
	char *port    = NULL;

	struct sockaddr_in *sockaddr_inPtr = (struct sockaddr_in*)addr;
	struct hostent *hostenPtr  = NULL;
	struct servent *serventPtr = NULL;

	char *endStr;
	long longVal;

	host = esif_ccb_strtok(inpAddr, ":", &context);
	port = esif_ccb_strtok(NULL, "\n", &context);

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
			if (inpAddr) {
				esif_ccb_free(inpAddr);
				inpAddr = NULL;
			}
			return -1;
		}
	} else {
		hostenPtr = gethostbyname(host);

		if (inpAddr) {
			esif_ccb_free(inpAddr);
			inpAddr = NULL;
		}

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
			if (inpAddr) {
				esif_ccb_free(inpAddr);
				inpAddr = NULL;
			}
			return -2;
		}

		if (longVal < 0L || longVal >= 32768) {
			if (inpAddr) {
				esif_ccb_free(inpAddr);
				inpAddr = NULL;
			}
			return -2;
		}

		sockaddr_inPtr->sin_port = htons((short)longVal);
	} else {
		serventPtr = getservbyname(port, protocol);
		if (!serventPtr) {
			if (inpAddr) {
				esif_ccb_free(inpAddr);
				inpAddr = NULL;
			}
			return -2;
		}

		sockaddr_inPtr->sin_port = (short)serventPtr->s_port;
	}


	*addrlen = sizeof *sockaddr_inPtr;

	if (inpAddr) {
		esif_ccb_free(inpAddr);
		inpAddr = NULL;
	}

	return 0;
}


static int esif_ws_server_write_to_socket (
	int clientSocket,
	const UInt8 *buffer,
	size_t bufferSize
	)
{
	ssize_t ret=0;
	
	ret = send(clientSocket, (char*)buffer, (int)bufferSize, 0);

	if (ret == -1) {
		esif_uf_ccb_sock_close(clientSocket);
		ESIF_TRACE_DEBUG("error in sending packets\n");
		return EXIT_FAILURE;
	}

	if (ret != (ssize_t)bufferSize) {
		esif_uf_ccb_sock_close(clientSocket);
		ESIF_TRACE_DEBUG("incomplete data \n");
		return EXIT_FAILURE;
	}

#if 0
	printf("packet of size %d  sent:\n", bufferSize);
	fwrite(buffer, 1, bufferSize, stdout);
	printf("\n");
#endif

	return EXIT_SUCCESS;
}

UInt8 g_ws_http_buffer[BUFFER_LENGTH];

/* This function processes requests for either websocket connections or
 * http connections.
 */
static eEsifError esif_ws_server_process_websok_or_http_request (int clientSocket)
{

	size_t numBytesRcvd    = 0;
	size_t frameSize       = BUFFER_LENGTH;
	UInt8 *data 		   = NULL;
	size_t dataSize        = 0;
	UInt8 *recieved_string = NULL;
	ssize_t messageLength  = 0;
	eEsifError result = ESIF_OK;
	clientRecord *recPtr = NULL;
	
	esif_ccb_memset(g_ws_http_buffer, 0, BUFFER_LENGTH);

	ESIF_TRACE_DEBUG("esif_ws_server_process_websok_or_http_request\n");
	if (g_client[clientSocket].frameType == INCOMPLETE_FRAME) {
		/*Pull messages from the client socket store length lenght of recieved message */
		messageLength = recv(clientSocket, (char*)g_ws_http_buffer + numBytesRcvd, BUFFER_LENGTH - (int)numBytesRcvd, 0);
		if (messageLength == 0 || messageLength == SOCKET_ERROR) {
			ESIF_TRACE_DEBUG("no messages received from the socket\n");
			result =  ESIF_E_WS_DISC;
		} else {
			ESIF_TRACE_DEBUG("%d bytes received\n", (int)messageLength);
		}


		/*Add the length of the message of the number of characters received */
		numBytesRcvd += messageLength;

		/*Is the socket in its opening state? */
		if (g_client[clientSocket].state == STATE_OPENING) {
			ESIF_TRACE_DEBUG("socket in its opening state\n");
			/*Determine the initial frame type:  http frame type or websocket frame type */
			g_client[clientSocket].frameType = esif_ws_get_initial_frame_type(g_ws_http_buffer, numBytesRcvd, &g_client[clientSocket].prot);

			/*Is the frame type http frame type? */
			if (g_client[clientSocket].frameType == HTTP_FRAME) {
				
				result = esif_ws_http_process_reqs(clientSocket, g_ws_http_buffer, (ssize_t)numBytesRcvd);
			}
		}	/* End of if (client[clientSocket].state == STATE_OPENING)*/
		else {
			/*
			 * The socket is not in its opening state nor does it have an http frame type
			 * Therefore, determine the frame type
			 */

		
			g_client[clientSocket].frameType = esif_ws_socket_get_subsequent_frame_type(g_ws_http_buffer, numBytesRcvd, &data, &dataSize);
			ESIF_TRACE_DEBUG("client[clientSocket].frameType: %d\n", g_client[clientSocket].frameType);

			/*Handle keep-alive messages from Internet Explorer */
			if (messageLength == 6 && g_client[clientSocket].frameType == PONG_FRAME) {
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
				esif_ws_socket_build_payload((UInt8*)"", esif_ccb_strlen("", 3), g_ws_http_buffer, &frameSize, TEXT_FRAME);

				esif_ws_server_write_to_socket(g_websocket_fd, g_ws_http_buffer, frameSize);
				esif_ws_server_setup_frame(g_websocket_fd, &frameSize, g_ws_http_buffer);
			}
		}	/* End of else if (client[clientSocket].state == STATE_OPENING)*/

		/*Now, if the frame type is an incomplete type or if it is an error type of frame */
		if ((g_client[clientSocket].frameType == INCOMPLETE_FRAME && numBytesRcvd == BUFFER_LENGTH) ||
			g_client[clientSocket].frameType == ERROR_FRAME) {
			/*Is the frame type an incomplete frame type */
			if (g_client[clientSocket].frameType == INCOMPLETE_FRAME) {
				ESIF_TRACE_DEBUG("websocket: incomplete frame received\n");
			} else {
				ESIF_TRACE_DEBUG("webscocket: improper format for frame\n");
			}	/*End of if (client[clientSocket].frameType == INCOMPLETE_FRAME) */

			/*
			 * If the socket frame type is in error or is incomplete and happens to
			 * be in its opening state, send a message to the client that the request is bad
			 */
			if (g_client[clientSocket].state == STATE_OPENING) {
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
				frameSize = esif_ccb_sprintf(BUFFER_LENGTH, (char*)g_ws_http_buffer,
											 "HTTP/1.1 400 Bad Request\r\n%s%s\r\n\r\n", "Sec-WebSocket-Version: ", "13");

				esif_ws_server_write_to_socket(clientSocket, g_ws_http_buffer, frameSize);
				ESIF_TRACE_DEBUG("websocket: error writing to socket line %d\n", __LINE__);
				g_websocket_state = closed;
				result =  ESIF_E_WS_DISC;
				return result;
			}	/*End of if (client[clientSocket].state == STATE_OPENING) */
			else {
				/*
				 * If the socket is not in its opening state while its frame type is in error or is incomplete
				 * setup a g_ws_http_buffer to store the payload to send to the client
				 */
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
				esif_ws_socket_build_payload(NULL, 0, g_ws_http_buffer, &frameSize, CLOSING_FRAME);
				if (esif_ws_server_write_to_socket(clientSocket, g_ws_http_buffer, frameSize) == EXIT_FAILURE) {
					ESIF_TRACE_DEBUG("websocket: error writing to socket line %d\n", __LINE__);
					g_websocket_state = closed;
					result =   ESIF_E_WS_DISC;
					return result;
				}

				/*
				 * Force the socket state into its closing state
				 */
				g_client[clientSocket].state = STATE_CLOSING;
				esif_ws_server_setup_frame(clientSocket, &frameSize, g_ws_http_buffer);
			}	/*End of else if (client[clientSocket].state == STATE_OPENING)  */
		}	/*End of if (client[clientSocket].frameType == INCOMPLETE_FRAME && numBytesRcvd == BUFFER_LENGTH  || client[clientSocket].frameType == ERROR_FRAME
			  */

		if (g_client[clientSocket].state == STATE_OPENING) {
			if (g_client[clientSocket].frameType == OPENING_FRAME) {
				/*Validate the resource */
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
				if (esif_ws_socket_build_response_header(&g_client[clientSocket].prot, g_ws_http_buffer, &frameSize) != 0)	{
					ESIF_TRACE_DEBUG("websocket: nable to build response header line %d\n", __LINE__);
					result =   ESIF_E_WS_DISC;
					return result;
				}

				if (esif_ws_server_write_to_socket(clientSocket, g_ws_http_buffer, frameSize) == EXIT_FAILURE) {
					ESIF_TRACE_DEBUG("websocket: error writing to socket line %d\n", __LINE__);
					g_websocket_state = closed;
					result =   ESIF_E_WS_DISC;
					return result;
				}


				g_client[clientSocket].state = STATE_NORMAL;
				/*This is a websocket connection*/
				g_websocket_fd    = clientSocket;
				g_websocket_state = opened;

				esif_ws_server_setup_frame(clientSocket, &frameSize, g_ws_http_buffer);

				esif_ws_server_handle_keep_alive_start();
				
			}	/*End of if (client[clientSocket].frameType == OPENING_FRAME) */
		}	/*if (client[clientSocket].state == STATE_OPENING) */
		else {
			/*Falls though here when requesting to disconnect*/
			if (g_client[clientSocket].frameType == CLOSING_FRAME) {
				if (g_client[clientSocket].state == STATE_CLOSING) {
					ESIF_TRACE_DEBUG("websocket: receive fails line %d\n", __LINE__);
				} else {
					esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
					esif_ws_socket_build_payload(NULL, 0, g_ws_http_buffer, &frameSize, CLOSING_FRAME);
					esif_ws_server_write_to_socket(clientSocket, g_ws_http_buffer, frameSize);
				}

				g_websocket_state = closed;
				esif_ws_socket_delete_existing_field(g_client[clientSocket].prot.keyField);
				g_client[clientSocket].prot.keyField = NULL;
				esif_ws_server_handle_keep_alive_stop();			
				result =   ESIF_E_WS_DISC;
				return result;

			} else if (g_client[clientSocket].frameType == TEXT_FRAME) {
				recieved_string = (UInt8*)esif_ccb_malloc(dataSize + 1);
				if (NULL == recieved_string) {
				
					result = OUT_OF_MEMORY;
				}
				esif_ccb_memcpy(recieved_string, data, dataSize);
				recieved_string[dataSize] = 0;

				/* Save incoming socket message if needed */

				esif_ws_server_set_rest_api((const char*)recieved_string, esif_ccb_strlen((const char*)recieved_string, BUFFER_LENGTH), clientSocket);
				esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);

				/* Get message to send*/

				if (MESSAGE_SUCCESS == getOutgoingWebsockMessage(clientSocket))
				{
					recPtr = &g_client[clientSocket];
					esif_ws_socket_build_payload((UInt8*)recPtr->buf.msgSend, recPtr->buf.sendSize, g_ws_http_buffer, &frameSize, TEXT_FRAME);
					esif_ccb_free(recPtr->buf.msgSend);
					recPtr->buf.msgSend = NULL;
					recPtr->buf.sendSize = 0;
					
					if (esif_ws_server_write_to_socket(clientSocket, g_ws_http_buffer, frameSize) == EXIT_FAILURE) {
						ESIF_TRACE_DEBUG("websocket: error writing to socket line %d\n", __LINE__);
						g_websocket_state = closed;
	
						if (recieved_string) {
							esif_ccb_free(recieved_string);
							recieved_string = NULL;
						}
	
						result =   ESIF_E_WS_DISC;
						return result;
					}
				}
				
				

				esif_ws_server_setup_frame(clientSocket, &frameSize, g_ws_http_buffer);
			} else {
				/*esif_ws_server_setup_buffer(&frameSize, g_ws_http_buffer);
				   esif_ws_socket_build_payload(NULL, 0, g_ws_http_buffer, &frameSize, CLOSING_FRAME);
				   esif_ws_server_write_to_socket(clientSocket, g_ws_http_buffer, frameSize);
				   g_websocket_state = closed;
				   return EOF;*/
			}
		}	/* End of else if client[clientSocket].state == STATE_OPENING*/
	}
	else
	{
		ESIF_TRACE_DEBUG("unhandled frame type %d \n", g_client[clientSocket].frameType);
		result =   ESIF_E_WS_DISC;
		return result;
	}

	if (recieved_string) {
		esif_ccb_free(recieved_string);
		recieved_string = NULL;
	}

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
	int socket,
	size_t *numRcv,
	UInt8 *buf
	)
{
	g_client[socket].frameType = INCOMPLETE_FRAME;
	
	*numRcv = 0;
	esif_ccb_memset(buf, 0, BUFFER_LENGTH);
}


static void esif_ws_server_destroy_ptr (char **ptr)
{
	if (ptr && *ptr) {
		esif_ccb_free(*ptr);
		*ptr = 0;
	}
}


void esif_ws_server_initialize_clients (void)
{
	int index=0;
	for (index = 0; index < NUM_CLIENTS; ++index) {
		g_client[index].state     = STATE_OPENING;
		g_client[index].frameType = INCOMPLETE_FRAME;
		esif_ws_socket_initialize_frame(&g_client[index].prot);
		esif_ws_socket_initialize_message_buffer(&g_client[index].buf);
	}
}

UInt8 g_keep_alive_thread_buffer[BUFFER_LENGTH];
static void*esif_ws_server_handle_keep_alive_thread (void *ptr)
{
	size_t frameSize = BUFFER_LENGTH;
	UNREFERENCED_PARAMETER(ptr);
	
	atomic_inc(&g_ws_threads);
	while (!atomic_read(&g_ws_quit)) {
		esif_ccb_sleep(g_ws_keepalive_sleeptime);

		if (g_websocket_state) {
			
			g_client[g_websocket_fd].frameType = TEXT_FRAME;
			g_client[g_websocket_fd].state     = STATE_NORMAL;
			

			esif_ws_server_setup_buffer(&frameSize, g_keep_alive_thread_buffer);
			esif_ws_socket_build_payload((UInt8*)"", esif_ccb_strlen("", 3), g_keep_alive_thread_buffer, &frameSize, TEXT_FRAME);
			if (esif_ws_server_write_to_socket(g_websocket_fd, g_keep_alive_thread_buffer, frameSize) == EXIT_FAILURE) {
				ESIF_TRACE_DEBUG("recv failed @ line %d\n", __LINE__);
			}
			esif_ws_server_setup_frame(g_websocket_fd, &frameSize, g_keep_alive_thread_buffer);
		}
	}
	atomic_dec(&g_ws_threads);
	return 0;
}


static eEsifError esif_ws_server_handle_keep_alive_start ()
{
	if (g_ws_keepalive_sleeptime > 0)
		esif_ccb_thread_create(&g_handle_keep_alive_thread, esif_ws_server_handle_keep_alive_thread, NULL);
	return ESIF_OK;
}


static void esif_ws_server_handle_keep_alive_stop ()
{
	// keepalive thread is stopped by esif_ws_init after esif_ws_exit is called
}

void esif_ws_close_socket(int client_socket, int force)
{
	if (client_socket >= 0 && client_socket < NUM_CLIENTS) {
		esif_ws_server_destroy_ptr(&g_client[client_socket].prot.hostField);
		esif_ws_server_destroy_ptr(&g_client[client_socket].prot.keyField);
		esif_ws_server_destroy_ptr(&g_client[client_socket].prot.originField);
		esif_ws_server_destroy_ptr(&g_client[client_socket].prot.webpage);
		esif_ws_server_destroy_ptr(&g_client[client_socket].prot.web_socket_field);
		esif_ws_server_destroy_ptr(&g_client[client_socket].buf.msgSend);
		esif_ws_server_destroy_ptr(&g_client[client_socket].buf.msgReceive);
		g_client[client_socket].buf.rcvSize = g_client[client_socket].buf.sendSize = 0;

		if (force || g_client[client_socket].state == STATE_NORMAL || g_client[client_socket].frameType == HTTP_FRAME) {
			esif_uf_ccb_sock_close(client_socket);
		}
		g_client[client_socket].state = STATE_CLOSING;
		g_client[client_socket].frameType = INCOMPLETE_FRAME;
		FD_CLR((u_int)client_socket, &readSet);
	}
}
