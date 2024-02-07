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

#pragma once

#include "Dptf.h"
#include "PolicyServices.h"
#include "PlatformConfigurationDataInterface.h"
#include "TableObjectType.h"

class PolicyServicesPlatformConfigurationData final : public PolicyServices, public PlatformConfigurationDataInterface
{
public:
	PolicyServicesPlatformConfigurationData(
		DptfManagerInterface* dptfManager,
		UIntN policyIndex);

	UInt32 readConfigurationUInt32(const std::string& key) override;
	UInt32 readConfigurationUInt32(const std::string& nameSpace, const std::string& key) override;
	void writeConfigurationUInt32(const std::string& key, UInt32 data) override;
	void writeConfigurationUInt32(const std::string& nameSpace, const std::string& key, UInt32 data) override;
	void writeConfigurationString(const std::string& nameSpace, const std::string& key, const std::string& data)
		override;
	void writeConfigurationString(const std::string& key, const std::string& data)
		override;
	std::string readConfigurationString(const std::string& nameSpace, const std::string& key) override;
	DptfBuffer readConfigurationBinary(const std::string& nameSpace, const std::string& key) override;
	void writeConfigurationBinary(
		void* bufferPtr,
		UInt32 bufferLength,
		UInt32 dataLength,
		const std::string& nameSpace,
		const std::string& key) override;
	void deleteConfigurationBinary(const std::string& nameSpace, const std::string& key) override;
	eEsifError sendCommand(UInt32 argc, const std::string& argv) override;

	TimeSpan getMinimumAllowableSamplePeriod() override;

	DptfBuffer getActiveRelationshipTable() override;
	void setActiveRelationshipTable(const DptfBuffer& data) override;
	DptfBuffer getThermalRelationshipTable() override;
	void setThermalRelationshipTable(const DptfBuffer& data) override;
	DptfBuffer getPassiveTable() override;
	void setPassiveTable(const DptfBuffer& data) override;
	DptfBuffer getAdaptivePerformanceConditionsTable(const std::string& uuid) override;
	DptfBuffer getAdaptivePerformanceActionsTable(const std::string& uuid) override;
	DptfBuffer getOemVariables() override;
	DptfBuffer getSwOemVariables() override;
	void setSwOemVariables(const DptfBuffer& data) override;
	UInt32 getProcessorConfigTdpControl(UIntN participantIndex, UIntN domainIndex) override;
	Power getProcessorConfigTdpLevel(UIntN participantIndex, UIntN domainIndex, UIntN configTdpControl) override;
	UInt32 getProcessorConfigTdpLock(UIntN participantIndex, UIntN domainIndex) override;
	Power getProcessorTdp(UIntN participantIndex, UIntN domainIndex) const override;
	Temperature getProcessorTjMax(UIntN participantIndex, UIntN domainIndex) override;
	DptfBuffer getPowerBossConditionsTable() override;
	DptfBuffer getPowerBossActionsTable() override;
	DptfBuffer getPowerBossMathTable() override;
	DptfBuffer getVoltageThresholdMathTable() override;
	DptfBuffer getEmergencyCallModeTable() override;
	DptfBuffer getPidAlgorithmTable() override;
	Bool getDisplayRequired() override;
	void setPidAlgorithmTable(const DptfBuffer& data) override;
	DptfBuffer getActiveControlPointRelationshipTable() override;
	TimeSpan getExpectedBatteryLife() override;
	UInt32 getAggressivenessLevel() override;
	DptfBuffer getDdrfTable() override;
	DptfBuffer getRfimTable() override;
	DptfBuffer getAggregateDisplayInformation() override;
	DptfBuffer getEnergyPerformanceOptimizerTable() override;
	void setEnergyPerformanceOptimizerTable(const DptfBuffer& data) override;
	DptfBuffer getTpgaTable() override;
	void setTpgaTable(const DptfBuffer& data) override;
	DptfBuffer getOpbtTable() override;
	void setOpbtTable(const DptfBuffer& data) override;

	void setActiveControlPointRelationshipTable(const DptfBuffer& data) override;
	DptfBuffer getPowerShareAlgorithmTable() override;
	void setPowerShareAlgorithmTable(const DptfBuffer& data) override;
	DptfBuffer getPowerShareAlgorithmTable2() override;
	void setPowerShareAlgorithmTable2(const DptfBuffer& data) override;
	DptfBuffer getIntelligentThermalManagementTable() override;
	void setIntelligentThermalManagementTable(const DptfBuffer& data) override;
	DptfBuffer getIntelligentThermalManagementTable3() override;
	void setIntelligentThermalManagementTable3(const DptfBuffer& data) override;
	void setPpmPackage(const DptfBuffer& data) override;
	void setPpmPackageForNonBalancedSchemePersonality(const DptfBuffer& data) override;
	DptfBuffer getPpmPackage(const DptfBuffer& data) override;
	void setPowerSchemeEpp(UInt32 value) override;
	void setActivePowerScheme() override;
	void setForegroundAppRatioPeriod(UInt32 value) override;
	void setProcessAffinityMask(const std::string& processName, UInt32 maskValue) override;
	void setApplicationCompatibility(const DptfBuffer& data) override;
	void deleteApplicationCompatibility() override;
	UInt32 getDynamicBoostState(UIntN participantIndex, UIntN domainIndex) override;
	void setDynamicBoostState(UIntN participantIndex, UIntN domainIndex, UInt32 value) override;
	UInt32 getTpgPowerStateWithoutCache(UIntN participantIndex, UIntN domainIndex) override;
	EnvironmentProfile getEnvironmentProfile() const override;
	void clearPpmPackageSettings(void) override;
	UInt32 getLogicalProcessorCount(UIntN participantIndex, UIntN domainIndex) override; 
	UInt32 getPhysicalCoreCount(UIntN participantIndex, UIntN domainIndex) override;
	DptfBuffer getSystemConfigurationFeatureTable() override;
	std::string getActiveTableName(TableObjectType::Type tableObjectType) override;

private:
	TimeSpan m_defaultSamplePeriod;
	static DptfBuffer createResetPrimitiveTupleBinary(esif_primitive_type primitive, UInt8 instance);
	static UInt16 createTupleDomain();

	void resetAllTables() const;
	void resetActiveRelationshipTable() const;
	void resetThermalRelationshipTable() const;
	void resetPassiveTable() const;
	void resetPidAlgorithmTable() const;
	void resetActiveControlPointRelationshipTable() const;
	void resetPowerShareAlgorithmTable() const;
	void resetPowerShareAlgorithmTable2() const;
	void resetIntelligentThermalManagementTable() const;
	void resetIntelligentThermalManagementTable3() const;
	void resetEnergyPerformanceOptimizerTable() const;
	void resetThirdPartyGraphicsTable() const;
	void resetThirdPartyGraphicsTable2() const;
};
