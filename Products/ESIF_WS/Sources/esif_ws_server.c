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
#include <ctype.h>
#include "esif_ws_socket.h"
#include "esif_ws_http.h"
#include "esif_ws_server.h"

#include "esif_uf_ccb_file.h"

#define BUFFER_LENGTH 0xFFFF

#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif

#define ESIF_UF_SHELL

#define USE_REST_API
// #undef USE_REST_API


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


/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */
/* Forward Decleration */
static void esif_ws_server_report_error (const char*);
static int esif_ws_server_create_inet_addr (void*, int*, char*, char*);
static int esif_ws_server_process_websok_or_http_request (int);
static int esif_ws_server_write_to_socket (int, const UInt8*, size_t);
static void esif_ws_server_setup_buffer (size_t*, UInt8*);
static void esif_ws_server_setup_frame (int clientSocket, size_t*, UInt8*);
static void esif_ws_server_destroy_ptr(char **ptr);


static eEsifError esif_ws_server_hanlde_keep_alive_start ();
static void esif_ws_server_hanlde_keep_alive_stop ();

int esif_ws_push_xml_data (char *xml_data);


char esif_ws_server_rest_api[256];
void esif_ws_server_set_rest_api (const char*, const size_t, int);
char*esif_ws_server_get_rest_api (void);

clientRecord *getOutgoingWebsockMessage (int);


clientRecord g_client[NUM_CLIENTS];

static int g_websocket_state = closed;
static int g_websocket_fd    = 0;
static esif_thread_t g_hanlde_keep_alive_thread;

esif_ccb_mutex_t g_web_socket_lock;


/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */
int esif_ws_init (char *directory)
{
	int index;
	int retVal;
	char *defSrvrAddr = (char*)"0.0.0.0:8888";

	struct sockaddr_in addrSrvr;
	struct sockaddr_in addrClient;

	int len_inet;
	int sockfd = -1;
	int client_socket = -1;
	int option = 1;

	int selRetVal;
	int maxfd;
	fd_set readSet;
	fd_set workingSet;

	struct timeval tv;	/* Timeout value */

	esif_ws_http_copy_server_root(directory);

	esif_uf_ccb_sock_init();
	printf("ESIF_UF Extension: Embedded WebServer %s\n", defSrvrAddr);

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


	retVal = bind(sockfd, (struct sockaddr*)&addrSrvr, len_inet);

	if (retVal < -1) {
		esif_uf_ccb_sock_close(sockfd);
		esif_ws_server_report_error("bind sysem call failed");
	}


	retVal = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

	if (retVal < 0) {
		esif_uf_ccb_sock_close(sockfd);
		esif_ws_server_report_error("setsockopt failed");
	}


	retVal = listen(sockfd, 10);


	if (retVal < 0) {
		esif_ws_server_report_error("listen system call failed");
	}

	FD_ZERO(&readSet);
	FD_SET((u_int)sockfd, &readSet);
	maxfd = sockfd + 1;

	for ( ; ; ) {
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

		if (selRetVal == -1) {
			exit(1);
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
				printf("Client %d connected\n", client_socket);

				/*
				 * File descriptor client_socket is set in the
				 * set of file descriptors. Therefore, process
				 * it.
				 */

				if (esif_ws_server_process_websok_or_http_request(client_socket) == EOF) {
					printf("Client %d disconnected\n", client_socket);
					FD_CLR((u_int)client_socket, &readSet);
				}
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.hostField);
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.keyField);
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.originField);
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.webpage);
				esif_ws_server_destroy_ptr(&g_client[client_socket].prot.web_socket_field);
			}
		}


		for ((client_socket = maxfd - 1); client_socket >= 0 && !FD_ISSET(client_socket, &readSet); client_socket = maxfd - 1)
			maxfd = client_socket;

	}
}


clientRecord*getOutgoingWebsockMessage (int clientSocket)
{
	char *mes = esif_ws_server_get_rest_api();

	g_client[clientSocket].buf.msgSend = (char*)esif_ccb_malloc(esif_ccb_strlen(mes, BUFFER_LENGTH));

	esif_ccb_memcpy(g_client[clientSocket].buf.msgSend, mes, esif_ccb_strlen(mes, BUFFER_LENGTH));
	g_client[clientSocket].buf.sendSize = (UInt32)esif_ccb_strlen(mes, BUFFER_LENGTH);

	return &g_client[clientSocket];
}


