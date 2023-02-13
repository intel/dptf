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
#pragma once
#include "esif_sdk.h"
#include "esif_sdk_iface.h"
#include "esif_sdk_iface_upe.h"
#include "esif_sdk_action_type.h"

#define UPE_ALWAYSFALSE (0)

/* TODO: Update with the action type supported by the UPE  */
#define ACTION_UPE_ACTION_TYPE ESIF_ACTION_JAVA

/* TODO: Update with the version of the action (informational only) */
#define ACTION_UPE_VERSION 1

/* TODO: Update with the filename of the action without any extension */
#define ACTION_UPE_NAME "upe_java"

/* TODO: Update with a description of the action (informational only - Up to ESIF_DESC_LEN - 1 chars) */
#define ACTION_UPE_DESC "Java Actions Through Android Binder"

#if defined(ESIF_ATTR_DEBUG)
#define ACTION_UPE_TRACE_LEVEL_DEFAULT eLogTypeDebug
#else
#define ACTION_UPE_TRACE_LEVEL_DEFAULT eLogTypeError
#endif

#define UPE_TRACE_IF_ACTIVE(level, fmt, ...) \
	do { \
		if (g_upeTraceLevel >= (level)) { \
			UpeTraceMessage(level, ESIF_FUNC, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
		} \
	} while ESIF_CONSTEXPR(UPE_ALWAYSFALSE)

#define UPE_TRACE_ERROR(fmt, ...) UPE_TRACE_IF_ACTIVE(eLogTypeError, "Error: " fmt, ##__VA_ARGS__)
#define UPE_TRACE_WARN(fmt, ...) UPE_TRACE_IF_ACTIVE(eLogTypeWarning, "Warning: " fmt, ##__VA_ARGS__)
#define UPE_TRACE_INFO(fmt, ...) UPE_TRACE_IF_ACTIVE(eLogTypeInfo, "Info: " fmt, ##__VA_ARGS__)
#define UPE_TRACE_DEBUG(fmt, ...) UPE_TRACE_IF_ACTIVE(eLogTypeDebug, "Debug: " fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

eEsifError ActionSendEvent(
	const esif_handle_t participantId,
	enum esif_event_type eventType,
	const EsifDataPtr dataPtr
	);

eEsifError ActionExecutePrimitive(
	const esif_handle_t participantId,
	const UInt16 primitiveId,
	const UInt8 instance,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

extern eLogType g_upeTraceLevel;
extern ActWriteLogFunction g_esifLogFuncPtr;

void UpeTraceMessage(
	eLogType level,
	const char *func,
	const char *file,
	int line,
	const char *fmt,
	...);

#ifdef __cplusplus
}
#endif

