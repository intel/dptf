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

#define ESIF_ATTR_SHA1	// SHA-1 Support only

#include <stdio.h>
#include <stdlib.h>

#include "esif_ccb_file.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"
#include "esif_ccb_time.h"
#include "esif_sdk_sha.h"

#include "esif_ws_server.h"
#include "esif_ws_http.h"
#include "esif_ws_version.h"

#ifdef ESIF_ATTR_OS_WINDOWS
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#include "esif_sdk_sha.c"	// Compile SHA code into this module

#define	HTTP_STATUS_OK						200
#define	HTTP_STATUS_NOT_MODIFIED			304
#define	HTTP_STATUS_BAD_REQUEST				400
#define	HTTP_STATUS_UNAUTHORIZED			401
#define	HTTP_STATUS_FORBIDDEN				403
#define	HTTP_STATUS_NOT_FOUND				404
#define	HTTP_STATUS_METHOD_NOT_ALLOWED		405
#define HTTP_STATUS_INTERNAL_SERVER_ERROR	500
#define HTTP_STATUS_NOT_IMPLEMENTED			501

#define WS_MAX_URL				(1024)			// Max HTTP URL
#define WS_MAX_HEADERS			(4*1024)		// Max HTTP Request+Headers
#define WS_MAX_DATETIMESTR		30				// Max HTTP GMT Date Time String Length

#define	MIME_TYPE_UNKNOWN	"application/octet-stream"

#define BASE64_ENCODED_LENGTH(bytes)	(((((bytes) + 2) / 3) * 4) + 1)

// Base64 Encode a Binary buffer into a Destination string
char *esif_base64_encode(
	char *destination,
	size_t dest_bytes,
	const void *source,
	size_t src_bytes)
{
	static const char base64_asciimap[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};
	char *result = NULL;
	size_t i = 0, j = 0;
	UInt8 block_of_bytes[4] = { 0 };
	const UInt8 *data = (const UInt8 *)source;

	if (dest_bytes >= (((src_bytes + 2) / 3) * 4) + 1) {

		for (i = 0; i < src_bytes / 3; ++i) {
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = ((data[i * 3 + 0] << 4) | (data[i * 3 + 1] >> 4)) & 0x3F;
			block_of_bytes[2] = ((data[i * 3 + 1] << 2) | (data[i * 3 + 2] >> 6)) & 0x3F;
			block_of_bytes[3] = data[i * 3 + 2] & 0x3F;

			for (j = 0; j < 4; ++j)
				*destination++ = base64_asciimap[block_of_bytes[j]];
		}

		switch (src_bytes % 3) {
		case 0:
			break;

		case 1:
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = (data[i * 3 + 0] << 4) & 0x3F;

			*destination++ = base64_asciimap[block_of_bytes[0]];
			*destination++ = base64_asciimap[block_of_bytes[1]];
			*destination++ = '=';
			*destination++ = '=';
			break;

		case 2:
			block_of_bytes[0] = data[i * 3 + 0] >> 2;
			block_of_bytes[1] = ((data[i * 3 + 0] << 4) | (data[i * 3 + 1] >> 4)) & 0x3F;
			block_of_bytes[2] = (data[i * 3 + 1] << 2) & 0x3F;

			*destination++ = base64_asciimap[block_of_bytes[0]];
			*destination++ = base64_asciimap[block_of_bytes[1]];
			*destination++ = base64_asciimap[block_of_bytes[2]];
			*destination++ = '=';
			break;

		default:
			break;
		}

		*destination = '\0';
		result = destination;
	}
	return result;
}

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

// Return given HTTP Request URI
char *WebClient_HttpGetURI(WebClientPtr self)
{
	char *result = NULL;
	if (self && self->httpRequest) {
		result = self->httpRequest[0];
	}
	return result;
}

