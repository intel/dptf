/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "Dptf.h"
#include "SensorOrientation.h"
#include "OnOffToggle.h"
#include "SensorSpatialOrientation.h"

class PlatformConfigurationDataInterface
{
public:
	virtual ~PlatformConfigurationDataInterface(){};

	virtual UInt32 readConfigurationUInt32(const std::string& key) = 0;
	virtual void writeConfigurationUInt32(const std::string& key, UInt32 data) = 0;
	virtual std::string readConfigurationString(const std::string& key) = 0;
	virtual DptfBuffer readConfigurationBinary(const std::string& key) = 0;

	virtual TimeSpan getMinimumAllowableSamplePeriod(void) = 0;

	// FIXME:  ESIF Primitives
	virtual DptfBuffer getActiveRelationshipTable(void) = 0;
	virtual void setActiveRelationshipTable(DptfBuffer data) = 0;
	virtual DptfBuffer getThermalRelationshipTable(void) = 0;
	virtual void setThermalRelationshipTable(DptfBuffer data) = 0;
	virtual DptfBuffer getPassiveTable(void) = 0;
	virtual DptfBuffer getAdaptivePerformanceConditionsTable(void) = 0;
	virtual DptfBuffer getAdaptivePerformanceParticipantConditionTable(void) = 0;
	virtual void setPassiveTable(DptfBuffer data) = 0;
	virtual DptfBuffer getAdaptivePerformanceActionsTable(void) = 0;
	virtual DptfBuffer getAdaptiveUserPresenceTable(void) = 0;
	virtual void setAdaptiveUserPresenceTable(DptfBuffer data) = 0;
	virtual DptfBuffer getOemVariables(void) = 0;
	virtual UInt64 getHwpfState(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt32 getSocWorkload(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt32 getSupportEppHint(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual DptfBuffer getPowerBossConditionsTable(void) = 0;
	virtual DptfBuffer getPowerBossActionsTable(void) = 0;
	virtual DptfBuffer getPowerBossMathTable(void) = 0;
	virtual DptfBuffer getVoltageThresholdMathTable(void) = 0;
	virtual DptfBuffer getEmergencyCallModeTable(void) = 0;
	virtual DptfBuffer getPidAlgorithmTable(void) = 0;
	virtual UInt32 getLastHidInputTime(void) = 0;
	virtual UInt32 getIsExternalMonitorConnected(void) = 0;
	virtual Bool getDisplayRequired(void) = 0;
	virtual Bool getIsCVFSensor(void) = 0;
	virtual void setPidAlgorithmTable(DptfBuffer data) = 0;
	virtual DptfBuffer getActiveControlPointRelationshipTable(void) = 0;
	virtual void setActiveControlPointRelationshipTable(DptfBuffer data) = 0;
	virtual DptfBuffer getPowerShareAlgorithmTable(void) = 0;
	virtual void setPowerShareAlgorithmTable(DptfBuffer data) = 0;
	virtual DptfBuffer getPowerShareAlgorithmTable2(void) = 0;
	virtual void setPowerShareAlgorithmTable2(DptfBuffer data) = 0;
	virtual void setScreenAutolock(UInt32 value) = 0;
	virtual void setWorkstationLock(UInt32 value) = 0;
	virtual void setWakeOnApproach(UInt32 value) = 0;
	virtual void setScreenState(UInt32 value) = 0;
	virtual void setWakeOnApproachDppeSetting(UInt32 value) = 0;
	virtual void setWakeOnApproachExternalMonitorDppeSetting(UInt32 value) = 0;
	virtual void setWakeOnApproachLowBatteryDppeSetting(UInt32 value) = 0;
	virtual void setWakeOnApproachBatteryRemainingPercentageDppeSetting(UInt32 value) = 0;
	virtual void setWalkAwayLockDppeSetting(UInt32 value) = 0;
	virtual void setWalkAwayLockExternalMonitorDppeSetting(UInt32 value) = 0;
	virtual void setWalkAwayLockPreDimWaitTimeDppeSetting(UInt32 value) = 0;
	virtual void setUserPresentWaitTimeoutDppeSetting(UInt32 value) = 0;
	virtual void setDimIntervalDppeSetting(UInt32 value) = 0;
	virtual void setDimScreenDppeSetting(UInt32 value) = 0;
	virtual void setHonorPowerRequestsForDisplayDppeSetting(UInt32 value) = 0;
	virtual void setHonorUserInCallDppeSetting(UInt32 value) = 0;
	virtual void setWalkAwayLockScreenLockWaitTimeDppeSetting(UInt32 value) = 0;
	virtual void setDisplayOffAfterLockDppeSetting(UInt32 value) = 0;
	virtual void setNoLockOnPresenceDppeSetting(UInt32 value) = 0;
	virtual void setNoLockOnPresenceExternalMonitorDppeSetting(UInt32 value) = 0;
	virtual void setNoLockOnPresenceBatteryDppeSetting(UInt32 value) = 0;
	virtual void setNoLockOnPresenceBatteryRemainingPercentageDppeSetting(UInt32 value) = 0;
	virtual void setNoLockOnPresenceResetWaitTimeDppeSetting(UInt32 value) = 0;
	//virtual void setFailSafeTimeoutDppeSetting(UInt32 value) = 0;
	virtual void setAdaptiveDimmingDppeSetting(UInt32 value) = 0;
	virtual void setAdaptiveDimmingExternalMonitorDppeSetting(UInt32 value) = 0;
	virtual void setAdaptiveDimmingPresentationModeDppeSetting(UInt32 value) = 0;
	virtual void setAdaptiveDimmingPreDimWaitTimeDppeSetting(UInt32 value) = 0;
	virtual void setMispredictionFaceDetectionDppeSetting(UInt32 value) = 0;
	virtual void setMispredictionTimeWindowDppeSetting(UInt32 value) = 0;
	virtual void setMisprediction1DimWaitTimeDppeSetting(UInt32 value) = 0;
	virtual void setMisprediction2DimWaitTimeDppeSetting(UInt32 value) = 0;
	virtual void setMisprediction3DimWaitTimeDppeSetting(UInt32 value) = 0;
	virtual void setMisprediction4DimWaitTimeDppeSetting(UInt32 value) = 0;
	virtual void setFailsafeTimeoutDppeSetting(UInt32 value) = 0;
	virtual void setWakeOnApproachEventDppeSetting(UInt32 value) = 0;
	virtual void setWalkAwayLockEventDppeSetting(UInt32 value) = 0;
	virtual void setExternalMonitorConnectedEventDppeSetting(UInt32 value) = 0;
	virtual void setUserNotPresentDimTarget(UInt32 value) = 0;
	virtual void setUserDisengagedDimmingInterval(UInt32 value) = 0;
	virtual void setUserDisengagedDimTarget(UInt32 value) = 0;
	virtual void setUserDisengagedDimWaitTime(UInt32 value) = 0;
};
