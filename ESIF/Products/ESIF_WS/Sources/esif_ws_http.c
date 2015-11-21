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

#include <stdio.h>
#include <stdlib.h>
#include "esif_ws_http.h"
#include "esif_uf_version.h"
#include "esif_ccb_file.h"
#include "esif_ws_socket.h"
#include "esif_ws_server.h"

#define VERSION "1.0"
#define UNKNOWN_MIME_TYPE	"application/octet-stream"

extType g_exts[] = {
	{(char*)"gif",  (char*)"image/gif"                  },
	{(char*)"jpg",  (char*)"image/jpg"                  },
	{(char*)"jpeg", (char*)"image/jpeg"                 },
	{(char*)"png",  (char*)"image/png"                  },
	{(char*)"ico",  (char*)"image/ico"                  },
	{(char*)"zip",  (char*)"image/zip"                  },
	{(char*)"gz",   (char*)"image/gz"                   },
	{(char*)"tar",  (char*)"image/tar"                  },
	{(char*)"htm",  (char*)"text/html"                  },
	{(char*)"html", (char*)"text/html"                  },
	{(char*)"xml",  (char*)"text/xml"                   },
	{(char*)"js",   (char*)"application/javascript"     },
	{(char*)"css",  (char*)"text/css"                   },
	{(char*)"txt",  (char*)"text/plain"                 },
	{(char*)0,      (char*)UNKNOWN_MIME_TYPE            }
};

size_t length;

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

// HTTP Status Codes Returned to Client
#define	HTTP_STATUS_OK						200
#define	HTTP_STATUS_NOT_MODIFIED			304
#define	HTTP_STATUS_BAD_REQUEST				400
#define	HTTP_STATUS_UNAUTHORIZED			401
#define	HTTP_STATUS_FORBIDDEN				403
#define	HTTP_STATUS_NOT_FOUND				404
#define	HTTP_STATUS_METHOD_NOT_ALLOWED		405
#define HTTP_STATUS_INTERNAL_SERVER_ERROR	500
#define HTTP_STATUS_NOT_IMPLEMENTED			501

/*
 *******************************************************************************
 ** EXTERN
 *******************************************************************************
 */

extern Bool g_ws_restricted;

/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */
static char *esif_ws_http_time_stamp(time_t, char *);
static time_t esif_ws_http_time_local(char *);
static void esif_ws_http_process_buffer(char*, ssize_t);
static void esif_ws_http_process_request(ClientRecordPtr , char *, ssize_t);
static int  esif_ws_http_process_static_page(ClientRecordPtr , char *, char *, ssize_t, char *);
static char *esif_ws_http_get_file_type(char *);
static void esif_ws_http_send_error_code(ClientRecordPtr , int);

/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */

eEsifError esif_ws_http_process_reqs (
	ClientRecordPtr connection,
	void *buf,
	ssize_t ret
	)
{
	char *buffer = (char*)buf;
	eEsifError result = ESIF_OK;

	ESIF_TRACE_DEBUG("esif_ws_http_process_reqs \n");
	esif_ws_http_process_buffer(buffer, ret);

	// ESIF_TRACE_DEBUG("Buffer received: %s\n", buffer);

	if (!strncmp(buffer, "GET ", 4) || !strncmp(buffer, "POST ", 5)) {	/* look for Get request method*/
		esif_ws_http_process_request(connection, buffer, ret);
		result = ESIF_E_WS_DISC; /* HTTP_STATUS_NOT_FOUND */
	} else {
		result = ESIF_E_WS_DISC; /* HTTP_STATUS_METHOD_NOT_ALLOWED */
	}
	return result;
}

static char*esif_ws_http_time_stamp (
	time_t what_time,
	char *buf
	)
{
	struct tm gmt={0};
	esif_ccb_gmtime(&gmt, &what_time);
	strftime(buf, 64, "%a, %d %b %Y %H:%M:%S GMT", &gmt);
	return buf;
}

// Convert "Ddd, dd Mmm yyyy hh:mm:ss GMT" format string to local time_t
static time_t esif_ws_http_time_local(char *str)
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
		if (timestamp.tm_year >= 1900)
			timestamp.tm_year -= 1900;

		// Compute local GMT offset to convert GMT time to local time
		time(&now);
		esif_ccb_gmtime(&gmt, &now);
		esif_ccb_localtime(&local, &now);
		datetime = mktime(&timestamp) - (mktime(&gmt) - mktime(&local));
	}
	return datetime;
}

