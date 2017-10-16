/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#pragma once

#ifdef ESIF_ATTR_USER

#include "esif_sdk_iface.h"

#define ESIF_INTERFACE_VERSION_1 1 /* Initial Release */
#define ESIF_INTERFACE_VERSION_2 2 /* Added fSendEventFuncPtr */
#define ESIF_INTERFACE_VERSION_3 3 /* Added fSendCommandFuncPtr*/
#define ESIF_INTERFACE_VERSION ESIF_INTERFACE_VERSION_3

/*
 * INTERFACE Flags
 * These flags will be used by the ESIF Configuration Management Data Base CMDB
 */
#define ESIF_SERVICE_CONFIG_PERSIST     0x00000001	/* Persist Data To Disk */
#define ESIF_SERVICE_CONFIG_SCRAMBLE    0x00000002	/* Scramble Data For Storage */
#define ESIF_SERVICE_CONFIG_READONLY    0x00000004	/* Data is Read-Only */
#define ESIF_SERVICE_CONFIG_NOCACHE     0x00000008	/* Data is not cached in memory (except key) */
#define ESIF_SERVICE_CONFIG_FILELINK    0x00000010	/* Data is a link to an external file */
#define ESIF_SERVICE_CONFIG_DELETE      0x00010000	/* Delete from memory and disk */
#define ESIF_SERVICE_CONFIG_COMPRESSED  0x40000000	/* Payload is Compressed */
#define ESIF_SERVICE_CONFIG_STATIC      0x80000000	/* Statically Linked Repository */

/*
 * EsifServiceInterface Callback Functions
 * These functions are only used by the INTERFACE as method to loosely
 * couple ESIF with its hosted application.  There are used for the application
 * to request information and services from the ESIF.
 */

/* Get Config */
typedef eEsifError(ESIF_CALLCONV *AppGetConfigFunction)(
	const void *esifHandle,		/* ESIF provided constext handle */
	const void *appHandle,		/* handle allocated by ESIF hosted application */
	const EsifDataPtr nameSpace,	/* Name space to use e.g. ESIF, DPTF, ACT, Etc. */
	const EsifDataPtr elementPath,	/* Element path e.g. /a/b/c must be unique within a name space */
	EsifDataPtr elementValue	/* Any valid esif_data_type maybe retrieved including ESIF_DATA_AUTO */
);

/* Set Config */
typedef eEsifError(ESIF_CALLCONV *AppSetConfigFunction)(
	const void *esifHandle,		/* ESIF provided context handle */
	const void *appHandle,		/* handled allocated by ESIF hosted application */
	const EsifDataPtr nameSpace,	/* Name space to use e.g. ESIF, DPTF, ACT, Etc. */
	const EsifDataPtr elementPath,	/* Element path e.g. /a/b/c must be unique within a name space */
	const EsifDataPtr elementValue,	/* Any valid esif_data_type maybe set accept ESIF_DATA_AUTO */
	const EsifFlags elementFlags	/* Flags for handling data including persist, scramble, etc. */
);

/*
 * Primitive Execution note that you may execute against any participant or the IETM object if the optional
 * participant handle is ESIF_NO_HANDLE So for example if you wanted to retrieve the ART you would mark
 * the participant as ESIF_E_NO_HANDLE.  If you wanted to retrieve GET_TEMPERATURE you would have to provide
 * a valid particpant handle and domain handle to that ESIF knows "which" temperature to retrieve.
 */
typedef eEsifError(ESIF_CALLCONV *AppPrimitiveFunction)(
	const void *esifHandle,		/* ESIF provided context handle */
	const void *appHandle,		/* handled allocated by ESIF hosted application */
	const void *participantHandle,	/* Optional participant handle ESIF_NO_HANDLE */
	const void *domainHandle,	/* Optional required if particpant handle is provided */
	const EsifDataPtr request,	/* Request data for SET_* based primitives */
	EsifDataPtr response,		/* Response data for GET_* based primitives */
	ePrimitiveType primitive,	/* Primitive ID e.g. GET_TEMPERATURE */
	const UInt8 instance		/* Primitive instance may be 255 or ESIF_NO_INSTANCE */
);

