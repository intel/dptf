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

#include <stdio.h>
#include <stdlib.h>
#include "esif_ws_http.h"
#include "esif_uf_version.h"

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


#include "esif_uf_ccb_file.h"

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

void esif_ws_close_socket(clientRecord *connection);

/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */
static char*esif_ws_http_time_stamp (time_t, char*);
static void esif_ws_http_process_buffer (char*, ssize_t);
static void esif_ws_http_process_request (clientRecord *, char*, ssize_t);
static int  esif_ws_http_process_static_page (clientRecord*, char*, char*, ssize_t, char*);
static char*esif_ws_http_get_file_type (char*);
static void esif_ws_http_send_error_code (clientRecord*, int);

/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */

eEsifError esif_ws_http_process_reqs (
	clientRecord *connection,
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
		result = ESIF_E_WS_DISC; /* 404 or Connection: close */
	} else {
		result = ESIF_E_WS_DISC; /* 405 Method not allowed*/
	}
	return result;
}


static char*esif_ws_http_time_stamp (
	time_t what_time,
	char *buf
	)
{
	struct tm gmt={0};
#ifdef ESIF_ATTR_OS_LINUX
	gmt = *gmtime(&what_time);
#else
	gmtime_s(&gmt, &what_time);
#endif
	strftime(buf, 64, "%a, %d %b %Y %H:%M:%S GMT", &gmt);
	return buf;
}


static int esif_ws_http_process_static_pages (
	clientRecord *connection,
	char *buffer,
	char *resource,
	ssize_t ret,
	char *fileType
	)
{
	struct stat st={0};
	char tmpbuffer[128]={0};
	char file_to_open[MAX_PATH]={0};
	char content_disposition[MAX_PATH]={0};
	FILE *file_fp = NULL;

	esif_build_path(file_to_open, sizeof(file_to_open), ESIF_PATHTYPE_UI, resource, NULL);
	esif_ccb_fopen(&file_fp, (esif_string)file_to_open, (esif_string)"rb");
	
	// Log file workaround: If not found in HTML folder, look in LOG folder
	if (NULL == file_fp) {
		char logpath[MAX_PATH]={0};
		esif_build_path(logpath, sizeof(logpath), ESIF_PATHTYPE_LOG, resource, NULL);
		esif_ccb_fopen(&file_fp, (esif_string)logpath, (esif_string)"rb");
		if (NULL != file_fp) 
			esif_ccb_strcpy(file_to_open, logpath, sizeof(file_to_open));
	}
	
	ESIF_TRACE_DEBUG("HTTP: file=%s, type=%s\n", file_to_open, fileType);
	if (NULL == file_fp) {
		ESIF_TRACE_DEBUG("failed to open file: %s\n", file_to_open);
		return 404;
	}

	if (esif_ccb_stat(file_to_open, &st) != 0) {
		ESIF_TRACE_DEBUG("Could not stat file descriptor \n");
		fclose(file_fp);
		return 404;
	}

	fseek(file_fp, (off_t)0, SEEK_END);
	fseek(file_fp, (off_t)0, SEEK_SET);

	// Add Content-Disposition header to prompt user with Save-As Dialog if unknown file type
	if (esif_ccb_strcmp(fileType, UNKNOWN_MIME_TYPE) == 0) {
		esif_ccb_sprintf(sizeof(content_disposition), content_disposition,  "Content-Disposition: attachment; filename=\"%s\";\n", resource);
	}

	esif_ccb_sprintf(BUFFER_LENGTH, buffer,	
					"HTTP/1.1 200 OK\n"
					"Server: ESIF_UF/%s\n"
					"Last-Modified: %s\n"
					"Date: %s\n"
					"Content-Type: %s\n"
					"Content-Length: %ld\n"
					"%s"
					"Connection: close\n"
					"\n",
				ESIF_UF_VERSION,
				esif_ws_http_time_stamp(st.st_mtime, tmpbuffer),
				esif_ws_http_time_stamp(time(0), tmpbuffer), 
				fileType, 
				(long)st.st_size, 
				content_disposition);

	send(connection->socket, buffer, (int)esif_ccb_strlen(buffer, BUFFER_LENGTH), 0);
	while ((ret = (int)fread(buffer, 1, BUFFER_LENGTH, file_fp)) > 0) {
		send(connection->socket, buffer, (int)ret, 0);
	}
	fclose(file_fp);

	return 0;
}


static void esif_ws_http_process_buffer (
	char *buffer,
	ssize_t size
	)
{
	if (size == 0 || size == -1) {
		ESIF_TRACE_DEBUG("failed to read browser request\n");
	}

	if (size > 0 && size < BUFFER_LENGTH) {
		buffer[size] = 0;
	} else {
		buffer[0] = 0;
	}
}


static void esif_ws_http_process_request (
	clientRecord *connection,
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

#ifdef ESIF_ATTR_OS_WINDOWS
	char *str=NULL;
#endif


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
		if (result > 0)
		{
			esif_ws_http_send_error_code(connection, result);	
		}
		return;
	}

	fileName = &resource[1];

	// resource++;
#ifdef ESIF_ATTR_OS_WINDOWS
	do {
		str = strchr(resource, '/');
		if (str) {
			str[0] = '\\';
		}
	} while (str != NULL);
	// ESIF_TRACE_DEBUG("str: %s\n", str);
#endif
	fileType = esif_ws_http_get_file_type(fileName);
	if (NULL == fileType) {
		fileType = UNKNOWN_MIME_TYPE;
	}

	result = esif_ws_http_process_static_pages(connection, buffer, fileName, ret, fileType);

	if (result > 0) {
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
	clientRecord *connection,
	int error_code
	)
{
	char buffer[MAX_PATH]={0};
	char *message = (error_code==404 ? "Not Found" : "Error");

	esif_ccb_sprintf(sizeof(buffer), buffer, (char*)"HTTP/1.1 %d %s\r\n\r\n<h1>%d %s</h1>", error_code, message, error_code, message);

	send(connection->socket, buffer, (int)esif_ccb_strlen(buffer, sizeof(buffer)), 0);
	esif_ws_close_socket(connection);
}