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
static void esif_ws_cgi_parse_query (char*, const char*, int, char*);

static void esif_ws_cgi_parse_script (const char*, int, char*);

static void esif_ws_cgi_redirect_output (char*, int);


/*
 *******************************************************************************
 ** PUBLIC
 *******************************************************************************
 */
int esif_ws_cgi_execute_cgi_script (
	const char *resourceLoc,
	int fd,
	char *path
	)
{
	char *stringQuery = NULL;

	stringQuery = (char*)strchr(resourceLoc, '?');

	if (stringQuery) {
		esif_ws_cgi_parse_query(stringQuery, resourceLoc, fd, path);
	} else {
		esif_ws_cgi_parse_script(resourceLoc, fd, path);
	}


	return EOF;
}


static void esif_ws_cgi_parse_query (
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
		#define MAX_SIZE 100

	cgi_script_size = query - beg_parms_loc_in_query;
	cgi_script = (char*)esif_ccb_malloc(cgi_script_size + 1);
	if (cgi_script == NULL) {
		exit(1);
	}
	// cgi_dir = (char*)esif_ccb_malloc(esif_ccb_strlen(cgi_dir, MAX_SIZE) + cgi_script_size);
	// if (cgi_dir == NULL)
	// exit(1);

	esif_ccb_memcpy(cgi_script, beg_parms_loc_in_query, cgi_script_size);
	// esif_ccb_memcpy(cgi_dir, DIRECTORY, esif_ccb_strlen(DIRECTORY, MAX_SIZE));

	cgi_script[cgi_script_size] = '\0';
	// cgi_dir[esif_ccb_strlen(DIRECTORY, MAX_SIZE)]='\0';
	printf("cgi_script 1: %s\n", cgi_script);
	query++;

	cgi_script_buf = cgi_script;

#ifdef ESIF_ATTR_OS_WINDOWS

	cgi_script_buf = (char*)strrchr(cgi_script_buf, '/');
	cgi_script_buf++;
	cgi_script_buf[esif_ccb_strlen(cgi_script_buf, MAX_SIZE)] = '\0';
#endif


	esif_ccb_strcpy(cgiParms, query, esif_ccb_strlen(query, MAX_SIZE));

	printf("cgi_dir: %s\n", cgi_dir);

	printf("cgiParms 1: %s\n", cgiParms);

	esif_ccb_strcat(cgi_dir, cgi_script_buf, sizeof(cgi_dir));


	esif_ws_cgi_redirect_output(cgi_dir, fd);
	if (cgi_script) {
		esif_ccb_free(cgi_script);
	}
	// if (cgi_dir)
	// esif_ccb_free(cgi_dir);
}


static void esif_ws_cgi_parse_script (
	const char *resourceLoc,
	int fd,
	char *full_script_name
	)
{
	char abbr_script_name[100];
	u32 i = 0;


	esif_ccb_memcpy(abbr_script_name, resourceLoc, esif_ccb_strlen(resourceLoc, MAX_SIZE));
	abbr_script_name[esif_ccb_strlen(resourceLoc, MAX_SIZE)] = 0;
	printf("cgi script name: %s\n", abbr_script_name);
	i = i;	// to satisfy compiler
#ifdef ESIF_ATTR_OS_WINDOWS
	for (i = 0; i < esif_ccb_strlen(abbr_script_name, MAX_SIZE); i++)
		if (abbr_script_name[i] == '/') {
			abbr_script_name[i] = '\\';
		}

#endif


	esif_ccb_strcat(full_script_name, abbr_script_name, sizeof(full_script_name));

	printf("full_cgi_script_name: %s\n", full_script_name);

	esif_ws_cgi_redirect_output(full_script_name, fd);
}


static void esif_ws_cgi_redirect_output (
	char *script,
	int fd
	)
{
	int current_out;
	char buffer[BUFFER_LENGTH];
	int ret;
	FILE *dataFile = NULL;
	#define MAX_SIZE 100
	printf("esif_ws_cgi_redirect_output-> full_script_name: %s\n", script);


	current_out = esif_ccb_dup(1);

	if (current_out == -1) {
		perror("_dup( 1 ) failure");
		exit(1);
	}

	esif_ccb_fopen(&dataFile, "data", "w");

	if (NULL == dataFile) {
		printf("failed to open file in writing mode \n");
		return;
	}

	if (-1 == esif_ccb_dup2(dataFile, 1)) {
		perror("Can't _dup2 stdout");
		exit(1);
	}

	printf("Execute the script\n");
	system(script);

	fflush(stdout);
	fclose(dataFile);

	esif_ccb_dup3(current_out, 1);
	esif_ccb_flushall();


	esif_ccb_fopen(&dataFile, "data", "r");
	if (NULL == dataFile) {
		printf("failed to open file in readig mode: \n");
		return;
	}

	(long)fseek(dataFile, (off_t)0, SEEK_END);
	(void)fseek(dataFile, (off_t)0, SEEK_SET);

	(void)esif_ccb_sprintf(BUFFER_LENGTH, buffer, (char*)"HTTP/1.1 200 OK\n");

	(void)send(fd, buffer, (int)esif_ccb_strlen(buffer, MAX_SIZE), 0);

	while ((ret = (int)fread(buffer, 1, BUFFER_LENGTH, dataFile)) > 0)
		(void)send(fd, buffer, ret, 0);

	fclose(dataFile);

	esif_ws_server_initialize_clients();

	esif_ws_http_set_login_requested(1);
	esif_ws_http_set_authenticated(1);
}