int esif_ws_push_xml_data (char *xml_data)
{
	UInt8 buffer[BUFFER_LENGTH];
	size_t frameSize = BUFFER_LENGTH;

	if (!g_websocket_state) {
		// printf("Websocket is not available\n");
		return 0;
	} else {
		esif_ccb_mutex_lock(&g_web_socket_lock);
		g_client[g_websocket_fd].frameType = TEXT_FRAME;
		g_client[g_websocket_fd].state     = STATE_NORMAL;
		esif_ccb_mutex_unlock(&g_web_socket_lock);

		esif_ws_server_setup_buffer(&frameSize, buffer);
		esif_ws_socket_build_payload((UInt8*)xml_data, esif_ccb_strlen(xml_data, BUFFER_LENGTH), buffer, &frameSize, TEXT_FRAME);
		if (esif_ws_server_write_to_socket(g_websocket_fd, buffer, frameSize) == EXIT_FAILURE) {
			printf("recv failed @ line %d\n", __LINE__);
		}
		esif_ws_server_setup_frame(g_websocket_fd, &frameSize, buffer);
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
	client[clientSocket].buf.msgReceive = (char*)esif_ccb_malloc(dataSize);
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

	g_client[clientSocket].buf.msgReceive = (char*)esif_ccb_malloc(dataSize);
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

		// aw printf("REST COMMAND IN(%u) |%s|\n", msg_id, command_buf);
		g_format   = FORMAT_XML;
		g_rest_out = parse_cmd(command_buf, TRUE);
		if (NULL != g_rest_out) {
			EsifString temp = esif_ccb_strdup(g_rest_out);
			esif_ccb_sprintf(OUT_BUF_LEN, g_rest_out, "%u:%s", msg_id, temp);
			esif_ccb_free(temp);
			// aw printf("REST COMMAND OUT(%u) |%s|\n", msg_id, g_rest_out);
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
	fputs(message, stderr);

	fputc('\n', stderr);

	exit(1);
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
	char *host    = esif_ccb_strtok(inpAddr, ":", &context);
	char *port    = esif_ccb_strtok(NULL, "\n", &context);

	struct sockaddr_in *sockaddr_inPtr = (struct sockaddr_in*)addr;
	struct hostent *hostenPtr  = NULL;
	struct servent *serventPtr = NULL;

	char *endStr;
	long longVal;

	context = context;	// keep compiler happy
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
			}
			return -1;
		}
	} else {
		hostenPtr = gethostbyname(host);

		if (inpAddr) {
			esif_ccb_free(inpAddr);
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
			}
			return -2;
		}

		if (longVal < 0L || longVal >= 32768) {
			if (inpAddr) {
				esif_ccb_free(inpAddr);
			}
			return -2;
		}

		sockaddr_inPtr->sin_port = htons((short)longVal);
	} else {
		serventPtr = getservbyname(port, protocol);
		if (!serventPtr) {
			if (inpAddr) {
				esif_ccb_free(inpAddr);
			}
			return -2;
		}

		sockaddr_inPtr->sin_port = (short)serventPtr->s_port;
	}


	*addrlen = sizeof *sockaddr_inPtr;

	if (inpAddr) {
		esif_ccb_free(inpAddr);
	}

	return 0;
}


static int esif_ws_server_write_to_socket (
	int clientSocket,
	const UInt8 *buffer,
	size_t bufferSize
	)
{
	ssize_t ret;


	ret = send(clientSocket, (char*)buffer, (int)bufferSize, 0);

	if (ret == -1) {
		esif_uf_ccb_sock_close(clientSocket);
		printf("error in sending packets\n");
		return EXIT_FAILURE;
	}

	if (ret != (ssize_t)bufferSize) {
		esif_uf_ccb_sock_close(clientSocket);
		printf("incomplete data \n");
		return EXIT_FAILURE;
	}

#if 0
	printf("packet of size %d  sent:\n", bufferSize);
	fwrite(buffer, 1, bufferSize, stdout);
	printf("\n");
#endif

	return EXIT_SUCCESS;
}


