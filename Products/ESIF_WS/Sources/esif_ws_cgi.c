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
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <limits.h>
#include <ctype.h>
#include <stdarg.h>


#include "esif.h"
#include "esif_ws_oswrap.h"
#include "esif_ws_http.h"
#include "esif_uf_ccb_system.h"


#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define MAX_SIZE 100

/*
 *******************************************************************************
 ** EXTERN
 *******************************************************************************
 */
extern void esif_ws_server_initialize_clients (void);


/*
 *******************************************************************************
 ** PRIVATE
 *******************************************************************************
 */
static eEsifError esif_ws_cgi_parse_query (char*, const char*, int, char*);

static int esif_ws_cgi_parse_script (const char*, int, char*);

static eEsifError esif_ws_cgi_redirect_output (char*, int);


/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */
eEsifError esif_ws_cgi_execute_cgi_script (
	const char *resourceLoc,
	int fd,
	char *path
	)
{
	char *stringQuery = NULL;
	int rc = EOF;

	stringQuery = (char*)strchr(resourceLoc, '?');

	if (stringQuery) {
		rc = esif_ws_cgi_parse_query(stringQuery, resourceLoc, fd, path);	    
	} else {
		esif_ws_cgi_parse_script(resourceLoc, fd, path);
	}


	return rc;
}


static eEsifError esif_ws_cgi_parse_query (
	char *query,
	const char *beg_parms_loc_in_query,
	int fd,
	char *cgi_dir
	)
{
	char *cgi_script;
	char *cgi_script_buf;
	// char *cgi_dir;
	char cgiParms[100];
	size_t cgi_script_size;
	eEsifError rc = ESIF_OK;
	

	cgi_script_size = query - beg_parms_loc_in_query;
	cgi_script = (char*)esif_ccb_malloc(cgi_script_size + 1);
	if (cgi_script == NULL) {
	    return ESIF_E_NOT_FOUND;
	}


	esif_ccb_memcpy(cgi_script, beg_parms_loc_in_query, cgi_script_size);


	cgi_script[cgi_script_size] = '\0';

	ESIF_TRACE_DEBUG("cgi_script 1: %s\n", cgi_script);
	query++;

	cgi_script_buf = cgi_script;

#ifdef ESIF_ATTR_OS_WINDOWS

	cgi_script_buf = (char*)strrchr(cgi_script_buf, '/');
	cgi_script_buf++;
	cgi_script_buf[esif_ccb_strlen(cgi_script_buf, MAX_SIZE)] = '\0';
#endif


	esif_ccb_strcpy(cgiParms, query, esif_ccb_strlen(query, MAX_SIZE));

	ESIF_TRACE_DEBUG("cgi_dir: %s\n", cgi_dir);

	ESIF_TRACE_DEBUG("cgiParms 1: %s\n", cgiParms);

	esif_ccb_strcat(cgi_dir, cgi_script_buf, esif_ccb_strlen(cgi_dir, MAX_SIZE));


	rc = esif_ws_cgi_redirect_output(cgi_dir, fd);
	if (cgi_script) {
		esif_ccb_free(cgi_script);
		cgi_script = NULL;
	}

	return rc;
}


static int esif_ws_cgi_parse_script (
	const char *resourceLoc,
	int fd,
	char *full_script_name
	)
{
	char abbr_script_name[100];
	u32 i = 0;
	int rc = 0;


	esif_ccb_memcpy(abbr_script_name, resourceLoc, esif_ccb_strlen(resourceLoc, MAX_SIZE));
	abbr_script_name[esif_ccb_strlen(resourceLoc, MAX_SIZE)] = 0;
	ESIF_TRACE_DEBUG("cgi script name: %s\n", abbr_script_name);
	i = i;	// to satisfy compiler
#ifdef ESIF_ATTR_OS_WINDOWS
	for (i = 0; i < esif_ccb_strlen(abbr_script_name, MAX_SIZE); i++)
		if (abbr_script_name[i] == '/') {
			abbr_script_name[i] = '\\';
		}

#endif


	esif_ccb_strcat(full_script_name, abbr_script_name, esif_ccb_strlen(full_script_name, MAX_SIZE));

	ESIF_TRACE_DEBUG("full_cgi_script_name: %s\n", full_script_name);

	esif_ws_cgi_redirect_output(full_script_name, fd);
	return rc;
}

#define MAX_BUFFER_SIZE 4096
static eEsifError esif_ws_cgi_redirect_output (
	char *script,
	int fd
	)
{
	int current_out;
	char buffer[MAX_BUFFER_SIZE];
	int ret;
	FILE *dataFile = NULL;
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_DEBUG("script: %s\n", script);
	
	current_out = esif_ccb_dup(1);

	if (current_out == -1) {
		ESIF_TRACE_DEBUG("_dup( 1 ) failure");
		return ESIF_E_NOT_FOUND;
	}

	esif_ccb_fopen(&dataFile, "data", "w");

	if (NULL == dataFile) {
		ESIF_TRACE_DEBUG("failed to open file in writing mode \n");
		return ESIF_E_NOT_FOUND;
	}

	if (-1 == esif_ccb_dup2(dataFile, 1)) {
		ESIF_TRACE_DEBUG("Can't _dup2 stdout");
		return ESIF_E_NOT_FOUND;
	}

	ESIF_TRACE_DEBUG("Execute the script\n");
	system(script);

	fflush(stdout);
	fclose(dataFile);

	(void)esif_ccb_dup3(current_out, 1);
	esif_ccb_flushall();


	esif_ccb_fopen(&dataFile, "data", "r");
	if (NULL == dataFile) {
		ESIF_TRACE_DEBUG("failed to open file in readig mode: \n");
		return ESIF_E_NOT_FOUND ;
	}

	(long)fseek(dataFile, (off_t)0, SEEK_END);
	(void)fseek(dataFile, (off_t)0, SEEK_SET);

	(void)esif_ccb_sprintf(MAX_BUFFER_SIZE, buffer, (char*)"HTTP/1.1 200 OK\n");

	(void)send(fd, buffer, (int)esif_ccb_strlen(buffer, MAX_SIZE), 0);

	while ((ret = (int)fread(buffer, 1, MAX_BUFFER_SIZE, dataFile)) > 0)
		(void)send(fd, buffer, ret, 0);

	fclose(dataFile);

	esif_ws_server_initialize_clients();

	esif_ws_http_set_login_requested(1);
	esif_ws_http_set_authenticated(1);

	return rc;
}


