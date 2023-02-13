/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#include "esif_sdk_data_misc.h"


#pragma pack(push, 1)

/* Policy Data */
typedef struct _t_EsifPolicyLogData_s {
	UInt32 policyIndex;
	UInt8 loaded;
	char policyName[MAX_PATH];
	char policyFileName[MAX_PATH];
	esif_guid_t policyGuid;
} EsifPolicyLogData, *EsifPolicyLogDataPtr;

/* Active Control Data */
typedef struct _t_EsifActiveControlCapability {
	UInt32 controlId;
	UInt32 speed;
	UInt32 lowerLimit;
	UInt32 upperLimit;
} EsifActiveControlCapability, *EsifActiveControlCapabilityPtr;

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

/* Energy Control Data */
typedef struct _t_EsifEnergyControl {
	UInt32 energyCounter;
	UInt32 instantaneousPower; 
} EsifEnergyControl, *EsifEnergyControlPtr;

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

typedef struct _t_EsifSocPowerFloorData {
	UInt32 socPowerFloorState;
	UInt32 isSupported;
} EsifSocPowerFloorData;

/* Power Control Data */
typedef struct _t_EsifPowerControl {
	EsifPowerData powerDataSet[MAX_POWER_CONTROL_TYPE];
	EsifSocPowerFloorData socPowerFloorData;
} EsifPowerControl, *EsifPowerControlPtr;

typedef struct _t_EsifPowerFilterData {
	UInt32 currentPowerSentToFilter;
	UInt32 powerCalculatedByFilter;
} EsifPowerFilterData;

/* Power Status Data */
typedef struct _t_EsifPowerStatus {
	EsifPowerFilterData powerFilterData;
	UInt32 currentPower;
} EsifPowerStatus, *EsifPowerStatusPtr;

/* Temperature Status Data */
typedef struct _t_EsifTemperatureStatus {
	UInt32 temperature;
} EsifTemperatureStatus, *EsifTemperatureStatusPtr;

/* Utilization Status Data */
typedef struct _t_EsifUtilizationStatus {
	UInt32 utilization;
} EsifUtilizationStatus, *EsifUtilizationStatusPtr;

/* Peak Power Control Data */
typedef struct _t_EsifPeakPowerControl {
	UInt32 acPeakPower;
	UInt32 dcPeakPower;
} EsifPeakPowerControl, *EsifPeakPowerControlPtr;

/* Processor Control Data */
typedef struct _t_EsifProcessorControlStatus {
	UInt32 tccOffset;
	UInt32 uvth;
} EsifProcessorControlStatus, *EsifProcessorControlStatusPtr;

/* Platform Power Status Data */
typedef struct _t_EsifPlatformPowerStatus {
	UInt32 platformRestOfPower;
	UInt32 adapterPowerRating;
	UInt32 platformPowerSource;
	UInt32 acNominalVoltage;
	UInt32 acOperationalCurrent;
	UInt32 ac1msOverload;
	UInt32 ac2msOverload;
	UInt32 ac10msOverload;
} EsifPlatformPowerStatus, *EsifPlatformPowerStatusPtr;

/* Battery Status Data */
typedef struct _t_EsifBatteryStatus {
	UInt32 maxBatteryPower;
	UInt32 steadyStateBatteryPower;
	UInt32 chargerType;
	UInt32 highFrequencyImpedance;
	UInt32 maxPeakCurrent;
	UInt32 noLoadVoltage;
	UInt32 batteryPercentage;
} EsifBatteryStatus, *EsifBatteryStatusPtr;

/* Temperature Control Data */
typedef struct _t_EsifTemperatureThresholdControl {
	UInt32 aux0;
	UInt32 aux1;
	UInt32 hysteresis;
} EsifTemperatureThresholdControl, *EsifTemperatureThresholdControlPtr;

#define MAX_FREQUENCY_CHANNEL_NUM 5

typedef struct _t_EsifFrequencyData {
	UInt32 centerFrequency;
	UInt32 leftFrequencySpread;
	UInt32 rightFrequencySpread;
} EsifFrequencyData;