// Return associated MIME Type for an HTTP Request based on file extension
const char *Http_GetMimeType(const char *resource)
{
	static struct {
		const char *ext;
		const char *type;
	} mimeTypes[] = {
		{ "gif",  "image/gif" },
		{ "jpg",  "image/jpg" },
		{ "jpeg", "image/jpeg" },
		{ "png",  "image/png" },
		{ "ico",  "image/ico" },
		{ "zip",  "image/zip" },
		{ "gz",   "image/gz" },
		{ "tar",  "image/tar" },
		{ "htm",  "text/html" },
		{ "html", "text/html" },
		{ "xml",  "text/xml" },
		{ "js",   "application/javascript" },
		{ "css",  "text/css" },
		{ "txt",  "text/plain" },
		{ NULL,   NULL }
	};
	char *extension = NULL;

	if (resource && ((extension = esif_ccb_strrchr(resource, '.')) != NULL)) {
		size_t j = 0;
		extension++;

		for (j = 0; mimeTypes[j].ext != NULL; j++) {
			if (esif_ccb_stricmp(mimeTypes[j].ext, extension) == 0) {
				return mimeTypes[j].type;
			}
		}
	}
	return MIME_TYPE_UNKNOWN;
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
			(esif_ccb_stricmp(connection, "Upgrade") != 0)) {
			rc = ESIF_E_INVALID_REQUEST_TYPE;
		}
		// Validate Version unless Protocol specified
		else if (!sec_websocket_protocol && esif_ccb_strcmp(sec_websocket_version, "13") != 0) {
			rc = ESIF_E_INVALID_REQUEST_TYPE;
		}

		// Validate Origin:
		// 1. Host: and Origin: must be an exact match (minus http prefix) unless Origin: in whitelist
		// 2. Host: and Origin: must be "127.0.0.1:port" or "localhost:port" for Restricted Mode
		if (rc == ESIF_OK) {
			char *origin_wsclient = (client->mode == ServerRestricted ? NULL : "chrome-extension://pfdhoblngboilpfeibdedpjgfnlcodoo");
			char *origin_whitelist[] = { "null", "file://", NULL, NULL }; // Origin: values sent by browsers when loading html via file instead of url
			char *origin_prefixes[] = { "http://", NULL }; // Origin: prefixes allowed for Restrcited and Non-Restricted modes
			Bool origin_valid = ESIF_FALSE;
			Bool prefix_valid = ESIF_FALSE;
			int  item = 0;
			origin_whitelist[2] = origin_wsclient;

			// Origin: exact matches in Whitelist allowed in BOTH Restricted and Non-Restricted modes
			// This is necessary so index.html can be loaded from filesystem to validate Web Server functionality in both modes
			for (item = 0; !origin_valid && origin_whitelist[item] != NULL; item++) {
				if (esif_ccb_stricmp(origin, origin_whitelist[item]) == 0) {
					origin_valid = ESIF_TRUE;
				}
			}

			// Origin: prefixes in Prefix List allowed in Restricted and Non-Restricted modes
			for (item = 0; !origin_valid && !prefix_valid && origin_prefixes[item] != NULL; item++) {
				size_t item_len = esif_ccb_strlen(origin_prefixes[item], WS_MAX_URL);
				if (esif_ccb_strncmp(origin, origin_prefixes[item], item_len) == 0) {
					origin += item_len;
					prefix_valid = ESIF_TRUE;
				}
			}

			// Origin: and Host: must be an exact match except for Whitelisted Origins
			// Only "127.0.0.1:port" "localhost:port" are valid Origins in Restricted mode except for Whitelisted Origins
			if (!origin_valid && prefix_valid && esif_ccb_stricmp(host, origin) == 0) {
				if ((client->mode != ServerRestricted) ||
					(esif_ccb_strncmp(origin, "127.0.0.1:", 10) == 0) ||
					(esif_ccb_strnicmp(origin, "localhost:", 10) == 0)) {
					origin_valid = ESIF_TRUE;
				}
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
				CRLF,
				response_key);

			rc = WebClient_Write(client, self->netBuf, bytes);
		}

		// Client is now a WebSocket Connection
		if (rc == ESIF_OK) {
			client->type = ClientWebsocket;

			// Only allow Event Subscribers from specific User Agents for now
			// Future versions should do this based on a Subscription request message from client
			if (user_agent && esif_ccb_stricmp(user_agent, "DptfMonitor/1.0") == 0) {
				client->isSubscriber = ESIF_TRUE;
			}

			// Clear HTTP Request
			esif_ccb_free(client->httpBuf);
			esif_ccb_free(client->httpRequest);
			client->httpStatus = 0;
			client->httpBuf = NULL;
			client->httpRequest = NULL;

			// Use Non-Blocking I/O for all WebSocket connections
			unsigned long nonBlockingOpt = 1;
			esif_ccb_socket_ioctl(client->socket, FIONBIO, &nonBlockingOpt);
			UNREFERENCED_PARAMETER(nonBlockingOpt);
		}
	}
	return rc;
}

