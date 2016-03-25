/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "esif_sdk_data.h"

#ifdef ESIF_ATTR_USER

#pragma pack(push, 1)

/* Policy Data */
typedef struct _t_EsifPolicyLogData_s {
	UInt32 policyIndex;
	char policyName[MAX_PATH];
	char policyFileName[MAX_PATH];
	esif_guid_t policyGuid;
} EsifPolicyLogData, *EsifPolicyLogDataPtr;

/* Active Control Data */
typedef struct _t_EsifActiveControlCapability {
	UInt32 controlId;
	UInt32 speed;
} EsifActiveControlCapability, *EsifActiveControlCapabilityPtr;

/* Config TDP Data */
typedef struct _t_EsifConfigTdpControl {
	UInt64 controlId;
	UInt64 tdpRatio;
	UInt64 tdpPower;
	UInt64 tdpFrequency;
} EsifConfigTdpControl, *EsifConfigTdpControlPtr;

/* Core Control Data */
typedef struct _t_EsifCoreControl {
	UInt32 activeLogicalProcessors;
	UInt32 minimumActiveCores;
	UInt32 maximumActiveCores;
} EsifCoreControl, *EsifCoreControlPtr;

/* Display Control Data */
typedef struct _t_EsifDisplayControl {
	UInt32 currentDPTFLimit;
	UInt32 lowerLimit;
	UInt32 upperLimit;
} EsifDisplayControl, *EsifDisplayControlPtr;

/* Domain Priority Data */
typedef struct _t_EsifDomainPriority {
	UInt32 priority;
} EsifDomainPriority, *EsifDomainPriorityPtr;

/* Performance Control Data */
typedef struct _t_EsifPerformanceControl {
	UInt32 pStateLimit;
	UInt32 lowerLimit;
	UInt32 upperLimit;
} EsifPerformanceControl, *EsifPerformanceControlPtr;

#define MAX_POWER_CONTROL_TYPE 4

typedef struct _t_EsifPowerData {	
	UInt32 powerType;
	UInt32 isEnabled;
	UInt32 powerLimit;
	UInt32 lowerLimit;
	UInt32 upperLimit;
	UInt32 stepsize;
	UInt32 minTimeWindow;
	UInt32 maxTimeWindow;
	UInt32 minDutyCycle;
	UInt32 maxDutyCycle;
} EsifPowerData;

/* Power Control Data */
typedef struct _t_EsifPowerControl {
	EsifPowerData powerDataSet[MAX_POWER_CONTROL_TYPE];
} EsifPowerControl, *EsifPowerControlPtr;

/* Power Status Data */
typedef struct _t_EsifPowerStatus {
	UInt32 power;
} EsifPowerStatus, *EsifPowerStatusPtr;

/* Temperature Status Data */
typedef struct _t_EsifTemperatureStatus {
	UInt32 temperature;
} EsifTemperatureStatus, *EsifTemperatureStatusPtr;

/* Utilization Status Data */
typedef struct _t_EsifUtilizationStatus {
	UInt32 utilization;
} EsifUtilizationStatus, *EsifUtilizationStatusPtr;

/* Pixel Clock Status Data */
typedef struct _t_EsifPixelClockStatus {
	//TBD
	UInt32 pixelClockStatus;
} EsifPixelClockStatus, *EsifPixelClockStatusPtr;

/* Pixel Clock Control Data */
typedef struct _t_EsifPixelClockControl {
	//TBD
	UInt32 pixelClockControl;
} EsifPixelClockControl, *EsifPixelClockControlPtr;

/* Platform Power Status Data */
typedef struct _t_EsifPlatformPowerStatus{
	//TBD
	UInt32 platformPowerStatus;
} EsifPlatformPowerStatus, *EsifPlatformPowerStatusPtr;

/* Temperature Control Data */
typedef struct _t_EsifTemperatureControl {
	UInt32 aux0;
	UInt32 aux1;
	UInt32 hysteresis;
} EsifTemperatureControl, *EsifTemperatureControlPtr;

/* Rfprofile Status Data */
typedef struct _t_EsifRfProfileStatus {
	UInt32 rfProfileFrequency;
} EsifRfProfileStatus, *EsifRfProfileStatusPtr;

/* Rfprofile Control Data */
typedef struct _t_EsifRfProfileControl {
	UInt32 rfProfileFrequency;
} EsifRfProfileControl, *EsifRfProfileControlPtr;

/* Network Control Data */
typedef struct _t_EsifNetworkControl {
	//TBD
	UInt32 networkControl;
} EsifNetworkControl, *EsifNetworkControlPtr;

/* XmitPower Control Data */
typedef struct _t_EsifXmitPowerControl {
	//TBD
	UInt32 xmitPowerControl;
} EsifXmitPowerControl, *EsifXmitPowerControlPtr;

/* Hdc Control Data */
typedef struct _t_EsifHdcControl {
	UInt32 hdcStatus;
	UInt32 hdcDutyCycle;
} EsifHdcControl, *EsifHdcControlPtr;

/* Psys Control Data */
typedef struct _t_EsifPSysControl {
	UInt32 powerLimitType;
	UInt32 powerLimit;
	UInt32 PowerLimitTimeWindow;
	UInt32 PowerLimitDutyCycle;
} EsifPSysControl, *EsifPSysControlPtr;

typedef union _t_EsifCapability {
	EsifActiveControlCapability activeControl;
	EsifConfigTdpControl configTdpControl;
	EsifCoreControl coreControl;
	EsifDisplayControl displayControl;
	EsifDomainPriority domainPriority;
	EsifPerformanceControl performanceControl;
	EsifPowerControl powerControl;
	EsifPowerStatus powerStatus;
	EsifTemperatureStatus temperatureStatus;
	EsifUtilizationStatus utilizationStatus;
	EsifPixelClockStatus pixelClockStatus;
	EsifPixelClockControl pixelClockControl;
	EsifPlatformPowerStatus platformPowerStatus;
	EsifTemperatureControl temperatureControl;
	EsifRfProfileStatus rfProfileStatus;
	EsifRfProfileControl rfProfileControl;
	EsifNetworkControl networkControl;
	EsifXmitPowerControl xmitPowerControl;
	EsifHdcControl HdcControl;
	EsifPSysControl psysControl;
} EsifCapability, *EsifCapabilityPtr;

/* Capability Data */
typedef struct _t_EsifCapabilityData {
	UInt32 type;
	UInt32 size;
	EsifCapability data;
} EsifCapabilityData, *EsifCapabilityDataPtr;
#pragma pack(pop)

#endif /* ESIF_ATTR_USER */
