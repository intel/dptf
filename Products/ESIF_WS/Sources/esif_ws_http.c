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


#define VERSION 23
// #define BASIC_AUTHENTICATION

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
	{(char*)0,      (char*)0                            }
};

size_t length;


static char g_server_root[1000];

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

extern eEsifError esif_ws_cgi_execute_cgi_script (const char*, int, char*);

void esif_ws_close_socket(int client_socket, int force);

/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */
static eEsifError esif_ws_http_serve_cgi_scripts (const char*, int);
static eEsifError esif_ws_http_process_post (const char*);
static char*esif_ws_http_time_stamp (time_t, char*);

static int esif_ws_http_server_static_pages (char*, char*, int, ssize_t, char*);

// static void esif_ws_http_handle_error_codes(char *, char *, int , ssize_t , char *);


static void esif_ws_http_send_error_code (int, int);

#ifdef BASIC_AUTHENTICATION
static void esif_ws_http_send_401 (int);
static int esif_ws_http_is_authorized (const char*);

#endif
static void esif_ws_http_process_buffer (char*, int);

static void esif_ws_http_process_get_or_post (char*, int, ssize_t);

static char*esif_ws_http_get_file_type (char*);

static int esif_ws_http_authenticated   = 0;
static int esif_ws_http_login_requested = 0;

const char *esif_ws_http_password = "intel123";


/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */
void esif_ws_http_copy_server_root (char *dir)
{
	#define MAX_LENGTH 200
	esif_ccb_memcpy(g_server_root, dir, esif_ccb_strlen(dir, MAX_LENGTH));
}


