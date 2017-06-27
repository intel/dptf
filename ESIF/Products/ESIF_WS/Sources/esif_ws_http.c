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

#include <stdio.h>
#include <stdlib.h>

#include "esif_ccb_file.h"
#include "esif_ccb_string.h"
#include "esif_ccb_time.h"

#include "esif_ws_http.h"
#include "esif_ws_socket.h"
#include "esif_ws_server.h"
#include "esif_ws_version.h"

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
static void esif_ws_http_process_buffer(char*, ssize_t, ssize_t);
static int esif_ws_http_process_request(ClientRecordPtr , char *, ssize_t);
static int  esif_ws_http_process_static_pages(ClientRecordPtr , char *, ssize_t, char *, char *);
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
	ssize_t bufSize,
	ssize_t msgLen
	)
{
	eEsifError rc = ESIF_OK;
	int httpStatus = HTTP_STATUS_OK;

	WS_TRACE_DEBUG("esif_ws_http_process_reqs \n");
	esif_ws_http_process_buffer((char *) buf, bufSize, msgLen);

	httpStatus = esif_ws_http_process_request(connection, buf, bufSize);
	if (httpStatus != HTTP_STATUS_OK) {
		rc = ESIF_E_WS_DISC;
	}
	return rc;
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

static int esif_ws_http_process_static_pages (
	ClientRecordPtr connection,
	char *buffer,
	ssize_t bufferSize,
	char *resource,
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
	ssize_t msgLen = 0;
	const char *docRoot = EsifWsDocRoot();
	const char *logRoot = EsifWsLogRoot();

	// Do not server pages in Restricted Mode
	if (g_ws_restricted)
		return HTTP_STATUS_FORBIDDEN;

	if (docRoot != NULL)
		esif_ccb_sprintf(sizeof(file_to_open), file_to_open, "%s" ESIF_PATH_SEP "%s", docRoot, resource);
	
	// Log file workaround: If not found in HTML folder, look in LOG folder
	if (esif_ccb_stat(file_to_open, &st) != 0 && logRoot != NULL) {
		esif_ccb_sprintf(sizeof(file_to_open), file_to_open, "%s" ESIF_PATH_SEP "%s", logRoot, resource);
		if (esif_ccb_stat(file_to_open, &st) != 0) {
			status = HTTP_STATUS_NOT_FOUND;
			goto exit;
		}
	}
	if (file_to_open[0] == 0) {
		goto exit;
	}

	// Check If-Modified-Since: header, if available, and return 304 Not Modified if requested file is unchanged
	if ((modified_gmt = esif_ccb_strstr(buffer, if_modified_since)) != NULL) {
		modified_gmt += esif_ccb_strlen(if_modified_since, MAX_PATH);
		time_t modified = esif_ws_http_time_local(modified_gmt);

		if (st.st_mtime <= modified) {
			status = HTTP_STATUS_NOT_MODIFIED;
			esif_ccb_sprintf(bufferSize, buffer,
					"HTTP/1.1 %d Not Modified" CRLF
					"Server: ESIF_UF/%s" CRLF
					"Date: %s" CRLF
					"Connection: close" CRLF
					CRLF,
				status,
				ESIF_WS_VERSION,
				esif_ws_http_time_stamp(time(0), tmpbuffer));

			esif_ws_client_write_to_socket(connection, buffer, esif_ccb_strlen(buffer, bufferSize));
			goto exit;
		}
	}

	// Open and Serve File
	file_fp = esif_ccb_fopen((esif_string)file_to_open, (esif_string)"rb", NULL);
	if (NULL == file_fp) {
		status = HTTP_STATUS_NOT_FOUND;
		goto exit;
	}

	esif_ccb_fseek(file_fp, (off_t)0, SEEK_END);
	esif_ccb_fseek(file_fp, (off_t)0, SEEK_SET);

	// Add Content-Disposition header to prompt user with Save-As Dialog if unknown file type
	if (esif_ccb_strcmp(fileType, UNKNOWN_MIME_TYPE) == 0) {
		esif_ccb_sprintf(sizeof(content_disposition), content_disposition,  "Content-Disposition: attachment; filename=\"%s\";" CRLF, resource);
	}

	status = HTTP_STATUS_OK;
	esif_ccb_sprintf(bufferSize, buffer,
					"HTTP/1.1 %d OK" CRLF
					"Server: ESIF_UF/%s" CRLF
					"Last-Modified: %s" CRLF
					"Date: %s" CRLF
					"Content-Type: %s" CRLF
					"Content-Length: %ld" CRLF
					"%s"
					"Connection: close" CRLF
					CRLF,
				status,
				ESIF_WS_VERSION,
				esif_ws_http_time_stamp(st.st_mtime, tmpbuffer),
				esif_ws_http_time_stamp(time(0), tmpbuffer), 
				fileType, 
				(long)st.st_size, 
				content_disposition);

	if (esif_ws_client_write_to_socket(connection, buffer, esif_ccb_strlen(buffer, bufferSize)) == EXIT_SUCCESS) {
		while ((msgLen = (int)esif_ccb_fread(buffer, bufferSize, 1, bufferSize, file_fp)) > 0) {
			if (esif_ws_client_write_to_socket(connection, buffer, msgLen) == EXIT_FAILURE) {
				break;
			}
		}
	}
	esif_ccb_fclose(file_fp);

exit:
	WS_TRACE_DEBUG("HTTP: status=%d, type=%s, file=%s\n", status, fileType, file_to_open);
	return status;
}


static void esif_ws_http_process_buffer (
	char *buffer,
	ssize_t bufferSize,
	ssize_t msgLen
	)
{
	if ((msgLen == 0) || (msgLen == -1)) {
		WS_TRACE_DEBUG("failed to read browser request\n");
	}

	if ((msgLen > 0) && (msgLen < bufferSize)) {
		buffer[msgLen] = '\0';
	} else {
		buffer[0] = '\0';
	}
}


static int esif_ws_http_process_request (
	ClientRecordPtr connection,
	char *buffer,
	ssize_t bufferSize
	)
{
	int httpStatus = HTTP_STATUS_OK;
	char *uriPtr = NULL;
	char *secondBlankPtr = NULL;
	char *questMarkCharPtr = NULL;
	char *resource = NULL;
	ssize_t resourceSize = 0;
	char *fileName = NULL;
	char *fileType = NULL;

	/*
	 * 5.1 - Request-Line = Method SP Request-URI SP HTTP-Version CRLF
	 * If method unknown, return 501 - Not Implemented.
	 */
	if (strncmp(buffer, "GET ", 4) &&
		strncmp(buffer, "POST ", 5)) {
		httpStatus = HTTP_STATUS_NOT_IMPLEMENTED;
		goto exit;
	}

	uriPtr = esif_ccb_strchr(buffer, ' ');
	if (NULL == uriPtr) {
		/* Logically should never happen; unless failure in strchr */
		httpStatus = HTTP_STATUS_INTERNAL_SERVER_ERROR;
		goto exit;
	}

	uriPtr++;
	if ('\0' == *uriPtr) {
		httpStatus = HTTP_STATUS_BAD_REQUEST;
		goto exit;
	}

	secondBlankPtr = esif_ccb_strchr(uriPtr, ' ');
	if (NULL == secondBlankPtr) {
		httpStatus = HTTP_STATUS_BAD_REQUEST;
		goto exit;	
	}

	questMarkCharPtr = esif_ccb_strchr(uriPtr, '?');

	/* special case */
	if (questMarkCharPtr && (questMarkCharPtr < secondBlankPtr)) {
		resourceSize = (questMarkCharPtr - uriPtr) + 1;
	} else {
		resourceSize = (secondBlankPtr - uriPtr) + 1;
	}

	resource = (char *)esif_ccb_malloc(resourceSize);
	if (NULL == resource) {
		httpStatus = HTTP_STATUS_INTERNAL_SERVER_ERROR;
		goto exit;
	}

	esif_ccb_memcpy(resource, uriPtr, resourceSize - 1);
	resource[resourceSize - 1] = '\0';

	WS_TRACE_DEBUG("resource b4: %s\n", resource);
	if (resource[1] == '\0') {
		WS_TRACE_DEBUG("empty resource: %s\n", resource);
		httpStatus = esif_ws_http_process_static_pages(connection, buffer, bufferSize, "index.html", "text/html");
		goto exit;
	}

	fileName = &resource[1];

	fileType = esif_ws_http_get_file_type(fileName);
	if (NULL == fileType) {
		fileType = UNKNOWN_MIME_TYPE;
	}

	httpStatus = esif_ws_http_process_static_pages(connection, buffer, bufferSize, fileName, fileType);
exit:
	esif_ccb_free(resource);
	if (httpStatus != HTTP_STATUS_OK) {
		esif_ws_http_send_error_code(connection, httpStatus);
	}
	return httpStatus;
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
		char *message = NULL;

		switch(error_code) {
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
		
		esif_ccb_sprintf(sizeof(buffer), buffer, (char*)"HTTP/1.1 %d %s" CRLF CRLF "<h1>%d %s</h1>", error_code, message, error_code, message);
		esif_ws_client_write_to_socket(connection, buffer, esif_ccb_strlen(buffer, sizeof(buffer)));
	}
	esif_ws_client_close_client(connection);
}