/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "esif_sdk_data.h"
#include "esif_sdk_iface_esif.h"

#define APP_INTERFACE_VERSION_1 1 /* Initial Version */
#define APP_INTERFACE_VERSION_2 2 /* New prototype for fAppCommandFuncPtr */
/*
* Uses new discovery function and exchange protocol and is not compatible with
* prior versions. ESIF responsible for allocation participant handles and
* removed interface header
*/
#define APP_INTERFACE_VERSION_3 3


#define APP_INTERFACE_VERSION_4 4 /* Handles moved to 64-bit vs. system-dependent VOID* */
#define APP_INTERFACE_VERSION   APP_INTERFACE_VERSION_4
#define APP_PARTICIPANT_VERSION 1
#define APP_DOMAIN_VERSION 1

/*
    INTERFACE Structures
    These structures are only used by the INTERFACE ESIF maintains a much
    more complex state for a participant and domain than it shares.
 */

#pragma pack(push, 1)

/* Application METADATA
 *
 * fPathHome may contain multiple paths separated by "|" in the following supported formats:
 *   Path1        = Single Path
 *   Path1|Path2  = Multiple Paths
 *   Path1|#Path2 = Multiple Paths [# = Dont use full path when calling fopen, dlopen, etc]
 */
typedef struct _t_AppData {
	/* Version 1 */
	EsifData   fPathHome;	/* Application Home Path(s) [see implementation for details] */
	eLogType   fLogLevel;	/* Current Logging/Trace Level */
} AppData, *AppDataPtr, **AppDataPtrLocation;

/* Participant METADATA */
typedef struct _t_AppParticipantData {
	/* Common */
	UInt8		fVersion;	/* ESIF Participant version */
	UInt8		fReserved[3];	/* Pad / Align */
	EsifData	fDriverType;	/* Guid is obtained from Driver */
	EsifData	fDeviceType;	/* Guid is obtained from DSP */
	EsifData	fName;		/* Name May Come From Driver/DSP */
	EsifData	fDesc;		/* Descriptoin From Driver/DSP */
	EsifData	fDriverName;	/* Driver Name */
	EsifData	fDeviceName;	/* Device Name */
	EsifData	fDevicePath;	/* Device Path */
	UInt8		fDomainCount;	/* Domain Count */
	UInt8		fReserved2[3];	/* Pad/Align */
	eParticipantBus fBusEnumerator;	/* Enumeration Type pci, acpi, platform, conjure etc.*/

	/* ACPI */
	EsifData    fAcpiDevice;	/* ACPI Device */
	EsifData    fAcpiScope;		/* ACPI Scope/Object ID \_SB_.IETM */
	EsifData    fAcpiUID;		/* ACPI Unique ID */
	eDomainType fAcpiType;		/* ACPI Domain/Participant Type e.g. THermalSensor, Power, Etc. */

	/* PCI */
	UInt16  fPciVendor;		/* Vendor */
	UInt16  fPciDevice;		/* Device */
	UInt8   fPciBus;		/* Bus */
	UInt8   fPciBusDevice;		/* Bus Device */
	UInt8   fPciFunction;		/* Function */
	UInt8   fPciRevision;		/* Revision */
	UInt8   fPciClass;		/* Class */
	UInt8   fPciSubClass;		/* Sub Class */
	UInt8   fPciProgIf;		/* Programming Interface */
	UInt8   fReserved3[1];		/* Pad/Align */
} AppParticipantData, *AppParticipantDataPtr, **AppParticipantDataPtrLocation;

/* Domain METADATA */
typedef struct _t_AppDomainData {
	UInt8      fVersion;		/* ESIF Domain version */
	UInt8      fReserved[3];	/* Pad/Align */
	EsifData    fName;		/* Name From DSP */
	EsifData    fDescription;	/* Description From DSP */
	EsifData    fGuid;		/* GUID From DSP */
	eDomainType fType;		/* Domain Type From DSP */
	EsifFlags   fCapability;	/* Domain Capability From DSP */
	UInt8 fCapabilityBytes[32];	/* Domain Capability Bytes */
} AppDomainData, *AppDomainDataPtr, **AppDomainDataPtrLocation;

#pragma pack(pop)

/*
    INTERFACE Enumerations
    These Enumerations are only used by the INTERFACE.
 */

/* Applicaiton State */
typedef enum _t_AppState {
	eAppStateDisabled = 0,	/* Disable Application */
	eAppStateEnabled		/* Enable Application */
} eAppState;