eEsifError esif_ws_http_process_reqs (
	int fd,
	void *buf,
	ssize_t ret
	)
{
	char *buffer = (char*)buf;
	char *cgiInd = 0;
	int result = EOF;
#ifdef BASIC_AUTHENTICATION
	char *resource = NULL;
	char *fileType = NULL;
#endif


	ESIF_TRACE_DEBUG("esif_ws_http_process_reqs \n");
	esif_ws_http_process_buffer(buffer, ret);

#ifdef BASIC_AUTHENTICATION
	if (!esif_ws_http_get_login_requested()) {
		ESIF_TRACE_DEBUG("Requesting login \n");
		resource = "login.html";
		fileType = "text/html";
		esif_ws_http_server_static_pages(buffer, resource, fd, ret, fileType);
		esif_ws_http_set_login_requested(1);
		result = 200;
	} else if (!esif_ws_http_get_authenticated()) {
		ESIF_TRACE_DEBUG("Authenticating login \n");
		if (esif_ws_http_is_authorized(buffer)) {
			esif_ws_http_set_authenticated(1);
			result = 200;
		} else {
			esif_ws_http_send_401(fd);
			esif_ws_http_set_login_requested(0);
			esif_ws_http_set_authenticated(0);
			result = EOF;
		}
	} else
#endif
	{
		// ESIF_TRACE_DEBUG("Buffer received: %s\n", buffer);

		if (!strncmp(buffer, "GET ", 4) || !strncmp(buffer, "POST ", 5)) {	/* look for Get request method*/
			/*is it a cgi script?*/
			cgiInd = strstr(buffer, "/cgi-bin/");
			if (cgiInd) {
				result = esif_ws_http_serve_cgi_scripts(buffer, fd);			
			} else {
				esif_ws_http_process_get_or_post(buffer, fd, ret);
				//result = EOF;
				result = ESIF_E_WS_DISC;
			}			
		} else {
			result = 405;	/*Method not allowed*/
		}
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


static eEsifError esif_ws_http_serve_cgi_scripts (
	const char *buffer,
	int fd
	)
{
	char *blankCharPtr=NULL;
	char *endPtr=NULL;
	char *uri=NULL;
	size_t uri_size=0;
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_DEBUG("CGI script\n");

	blankCharPtr = strchr((const char*)buffer, ' ');
	blankCharPtr++;
	endPtr = strchr(blankCharPtr, ' ');


	uri_size = endPtr - blankCharPtr;

	uri = (char*)esif_ccb_malloc(uri_size + 1);
	if (uri == NULL) {
		return ESIF_E_NO_MEMORY;
	}


	esif_ccb_memcpy(uri, buffer + 4, uri_size + 1);
	uri[uri_size] = 0;

	ESIF_TRACE_DEBUG("uri: %s\n", uri);

	if (!strncmp(buffer, "POST ", 5)) {
		rc = esif_ws_http_process_post((const char*)buffer);
	}

	rc = esif_ws_cgi_execute_cgi_script(uri, fd, g_server_root);

	if (uri) {
		esif_ccb_free(uri);
		uri = NULL;
	}

	return rc;
}


static eEsifError esif_ws_http_process_post (const char *buffer)
{
	char *begPtr=NULL;
	char *endPtr=NULL;
	char *namebox=NULL;
	char *nameboxVal     = NULL;
	char *passwordbox=NULL;
	char *passwordboxVal = NULL;
	size_t namebox_size=0;
	size_t passwordbox_size=0;
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_DEBUG("buffer from POST: %s\n", buffer);

	namebox = strstr(buffer, "namebox");
	if (namebox) {
		begPtr = strchr((const char*)namebox, '=');
		begPtr++;
		endPtr = strchr(begPtr, '&');
		namebox_size = endPtr - begPtr;
		nameboxVal   = (char*)esif_ccb_malloc(namebox_size + 1);
		if (NULL == nameboxVal) {
			return ESIF_E_NOT_FOUND;
		}
		esif_ccb_memcpy(nameboxVal, namebox + 8, namebox_size + 1);
		nameboxVal[namebox_size] = 0;
		ESIF_TRACE_DEBUG("name: %s ", nameboxVal);
	}

	passwordbox = strstr(buffer, "passwordbox");

	if (passwordbox) {
		begPtr = strchr((const char*)passwordbox, '=');
		begPtr++;
		endPtr = strchr(begPtr, '&');
		passwordbox_size = endPtr - begPtr;
		passwordboxVal   = (char*)esif_ccb_malloc(passwordbox_size + 1);
		if (NULL == passwordboxVal) {
			return ESIF_E_NOT_FOUND;
		}
		esif_ccb_memcpy(passwordboxVal, passwordbox + 12, passwordbox_size + 1);
		passwordboxVal[passwordbox_size] = 0;
		ESIF_TRACE_DEBUG("password: %s\n", passwordboxVal);
	}

	if (nameboxVal) {
		esif_ccb_free(nameboxVal);
		nameboxVal = NULL;
	}

	if (passwordboxVal) {
		esif_ccb_free(passwordboxVal);
		passwordboxVal = NULL;
	}

	return rc;
}


static int esif_ws_http_server_static_pages (
	char *buffer,
	char *resource,
	int fd,
	ssize_t ret,
	char *fileType
	)
{
	struct stat st={0};
	char tmpbuffer[128]={0};
	char file_to_open[1000]={0};
	FILE *file_fp = NULL;
	#define MAX_SIZE 200


	esif_ccb_memcpy(file_to_open, g_server_root, esif_ccb_strlen(g_server_root, MAX_SIZE));
	file_to_open[esif_ccb_strlen(g_server_root, MAX_SIZE)] = '\0';
	// ESIF_TRACE_DEBUG("file to open after esif_ccb_memcpy: %s\n", file_to_open);
	// ESIF_TRACE_DEBUG("resource : %s\n", resource);

#ifdef ESIF_ATTR_OS_WINDOWS

	strcat_s((esif_string)file_to_open, 1000, resource);
	esif_ccb_fopen(&file_fp, (esif_string)file_to_open, (esif_string)"rb");
#else
	strcat((esif_string)file_to_open, resource);
	esif_ccb_fopen(&file_fp, (esif_string)file_to_open, (esif_string)"r");
#endif
	ESIF_TRACE_DEBUG("file to open: %s\n", file_to_open);
	ESIF_TRACE_DEBUG("file type: %s\n", fileType);


	if (NULL == file_fp) {
		ESIF_TRACE_DEBUG("failed to open file: %s\n", file_to_open);
		return 404;
	}

	if (esif_ccb_stat(file_to_open, &st) != 0) {
		ESIF_TRACE_DEBUG("Could not stat file descriptor \n");
		fclose(file_fp);
		return 404;
	}


	(long)fseek(file_fp, (off_t)0, SEEK_END);
	(void)fseek(file_fp, (off_t)0, SEEK_SET);

	if (!strcmp(fileType, "text/xml")) {
		(void)esif_ccb_sprintf(BUFFER_LENGTH, buffer, (char*)"HTTP/1.1 200 OK\n<?xml version==\"1.0\" encoding=\"utf-8\"?>\n"
															 "Server: server/%d.0\n"
															 "Content-Type: %s\nContent-Length: %ld\nConnection: close\n\n",
							   VERSION, fileType, (long)st.st_size);
	} else {
		(void)esif_ccb_sprintf(BUFFER_LENGTH, buffer, (char*)"HTTP/1.1 200 OK\n"
															 "Server: server/%d.0\nLast-Modified: %s\nDate: %s\n"
															 "Content-Type: %s\nContent-Length: %ld\nConnection: close\n\n",
							   VERSION,
							   esif_ws_http_time_stamp(st.st_mtime, tmpbuffer), esif_ws_http_time_stamp(time(0), tmpbuffer), fileType, (long)st.st_size);
	}

	(void)send(fd, buffer, (int)esif_ccb_strlen(buffer, MAX_SIZE), 0);
	while ((ret = (int)fread(buffer, 1, BUFFER_LENGTH, file_fp)) > 0) {
		(void)send(fd, buffer, ret, 0);
	}
	fclose(file_fp);

	return 0;
}


#ifdef BASIC_AUTHENTICATION
static void esif_ws_http_send_401 (int fd)
{
	char buffer[256]={0};
	char tmpbuffer[128]={0};
	char *fileType = "text/html";
	FILE *file_fp  = NULL;
	int len=0;
	struct stat st={0};
	int ret=0;


	char *fileName = "error.html";

	ESIF_TRACE_DEBUG("fileName :%s\n", fileName);
	esif_ccb_fopen(&file_fp, fileName, "r");

	if (NULL == file_fp) {
		ESIF_TRACE_DEBUG("failed to open file: \n");
		return;
	}

	if (esif_ccb_stat(fileName, &st) != 0) {
		ESIF_TRACE_DEBUG("Could not stat file descriptor \n");
		return;
	}

	len = (long)fseek(file_fp, (off_t)0, SEEK_END);

	(void)fseek(file_fp, (off_t)0, SEEK_SET);

	(void)esif_ccb_sprintf(256, buffer, (char*)"HTTP/1.1 401 Unauthorized\n"
											   "Server: server/%d.0\nLast-Modified: %s\nDate: %s\n"
											   "Content-Type: %s\nContent-Length: %ld\nConnection: close\n\n",
						   VERSION, esif_ws_http_time_stamp(st.st_mtime, tmpbuffer), esif_ws_http_time_stamp(time(0), tmpbuffer), fileType, (long)st.st_size);

	(void)send(fd, buffer, (int)esif_ccb_strlen(buffer), 0);

	while ((ret = (int)fread(buffer, 1, 256, file_fp)) > 0)
		(void)send(fd, buffer, ret, 0);

	fclose(file_fp);
}


static int esif_ws_http_is_authorized (const char *buffer)
{
	char *begPtr=NULL;
	char *endPtr=NULL;
	char *namebox=NULL;
	char *nameboxVal=NULL;
	char *passwordbox=NULL;
	char *passwordboxVal = NULL;
	int result=0;

	ESIF_TRACE_DEBUG("esif_ws_http_is_authorized : Buffer received: %s\n", buffer);

	namebox = strstr(buffer, "namebox");
	if (namebox) {
		begPtr     = strchr((const char*)namebox, '=');
		begPtr++;
		endPtr     = strchr(begPtr, '&');
		nameboxVal = (char*)esif_ccb_malloc(endPtr - begPtr + 1);
		esif_ccb_memcpy(nameboxVal, namebox + 8, endPtr - begPtr + 1);
		nameboxVal[endPtr - begPtr] = 0;
		ESIF_TRACE_DEBUG("name: %s ", nameboxVal);
	}

	passwordbox = strstr(buffer, "passwordbox");

	if (passwordbox) {
		begPtr = strchr((const char*)passwordbox, '=');
		begPtr++;
		endPtr = strchr(begPtr, '&');
		passwordboxVal = (char*)esif_ccb_malloc(endPtr - begPtr + 1);
		esif_ccb_memcpy(passwordboxVal, passwordbox + 12, endPtr - begPtr + 1);
		passwordboxVal[endPtr - begPtr] = 0;
		ESIF_TRACE_DEBUG("esif_ws_http_password: %s\n", passwordboxVal);
	}

	if (passwordboxVal && !strcmp(passwordboxVal, esif_ws_http_password)) {
		result = 1;
	} else {
		result = 0;
	}

	return result;
}


#endif

static void esif_ws_http_process_buffer (
	char *buffer,
	int size
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


static void esif_ws_http_process_get_or_post (
	char *buffer,
	int fd,
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
		result = esif_ws_http_server_static_pages(buffer, "index.html", fd, ret, "text/html");
		if (result > 0)
		{
			esif_ws_http_send_error_code(fd, result);
			
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
	if (NULL != fileType) {
		result = esif_ws_http_server_static_pages(buffer, fileName, fd, ret, fileType);
		
		if (result > 0)
		{
			esif_ws_http_send_error_code(fd, result);
			return; // DWP
		}
	
	} else {
		ESIF_TRACE_DEBUG("File type is not valid\n");
	}
	
	esif_ws_http_send_error_code(fd, result);
	result = result;
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
		if (!strncmp(ext, g_exts[i].fileExtension, esif_ccb_strlen(ext, MAX_SIZE))) {
			fileType = g_exts[i].fileType;
			break;
		}

	return fileType;
}


void esif_ws_http_set_authenticated (int value) {esif_ws_http_authenticated = value;}

void esif_ws_http_set_login_requested (int value) {esif_ws_http_login_requested = value;}

int esif_ws_http_get_authenticated (void) {return esif_ws_http_authenticated;}

int esif_ws_http_get_login_requested (void) {return esif_ws_http_login_requested;}

#define MAX_BUFFER_SIZE 100
static void esif_ws_http_send_error_code (
	int fd,
	int error_code
	)
{
	char buffer[MAX_BUFFER_SIZE]={0};

	error_code = error_code;

	(void)esif_ccb_sprintf(MAX_BUFFER_SIZE, buffer, (char*)"HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Not Found</h1>");

	(void)send(fd, buffer, (int)esif_ccb_strlen(buffer, MAX_SIZE), 0);
	esif_ws_close_socket(fd, 1);
}