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

/*
 * TODO: The following must be declared prior to SDK include files or defined
 * in the project pre-processor macros.  The appropriate OS-type should be
 * selected.
 */
// #define ESIF_ATTR_USER
// #define ESIF_ATTR_OS_WINDOWS
// #define ESIF_ATTR_OS_LINUX

#include "upe.h"
#include "esif_sdk_iface_upe.h"
#include "esif_sdk_action_type.h"
#include "esif_ccb_string.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_time.h"
#include "esif_ccb_string.h"
#include "esif_ccb_file.h"

#ifdef ESIF_ATTR_OS_WINDOWS
/*
 * TODO:  The banned.h file is not included as part of the SDK sample code.
 * If desired, include the file in Windows builds.  The file is available from
 * Microsoft at https://www.microsoft.com/en-us/download/details.aspx?id=24817
 * The Windows banned-API check header must be included after all other headers,
 * or issues can be identified against Windows SDK/DDK included headers which
 * we have no control over. The header is not provided with the SDK code,
 */
#define _SDL_BANNED_RECOMMENDED
/* #include "banned.h" */
#endif

extern "C" {

void UpeTraceMessage(
	eLogType level,
	const char *func,
	const char *file,
	int line,
	const char *msg,
	...)
{
	EsifData esifDataMsg = {ESIF_DATA_STRING};
	char *buffer = NULL;
	char const *fmtDetail = "%s:[%s@%s#%d]<%llu ms>: ";
	int numChars = 0;
	int offset = 0;
	const char *sep = NULL;
	size_t  msglen = 0;
	va_list args;
	esif_ccb_time_t msec = 0;

	if (g_esifLogFuncPtr == NULL) {
		goto exit;
	}

	esif_ccb_system_time(&msec);

	level = esif_ccb_min(level, eLogTypeDebug);

	if ((sep = strrchr(file, *ESIF_PATH_SEP)) != NULL) {
		file = sep + 1;
	}

	va_start(args, msg);
	msglen = esif_ccb_vscprintf(msg, args) +
		esif_ccb_strlen(ACTION_UPE_NAME, MAX_PATH) +
		esif_ccb_strlen(func, MAX_PATH) +
		esif_ccb_strlen(file, MAX_PATH) +
		esif_ccb_strlen(fmtDetail, MAX_PATH) +
		20; /* Include room for integer expansion in fmtDetail */
	va_end(args);

	buffer = (char *)esif_ccb_malloc(msglen);
	if (NULL == buffer) {
		goto exit;
	}

	/* Print the detail information first; and then append the actual formatted message */
	numChars =  esif_ccb_sprintf(msglen, buffer, fmtDetail, ACTION_UPE_NAME, func, file, line, msec);

	offset = numChars;
	va_start(args, msg);
	numChars += esif_ccb_vsprintf(msglen - offset, buffer + offset, msg, args);
	va_end(args);
	/* Append a carriage return for Windows */
	if (numChars && (buffer[numChars -1] != '\n')) {
		esif_ccb_strcat(buffer, "\n", msglen);
	}

	esifDataMsg.buf_ptr = buffer;
	esifDataMsg.buf_len = (UInt32)msglen;
	esifDataMsg.data_len = (UInt32)msglen;
	(*g_esifLogFuncPtr)(&esifDataMsg, level);
exit:
	esif_ccb_free(buffer);
	return;
}

} // extern "C"


