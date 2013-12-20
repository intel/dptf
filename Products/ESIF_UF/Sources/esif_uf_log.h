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

#ifndef _ESIF_UF_DEBUG_
#define _ESIF_UF_DEBUG_

#include <stdio.h>
#include <stdarg.h>

#ifdef ESIF_ATTR_OS_WINDOWS
#include "win\messages.h"
void report_event_to_event_log (WORD eventCategory, WORD eventType, char *pFormat, ...);

#endif

// TODO: Move this to a common header
extern char*esif_str_replace (char *orig, char *rep, char *with);

/* Primitive Opcodes String */
static ESIF_INLINE esif_string esif_log_type_str (eLogType logType)
{
	#define CREATE_LOG_TYPE(lt) case lt: \
	str = (char*) #lt;break;
	esif_string str = (esif_string)ESIF_NOT_AVAILABLE;
	switch (logType) {
		CREATE_LOG_TYPE(eLogTypeFatal)
		CREATE_LOG_TYPE(eLogTypeError)
		CREATE_LOG_TYPE(eLogTypeWarning)
		CREATE_LOG_TYPE(eLogTypeInfo)
		CREATE_LOG_TYPE(eLogTypeDebug)
	}
	return str;
}


#include "esif_uf_version.h"
static void PostLog (
	const char *thePackage,
	const char *theModule,
	const eLogType theType,
	const char *theFunction,
	const int theLine,
	const char *theFormat,
	...
	)
{
	#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	int str_len    = 0;
	char *replaced = 0;

	va_list argList;

	/* Build Message */
	esif_ccb_sprintf(BUF_SIZE, buf, "EsifVer(%s) %s::%s TYPE: %s(%u) FUNC: %s LINE: %u MSG:\n",
					 ESIF_UF_VERSION,
					 thePackage,
					 theModule,
					 esif_log_type_str(theType),
					 theType,
					 theFunction,
					 theLine);

	str_len = (int)esif_ccb_strlen(buf, BUF_SIZE);

	/* VA Component */
	va_start(argList, theFormat);
	esif_ccb_vsprintf(BUF_SIZE - str_len, &buf[str_len], theFormat, argList);
	va_end(argList);

	printf("%s\n", buf);

	/* JDH  Send To OS Kludge For Alpha Abstract This */
#ifdef ESIF_ATTR_OS_WINDOWS
	// Escape any "%" since the event log function expects the 3rd parameter to be a format string
	if ((replaced = esif_str_replace(buf, "%", "%%")) != NULL) {
		esif_ccb_strcpy(buf, replaced, BUF_SIZE);
		esif_ccb_free(replaced);
	}

	switch (theType) {
	case eLogTypeDebug:
		report_event_to_event_log(CATEGORY_GENERAL, EVENTLOG_INFORMATION_TYPE, buf);
		break;

	case eLogTypeError:
		report_event_to_event_log(CATEGORY_GENERAL, EVENTLOG_ERROR_TYPE, buf);
		break;

	case eLogTypeFatal:
		report_event_to_event_log(CATEGORY_GENERAL, EVENTLOG_ERROR_TYPE, buf);
		break;

	case eLogTypeWarning:
		report_event_to_event_log(CATEGORY_GENERAL, EVENTLOG_WARNING_TYPE, buf);
		break;

	case eLogTypeInfo:
	default:
		report_event_to_event_log(CATEGORY_GENERAL, EVENTLOG_INFORMATION_TYPE, buf);
		break;
	}
#endif
}


#define POST_LOG_TYPE(type, format, ...)   PostLog(POST_LOG_PACKAGE, POST_LOG_MODULE, type, __FUNCTION__, __LINE__, format, __VA_ARGS__);
#define POST_LOG_FATAL(format, ...)   PostLog(POST_LOG_PACKAGE, POST_LOG_MODULE, eLogTypeFatal, __FUNCTION__, __LINE__, format, __VA_ARGS__);
#define POST_LOG_ERROR(format, ...)   PostLog(POST_LOG_PACKAGE, POST_LOG_MODULE, eLogTypeError, __FUNCTION__, __LINE__, format, __VA_ARGS__);
#define POST_LOG_WARNING(format, ...) PostLog(POST_LOG_PACKAGE, POST_LOG_MODULE, eLogTypeWarning, __FUNCTION__, __LINE__, format, __VA_ARGS__);
#define POST_LOG_INFO(format, ...)    PostLog(POST_LOG_PACKAGE, POST_LOG_MODULE, eLogTypeInfo, __FUNCTION__, __LINE__, format, __VA_ARGS__);
#define POST_LOG_DEBUG(format, ...)   PostLog(POST_LOG_PACKAGE, POST_LOG_MODULE, eLogTypeDebug, __FUNCTION__, __LINE__, format, __VA_ARGS__);

#endif	// _ESIF_UF_IFACE_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
