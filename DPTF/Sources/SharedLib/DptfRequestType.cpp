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

#include "DptfRequestType.h"
#include "DptfExceptions.h"

std::string DptfRequestType::ToString(DptfRequestType::Enum requestType)
{
	switch (requestType)
	{
	case ClearPolicyRequestsForAllControls:
		return "ClearPolicyRequestsForAllControls";
	case ClearCachedData:
		return "ClearCachedData";
	case ActiveControlGetStaticCaps:
		return "ActiveControlGetStaticCaps";
	case ActiveControlGetDynamicCaps:
		return "ActiveControlGetDynamicCaps";
	case ActiveControlGetStatus:
		return "ActiveControlGetStatus";
	case ActiveControlGetControlSet:
		return "ActiveControlGetControlSet";
	case ActiveControlGetFanOperatingMode:
		return "ActiveControlGetFanOperatingMode";
	case ActiveControlSetFanSpeed:
		return "ActiveControlSetFanSpeed";
	case ActiveControlSetDynamicCaps:
		return "ActiveControlSetDynamicCaps";
	case ActiveControlSetFanCapsLock:
		return "ActiveControlSetFanCapsLock";
	case ActiveControlSetFanDirection:
		return "ActiveControlSetFanDirection";
	case ActiveControlSetFanOperatingMode:
		return "ActiveControlSetFanOperatingMode";
	case BatteryStatusGetMaxBatteryPower:
		return "BatteryStatusGetMaxBatteryPower";
	case BatteryStatusGetBatteryStatus:
		return "BatteryStatusGetBatteryStatus";
	case BatteryStatusGetBatteryInformation:
		return "BatteryStatusGetBatteryInformation";
	case BatteryStatusGetChargerType:
		return "BatteryStatusGetChargerType";
	case BatteryStatusGetBatterySteadyState:
		return "BatteryStatusGetBatterySteadyState";
	case BatteryStatusGetBatteryHighFrequencyImpedance:
		return "BatteryStatusGetBatteryHighFrequencyImpedance";
	case BatteryStatusGetBatteryNoLoadVoltage:
		return "BatteryStatusGetBatteryNoLoadVoltage";
	case BatteryStatusGetBatteryMaxPeakCurrent:
		return "BatteryStatusGetBatteryMaxPeakCurrent";
	case BatteryStatusGetBatteryPercentage:
		return "BatteryStatusGetBatteryPercentage";
	case BatteryStatusSetBatteryPercentage:
		return "BatteryStatusSetBatteryPercentage";
	case PlatformNotificationSetOsc:
		return "PlatformNotificationSetOsc";
	case PlatformNotificationSetApplicationAliveResponse:
		return "PlatformNotificationSetApplicationAliveResponse";
	case PlatformNotificationSetPolicySystemMode:
		return "PlatformNotificationSetPolicySystemMode";
	case PlatformNotificationAppBroadcastSend:
		return "PlatformNotificationAppBroadcastSend";
	case PlatformNotificationFanOperatingModeChanged:
		return "PlatformNotificationFanOperatingModeChanged";
	case SocWorkloadClassificationGetSocWorkload:
		return "SocWorkloadClassificationGetSocWorkload";
	case ProcessorControlSetUnderVoltageThreshold:
		return "ProcessorControlSetUnderVoltageThreshold";
	case ProcessorControlSetTccOffsetTemperature:
		return "ProcessorControlSetTccOffsetTemperature";
	case ProcessorControlSetPerfPreferenceMax:
		return "ProcessorControlSetPerfPreferenceMax";
	case ProcessorControlSetPerfPreferenceMin:
		return "ProcessorControlSetPerfPreferenceMin";
	case ProcessorControlGetMaxTccOffsetTemperature:
		return "ProcessorControlGetMaxTccOffsetTemperature";
	case ProcessorControlGetMinTccOffsetTemperature:
		return "ProcessorControlGetMinTccOffsetTemperature";
	case ProcessorControlGetPcieThrottleRequestState:
		return "ProcessorControlGetPcieThrottleRequestState";
	case TemperatureControlGetTemperatureStatus:
		return "TemperatureControlGetTemperatureStatus";
	case TemperatureControlGetTemperatureThresholds:
		return "TemperatureControlGetTemperatureThresholds";
	case TemperatureControlSetTemperatureThresholds:
		return "TemperatureControlSetTemperatureThresholds";
	case TemperatureControlGetPowerShareTemperatureThreshold:
		return "TemperatureControlGetPowerShareTemperatureThreshold";
	case TemperatureControlGetCalibrationTable:
		return "TemperatureControlGetCalibrationTable";
	case TemperatureControlGetPollingTable:
		return "TemperatureControlGetPollingTable";
	case TemperatureControlIsVirtualTemperatureControl:
		return "TemperatureControlIsVirtualTemperatureControl";
	case TemperatureControlSetVirtualTemperature:
		return "TemperatureControlSetVirtualTemperature";
	case DynamicEppGetEppSensitivityHint:
		return "DynamicEppGetEppSensitivityHint";
	case DynamicEppGetDynamicEppSupport:
		return "DynamicEppGetDynamicEppSupport";
	case DynamicEppSetDynamicEppSupport:
		return "DynamicEppSetDynamicEppSupport";
	case SocWorkloadClassificationGetExtendedWorkloadPrediction:
		return "SocWorkloadClassificationGetExtendedWorkloadPrediction";
	case DataGetConfigurationFileContent:
		return "DataGetConfigurationFileContent";
	case PublishEvent:
		return "PublishEvent";
	case CreateParticipant:
		return "CreateParticipant";
	case DeleteParticipant:
		return "DeleteParticipant";
	default:
		throw dptf_exception("Invalid request type given to ToString");
	}
}
