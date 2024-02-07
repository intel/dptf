/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#define ESIF_ATTR_SHA1	// SHA-1 Support only
#define ESIF_SDK_BASE64_ENCODER // Inlude Base-64 Encode Source

#include "esif_ccb_file.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"
#include "esif_ccb_time.h"
#include "esif_sdk_sha.h"
#include "esif_sdk_base64.h"

#include "ipfsrv_ws_server.h"
#include "ipfsrv_ws_http.h"
#include "ipfsrv_ws_version.h"

#include "ipfsrv_appmgr.h"


#include "esif_sdk_sha.c"	// Compile SHA code into this module

#define WS_MAX_URL				(1024)			// Max HTTP URL
#define WS_MAX_HEADERS			(4*1024)		// Max HTTP Request+Headers

// Process HTTP Request Headers into Request Buffers (if a complete request is available)
esif_error_t WebClient_HttpParseHeaders(WebClientPtr self, u8 *buffer, size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;
	char *httpBuf = NULL;
	char **httpRequest = NULL;

	if (self && buffer && buf_len > 0) {
		const char *crlfcrlf = CRLF CRLF;
		char *tok = NULL;
		char *ctx = NULL;

		rc = ESIF_E_INVALID_REQUEST_TYPE;

		// HTTP Request Headers must terminate with CRLFCRLF
		if (esif_ccb_memstr((char *)buffer, buf_len, crlfcrlf, esif_ccb_strlen(crlfcrlf, buf_len)) == NULL) {
			return ESIF_E_WS_INCOMPLETE;
		}
		if ((httpBuf = esif_ccb_malloc(buf_len + 1)) == NULL) {
			return ESIF_E_NO_MEMORY;
		}
		esif_ccb_memcpy(httpBuf, (char *)buffer, buf_len);
		httpBuf[buf_len] = 0;

		// Parse HTTP Request and Headers
		if ((tok = esif_ccb_strtok(httpBuf, CRLF, &ctx)) != NULL) {
			size_t requestLen = 0;
			size_t requestCount = 0;
			size_t growby = 5;

			// Parse HTTP Request and Headers (GET Request = httpRequest[0])
			do {
				if (requestCount + 1 >= requestLen) {
					requestLen += growby;
					char **new_request = (char **)esif_ccb_realloc(httpRequest, (sizeof(char *) * requestLen));
					if (new_request == NULL) {
						rc = ESIF_E_NO_MEMORY;
						goto exit;
					}
					esif_ccb_memset(&new_request[requestLen - growby], 0, sizeof(char *) * growby);
					httpRequest = new_request;
				}

				// Trim trailing whitespace
				size_t eoln = esif_ccb_strlen(tok, WS_MAX_HEADERS);
				while (eoln > 0 && tok[eoln - 1] == ' ') {
					tok[--eoln] = '\0';
				}
				if (httpRequest) {
					httpRequest[requestCount++] = tok;
				}

			} while ((tok = esif_ccb_strtok(NULL, CRLF, &ctx)) != NULL);

			// Validate HTTP Request
			if (httpRequest && httpRequest[0]) {
				char *reqctx = NULL;
				char *method = esif_ccb_strtok(httpRequest[0], " ", &reqctx);
				char *uri = (method ? esif_ccb_strtok(NULL, " ", &reqctx) : NULL);
				char *proto = (uri ? esif_ccb_strtok(NULL, " ", &reqctx) : NULL);

				// Only "GET <uri> HTTP/1.1" Requests supported at this time
				if (method && uri && proto && esif_ccb_strcmp(method, "GET") == 0 && esif_ccb_strcmp(proto, "HTTP/1.1") == 0) {
					esif_ccb_memmove(httpRequest[0], uri, esif_ccb_strlen(uri, WS_MAX_HEADERS) + 1);
					esif_ccb_free(self->httpBuf);
					esif_ccb_free(self->httpRequest);
					self->httpBuf = httpBuf;
					self->httpRequest = httpRequest;
					httpBuf = NULL;
					httpRequest = NULL;
					rc = ESIF_OK;
				}
			}
		}
	}
exit:
	esif_ccb_free(httpBuf);
	esif_ccb_free(httpRequest);
	return rc;
}

// Return given HTTP Request Header (or NULL if it does not exist)
char *WebClient_HttpGetHeader(WebClientPtr self, const char *label)
{
	char *result = NULL;
	if (self && label && self->httpRequest) {
		const char separator[] = ": ";
		size_t j = 0;
		size_t labelLen = esif_ccb_strlen(label, WS_MAX_HEADERS);
		for (j = 0; self->httpRequest[j]; j++) {
			if (j > 0 &&
				(esif_ccb_strnicmp(self->httpRequest[j], label, labelLen) == 0) &&
				(esif_ccb_strncmp(&self->httpRequest[j][labelLen], separator, sizeof(separator) - 1) == 0)) {
				return &self->httpRequest[j][labelLen + sizeof(separator) - 1];
			}
		}
	}
	return result;
}

