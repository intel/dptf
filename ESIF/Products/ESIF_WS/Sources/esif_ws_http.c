/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "esif_ccb_file.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"
#include "esif_ccb_time.h"
#include "esif_sdk_sha.h"
#include "esif_sdk_base64.h"

#include "esif_ws_server.h"
#include "esif_ws_http.h"
#include "esif_ws_version.h"

#ifdef ESIF_ATTR_OS_WINDOWS
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#include "esif_sdk_sha.c"		// Compile SHA code into this module

#define	HTTP_STATUS_OK						200
#define	HTTP_STATUS_REDIRECT				302
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
#define MIME_TYPE_ENCODING	"; charset=utf-8"

static struct http_codes_s {
	int code;
	char* message;
} g_httpCodes[] = {
	{ HTTP_STATUS_OK,					"OK" },
	{ HTTP_STATUS_REDIRECT,				"Found" },
	{ HTTP_STATUS_NOT_MODIFIED,			"Not Modified" },
	{ HTTP_STATUS_BAD_REQUEST,			"Bad Request" },
	{ HTTP_STATUS_UNAUTHORIZED,			"Unauthorized" },
	{ HTTP_STATUS_FORBIDDEN,			"Forbidden" },
	{ HTTP_STATUS_NOT_FOUND,			"Not Found" },
	{ HTTP_STATUS_METHOD_NOT_ALLOWED,	"Method Not Allowed" },
	{ HTTP_STATUS_INTERNAL_SERVER_ERROR,"Server Error" },
	{ HTTP_STATUS_NOT_IMPLEMENTED,		"Not Implemented" },
};

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
				else {
					WS_TRACE_ERROR("HTTP Error [IP=%s]: Invalid Request: %s %s %s\n", self->ipAddr, method, uri, proto);
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
		{ "htm",  "text/html" MIME_TYPE_ENCODING },
		{ "html", "text/html" MIME_TYPE_ENCODING },
		{ "xml",  "text/xml" MIME_TYPE_ENCODING },
		{ "js",   "application/javascript" MIME_TYPE_ENCODING },
		{ "css",  "text/css" MIME_TYPE_ENCODING },
		{ "txt",  "text/plain" MIME_TYPE_ENCODING },
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
			char *origin_whitelist[] = { "null", "file://", NULL }; // Origin: values sent by browsers when loading html via file instead of url
			char *origin_prefixes[] = { "http://", NULL }; // Origin: prefixes allowed
			Bool origin_valid = ESIF_FALSE;
			Bool prefix_valid = ESIF_FALSE;
			int  item = 0;

			// Origin: exact matches in Whitelist allowed in addition to prefixes.
			// This is necessary so index.html can be loaded from filesystem to validate Web Server functionality in both modes
			for (item = 0; !origin_valid && origin_whitelist[item] != NULL; item++) {
				if (esif_ccb_stricmp(origin, origin_whitelist[item]) == 0) {
					origin_valid = ESIF_TRUE;
				}
			}

			// Origin: prefixes in Prefix List allowed.
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

			// Clear HTTP Request
			esif_ccb_free(client->httpBuf);
			esif_ccb_free(client->httpRequest);
			client->httpStatus = 0;
			client->httpBuf = NULL;
			client->httpRequest = NULL;

			// Use Non-Blocking I/O for all WebSocket connections
			unsigned long nonBlockingOpt = 1;
			esif_ccb_socket_ioctl(client->socket, FIONBIO, &nonBlockingOpt);
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
esif_error_t WebServer_HttpSendError(WebServerPtr self, WebClientPtr client, int httpStatus, const char* fmt, ...)
{
	esif_error_t rc = ESIF_E_PARAMETER_IS_NULL;

	// Return HTTP Status
	if (self && client) {
		client->httpStatus = httpStatus;

		rc = ESIF_OK;
		char *message = "Server Error";
		char *redirect = "";
			
		for (size_t j = 0; j < ESIF_ARRAY_LEN(g_httpCodes); j++) {
			if (g_httpCodes[j].code == httpStatus || g_httpCodes[j].code == 0) {
				message = g_httpCodes[j].message;
				if (httpStatus == HTTP_STATUS_REDIRECT) {
					redirect = "Location: /index.html" CRLF;
				}
				break;
			}
		}

		// Log Optional Custom Message
		if (fmt) {
			va_list args;
			va_start(args, fmt);
			int len = esif_ccb_vscprintf(fmt, args);
			char* logmsg = NULL;
			if (len > 0 && (logmsg = esif_ccb_malloc((size_t)len + 1)) != NULL) {
				esif_ccb_vsprintf((size_t)len + 1, logmsg, fmt, args);
				WS_TRACE_ERROR(
					"HTTP Error [IP=%s]: %d %s: %s\n",
					client->ipAddr,
					httpStatus,
					message,
					logmsg
				);
				esif_ccb_free(logmsg);
			}
			va_end(args);
		}

		size_t bytes = esif_ccb_sprintf(self->netBufLen, (char *)self->netBuf,
			"HTTP/1.1 %d %s" CRLF
			"%s"
			"Content-Length: 0" CRLF
			"Connection: close" CRLF
			CRLF,
			httpStatus,
			message,
			redirect
		);

		rc = WebClient_Write(client, self->netBuf, bytes);
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
		const char *docType = MIME_TYPE_UNKNOWN;
		char *uri = WebClient_HttpGetURI(client);
		char resource[MAX_PATH] = { 0 };
		char resource_path[MAX_PATH] = { 0 };
		Bool completed = ESIF_FALSE;

		// Serve Requested Document except for invalid requests
		if (docRoot && uri && esif_ccb_strstr(uri, "..") == NULL) {
			struct stat st = { 0 };

			// Default Document
			esif_ccb_strcpy(resource, uri, sizeof(resource));
			if (esif_ccb_strcmp(resource, "/") == 0) {
				rc = WebServer_HttpSendError(self, client, HTTP_STATUS_REDIRECT, NULL);
				completed = ESIF_TRUE;
			}
			else {
				docType = Http_GetMimeType(resource);

				// Look for URI in HTTP Document Root
				esif_ccb_sprintf(sizeof(resource_path), resource_path, "%s%s", docRoot, resource);
				if (esif_ccb_stat(resource_path, &st) != 0) {
					rc = WebServer_HttpSendError(self, client, HTTP_STATUS_NOT_FOUND, "Path=%s", resource);
					completed = ESIF_TRUE;
				}
			}

			// Open and Serve File
			if (completed == ESIF_FALSE) {
				FILE *fp = esif_ccb_fopen(resource_path, "rb", NULL);
				if (fp == NULL) {
					WebServer_HttpSendError(self, client, HTTP_STATUS_NOT_FOUND, "PathName=%s", resource_path);
					rc = ESIF_E_IO_OPEN_FAILED;
				}
				else {
					const char content_fmt[] = "Content-Disposition: attachment; filename=\"%s\";" CRLF;
					char content_disposition[MAX_PATH + sizeof(content_fmt)] = { 0 };
					char datebuf[WS_MAX_DATETIMESTR] = { 0 };

					// Add Content-Disposition header to prompt user with Save-As Dialog if unknown file type
					if (esif_ccb_strcmp(docType, MIME_TYPE_UNKNOWN) == 0) {
						esif_ccb_sprintf(sizeof(content_disposition), content_disposition, content_fmt, uri);
					}

					client->httpStatus = HTTP_STATUS_OK;
					size_t bytes = esif_ccb_sprintf(self->netBufLen, (char *)self->netBuf,
						"HTTP/1.1 %d OK" CRLF
						"Server: ESIF_UF/%s" CRLF
						"Date: %s" CRLF
						"Cache-Control: no-store, no-cache, must-revalidate" CRLF
						"Content-Type: %s" CRLF
						"Content-Length: %ld" CRLF
						"%s"
						"Content-Security-Policy: frame-ancestors 'none';" CRLF
						"Content-Security-Policy: default-src 'self' 'unsafe-inline' data:;" CRLF
						"Content-Security-Policy: script-src 'self' 'unsafe-inline';" CRLF
						"Content-Security-Policy: connect-src 'self';" CRLF
						"Content-Security-Policy: form-action 'none';" CRLF
						"Content-Security-Policy: sandbox allow-scripts allow-same-origin;" CRLF
						"X-Content-Type-Options: nosniff" CRLF
						"X-XSS-Protection: 1; mode=block" CRLF
						"X-Frame-Options: deny" CRLF
						"Connection: close" CRLF
						CRLF,
						client->httpStatus,
						ESIF_WS_VERSION,
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
			WebServer_HttpSendError(self, client, HTTP_STATUS_BAD_REQUEST, "URI=%s", uri);
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
			// Process Regular HTTP Request
			else {
				rc = WebServer_HttpResponse(self, client);
			}
		}

		// Buffer Incomplete Requests and wait for more data
		if (rc == ESIF_E_WS_INCOMPLETE) {
			rc = ESIF_OK;
		}
	}
	return rc;
}