/* Rfprofile Status Data */
typedef struct _t_EsifRfProfileStatus {
	EsifFrequencyData rfProfileFrequencyData[MAX_FREQUENCY_CHANNEL_NUM];
} EsifRfProfileStatus, *EsifRfProfileStatusPtr;

/* Rfprofile Control Data */
typedef struct _t_EsifRfProfileControl {
	UInt32 rfProfileMinFrequency;
	UInt32 rfProfileCenterFrequency;
	UInt32 rfProfileMaxFrequency;
	UInt32 rfProfileSSC;
} EsifRfProfileControl, *EsifRfProfileControlPtr;

/* Psys Control Data */
typedef struct _t_EsifPSysControlData {
	UInt32 powerLimitType;
	UInt32 powerLimit;
	UInt32 PowerLimitTimeWindow;
	UInt32 PowerLimitDutyCycle;
} EsifPSysControlData, *EsifPSysControlDataPtr;

#define MAX_PSYS_CONTROL_TYPE 3

/* Psys Control */
typedef struct _t_EsifPSysControl {
	EsifPSysControlData powerDataSet[MAX_PSYS_CONTROL_TYPE];
} EsifPSysControl, *EsifPSysControlPtr;

#define MAX_PORT_NUMBER 8
/* Platform Power Control Data */
typedef struct _t_EsifPlatformPowerControl {
	UInt32 portDataSet[MAX_PORT_NUMBER];
} EsifPlatformPowerControl, *EsifPlatformPowerControlPtr;

typedef struct _t_EsifManagerStatusData {
	UInt32 osPowerSource;
	UInt32 batteryPercent;
	UInt32 dockMode;
	UInt32 gameMode;
	UInt32 lidState;
	UInt32 powerSlider;
	UInt32 userInteraction;
	UInt32 userPresence;
	UInt32 screenState;
	UInt32 deviceOrientation;
	UInt32 inMotion;
	UInt32 systemCoolingMode;
	UInt32 platformType;
	UInt32 displayOrientation;
	esif_guid_t powerSchemePersonality;
	UInt32 mixedRealityMode;
	UInt32 platformUserPresence;
	UInt32 foregroundBackgroundRatio;
	UInt32 collaboration;
	EsifPpmParamValues ppmParamValues;
} EsifManagerStatusData, * EsifManagerStatusDataPtr;

/* Workload Classification Data */
typedef struct _t_EsifWorkloadClassification {
	UInt32 socWorkload;
} EsifWorkloadClassification, * EsifWorkloadClassificationPtr;

/* Dynamic EPP Data */
typedef struct _t_EsifDynamicEppControl {
	UInt32 eppHint;
	UInt32 eppValue;
} EsifDynamicEppControl, * EsifDynamicEppControlPtr;

typedef union _t_EsifCapability {
	EsifActiveControlCapability activeControl;
	EsifCoreControl coreControl;
	EsifDisplayControl displayControl;
	EsifDomainPriority domainPriority;
	EsifEnergyControl energyControl;
	EsifPerformanceControl performanceControl;
	EsifPowerControl powerControl;
	EsifPowerStatus powerStatus;
	EsifTemperatureStatus temperatureStatus;
	EsifUtilizationStatus utilizationStatus;
	EsifPeakPowerControl peakPowerControl;
	EsifProcessorControlStatus processorControlStatus;
	EsifPlatformPowerStatus platformPowerStatus;
	EsifTemperatureThresholdControl temperatureThresholdControl;
	EsifRfProfileStatus rfProfileStatus;
	EsifRfProfileControl rfProfileControl;
	EsifPSysControl psysControl;
	EsifPlatformPowerControl platformPowerControl;
	EsifBatteryStatus batteryStatus;
	EsifManagerStatusData managerStatus;
	EsifWorkloadClassification workloadClassification;
	EsifDynamicEppControl dynamicEppControl;
} EsifCapability, *EsifCapabilityPtr;

/* Capability Data */
typedef struct _t_EsifCapabilityData {
	UInt32 type;
	UInt32 size;
	EsifCapability data;
} EsifCapabilityData, *EsifCapabilityDataPtr;

#pragma pack(pop)