/*
    ESIF_APP_INTERFACE Callback Functions
    These functions are only used by the INTERFACE as method to loosely
    couple ESIF with its hosted application.  There are used for ESIF to
    request information and services from the applicaion.  There is a
    corresponding set of interfaces defined below for allow the application
    to provide key services that ESIF provides as well.
 */

/*

    These maybe called to gather Applicaion information before the Allocate
    Handle and subsequent create functions are called.
 */

/*
** ============================================================================
** APPLICATION Level CallBacks
** ============================================================================
*/

typedef eEsifError (ESIF_CALLCONV *AppGetDescriptionFunction) (EsifDataPtr appDescription);
typedef eEsifError (ESIF_CALLCONV *AppGetNameFunction)        (EsifDataPtr appName);
typedef eEsifError (ESIF_CALLCONV *AppGetVersionFunction)     (EsifDataPtr appVersion);

typedef eEsifError (ESIF_CALLCONV *AppCreateFunction)(
	AppInterfaceSetPtr ifaceSetPtr,			/* The App MUST fill in all pointers */
	const esif_handle_t esifHandle,			/* ESIF will provide App MUST save for use with callbacks */
	esif_handle_t *appHandlePtr,			/* Returns allocated handle for application */
	const AppDataPtr appData,				/* Application Metadata */
	const eAppState initialAppState	/* Initial state of application */
	);

/*
    DEPENDENCY
    The remaining functions are dependent on application create.
 */

/* Destroy */
typedef eEsifError (ESIF_CALLCONV *AppDestroyFunction)(const esif_handle_t appHandle);

/* Suspend */
typedef eEsifError (ESIF_CALLCONV *AppSuspendFunction)(const esif_handle_t appHandle);

/* Resume */
typedef eEsifError (ESIF_CALLCONV *AppResumeFunction)(const esif_handle_t appHandle);

/* Banner */
typedef eEsifError (ESIF_CALLCONV *AppGetIntroFunction)(
	const esif_handle_t appHandle,	/* Allocated handle for application */
	EsifDataPtr appIntro	/* Applicaiton banner / greeting / app information to display for CLI */
	);

/* CLI Command */
typedef eEsifError(ESIF_CALLCONV *AppCommandFunction)(
	const esif_handle_t appHandle,	  /* Allocated handle for application */
	const UInt32 argc,        /* command arguments count (1 or more) */
	const EsifDataArray argv,   /* array of command arguments must be ESIF_DATA_STRING today */
	EsifDataPtr response      /* response must be ESIF_DATA_STRING today */
	);

/* Get Status */
typedef enum _t_eAppStatusCommand {
	eAppStatusCommandGetXSLT = 0,		/* Get the XSLT information */
	eAppStatusCommandGetGroups,			/* Get the group id/name pairs */
	eAppStatusCommandGetModulesInGroup,	/* Get the module id/name pairs for a given group id */
	eAppStatusCommandGetModuleData,		/* Get the data for a given group id and module id */
} eAppStatusCommand;

typedef eEsifError (ESIF_CALLCONV *AppGetStatusFunction)(
	const esif_handle_t appHandle,		/* Allocated handle for application */
	const eAppStatusCommand command,/* Command */
	const UInt32 appStatusIn,	/* Command Data (High Word Group, Low Word Module) */
	EsifDataPtr appStatusOut	/* Status output string if XML please use ESIF_DATA_XML */
	);

/*
** ============================================================================
** PARTICIPANT Level CallBacks
** ============================================================================
*/

typedef enum _t_eParticipantState {
	eParticipantStateDisabled = 0,	/* Participant Disabled */
	eParticipantStateEnabled		/* Participant Enabled */
} eParticipantState;

typedef eEsifError (ESIF_CALLCONV *AppParticipantCreateFunction)(
	const esif_handle_t appHandle,	/* Allocated handle for application */
	const esif_handle_t participantHandle,		/* A particpant handle */
	const AppParticipantDataPtr participantData,			/* Participant Metadata */
	const eParticipantState participantInitialState	/* Participant inital State */
	);

/* Destroy */
typedef eEsifError (ESIF_CALLCONV *AppParticipantDestroyFunction)(
	const esif_handle_t appHandle,	/* Allocated handle for application */
	const esif_handle_t participantHandle /* A participant handle */
	);