/* This function processes requests for either websocket connections or
 * http connections.
 */
static int esif_ws_server_process_websok_or_http_request (int clientSocket)
{
	UInt8 buffer[BUFFER_LENGTH];

	size_t numBytesRcvd    = 0;
	size_t frameSize       = BUFFER_LENGTH;
	UInt8 *data = NULL;
	size_t dataSize        = 0;
	UInt8 *recieved_string = NULL;
	ssize_t messageLength;
	int result = 0;


#ifdef USE_REST_API
	clientRecord *recPtr;
#endif


	esif_ccb_memset(buffer, 0, BUFFER_LENGTH);

	printf("esif_ws_server_process_websok_or_http_request\n");
	if (g_client[clientSocket].frameType == INCOMPLETE_FRAME) {
		/*Pull messages from the client socket store length lenght of recieved message */
		messageLength = recv(clientSocket, (char*)buffer + numBytesRcvd, BUFFER_LENGTH - (int)numBytesRcvd, 0);
		if (!messageLength) {
			printf("no messages received from the socket\n");
			result = EOF;
		} else {
			printf("%d bytes received\n", (int)messageLength);
		}


		/*Add the length of the message of the number of characters received */
		numBytesRcvd += messageLength;

		/*Is the socket in its opening state? */
		if (g_client[clientSocket].state == STATE_OPENING) {
			printf("socket in its opening state\n");
			/*Determine the initial frame type:  http frame type or websocket frame type */
			g_client[clientSocket].frameType = esif_ws_get_initial_frame_type(buffer, numBytesRcvd, &g_client[clientSocket].prot);

			/*Is the frame type http frame type? */
			if (g_client[clientSocket].frameType == HTTP_FRAME || !esif_ccb_strncmp((const char*)buffer, "POST ", 4)) {
				result = esif_ws_http_process_reqs(clientSocket, buffer, (ssize_t)numBytesRcvd);
				return EOF;
			}
		}	/* End of if (client[clientSocket].state == STATE_OPENING)*/
		else {
			/*
			 * The socket is not in its opening state nor does it have an http frame type
			 * Therefore, determine the frame type
			 */

			printf("get frame type\n");
			g_client[clientSocket].frameType = esif_ws_socket_get_subsequent_frame_type(buffer, numBytesRcvd, &data, &dataSize);
			printf("client[clientSocket].frameType: %d\n", g_client[clientSocket].frameType);

			/*Handle keep-alive messages from Internet Explorer */
			if (messageLength == 6 && g_client[clientSocket].frameType == PONG_FRAME) {
				esif_ws_server_setup_buffer(&frameSize, buffer);
				esif_ws_socket_build_payload((UInt8*)"", esif_ccb_strlen("", 3), buffer, &frameSize, TEXT_FRAME);

				esif_ws_server_write_to_socket(g_websocket_fd, buffer, frameSize);
				esif_ws_server_setup_frame(g_websocket_fd, &frameSize, buffer);
			}
		}	/* End of else if (client[clientSocket].state == STATE_OPENING)*/

		/*Now, if the frame type is an incomplete type or if it is an error type of frame */
		if ((g_client[clientSocket].frameType == INCOMPLETE_FRAME && numBytesRcvd == BUFFER_LENGTH) ||
			g_client[clientSocket].frameType == ERROR_FRAME) {
			/*Is the frame type an incomplete frame type */
			if (g_client[clientSocket].frameType == INCOMPLETE_FRAME) {
				printf("websocket: incomplete frame received\n");
			} else {
				printf("webscocket: improper format for frame\n");
			}	/*End of if (client[clientSocket].frameType == INCOMPLETE_FRAME) */

			/*
			 * If the socket frame type is in error or is incomplete and happens to
			 * be in its opening state, send a message to the client that the request is bad
			 */
			if (g_client[clientSocket].state == STATE_OPENING) {
				esif_ws_server_setup_buffer(&frameSize, buffer);
				frameSize = esif_ccb_sprintf(BUFFER_LENGTH, (char*)buffer,
											 "HTTP/1.1 400 Bad Request\r\n%s%s\r\n\r\n", "Sec-WebSocket-Version: ", "13");

				esif_ws_server_write_to_socket(clientSocket, buffer, frameSize);
				printf("websocket: error writing to socket line %d\n", __LINE__);
				g_websocket_state = closed;
				return EOF;
			}	/*End of if (client[clientSocket].state == STATE_OPENING) */
			else {
				/*
				 * If the socket is not in its opening state while its frame type is in error or is incomplete
				 * setup a buffer to store the payload to send to the client
				 */
				esif_ws_server_setup_buffer(&frameSize, buffer);
				esif_ws_socket_build_payload(NULL, 0, buffer, &frameSize, CLOSING_FRAME);
				if (esif_ws_server_write_to_socket(clientSocket, buffer, frameSize) == EXIT_FAILURE) {
					printf("websocket: error writing to socket line %d\n", __LINE__);
					g_websocket_state = closed;
					return EOF;
				}

				/*
				 * Force the socket state into its closing state
				 */
				g_client[clientSocket].state = STATE_CLOSING;
				esif_ws_server_setup_frame(clientSocket, &frameSize, buffer);
			}	/*End of else if (client[clientSocket].state == STATE_OPENING)  */
		}	/*End of if (client[clientSocket].frameType == INCOMPLETE_FRAME && numBytesRcvd == BUFFER_LENGTH  || client[clientSocket].frameType == ERROR_FRAME
			  */

		if (g_client[clientSocket].state == STATE_OPENING) {
			if (g_client[clientSocket].frameType == OPENING_FRAME) {
				/*Validate the resource */
				esif_ws_server_setup_buffer(&frameSize, buffer);
				esif_ws_socket_build_response_header(&g_client[clientSocket].prot, buffer, &frameSize);

				if (esif_ws_server_write_to_socket(clientSocket, buffer, frameSize) == EXIT_FAILURE) {
					printf("websocket: error writing to socket line %d\n", __LINE__);
					g_websocket_state = closed;
					return EOF;
				}


				g_client[clientSocket].state = STATE_NORMAL;
				/*This is a websocket connection*/
				g_websocket_fd    = clientSocket;
				g_websocket_state = opened;

				esif_ws_server_setup_frame(clientSocket, &frameSize, buffer);

				esif_ccb_mutex_init(&g_web_socket_lock);

				esif_ws_server_hanlde_keep_alive_start();
			}	/*End of if (client[clientSocket].frameType == OPENING_FRAME) */
		}	/*if (client[clientSocket].state == STATE_OPENING) */
		else {
			/*Falls though here when requesting to disconnect*/
			if (g_client[clientSocket].frameType == CLOSING_FRAME) {
				if (g_client[clientSocket].state == STATE_CLOSING) {
					printf("websocket: receive fails line %d\n", __LINE__);
				} else {
					esif_ws_server_setup_buffer(&frameSize, buffer);
					esif_ws_socket_build_payload(NULL, 0, buffer, &frameSize, CLOSING_FRAME);
					esif_ws_server_write_to_socket(clientSocket, buffer, frameSize);
				}

				g_websocket_state = closed;
				esif_ws_socket_delete_existing_field(g_client[clientSocket].prot.keyField);
				esif_ws_server_hanlde_keep_alive_stop();
				return EOF;
			} else if (g_client[clientSocket].frameType == TEXT_FRAME) {
				// aw printf("client[clientSocket].frameType == TEXT_FRAME\n");
				recieved_string = (UInt8*)esif_ccb_malloc(dataSize + 1);
				if (NULL == recieved_string) {
					exit(1);
				}
				esif_ccb_memcpy(recieved_string, data, dataSize);
				recieved_string[dataSize] = 0;

				/* Save incoming socket message if needed */
				// aw printf("received RESt api: %s\n",recieved_string);

				esif_ws_server_set_rest_api((const char*)recieved_string, esif_ccb_strlen((const char*)recieved_string, BUFFER_LENGTH), clientSocket);
				esif_ws_server_setup_buffer(&frameSize, buffer);

				/* Get message to send*/
#ifdef USE_REST_API
				recPtr = getOutgoingWebsockMessage(clientSocket);
				// aw printf("send REST api: %s   esif_ccb_strlen: %d\n",recPtr->buf.msgSend, (int)esif_ccb_strlen(recPtr->buf.msgSend, BUFFER_LENGTH));
				esif_ws_socket_build_payload((UInt8*)recPtr->buf.msgSend, recPtr->buf.sendSize, buffer, &frameSize, TEXT_FRAME);
				esif_ccb_free(recPtr->buf.msgSend);
				recPtr->buf.msgSend = NULL;
				recPtr->buf.sendSize = 0;
#else
				// aw printf("send REST api: %s   esif_ccb_strlen: %d\n",recieved_string, (int)esif_ccb_strlen((const char *)recieved_string, BUFFER_LENGTH) );
				esif_ws_socket_build_payload(recieved_string, dataSize, buffer, &frameSize, TEXT_FRAME);
#endif

				if (esif_ws_server_write_to_socket(clientSocket, buffer, frameSize) == EXIT_FAILURE) {
					printf("websocket: error writing to socket line %d\n", __LINE__);
					g_websocket_state = closed;

					if (recieved_string) {
						esif_ccb_free(recieved_string);
					}
					return EOF;
				}

				esif_ws_server_setup_frame(clientSocket, &frameSize, buffer);
			} else {
				/*esif_ws_server_setup_buffer(&frameSize, buffer);
				   esif_ws_socket_build_payload(NULL, 0, buffer, &frameSize, CLOSING_FRAME);
				   esif_ws_server_write_to_socket(clientSocket, buffer, frameSize);
				   g_websocket_state = closed;
				   return EOF;*/
			}
		}	/* End of else if client[clientSocket].state == STATE_OPENING*/
	}

	if (recieved_string) {
		esif_ccb_free(recieved_string);
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
	esif_ccb_mutex_lock(&g_web_socket_lock);
	g_client[socket].frameType = INCOMPLETE_FRAME;
	esif_ccb_mutex_unlock(&g_web_socket_lock);
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
	int index;
	for (index = 0; index < NUM_CLIENTS; ++index) {
		g_client[index].state     = STATE_OPENING;
		g_client[index].frameType = INCOMPLETE_FRAME;
		esif_ws_socket_initialize_frame(&g_client[index].prot);
		esif_ws_socket_initialize_message_buffer(&g_client[index].buf);
	}
}


static void*esif_ws_server_hanlde_keep_alive_thread (void *ptr)
{
	// UNREFERENCED_PARAMETER(ptr);
	UInt8 buffer[BUFFER_LENGTH];
	size_t frameSize = BUFFER_LENGTH;
	ptr = ptr;	// keep compiler happy

	for ( ; ; ) {
		esif_ccb_sleep(20);

		if (g_websocket_state) {
			// printf("send keep alive message \n");
			esif_ccb_mutex_lock(&g_web_socket_lock);
			g_client[g_websocket_fd].frameType = TEXT_FRAME;
			g_client[g_websocket_fd].state     = STATE_NORMAL;
			esif_ccb_mutex_unlock(&g_web_socket_lock);

			esif_ws_server_setup_buffer(&frameSize, buffer);
			esif_ws_socket_build_payload((UInt8*)"", esif_ccb_strlen("", 3), buffer, &frameSize, TEXT_FRAME);
			if (esif_ws_server_write_to_socket(g_websocket_fd, buffer, frameSize) == EXIT_FAILURE) {
				printf("recv failed @ line %d\n", __LINE__);
			}
			esif_ws_server_setup_frame(g_websocket_fd, &frameSize, buffer);
		}
	}
}


static eEsifError esif_ws_server_hanlde_keep_alive_start ()
{
	esif_ccb_thread_create(&g_hanlde_keep_alive_thread, esif_ws_server_hanlde_keep_alive_thread, NULL);
	return ESIF_OK;
}


static void esif_ws_server_hanlde_keep_alive_stop ()
{
	// TODO shutdown _keep_alive thread cleanly.
	esif_ccb_mutex_destroy(&g_web_socket_lock);
}