// Upgrade an HTTP Connection to Websocket Protocol
esif_error_t WebServer_HttpWebsocketUpgrade(WebServerPtr self, WebClientPtr client)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && client) {
		char *upgrade = WebClient_HttpGetHeader(client, "Upgrade");
		char *connection = WebClient_HttpGetHeader(client, "Connection");
		char *host = WebClient_HttpGetHeader(client, "Host");
		char *origin = WebClient_HttpGetHeader(client, "Origin");
		char *user_agent = WebClient_HttpGetHeader(client, "User-Agent");
		char *sec_websocket_key = WebClient_HttpGetHeader(client, "Sec-WebSocket-Key");
		char *sec_websocket_protocol = WebClient_HttpGetHeader(client, "Sec-WebSocket-Protocol");
		char *sec_websocket_version = WebClient_HttpGetHeader(client, "Sec-WebSocket-Version");

		// Validate Required Headers
		rc = ESIF_OK;
		if (!upgrade || !connection || !host || !origin || !sec_websocket_key || !sec_websocket_version ||
			(esif_ccb_stricmp(upgrade, "websocket") != 0) ||
			(esif_ccb_strstr(connection, "Upgrade") == NULL)) {
			rc = ESIF_E_INVALID_REQUEST_TYPE;
		}
		// Validate Version unless Protocol specified
		else if (!sec_websocket_protocol && esif_ccb_strcmp(sec_websocket_version, "13") != 0) {
			rc = ESIF_E_INVALID_REQUEST_TYPE;
		}

		// Validate Origin:
		// 1. Host: and Origin: must be an exact match (minus http prefix) unless Origin: in whitelist
		if (rc == ESIF_OK) {
			char *origin_prefixes[] = { "http://", NULL }; // Origin: prefixes allowed
			Bool origin_valid = ESIF_FALSE;
			Bool prefix_valid = ESIF_FALSE;
			int  item = 0;
			
			// Origin: prefixes in Prefix List allowed
			for (item = 0; !origin_valid && !prefix_valid && origin_prefixes[item] != NULL; item++) {
				size_t item_len = esif_ccb_strlen(origin_prefixes[item], WS_MAX_URL);
				if (esif_ccb_strncmp(origin, origin_prefixes[item], item_len) == 0) {
					origin += item_len;
					prefix_valid = ESIF_TRUE;
				}
			}

			// Origin: and Host: must be an exact match except for Whitelisted Origins
			if (!origin_valid && prefix_valid && esif_ccb_stricmp(host, origin) == 0) {
				origin_valid = ESIF_TRUE;
			}

			// Return error if neither Whitelisted Origin nor a matching Host: and Origin: is present
			if (!origin_valid) {
				rc = ESIF_E_INVALID_REQUEST_TYPE;
			}
		}

		// Build and Send Websocket Protocol Upgrade Response
		if (rc == ESIF_OK) {
			esif_sha1_t sha_digest;
			char response_key[BASE64_ENCODED_LENGTH(sizeof(sha_digest.hash))] = { 0 };
			char guid_key[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
			size_t bytes = 0;

			esif_sha1_init(&sha_digest);
			esif_sha1_update(&sha_digest, sec_websocket_key, esif_ccb_strlen(sec_websocket_key, WS_MAX_HEADERS));
			esif_sha1_update(&sha_digest, guid_key, sizeof(guid_key) - 1);
			esif_sha1_finish(&sha_digest);
			esif_base64_encode(response_key, sizeof(response_key), sha_digest.hash, sha_digest.hashsize);

			bytes = esif_ccb_sprintf(self->netBufLen, (char *)self->netBuf,
				"HTTP/1.1 101 Switching Protocols" CRLF
				"Upgrade: websocket" CRLF
				"Connection: Upgrade" CRLF
				"Sec-WebSocket-Accept: %s" CRLF
				"Content-Length: 0" CRLF
				CRLF,
				response_key);

			rc = WebClient_Write(client, self->netBuf, bytes);
		}

		// Client is now a WebSocket Connection
		if (rc == ESIF_OK) {
			client->type = ClientWebsocket;

			// Only accept connections from specific User Agents for now
			if (user_agent && esif_ccb_stricmp(user_agent, "IpfClient/1.0") == 0) {
				client->ipfHandle = AppSessionMgr_GenerateHandle();
			}

			// Clear HTTP Request
			esif_ccb_free(client->httpBuf);
			esif_ccb_free(client->httpRequest);
			client->httpBuf = NULL;
			client->httpRequest = NULL;

			// Use Non-Blocking I/O for all WebSocket connections
			unsigned long nonBlockingOpt = 1;
			esif_ccb_socket_ioctl(client->socket, FIONBIO, &nonBlockingOpt);

			// Create RPC Worker Thread
			client->rpcWorker = WebWorker_Create();
			rc = WebWorker_Start(client->rpcWorker);
		}
	}
	return rc;
}

// Process an HTTP Request, if a complete Request is available
esif_error_t WebServer_HttpRequest(WebServerPtr self, WebClientPtr client, u8 *buffer, size_t buf_len)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && client && buffer && buf_len > 0) {

		// Parse HTTP Request and Headers
		rc = WebClient_HttpParseHeaders(client, buffer, buf_len);

		if (rc == ESIF_OK) {
			char *upgrade = WebClient_HttpGetHeader(client, "Upgrade");

			// WebSocket Upgrade Request: "Upgrade: websocket"
			if (upgrade && (esif_ccb_stricmp(upgrade, "websocket") == 0)) {
				rc = WebServer_HttpWebsocketUpgrade(self, client);
			}
			// All other HTTP Requests are Forbidden
			else {
				rc = ESIF_E_WS_UNAUTHORIZED;
			}
		}

		// Buffer Incomplete Requests and wait for more data
		if (rc == ESIF_E_WS_INCOMPLETE) {
			rc = ESIF_OK;
		}
	}
	return rc;
}