/*
** ============================================================================
** DOMAIN Level CallBacks
** ============================================================================
*/

typedef enum _t_eDomainState {
	eDomainStateDisabled = 0,	/* Domain Disabld */
	eDomainStateEnabled			/* Domain Enabled */
} eDomainState;

typedef eEsifError (ESIF_CALLCONV *AppDomainCreateFunction)(
	const esif_handle_t appHandle,/* Allocated handle for application */
	const esif_handle_t participantHandle,	/* Allocated participant handle by ESIF */
	const esif_handle_t domainHandle,		/* Allocated domain handle by ESIF */
	const AppDomainDataPtr domainDataPtr,	/* Domain Metadata and configuration details */
	const eDomainState domainInitialState	/* Domain initial state */
	);

/* Destroy */
typedef eEsifError (ESIF_CALLCONV *AppDomainDestroyFunction)(
	const esif_handle_t appHandle,			/* Allocated handle for application */
	const esif_handle_t participantHandle,	/* An allocated handle for participant */
	const esif_handle_t domainHandle		/* An allocated handle for domain */
	);

/*
** ============================================================================
** EVENT CallBacks
** ============================================================================
*/

typedef eEsifError (ESIF_CALLCONV *AppEventFunction)(
	const esif_handle_t appHandle,			/* Allocated handle for application */
	const esif_handle_t participantHandle,	/* Optional Participant ESIF_HANDLE_PRIMARY_PARTICIPANT indicates App Level Event */
	const esif_handle_t domainHandle,	/* Optional Domain ESIF_INVALID_HANDLE indicates non-domain Level Event */
	const EsifDataPtr eventData,	/* Data included with the event if any MAY Be NULL */
	const EsifDataPtr eventGuid	/* Event GUID */
	);

/*
    ESIF Application Interface ESIF --> APPLICATION
    Forward declared and typedef in esif_uf_iface.h
 */

#pragma pack(push, 1)

struct _t_AppInterface {
	/* Application */
	AppCreateFunction      fAppCreateFuncPtr;
	AppDestroyFunction     fAppDestroyFuncPtr;
	AppSuspendFunction     fAppSuspendFuncPtr; //optional
	AppResumeFunction      fAppResumeFuncPtr;  //optional
	AppGetVersionFunction  fAppGetVersionFuncPtr;  //optional
	AppCommandFunction     fAppCommandFuncPtr; /* Version 2: New Prototype*/ //optional
	AppGetIntroFunction   fAppGetIntroFuncPtr;  //optional
	AppGetDescriptionFunction fAppGetDescriptionFuncPtr; //optional
	AppGetNameFunction     fAppGetNameFuncPtr;
	AppGetStatusFunction   fAppGetStatusFuncPtr; //remove - go through command

	/* Participant */
	AppParticipantCreateFunction    fParticipantCreateFuncPtr;
	AppParticipantDestroyFunction   fParticipantDestroyFuncPtr;

	/* Domain */
	AppDomainCreateFunction    fDomainCreateFuncPtr;
	AppDomainDestroyFunction   fDomainDestroyFuncPtr;

	/* Event */
	AppEventFunction  fAppEventFuncPtr;  //optional
};


/*
* During interface exchange, ESIF will first fill out the header for the
* application.  The application must verify compatibilty and size constraints
* before popultating its interface items.  If not complatible, the application
* shall return an appropriate error code and not fill in its interface items.
*/
struct _t_AppInterfaceSet {
	EsifIfaceHdr hdr;
	AppInterface appIface;
	EsifInterface esifIface;
};

#pragma pack(pop)

/* C++ */

#ifdef __cplusplus
extern "C" {
#endif

/*
    Entry function this symbol must be available when the app.so/dll modules is
    loaded.  For Unix/Linux nothing special is required for widnows you must
    export this symbol see the REFERENCE IMPLEMENATION for the optional methods
    for doing this.  Note this is the only function that is linked to. So when
    you declare all of your other functions make the static and well access them
    via Function Pointers.  This will remove the linker cruft trom the static
    funcions.  And provide a simple private type of encapsulation for the module.
 */
ESIF_EXPORT eEsifError GetApplicationInterfaceV2(AppInterfaceSetPtr theIface);

#ifdef __cplusplus
}
#endif

#define GET_APPLICATION_INTERFACE_FUNCTION "GetApplicationInterfaceV2"

#endif /* USER */