// Convert a local time_t to "Ddd, dd Mmm yyyy hh:mm:ss GMT" format string
static char *Http_GmtFromLocalTime(
	char *buffer,
	size_t buf_len,
	time_t when
)
{
	struct tm gmt = { 0 };
	esif_ccb_gmtime(&gmt, &when);
	strftime(buffer, buf_len, "%a, %d %b %Y %H:%M:%S GMT", &gmt);
	return buffer;
}

// Convert "Ddd, dd Mmm yyyy hh:mm:ss GMT" format string to a local time_t
static time_t Http_LocalTimeFromGmt(char *str)
{
	static const char *szMonthName[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static const char *szDayName[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char month[4] = { 0 };
	char day[4] = { 0 };
	struct tm timestamp = { 0 };
	time_t datetime = 0;

	if (esif_ccb_sscanf(str, "%3s, %d %3s %d %d:%d:%d GMT",
		SCANFBUF(day, sizeof(day)),
		&timestamp.tm_mday,
		SCANFBUF(month, sizeof(month)),
		&timestamp.tm_year,
		&timestamp.tm_hour,
		&timestamp.tm_min,
		&timestamp.tm_sec) == 7) {
		struct tm gmt = { 0 };
		struct tm local = { 0 };
		time_t now = 0;
		int j = 0;

		for (j = 0; j < sizeof(szMonthName) / sizeof(szMonthName[0]); j++) {
			if (esif_ccb_strnicmp(month, szMonthName[j], 3) == 0) {
				timestamp.tm_mon = j;
				break;
			}
		}
		for (j = 0; j < sizeof(szDayName) / sizeof(szDayName[0]); j++) {
			if (esif_ccb_strnicmp(day, szDayName[j], 3) == 0) {
				timestamp.tm_wday = j;
				break;
			}
		}
		if (timestamp.tm_year >= 1900 && timestamp.tm_year < 3000)
			timestamp.tm_year -= 1900;

		// Compute local GMT offset to convert GMT time to local time
		time(&now);
		esif_ccb_gmtime(&gmt, &now);
		esif_ccb_localtime(&local, &now);
		datetime = mktime(&timestamp) - (mktime(&gmt) - mktime(&local));
	}
	return datetime;
}

// Send an HTTP Status code to the client and close the connection
esif_error_t WebServer_HttpSendError(WebServerPtr self, WebClientPtr client, int httpStatus)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Close connection with no response in Restricted Mode, otherwise return HTTP Status
	if (self && client) {
		client->httpStatus = httpStatus;

		rc = ESIF_OK;
		if (client->mode != ServerRestricted) {
			char *message = NULL;

			switch (httpStatus) {
			case HTTP_STATUS_NOT_FOUND:
				message = "Not Found";
				break;
			case HTTP_STATUS_NOT_IMPLEMENTED:
				message = "Not Implemented";
				break;
			default:
				message = "Server Error";
				break;
			}

			size_t bytes = esif_ccb_sprintf(self->netBufLen, (char *)self->netBuf,
				"HTTP/1.1 %d %s" CRLF
				"Connection: close" CRLF
				CRLF
				"<h1>%d %s</h1>",
				httpStatus,
				message,
				httpStatus,
				message);

			rc = WebClient_Write(client, self->netBuf, bytes);
		}
		WebClient_Close(client);
	}
	return rc;
}

// Send an HTTP Response for the given Request
esif_error_t WebServer_HttpResponse(WebServerPtr self, WebClientPtr client)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	if (self && client) {
		const char *docRoot = EsifWsDocRoot();
		const char *logRoot = EsifWsLogRoot();
		const char *docType = MIME_TYPE_UNKNOWN;
		char *uri = WebClient_HttpGetURI(client);
		char *if_modified_since = NULL;
		char resource[MAX_PATH] = { 0 };
		char resource_path[MAX_PATH] = { 0 };
		Bool completed = ESIF_FALSE;

		// Serve Requested Document except for invalid requests
		if (docRoot && uri && esif_ccb_strstr(uri, "..") == NULL) {
			struct stat st = { 0 };

			// Default Document
			esif_ccb_strcpy(resource, uri, sizeof(resource));
			if (esif_ccb_strcmp(resource, "/") == 0) {
				esif_ccb_strcat(resource, "index.html", sizeof(resource));
			}
			docType = Http_GetMimeType(resource);

			// Look for URI in HTTP Document Root first, then Log file Root
			esif_ccb_sprintf(sizeof(resource_path), resource_path, "%s%s", docRoot, resource);
			if (logRoot != NULL && esif_ccb_stat(resource_path, &st) != 0) {
				esif_ccb_sprintf(sizeof(resource_path), resource_path, "%s%s", logRoot, resource);
				if (esif_ccb_stat(resource_path, &st) != 0) {
					rc = WebServer_HttpSendError(self, client, HTTP_STATUS_NOT_FOUND);
					completed = ESIF_TRUE;
				}
			}

			// Check If-Modified-Since: header, if available, and return 304 Not Modified if requested file is unchanged
			if (completed == ESIF_FALSE && ((if_modified_since = WebClient_HttpGetHeader(client, "If-Modified-Since")) != NULL)) {
				time_t modified = Http_LocalTimeFromGmt(if_modified_since);
				char gmtdate[WS_MAX_DATETIMESTR] = { 0 };

				if (st.st_mtime <= modified) {
					client->httpStatus = HTTP_STATUS_NOT_MODIFIED;
					size_t bytes = esif_ccb_sprintf(self->netBufLen, (char *)self->netBuf,
						"HTTP/1.1 %d Not Modified" CRLF
						"Server: ESIF_UF/%s" CRLF
						"Date: %s" CRLF
						"Connection: close" CRLF
						CRLF,
						client->httpStatus,
						ESIF_WS_VERSION,
						Http_GmtFromLocalTime(gmtdate, sizeof(gmtdate), time(0)));

					rc = WebClient_Write(client, self->netBuf, bytes);
					completed = ESIF_TRUE;
				}
			}

			// Open and Serve File
			if (completed == ESIF_FALSE) {
				FILE *fp = esif_ccb_fopen(resource_path, "rb", NULL);
				if (fp == NULL) {
					WebServer_HttpSendError(self, client, HTTP_STATUS_NOT_FOUND);
					rc = ESIF_E_IO_OPEN_FAILED;
				}
				else {
					const char *content_fmt = "Content-Disposition: attachment; filename=\"%s\";" CRLF;
					char content_disposition[MAX_PATH + sizeof(content_fmt)] = { 0 };
					char modifiedbuf[WS_MAX_DATETIMESTR] = { 0 };
					char datebuf[WS_MAX_DATETIMESTR] = { 0 };

					// Add Content-Disposition header to prompt user with Save-As Dialog if unknown file type
					if (esif_ccb_strcmp(docType, MIME_TYPE_UNKNOWN) == 0) {
						esif_ccb_sprintf(sizeof(content_disposition), content_disposition, content_fmt, uri);
					}

					client->httpStatus = HTTP_STATUS_OK;
					size_t bytes = esif_ccb_sprintf(self->netBufLen, (char *)self->netBuf,
						"HTTP/1.1 %d OK" CRLF
						"Server: ESIF_UF/%s" CRLF
						"Last-Modified: %s" CRLF
						"Date: %s" CRLF
						"Content-Type: %s" CRLF
						"Content-Length: %ld" CRLF
						"%s"
						"Connection: close" CRLF
						CRLF,
						client->httpStatus,
						ESIF_WS_VERSION,
						Http_GmtFromLocalTime(modifiedbuf, sizeof(modifiedbuf), st.st_mtime),
						Http_GmtFromLocalTime(datebuf, sizeof(datebuf), time(0)),
						docType,
						(long)st.st_size,
						content_disposition);

					rc = WebClient_Write(client, self->netBuf, bytes);
					while (rc == ESIF_OK && ((bytes = esif_ccb_fread(self->netBuf, self->netBufLen, 1, self->netBufLen, fp)) > 0)) {
						rc = WebClient_Write(client, self->netBuf, bytes);
					}
					esif_ccb_fclose(fp);
				}
				completed = ESIF_TRUE;
			}
		}

		// Close Connection for unhandled requests
		if (completed == ESIF_FALSE) {
			WebServer_HttpSendError(self, client, HTTP_STATUS_BAD_REQUEST);
			rc = ESIF_E_WS_DISC;
		}
		WS_TRACE_DEBUG("HTTP: rc=%s (%d), status=%d, type=%s, file=%s\n", esif_rc_str(rc), rc, client->httpStatus, docType, resource_path);
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
			// Process Regular HTTP Request unless Restricted Mode
			else {
				if (client->mode == ServerRestricted) {
					rc = ESIF_E_WS_UNAUTHORIZED;
				}
				else {
					rc = WebServer_HttpResponse(self, client);
				}
			}
		}

		// Buffer Incomplete Requests and wait for more data
		if (rc == ESIF_E_WS_INCOMPLETE) {
			rc = ESIF_OK;
		}
	}
	return rc;
}
