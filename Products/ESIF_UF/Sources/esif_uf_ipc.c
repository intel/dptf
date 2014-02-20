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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_IPC

#include "esif_uf.h"		/* Upper Framework */
#include "esif_ipc.h"		/* IPC Abstraction */
#include "esif_uf_shell.h"	/* Upper Framework Shell */
#include "esif_dsp.h"		/* Device Support Package */
#include "esif_version.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

void ipc_disconnect();

int g_timestamp = 0;	// Time Stamp IPC Execute
int g_ipcmode   = 0;	// IPC Mode

// Time Helper
int timeval_subtract(
	struct timeval *result,
	struct timeval *x,
	struct timeval *y
	)
{
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec  += nsec;
	}
	if ((x->tv_usec - y->tv_usec) > 1000000) {
		int nsec = (x->tv_usec - y->tv_usec) / 1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec  -= nsec;
	}

	/* Compute the time remaining to wait.
	   tv_usec is certainly positive. */
	result->tv_sec  = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}


#define IPC_DEBUG	// ESIF_DEBUG

#define SESSION_ID "ESIF"
extern int g_quit;
extern int g_disconnectClient;
extern int g_timestamp;

enum esif_rc ipc_execute(struct esif_ipc *ipc);

esif_handle_t g_ipc_handle = ESIF_INVALID_HANDLE;

// String To Short
u16 convert_string_to_short(char *two_character_string)
{
	return *((short *)(two_character_string));
}


///////////////////////////////////////////////////////////////////////////////
// IPC
///////////////////////////////////////////////////////////////////////////////

extern char g_esif_kernel_version[64]; // "Kernel Version = XXXXX\n"
extern char g_out_buf[64 * 1024];

// This extracts the Kernel version from the string returned by esif_cmd_info()
static void extract_kernel_version(char *str, size_t buf_len)
{
	char *prefix = "Kernel Version = ";
	size_t len = esif_ccb_strlen(prefix, buf_len);

	if (str != NULL && esif_ccb_strnicmp(str, "Kernel Version = ", len) == 0) {
		esif_ccb_strcpy(str, str+len, buf_len);
		len = esif_ccb_strlen(str, buf_len);
		if (len > 1 && str[len-1] == '\n') {
			str[len-1] = 0;
		}
	}
}

// IPC Connect
void ipc_connect()
{
	g_ipc_handle = esif_ipc_connect((char *)SESSION_ID);
	if (g_ipc_handle != ESIF_INVALID_HANDLE) {
		char *kern_str = esif_cmd_info(g_out_buf);
		ESIF_TRACE_DEBUG("ESIF IPC Kernel Device Opened\n");
		if (NULL != kern_str) {
			// Extract just the Kernel LF Version from the result string
			extract_kernel_version(kern_str, sizeof(g_out_buf));

			// Validate Kernel LF version is compatible with UF version
			if (esif_ccb_strcmp(kern_str, ESIF_VERSION) == 0) {
				ESIF_TRACE_DEBUG("Kernel Version: %s", kern_str);
				esif_ccb_sprintf(sizeof(g_esif_kernel_version), g_esif_kernel_version, "%s", kern_str);
			}
			else {
				ESIF_TRACE_ERROR("ESIF_LF Version (%s) Incompatible with ESIF_UF Version (%s)\n", kern_str, ESIF_VERSION);
				ipc_disconnect();
			}
		}
	}
}


// IPC Auto Connect
void ipc_autoconnect()
{
	if (g_ipc_handle != ESIF_INVALID_HANDLE) {
		return;
	}

	while (!g_quit) {
		ipc_connect();
		if (g_ipc_handle != ESIF_INVALID_HANDLE) {
			break;
		}
		esif_ccb_sleep(1);
	}
}


// IPC Disconnect
void ipc_disconnect()
{
	esif_ipc_disconnect(g_ipc_handle);
	g_ipc_handle = ESIF_INVALID_HANDLE;

	ESIF_TRACE_DEBUG("ESIF IPC Kernel Device Closed\n");
}


/* Declared and Handled By UF Shell */
extern struct esif_uf_dm g_dm;
extern int g_dst;

// IPC Execute
enum esif_rc ipc_execute(struct esif_ipc *ipc)
{
	enum esif_rc rc = ESIF_OK;
	struct timeval start  = {0};
	struct timeval finish = {0};
	struct timeval result;

	if (g_ipc_handle == ESIF_INVALID_HANDLE) {
		rc = ESIF_E_NO_LOWER_FRAMEWORK;
		goto exit;
	}

	if (g_timestamp) {
		esif_ccb_get_time(&start);
	}
	rc = esif_ipc_execute(g_ipc_handle, ipc);
	if (g_timestamp) {
		esif_ccb_get_time(&finish);
	}

	if (g_timestamp) {
		ESIF_TRACE_DEBUG("Start time: %06lu.%06lu\n",
						 start.tv_sec, start.tv_usec);
		ESIF_TRACE_DEBUG("Finish time: %06lu.%06lu\n",
						 finish.tv_sec, finish.tv_usec);

		timeval_subtract(&result, &finish, &start);
		ESIF_TRACE_DEBUG("IPC Exec Time: %06lu.%06lu (%06lu usecs)\n",
						 result.tv_sec, result.tv_usec, result.tv_usec);
	}

exit:
	return rc;
}