/* Write Log */
typedef eEsifError(ESIF_CALLCONV *AppWriteLogFunction)(
	const void *esifHandle,		/* ESIF provided context handle */
	const void *appHandle,		/* handle allocated by ESIF hosted applicaiton */
	const void *participantHandle,	/* optional may be ESIF_NO_HANDLE used to augment log detail*/
	const void *domainHandle,	/* optional may be ESIF_NO_HANDLE used to augment log detail*/
	const EsifDataPtr message,	/* Message For Log */
	const eLogType logType		/* Log Type e.g. crticial, debug, info,e tc */
);

/* Event Register */
typedef eEsifError(ESIF_CALLCONV *AppEventRegisterFunction)(
	const void *esifHandle,		/* ESIF provided context handle */
	const void *appHandle,		/* handle allocated by ESIF hosted application */
	const void *participantHandle,	/* optional may be ESIF_NO_HANDLE indicates app event regisration */
	const void *domainHandle,	/* optional may be ESIF_NO_HANDLE indicates participant event registration */
	const EsifDataPtr eventGuid	/* Event GUID to be registered */
);

/* Event Unregister */
typedef eEsifError(ESIF_CALLCONV *AppEventUnregisterFunction)(
	const void *esifHandle,		/* ESIF provided context handle */
	const void *appHandle,		/* handle allocated by ESIF hosted application */
	const void *participantHandle,	/* optional may be ESIF_NO_HANDLE indicates app event registration */
	const void *domainHandle,	/* optional may be ESIF_NO_HANDLE indicates participant event registraion */
	const EsifDataPtr eventGuid	/* Event GUID to be unregistered */
);

/* Version 2*/
/* Send Event  Application -> ESIF */
typedef eEsifError(ESIF_CALLCONV *AppSendEventFunction)(
	const void *esifHandle,      /* ESIF provided context handle */
	const void *appHandle,		/* Allocated handle for application */
	const void *participantHandle,	/* Optional Participant ESIF_NO_HANDLE indicates App Level Event */
	const void *domainHandle,	/* Optional Domain ESIF_NO_HANDLE indicates non-domain Level Event */
	const EsifDataPtr eventData,	/* Data included with the event if any MAY Be NULL */
	const EsifDataPtr eventGuid	/* Event GUID */
	);

/* CLI Command   Application -> ESIF */
typedef eEsifError(ESIF_CALLCONV *AppSendCommandFunction)(
	const void *esifHandle,   /* ESIF provided context handle */
	const void *appHandle,	  /* Allocated handle for application */
	const UInt32 argc,        /* command arguments count (1 or more) */
	const EsifDataPtr  argv,        /* array of command arguments must be ESIF_DATA_STRING today */
	EsifDataPtr response      /* response must be ESIF_DATA_STRING today */
	);
/*
 * ESIF Service Interface ESIF <-- APPLICATION
 * Forward declared and typedef in esif_uf_iface.h
 */
#pragma pack(push, 1)

struct _t_EsifInterface {
	/* Interface */
	eIfaceType  fIfaceType;
	UInt16      fIfaceVersion;
	UInt16      fIfaceSize;

	/* Version 1 */
	/* Configuration Management */
	AppGetConfigFunction  fGetConfigFuncPtr;
	AppSetConfigFunction  fSetConfigFuncPtr;

	/* Execute */
	AppPrimitiveFunction  fPrimitiveFuncPtr;
	AppWriteLogFunction   fWriteLogFuncPtr;

	/* Event Registration */
	AppEventRegisterFunction    fRegisterEventFuncPtr;
	AppEventUnregisterFunction  fUnregisterEventFuncPtr;

	/* Version 2 */
	AppSendEventFunction fSendEventFuncPtr;

	/* Version 3 */
	AppSendCommandFunction fSendCommandFuncPtr;
};

#pragma pack(pop)

#endif /* USER */