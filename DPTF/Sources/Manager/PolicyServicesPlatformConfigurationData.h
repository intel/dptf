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
#include "PolicyServices.h"
#include "PlatformConfigurationDataInterface.h"

class PolicyServicesPlatformConfigurationData final : public PolicyServices, public PlatformConfigurationDataInterface
{
public:
	PolicyServicesPlatformConfigurationData(DptfManagerInterface* dptfManager, UIntN policyIndex);

	virtual UInt32 readConfigurationUInt32(const std::string& key) override final;
	virtual void writeConfigurationUInt32(const std::string& key, UInt32 data) override final;
	virtual std::string readConfigurationString(const std::string& key) override final;
	virtual DptfBuffer readConfigurationBinary(const std::string& key) override final;

	virtual TimeSpan getMinimumAllowableSamplePeriod(void) override final;

	virtual DptfBuffer getActiveRelationshipTable(void) override final;
	virtual void setActiveRelationshipTable(DptfBuffer data) override final;
	virtual DptfBuffer getThermalRelationshipTable(void) override final;
	virtual void setThermalRelationshipTable(DptfBuffer data) override final;
	virtual DptfBuffer getPassiveTable(void) override final;
	virtual void setPassiveTable(DptfBuffer data) override final;
	virtual DptfBuffer getAdaptiveUserPresenceTable(void) override final;
	virtual void setAdaptiveUserPresenceTable(DptfBuffer data) override final;
	virtual DptfBuffer getAdaptivePerformanceConditionsTable(void) override final;
	virtual DptfBuffer getAdaptivePerformanceParticipantConditionTable(void) override final;
	virtual DptfBuffer getAdaptivePerformanceActionsTable(void) override final;
	virtual DptfBuffer getOemVariables(void) override final;
	virtual UInt64 getHwpfState(UIntN participantIndex, UIntN domainIndex) override final;
	virtual UInt32 getSocWorkload(UIntN participantIndex, UIntN domainIndex) override final;
	virtual UInt32 getSupportEppHint(UIntN participantIndex, UIntN domainIndex) override final;
	virtual DptfBuffer getPowerBossConditionsTable(void) override final;
	virtual DptfBuffer getPowerBossActionsTable(void) override final;
	virtual DptfBuffer getPowerBossMathTable(void) override final;
	virtual DptfBuffer getVoltageThresholdMathTable(void) override final;
	virtual DptfBuffer getEmergencyCallModeTable(void) override final;
	virtual DptfBuffer getPidAlgorithmTable(void) override final;
	virtual UInt32 getLastHidInputTime(void) override final;
	virtual UInt32 getIsExternalMonitorConnected(void) override final;
	virtual Bool getDisplayRequired(void) override final;
	virtual Bool getIsCVFSensor(void) override final;
	virtual void setPidAlgorithmTable(DptfBuffer data) override final;
	virtual DptfBuffer getActiveControlPointRelationshipTable(void) override final;
	virtual void setActiveControlPointRelationshipTable(DptfBuffer data) override final;
	virtual DptfBuffer getPowerShareAlgorithmTable() override final;
	virtual void setPowerShareAlgorithmTable(DptfBuffer data) override final;
	virtual DptfBuffer getPowerShareAlgorithmTable2() override final;
	virtual void setPowerShareAlgorithmTable2(DptfBuffer data) override final;
	virtual void setScreenAutolock(UInt32 value) override final;
	virtual void setWorkstationLock(UInt32 value) override final;
	virtual void setWakeOnApproach(UInt32 value) override final;
	virtual void setScreenState(UInt32 value) override final;
	virtual void setWakeOnApproachDppeSetting(UInt32 value) override final;
	virtual void setWakeOnApproachExternalMonitorDppeSetting(UInt32 value) override final;
	virtual void setWakeOnApproachLowBatteryDppeSetting(UInt32 value) override final;
	virtual void setWakeOnApproachBatteryRemainingPercentageDppeSetting(UInt32 value) override final;
	virtual void setWalkAwayLockDppeSetting(UInt32 value) override final;
	virtual void setWalkAwayLockExternalMonitorDppeSetting(UInt32 value) override final;
	virtual void setWalkAwayLockPreDimWaitTimeDppeSetting(UInt32 value) override final;
	virtual void setUserPresentWaitTimeoutDppeSetting(UInt32 value) override final;
	virtual void setDimIntervalDppeSetting(UInt32 value) override final;
	virtual void setDimScreenDppeSetting(UInt32 value) override final;
	virtual void setHonorPowerRequestsForDisplayDppeSetting(UInt32 value) override final;
	virtual void setHonorUserInCallDppeSetting(UInt32 value) override final;
	virtual void setWalkAwayLockScreenLockWaitTimeDppeSetting(UInt32 value) override final;
	virtual void setDisplayOffAfterLockDppeSetting(UInt32 value) override final;
	virtual void setNoLockOnPresenceDppeSetting(UInt32 value) override final;
	virtual void setNoLockOnPresenceExternalMonitorDppeSetting(UInt32 value) override final;
	virtual void setNoLockOnPresenceBatteryDppeSetting(UInt32 value) override final;
	virtual void setNoLockOnPresenceBatteryRemainingPercentageDppeSetting(UInt32 value) override final;
	virtual void setNoLockOnPresenceResetWaitTimeDppeSetting(UInt32 value) override final;
	//virtual void setFailSafeTimeoutDppeSetting(UInt32 value) override final;
	virtual void setAdaptiveDimmingDppeSetting(UInt32 value) override final;
	virtual void setAdaptiveDimmingExternalMonitorDppeSetting(UInt32 value) override final;
	virtual void setAdaptiveDimmingPresentationModeDppeSetting(UInt32 value) override final;
	virtual void setAdaptiveDimmingPreDimWaitTimeDppeSetting(UInt32 value) override final;
	virtual void setMispredictionFaceDetectionDppeSetting(UInt32 value) override final;
	virtual void setMispredictionTimeWindowDppeSetting(UInt32 value) override final;
	virtual void setMisprediction1DimWaitTimeDppeSetting(UInt32 value) override final;
	virtual void setMisprediction2DimWaitTimeDppeSetting(UInt32 value) override final;
	virtual void setMisprediction3DimWaitTimeDppeSetting(UInt32 value) override final;
	virtual void setMisprediction4DimWaitTimeDppeSetting(UInt32 value) override final;
	virtual void setFailsafeTimeoutDppeSetting(UInt32 value) override final;
	virtual void setWakeOnApproachEventDppeSetting(UInt32 value) override final;
	virtual void setWalkAwayLockEventDppeSetting(UInt32 value) override final;
	virtual void setExternalMonitorConnectedEventDppeSetting(UInt32 value) override final;
	virtual void setUserNotPresentDimTarget(UInt32 value) override final;
	virtual void setUserDisengagedDimmingInterval(UInt32 value) override final;
	virtual void setUserDisengagedDimTarget(UInt32 value) override final;
	virtual void setUserDisengagedDimWaitTime(UInt32 value) override final;

private:
	TimeSpan m_defaultSamplePeriod;
	DptfBuffer createResetPrimitiveTupleBinary(esif_primitive_type primitive, UInt8 instance) const;
	UInt16 createTupleDomain() const;

	void resetAllTables(void);
	void resetActiveRelationshipTable(void);
	void resetThermalRelationshipTable(void);
	void resetPassiveTable(void);
	void resetPidAlgorithmTable(void);
	void resetActiveControlPointRelationshipTable(void);
	void resetPowerShareAlgorithmTable(void);
	void resetPowerShareAlgorithmTable2(void);
};
