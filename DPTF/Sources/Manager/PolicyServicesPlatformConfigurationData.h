/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
	virtual UInt32 readConfigurationUInt32(const std::string& nameSpace, const std::string& key) override final;
	virtual void writeConfigurationUInt32(const std::string& key, UInt32 data) override final;
	virtual std::string readConfigurationString(const std::string& nameSpace, const std::string& key) override final;
	virtual DptfBuffer readConfigurationBinary(const std::string& nameSpace, const std::string& key) override final;
	virtual void writeConfigurationBinary(
		void* bufferPtr,
		UInt32 bufferLength,
		UInt32 dataLength,
		const std::string& nameSpace,
		const std::string& key) override final;
	virtual void deleteConfigurationBinary(const std::string& nameSpace, const std::string& elementPath) override final;
	virtual eEsifError sendCommand(UInt32 argc, const std::string& argv) override final;

	virtual TimeSpan getMinimumAllowableSamplePeriod(void) override final;

	virtual DptfBuffer getActiveRelationshipTable(void) override final;
	virtual void setActiveRelationshipTable(DptfBuffer data) override final;
	virtual DptfBuffer getThermalRelationshipTable(void) override final;
	virtual void setThermalRelationshipTable(DptfBuffer data) override final;
	virtual DptfBuffer getPassiveTable(void) override final;
	virtual void setPassiveTable(DptfBuffer data) override final;
	virtual DptfBuffer getAdaptivePerformanceConditionsTable(std::string uuid) override final;
	virtual DptfBuffer getAdaptivePerformanceActionsTable(std::string uuid) override final;
	virtual DptfBuffer getOemVariables(void) override final;
	virtual DptfBuffer getSwOemVariables(void) override final;
	virtual void setSwOemVariables(const DptfBuffer& swOemVariablesData) override final;
	virtual UInt64 getHwpfState(UIntN participantIndex, UIntN domainIndex) override final;
	virtual UInt32 getProcessorConfigTdpControl(UIntN participantIndex, UIntN domainIndex) override final;
	virtual Power getProcessorConfigTdpLevel(UIntN participantIndex, UIntN domainIndex, UIntN configTdpControl)
		override final;
	virtual UInt32 getProcessorConfigTdpLock(UIntN participantIndex, UIntN domainIndex) override final;
	virtual Power getProcessorTdp(UIntN participantIndex, UIntN domainIndex) override final;
	virtual Temperature getProcessorTjMax(UIntN participantIndex, UIntN domainIndex) override final;
	virtual DptfBuffer getPowerBossConditionsTable(void) override final;
	virtual DptfBuffer getPowerBossActionsTable(void) override final;
	virtual DptfBuffer getPowerBossMathTable(void) override final;
	virtual DptfBuffer getVoltageThresholdMathTable(void) override final;
	virtual DptfBuffer getEmergencyCallModeTable(void) override final;
	virtual DptfBuffer getPidAlgorithmTable(void) override final;
	virtual Bool getDisplayRequired(void) override final;
	virtual void setPidAlgorithmTable(DptfBuffer data) override final;
	virtual DptfBuffer getActiveControlPointRelationshipTable(void) override final;
	virtual TimeSpan getExpectedBatteryLife(void) override final;
	virtual UInt32 getAggressivenessLevel(void) override final;
	virtual DptfBuffer getDdrfTable(void) override final;
	virtual DptfBuffer getAggregateDisplayInformation(void) override final;
	virtual DptfBuffer getEnergyPerformanceOptimizerTable(void) override final;
	virtual void setEnergyPerformanceOptimizerTable(DptfBuffer data) override final;
	virtual DptfBuffer getTpgaTable(void) override final;
	virtual void setTpgaTable(DptfBuffer data) override final;

	virtual void setActiveControlPointRelationshipTable(DptfBuffer data) override final;
	virtual DptfBuffer getPowerShareAlgorithmTable() override final;
	virtual void setPowerShareAlgorithmTable(DptfBuffer data) override final;
	virtual DptfBuffer getPowerShareAlgorithmTable2() override final;
	virtual void setPowerShareAlgorithmTable2(DptfBuffer data) override final;
	virtual DptfBuffer getIntelligentThermalManagementTable() override final;
	virtual void setIntelligentThermalManagementTable(DptfBuffer data) override final;
	virtual void setPpmPackage(DptfBuffer package) override final;
	virtual DptfBuffer getPpmPackage(DptfBuffer requestpackage) override final;
	virtual void setPowerSchemeEpp(UInt32 value) override final;
	virtual void setActivePowerScheme() override final;
	virtual void setForegroundAppRatioPeriod(UInt32 value) override final;
	virtual UInt32 getDynamicBoostState(UIntN participantIndex, UIntN domainIndex) override final;
	virtual void setDynamicBoostState(UIntN participantIndex, UIntN domainIndex, UInt32 value) override final;
	virtual UInt32 getTpgPowerStateWithoutCache(UIntN participantIndex, UIntN domainIndex) override final;
	virtual UInt64 getPlatformCpuId() override final;

	virtual void clearPpmPackageSettings(void) override final;

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
	void resetIntelligentThermalManagementTable(void);
	void resetEnergyPerformanceOptimizerTable(void);
	void resetThirdPartyGraphicsTable(void);
};
