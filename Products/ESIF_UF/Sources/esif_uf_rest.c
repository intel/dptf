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

// #define ESIF_TRACE_DEBUG_DISABLED

#include "esif_uf_version.h"
#include "esif_uf.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Version */
const EsifString g_esif_erapi_version = ESIF_UF_VERSION;

#define REST_DEBUG ESIF_DEBUG

static esif_thread_t g_restthread;
static int g_restquit = ESIF_FALSE;

int esif_ws_push_xml_data (char *xml_data);

void*esif_rest_worker_thread (void *ptr)
{
	char filename_in[ESIF_PATH_LEN];
	char filename_out[ESIF_PATH_LEN];
	char status_out[ESIF_PATH_LEN];

	UNREFERENCED_PARAMETER(ptr);

#if defined(ESIF_ATTR_OS_WINDOWS)
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_in, "c:\\windows\\temp\\cmd.in");
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_out, "c:\\windows\\temp\\cmd.out");
	esif_ccb_sprintf(ESIF_PATH_LEN, status_out, "c:\\windows\\temp\\status.out");
#elif defined(ESIF_ATTR_OS_ANDROID)
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_in, "/storage/emulated/legacy/cmd.in");
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_out, "/storage/emulated/legacy/cmd.out");
	esif_ccb_sprintf(ESIF_PATH_LEN, status_out, "/storage/emulated/legacy/status.out");
#elif defined(ESIF_ATTR_OS_LINUX)
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_in, "/usr/local/bin/cmd.in");
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_out, "/usr/local/bin/cmd.out");
	esif_ccb_sprintf(ESIF_PATH_LEN, status_out, "/usr/local/bin/status.out");
#elif defined(ESIF_ATTR_OS_CHROME)
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_in, "/tmp/cmd.in");
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_out, "/tmp/cmd.out");
	esif_ccb_sprintf(ESIF_PATH_LEN, status_out, "/tmp/status.out");
#else
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_in, "%s%scmd.in", esif_build_path(filename_in, ESIF_PATH_LEN, ESIF_DIR_UI, ESIF_DIR_REST), ESIF_PATH_SEP);
	esif_ccb_sprintf(ESIF_PATH_LEN, filename_out, "%s%scmd.out", esif_build_path(filename_out, ESIF_PATH_LEN, ESIF_DIR_UI, ESIF_DIR_REST), ESIF_PATH_SEP);
	esif_ccb_sprintf(ESIF_PATH_LEN, status_out, "%s%sstatus.out", esif_build_path(status_out, ESIF_PATH_LEN, ESIF_DIR_UI, ESIF_DIR_REST), ESIF_PATH_SEP);
#endif

/* Preferred */
#ifdef ESIF_ATTR_WEBSOCKET
	printf("\nESIF_UF Extension: REST WebSocket\n");
	while (!g_restquit) {
		char cmd[80] = "status";
		EsifString result = parse_cmd(cmd, ESIF_TRUE);
		if (NULL != result) {
			esif_ws_push_xml_data(result);
		}
		esif_ccb_sleep_msec(1000);
	}
/*  Last Resort */
#else
	printf("\nESIF_UF Extension: REST FILE API path = %s\n", filename_in);

	while (!g_restquit) {
		FILE *fp_cmdin_ptr  = NULL;
		FILE *fp_cmdout_ptr = NULL;
		esif_ccb_fopen(&fp_cmdin_ptr, filename_in, "r");
		if (NULL == fp_cmdin_ptr) {
			FILE *fp_status_ptr   = NULL;
			struct stat file_stat = {0};

			// ESIF_TRACE_DEBUG("%s: REST API MailBox Empty [%s]\n", ESIF_FUNC, filename_in);

			if (esif_ccb_stat(status_out, &file_stat) != 0) {
				char cmd[80] = "status";
				EsifString result = parse_cmd(cmd, ESIF_TRUE);
				if (result != NULL) {
					esif_ccb_fopen(&fp_status_ptr, status_out, "w");
					if (NULL != fp_status_ptr) {
						fwrite(result, 1, esif_ccb_strlen(result, OUT_BUF_LEN), fp_status_ptr);
						fclose(fp_status_ptr);
					}
				}
			}
			esif_ccb_sleep_msec(100);
		} else {
			#define BUF_SIZE 4096
			char cmd_in[BUF_SIZE];
			size_t cmd_in_read     = 0;
			EsifString result      = NULL;
			EsifString ptr         = NULL;
			u32 msg_id = 0;
			EsifString command_buf = NULL;

			cmd_in_read = esif_ccb_fread(cmd_in, BUF_SIZE, 1, BUF_SIZE, fp_cmdin_ptr);
			cmd_in[cmd_in_read] = 0;

			ptr = cmd_in;
			while (*ptr != '\0') {
				if (*ptr == '\r' || *ptr == '\n' || *ptr == '#') {
					*ptr = '\0';
				}
				ptr++;
			}

			/* Messages may be sequenced if so they will start with 123:participants */
			command_buf = strchr(cmd_in, ':');
			if (NULL != command_buf) {
				*command_buf = 0;	/* replace : with NULL to terminate string 1 e.g. sequence number */
				command_buf++;		/* Move past NULL to start of string 2 e.g. command already NULL terminated */
				msg_id = atoi(cmd_in);
			} else {
				command_buf = &cmd_in[0];
				msg_id = 0;
			}

			ESIF_TRACE_DEBUG("%s: REST API MailBox Ready Bytes (%u) id %u cmd=%s\n",
							 ESIF_FUNC, (int)cmd_in_read, msg_id, command_buf);

			/* Have Command Hand To Shell */
			g_format = FORMAT_XML;
			result   = parse_cmd(command_buf, ESIF_TRUE);
			if (NULL != result) {
				if (msg_id > 0) {
					EsifString temp = esif_ccb_strdup(result);
					esif_ccb_sprintf(OUT_BUF_LEN, result, "%u:%s", msg_id, temp);
					esif_ccb_free(temp);
				}

				ESIF_TRACE_DEBUG("%s", result);
				esif_ccb_unlink(filename_out);
				esif_ccb_fopen(&fp_cmdout_ptr, filename_out, "w");
				if (NULL != fp_cmdout_ptr) {
					fwrite(result, 1, esif_ccb_strlen(result, OUT_BUF_LEN), fp_cmdout_ptr);
					fclose(fp_cmdout_ptr);
				}
			}

			if (fp_cmdin_ptr) {
				fclose(fp_cmdin_ptr);
			}
			esif_ccb_unlink(filename_in);
		}
	}
#endif
	return 0;
}


eEsifError EsifRestStart ()
{
	g_restquit = ESIF_FALSE;
	esif_ccb_thread_create(&g_restthread, esif_rest_worker_thread, NULL);

	return ESIF_OK;
}


eEsifError EsifRestStop ()
{
	ESIF_TRACE_DEBUG("%s:\n", ESIF_FUNC);
	g_restquit = ESIF_TRUE;
	return ESIF_OK;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
