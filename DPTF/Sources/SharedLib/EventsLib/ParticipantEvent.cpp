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

#include "ParticipantEvent.h"
#include "Dptf.h"

//
// FIXME: the code in this file may be called enough that we should change it to an index based lookup.  Need
// to run a profiler and see.
//

#define CASE(eventType)                                                                                                \
	case eventType:                                                                                                    \
		return FrameworkEvent::eventType;

namespace ParticipantEvent
{
	FrameworkEvent::Type ToFrameworkEvent(ParticipantEvent::Type participantEventType)
	{
		switch (participantEventType)
		{
			CASE(DptfConnectedStandbyEntry)
			CASE(DptfConnectedStandbyExit)
			CASE(DptfSuspend)
			CASE(DptfResume)
			CASE(ParticipantSpecificInfoChanged)
			CASE(DomainCoreControlCapabilityChanged)
			CASE(DomainDisplayControlCapabilityChanged)
			CASE(DomainDisplayStatusChanged)
			CASE(DomainPerformanceControlCapabilityChanged)
			CASE(DomainPerformanceControlsChanged)
			CASE(DomainPowerControlCapabilityChanged)
			CASE(DomainPriorityChanged)
			CASE(DomainRadioConnectionStatusChanged)
			CASE(DomainRfProfileChanged)
			CASE(DomainTemperatureThresholdCrossed)
			CASE(DomainVirtualSensorCalibrationTableChanged)
			CASE(DomainVirtualSensorPollingTableChanged)
			CASE(DomainVirtualSensorRecalcChanged)
			CASE(DomainBatteryStatusChanged)
			CASE(DomainBatteryInformationChanged)
			CASE(DomainBatteryHighFrequencyImpedanceChanged)
			CASE(DomainBatteryNoLoadVoltageChanged)
			CASE(DomainMaxBatteryPeakCurrentChanged)
			CASE(DomainPlatformPowerSourceChanged)
			CASE(DomainAdapterPowerRatingChanged)
			CASE(DomainChargerTypeChanged)
			CASE(DomainPlatformRestOfPowerChanged)
			CASE(DomainMaxBatteryPowerChanged)
			CASE(DomainPlatformBatterySteadyStateChanged)
			CASE(DomainACNominalVoltageChanged)
			CASE(DomainACOperationalCurrentChanged)
			CASE(DomainAC1msPercentageOverloadChanged)
			CASE(DomainAC2msPercentageOverloadChanged)
			CASE(DomainAC10msPercentageOverloadChanged)
			CASE(DomainEnergyThresholdCrossed)
			CASE(DomainFanCapabilityChanged)
			CASE(DomainSocWorkloadClassificationChanged)
			CASE(DomainEppSensitivityHintChanged)
			CASE(DomainExtendedWorkloadPredictionChanged)
			CASE(DomainFanOperatingModeChanged)
			CASE(DomainSocPowerFloorChanged)
			CASE(DomainHardwareSocWorkloadHintChanged)
			CASE(DomainPcieThrottleRequested)
			CASE(DptfParticipantActivityLoggingEnabled)
			CASE(DptfParticipantActivityLoggingDisabled)
			CASE(DptfParticipantControlAction)
			CASE(DptfAppBroadcastPrivileged)
			CASE(DptfExtendedWorkloadPredictionEventRegistrationChanged)
		default:
			throw dptf_exception("ParticipantEvent::Type is invalid.");
		}
	}
}
