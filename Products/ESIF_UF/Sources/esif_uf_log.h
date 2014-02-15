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

#ifndef ESIF_ATTR_OS_WINDOWS
# include <syslog.h>
#endif

#ifdef ESIF_ATTR_OS_WINDOWS
#include "win\messages.h"
void report_event_to_event_log(WORD eventCategory, WORD eventType, char *pFormat, ...);

#endif

// TODO: Move this to a common header
extern char *esif_str_replace(char *orig, char *rep, char *with);

/* Primitive Opcodes String */
static ESIF_INLINE esif_string esif_log_type_str(eLogType logType)
{
	#define CREATE_LOG_TYPE(lt) case lt: \
	str = (char *) #lt;break;
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

#endif	// _ESIF_UF_IFACE_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
