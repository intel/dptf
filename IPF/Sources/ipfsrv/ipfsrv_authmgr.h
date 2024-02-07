/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "esif_sdk_data.h"
#include "esif_sdk_iface.h"
#include "esif_sdk_dcfg.h"

// Static Handles for Authentication Roles. These must be sequential (gaps allowed) and (handle & 0xffff) < IPFAUTH_MAX_ROLES
#define IPFAUTH_ROLE_NONE			((esif_handle_t)0xA0000)	// No Access
#define IPFAUTH_ROLE_ROOT			((esif_handle_t)0xA0001)	// All Access
#define IPFAUTH_ROLE_PUBLIC			((esif_handle_t)0xA0002)	// Public (Unauthenticated) Access (Read-Only + Limited Primitives & Events)
#define IPFAUTH_ROLE_ELEVATED		((esif_handle_t)0xA0003)	// Elevated (Authenticated) Access (Read/Write + Limited Primitives & Events)
#define IPFAUTH_ROLE_DPTF			((esif_handle_t)0xA0004)	// Dynamic Tuning Technology Manager (DPTF) Role
#define IPFAUTH_ROLE_DPTFUI			((esif_handle_t)0xA0005)	// Dynamic Tuning Technology UI (DPTF UI) Role
#define IPFAUTH_ROLE_IPFUI			((esif_handle_t)0xA0006)	// Innovation Platform Framework UI Role
#define IPFAUTH_ROLE_ICST			((esif_handle_t)0xA0007)	// Intel Context Sensing Technology Role
#define IPFAUTH_ROLE_ICSTSVC		((esif_handle_t)0xA0008)	// Intel Context Sensing Technology Service Role (LocalSystem)
#define IPFAUTH_ROLE_DPTFTCS		((esif_handle_t)0xA0009)	// Dynamic Tuning Technology Telemetry Collection Service (DPTFTCS) Role
#define IPFAUTH_ROLE_XTU			((esif_handle_t)0xA000A)	// Intel Extreme Tuning Utility(XTU) Role

#define IPFAUTH_MAX_ROLES			11							// Max number of IPFAUTH_ROLEs defined above

// Pseudo-Name to identify ACLs that Nobody can access
#define IPFAUTH_ACL_NOBODY			"---------:nobody:nobody"

/*
** Authentication Manager Functions (Access Control)
*/
typedef struct AccessDef_s {
	const char		*serverAddr;	// Server Address URL
	esif_handle_t	authHandle;		// Authorization Role Handle
	const char		*accessControl;	// OS-Specific Access Control List
	const char		*appKey;		// Optional OS-specific App Key used to check if a known IPF App is installed
	DCfgOptions		dcfgOpt;		// Optional DCFG bitmask for this server address
} AccessDef;

esif_error_t AccessMgr_Init(void);
void AccessMgr_Exit(void);
const AccessDef *AccessMgr_GetAccessDefs(void);

/*
** Authorization Manager Functions (Role Based Permissions)
*/
esif_error_t AuthMgr_Init(void);
void AuthMgr_Exit(void);
esif_string AuthMgr_GetAuthNameByEsifHandle(esif_handle_t esifHandle);
esif_string AuthMgr_GetAuthNameByAuthHandle(esif_handle_t authHandle);

/*
** Authorization Manager Esif Interface Functions
** Call Directly into these functions for Client RPC Requests
** Authorized calls will pass through to ESIF, Unauthorized calls will return ESIF_E_SESSION_PERMISSION_DENIED
*/
esif_error_t ESIF_CALLCONV AuthMgr_EsifGetConfig(
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	EsifDataPtr elementValue);

esif_error_t ESIF_CALLCONV AuthMgr_EsifSetConfig(
	const esif_handle_t esifHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	EsifDataPtr elementValue,
	const EsifFlags elementFlags);

esif_error_t ESIF_CALLCONV AuthMgr_EsifPrimitive(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr request,
	EsifDataPtr response,
	const ePrimitiveType primitive,
	const UInt8 instance);

esif_error_t ESIF_CALLCONV AuthMgr_EsifWriteLog(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr message,
	const eLogType logType);

esif_error_t ESIF_CALLCONV AuthMgr_EsifEventRegister(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuid);

esif_error_t ESIF_CALLCONV AuthMgr_EsifEventUnregister(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuid);

esif_error_t ESIF_CALLCONV AuthMgr_EsifSendEvent(
	const esif_handle_t esifHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventData,
	const EsifDataPtr eventGuid);

esif_error_t ESIF_CALLCONV AuthMgr_EsifSendCommand(
	const esif_handle_t esifHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response);