static int esif_ws_http_process_static_pages (
	ClientRecordPtr connection,
	char *buffer,
	char *resource,
	ssize_t ret,
	char *fileType
	)
{
	int status = HTTP_STATUS_INTERNAL_SERVER_ERROR;
	static const char *if_modified_since = "If-Modified-Since: ";
	char *modified_gmt = NULL;
	struct stat st = { 0 };
	char tmpbuffer[128]={0};
	char file_to_open[MAX_PATH]={0};
	char content_disposition[MAX_PATH]={0};
	FILE *file_fp = NULL;

	// Do not server pages in Restricted Mode
	if (g_ws_restricted)
		return HTTP_STATUS_FORBIDDEN;

	esif_build_path(file_to_open, sizeof(file_to_open), ESIF_PATHTYPE_UI, resource, NULL);
	
	// Log file workaround: If not found in HTML folder, look in LOG folder
	if (esif_ccb_stat(file_to_open, &st) != 0) {
		char logpath[MAX_PATH] = { 0 };
		esif_build_path(logpath, sizeof(logpath), ESIF_PATHTYPE_LOG, resource, NULL);
		if (esif_ccb_stat(logpath, &st) != 0) {
			status = HTTP_STATUS_NOT_FOUND;
			goto exit;
		}
		esif_ccb_strcpy(file_to_open, logpath, sizeof(file_to_open));
	}

	// Check If-Modified-Since: header, if available, and return 304 Not Modified if requested file is unchanged
	if ((modified_gmt = esif_ccb_strstr(buffer, if_modified_since)) != NULL) {
		modified_gmt += esif_ccb_strlen(if_modified_since, MAX_PATH);
		time_t modified = esif_ws_http_time_local(modified_gmt);

		if (st.st_mtime <= modified) {
			status = HTTP_STATUS_NOT_MODIFIED;
			esif_ccb_sprintf(WS_BUFFER_LENGTH, buffer,
					"HTTP/1.1 %d Not Modified\n"
					"Server: ESIF_UF/%s\n"
					"Date: %s\n"
					"Connection: close\n"
					"\n",
				status,
				ESIF_UF_VERSION,
				esif_ws_http_time_stamp(time(0), tmpbuffer));

			send(connection->socket, buffer, (int)esif_ccb_strlen(buffer, WS_BUFFER_LENGTH), ESIF_WS_SEND_FLAGS);
			goto exit;
		}
	}

	// Open and Serve File
	esif_ccb_fopen(&file_fp, (esif_string)file_to_open, (esif_string)"rb");
	if (NULL == file_fp) {
		status = HTTP_STATUS_NOT_FOUND;
		goto exit;
	}

	esif_ccb_fseek(file_fp, (off_t)0, SEEK_END);
	esif_ccb_fseek(file_fp, (off_t)0, SEEK_SET);

	// Add Content-Disposition header to prompt user with Save-As Dialog if unknown file type
	if (esif_ccb_strcmp(fileType, UNKNOWN_MIME_TYPE) == 0) {
		esif_ccb_sprintf(sizeof(content_disposition), content_disposition,  "Content-Disposition: attachment; filename=\"%s\";\n", resource);
	}

	status = HTTP_STATUS_OK;
	esif_ccb_sprintf(WS_BUFFER_LENGTH, buffer,
					"HTTP/1.1 %d OK\n"
					"Server: ESIF_UF/%s\n"
					"Last-Modified: %s\n"
					"Date: %s\n"
					"Content-Type: %s\n"
					"Content-Length: %ld\n"
					"%s"
					"Connection: close\n"
					"\n",
				status,
				ESIF_UF_VERSION,
				esif_ws_http_time_stamp(st.st_mtime, tmpbuffer),
				esif_ws_http_time_stamp(time(0), tmpbuffer), 
				fileType, 
				(long)st.st_size, 
				content_disposition);

	send(connection->socket, buffer, (int)esif_ccb_strlen(buffer, WS_BUFFER_LENGTH), ESIF_WS_SEND_FLAGS);
	while ((ret = (int)esif_ccb_fread(buffer, WS_BUFFER_LENGTH, 1, WS_BUFFER_LENGTH, file_fp)) > 0) {
		send(connection->socket, buffer, (int)ret, ESIF_WS_SEND_FLAGS);
	}
	esif_ccb_fclose(file_fp);

exit:
	ESIF_TRACE_DEBUG("HTTP: status=%d, type=%s, file=%s\n", status, fileType, file_to_open);
	return status;
}


