/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "esif_ccb.h"
#include "esif_sdk.h"
#include "esif_uf_shell.h"
#include "esif_uf_loggingmgr.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#include "powrprof.h"

#define WIN10_LIMITS_MANAGEMENT_DLL               "api-ms-win-power-limitsmanagement-l1-1-0.dll"
#define THERMALAPI_NO_OF_ARGUMENTS                2

#define POWR_PROF_DLL                             "powrprof.dll"
#define POWER_REPORT_THERMAL_EVENT_STR            "PowerReportThermalEvent"
#define POWER_REPORT_LIMITS_EVENT_STR             "PowerReportLimitsEvent"

extern Bool g_thermalApiPolicyStarted;
extern Bool g_thermalApiMonitorStarted;
extern Bool g_thermalApiMitigationStarted;
//
// _THERMAL_EVENT and PowerReportThermalEvent have been defined in WinBlue/Win8.1 WDK
//
#ifndef THERMAL_EVENT_VERSION

#define THERMAL_EVENT_VERSION 1

typedef struct _THERMAL_EVENT {
	ULONG   Version;
	ULONG   Size;
	ULONG   Type;
	ULONG   Temperature;
	ULONG   TripPointTemperature;
	LPWSTR  Initiator;
} THERMAL_EVENT, *PTHERMAL_EVENT;

#endif

//
// _ENVIRONMENTAL_MANAGEMENT_EVENT and PowerReportLimitsEvent have been defined in Win10 WDK
//
#ifndef ENVIRONMENTAL_MANAGEMENT_EVENT_VERSION

typedef struct _ENVIRONMENTAL_MANAGEMENT_EVENT {
	ULONG Version;
	ULONG Size;
	ULONG EventType;
	ULONG MonitorType;
	ULONG LowerThreshold;
	ULONG UpperThreshold;
	ULONG Value;
	LPWSTR Initiator;
} ENVIRONMENTAL_MANAGEMENT_EVENT, *PENVIRONMENTAL_MANAGEMENT_EVENT;

#define ENVIRONMENTAL_MANAGEMENT_EVENT_VERSION 2
#define ENVIORNMENTAL_VALUE_UNSPECIFIED        0

#define ENVIRONMENTAL_EVENT_SHUTDOWN           0
#define ENVIRONMENTAL_EVENT_HIBERNATE          1
#define ENVIRONMENTAL_EVENT_UNSPECIFIED        0xffffffff

#endif

typedef DWORD(WINAPI * PFNPOWERREPORTLIMITSEVENT)(
	PENVIRONMENTAL_MANAGEMENT_EVENT Event);

typedef DWORD(WINAPI * PFNPOWERREPORTTHERMALEVENT)(
	PTHERMAL_EVENT Event);

#define THERMAL_EVENT_SHUTDOWN 0
#define THERMAL_EVENT_HIBERNATE 1
#define THERMAL_EVENT_UNSPECIFIED 0xffffffff
//
//	Externs
//
extern esif_lib_t g_thermalApiLib;
extern atomic_t g_thermalApiLibRefCount;					/* Reference count */

#ifdef __cplusplus
extern "C" {
#endif

char *EsifShellCmdThermalApi(EsifShellCmdPtr shell);
eEsifError EsifThermalApi_Init(void);
void EsifThermalApi_Exit(void);

//Utility Functions
eEsifError ThermalApi_LoadLibrary(void);
eEsifError ThermalApi_UnloadLibrary(void);

void ThermalApi_ParticipantCreate(EsifUpPtr upPtr);
void ThermalApi_ParticipantDestroy(UInt8 participantId);

void ThermalApi_ReportThermalEvent(
	UInt32 EventFlag,
	UInt32 temperature,
	UInt32 tripPointTemperature
	);

#ifdef __cplusplus
}
#endif

#define esif_ccb_report_thermal_event(EventFlag, temperature, tripPointTemperature) \
	ThermalApi_ReportThermalEvent(EventFlag, temperature, tripPointTemperature) 
#define EsifThermalApi_ParticipantCreate(upPtr) \
	ThermalApi_ParticipantCreate(upPtr)

#else  //NOT ESIF_ATTR_OS_WINDOWS

static ESIF_INLINE char *EsifShellCmdThermalApi(EsifShellCmdPtr shell)
{
	char *output = shell->outbuf;
	esif_ccb_sprintf(OUT_BUF_LEN, output, "%s\n", esif_rc_str(ESIF_E_NOT_IMPLEMENTED));
	return output;
}

// Implementation is in esif_uf_thermalapi_os_lin.c
static ESIF_INLINE void EsifThermalApi_ParticipantCreate(EsifUpPtr upPtr)
{
	UNREFERENCED_PARAMETER(upPtr);
}

static ESIF_INLINE void esif_ccb_report_thermal_event(
	UInt32 EventFlag,
	UInt32 temperature,
	UInt32 tripPointTemperature)
{
	UNREFERENCED_PARAMETER(EventFlag);
	UNREFERENCED_PARAMETER(temperature);
	UNREFERENCED_PARAMETER(tripPointTemperature);
}

#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
