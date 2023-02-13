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
#include "esif_ccb.h"
#include "esif_sdk.h"
#include "esif_uf_shell.h"
#include "esif_uf_loggingmgr.h"


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
	UInt32 tripPointTemperature,
	EsifString participantName
	)
{
	UNREFERENCED_PARAMETER(EventFlag);
	UNREFERENCED_PARAMETER(temperature);
	UNREFERENCED_PARAMETER(tripPointTemperature);
	UNREFERENCED_PARAMETER(participantName);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