static void esif_ws_http_process_buffer (
	char *buffer,
	ssize_t size
	)
{
	if (size == 0 || size == -1) {
		ESIF_TRACE_DEBUG("failed to read browser request\n");
	}

	if (size > 0 && size < WS_BUFFER_LENGTH) {
		buffer[size] = 0;
	} else {
		buffer[0] = 0;
	}
}


static void esif_ws_http_process_request (
	ClientRecordPtr connection,
	char *buffer,
	ssize_t ret
	)
{
	char *blankCharPtr0    = NULL;
	char *blankCharPtr1    = NULL;
	char *questMarkCharPtr = NULL;
	char resource[100]={0};
	char *fileName=NULL;
	char *method=NULL;
	char *fileType = NULL;
	int result     = 0;

	if (!strncmp(buffer, "GET ", 4)) {
		method = "GET";
	} else {
		method = "POST";
	}

	blankCharPtr0    = strchr(buffer, ' ');
	blankCharPtr0++;
	blankCharPtr1    = strchr(blankCharPtr0, ' ');
	questMarkCharPtr = strchr(blankCharPtr0, '?');


	/* special case */
	if (questMarkCharPtr && questMarkCharPtr < blankCharPtr1) {
		// resource =  (char *)esif_ccb_malloc(questMarkCharPtr - blankCharPtr0 );
		if (!strcmp(method, "GET")) {
			esif_ccb_memcpy(resource, buffer + 4, questMarkCharPtr - blankCharPtr0);
		} else {
			esif_ccb_memcpy(resource, buffer + 5, questMarkCharPtr - blankCharPtr0);
		}
		resource[questMarkCharPtr - blankCharPtr0] = 0;
	} else {
		// resource =  (char *)esif_ccb_malloc(blankCharPtr1 - blankCharPtr0);
		if (!strcmp(method, "GET")) {
			esif_ccb_memcpy(resource, buffer + 4, blankCharPtr1 - blankCharPtr0);
		} else {
			esif_ccb_memcpy(resource, buffer + 5, blankCharPtr1 - blankCharPtr0);
		}
		resource[blankCharPtr1 - blankCharPtr0] = 0;
	}

	ESIF_TRACE_DEBUG("resource b4: %s\n", resource);
	if (resource[1] == '\0') {
		ESIF_TRACE_DEBUG("empty resource: %s\n", resource);
		result = esif_ws_http_process_static_pages(connection, buffer, "index.html", ret, "text/html");
		if (result != HTTP_STATUS_OK)
		{
			esif_ws_http_send_error_code(connection, result);	
		}
		return;
	}

	fileName = &resource[1];

	fileType = esif_ws_http_get_file_type(fileName);
	if (NULL == fileType) {
		fileType = UNKNOWN_MIME_TYPE;
	}

	result = esif_ws_http_process_static_pages(connection, buffer, fileName, ret, fileType);

	if (result != HTTP_STATUS_OK) {
		esif_ws_http_send_error_code(connection, result);
	}
}


static char*esif_ws_http_get_file_type (char *resource)
{
	char *fileType = NULL;
	char *ext = NULL;
	int i=0;

	ext = strrchr(resource, '.');
	if (!ext)
	{
		return NULL;
	}
	ext++;

	for (i = 0; g_exts[i].fileExtension != 0; i++)
		if (!strncmp(ext, g_exts[i].fileExtension, esif_ccb_strlen(ext, MAX_PATH))) {
			fileType = g_exts[i].fileType;
			break;
		}
	
	if (fileType == NULL)
		fileType = g_exts[i].fileType;
	return fileType;
}


static void esif_ws_http_send_error_code (
	ClientRecordPtr connection,
	int error_code
	)
{
	// Close connection with no response in Restricted Mode, otherwise return HTTP Status
	if (!g_ws_restricted) {
		char buffer[MAX_PATH] = { 0 };
		char *message = (error_code == HTTP_STATUS_NOT_FOUND ? "Not Found" : "Server Error");

		esif_ccb_sprintf(sizeof(buffer), buffer, (char*)"HTTP/1.1 %d %s\r\n\r\n<h1>%d %s</h1>", error_code, message, error_code, message);
		send(connection->socket, buffer, (int)esif_ccb_strlen(buffer, sizeof(buffer)), ESIF_WS_SEND_FLAGS);
	}
	esif_ws_client_close_client(connection);
}