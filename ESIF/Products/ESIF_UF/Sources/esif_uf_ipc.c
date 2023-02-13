/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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


void ipc_disconnect();

int g_timestamp = 0;	// Time Stamp IPC Execute

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


#define SESSION_ID "IPF_UF"
extern int g_quit;
extern int g_disconnectClient;
extern int g_timestamp;

esif_os_handle_t g_ipc_handle = INVALID_HANDLE_VALUE;

// String To Short
u16 domain_str_to_short(char *two_character_string)
{
	UInt16 domain = *((short*)(two_character_string));
	*((char *)&domain) = (char)toupper(*two_character_string);
	return domain;
}


///////////////////////////////////////////////////////////////////////////////
// IPC
///////////////////////////////////////////////////////////////////////////////

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS

eEsifError ipc_connect()
{
	return ESIF_E_NO_LOWER_FRAMEWORK;
}

eEsifError ipc_autoconnect(UInt32 max_retries)
{
	UNREFERENCED_PARAMETER(max_retries);
	return ESIF_E_NO_LOWER_FRAMEWORK;
}

void ipc_disconnect()
{
}

int ipc_isconnected()
{
	return ESIF_FALSE;
}

enum esif_rc ipc_execute(struct esif_ipc *ipc)
{
	UNREFERENCED_PARAMETER(ipc);
	return ESIF_E_NO_LOWER_FRAMEWORK;
}

#else

extern char g_esif_kernel_version[64]; // "Kernel Version = XXXXX\n"

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
eEsifError ipc_connect()
{
	eEsifError rc = ESIF_OK;
	int check_kernel_version = ESIF_TRUE;

	ESIF_TRACE_ENTRY_INFO();

	// Exit if IPC already connected
	if (g_ipc_handle != INVALID_HANDLE_VALUE) {
		return ESIF_OK;
	}

	// Connect to LF
	g_ipc_handle = esif_ipc_connect((char *)SESSION_ID);
	if (g_ipc_handle == INVALID_HANDLE_VALUE) {
		ESIF_TRACE_WARN("IPF LF is not available\n");
		rc = ESIF_E_NO_LOWER_FRAMEWORK;
	}
	else {
		char *outbuf = esif_ccb_malloc(OUT_BUF_LEN);
		char *kern_str = (outbuf != NULL ? esif_cmd_info(outbuf) : NULL);
		ESIF_TRACE_DEBUG("IPF IPC Kernel Device Opened\n");
		if (NULL != kern_str) {
			// Extract just the Kernel LF Version from the result string
			extract_kernel_version(kern_str, OUT_BUF_LEN);

			// Bypass Kernel Version check for DEBUG builds
			#if defined(ESIF_ATTR_DEBUG)
			check_kernel_version = ESIF_FALSE;
			#endif

			// Validate Kernel LF version is compatible with UF version
			if (check_kernel_version == ESIF_FALSE || esif_ccb_strcmp(kern_str, ESIF_VERSION) == 0) {
				ESIF_TRACE_INFO("Kernel Version: %s\n", kern_str);
				esif_ccb_sprintf(sizeof(g_esif_kernel_version), g_esif_kernel_version, "%s", kern_str);
			}
			else {
				ESIF_TRACE_ERROR("IPF_LF Version (%s) Incompatible with IPF_UF Version (%s)\n", kern_str, ESIF_VERSION);
				ipc_disconnect();
				rc = ESIF_E_NOT_SUPPORTED;
			}
		}
		esif_ccb_free(outbuf);
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}

// IPC Auto Connect
eEsifError ipc_autoconnect(UInt32 max_retries)
{
	eEsifError rc = ESIF_OK;
	UInt32 connect_retries = 0;

	ESIF_TRACE_ENTRY_INFO();

	if (g_ipc_handle != INVALID_HANDLE_VALUE) {
		return rc;
	}

	// Attempt to connect to LF indefinitely until ESIF exits (unless the LF version is unsupported)
	while (!g_quit) {
		rc = ipc_connect();
		if (rc == ESIF_OK || rc == ESIF_E_NOT_SUPPORTED) {
			break;
		}

		if (max_retries > 0 && ++connect_retries >= max_retries) {
			ESIF_TRACE_ERROR("Unable to do an IPC connect\n");
			break;
		}

		esif_ccb_sleep(1);
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}

// IPC Disconnect
void ipc_disconnect()
{
	ESIF_TRACE_ENTRY_INFO();

	if (g_ipc_handle != INVALID_HANDLE_VALUE) {
		esif_ipc_disconnect(g_ipc_handle);
		g_ipc_handle = INVALID_HANDLE_VALUE;
		ESIF_TRACE_DEBUG("IPF IPC Kernel Device Closed\n");
	}

	ESIF_TRACE_EXIT_INFO();
}

// Is IPC Connected?
int ipc_isconnected()
{
	return (g_ipc_handle != INVALID_HANDLE_VALUE);
}

/* Declared and Handled By UF Shell */
extern struct esif_uf_dm g_dm;

// IPC Execute
enum esif_rc ipc_execute(struct esif_ipc *ipc)
{
	enum esif_rc rc = ESIF_OK;
	struct timeval start  = {0};
	struct timeval finish = {0};
	struct timeval result;

	if (g_ipc_handle == INVALID_HANDLE_VALUE) {
		ESIF_TRACE_WARN("IPF LF is not available\n");
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

#endif
