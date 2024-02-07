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
#include "EnvironmentProfile.h"
#include "TableObjectType.h"

class PlatformConfigurationDataInterface
{
public:
	virtual ~PlatformConfigurationDataInterface() = default;

	virtual UInt32 readConfigurationUInt32(const std::string& key) = 0;
	virtual UInt32 readConfigurationUInt32(const std::string& nameSpace, const std::string& key) = 0;
	virtual void writeConfigurationUInt32(const std::string& key, UInt32 data) = 0;
	virtual void writeConfigurationUInt32(const std::string& nameSpace, const std::string& key, UInt32 data) = 0;
	virtual void writeConfigurationString(
		const std::string& nameSpace,
		const std::string& key,
		const std::string& data) = 0;
	virtual void writeConfigurationString(
		const std::string& key,
		const std::string& data) = 0;
	virtual std::string readConfigurationString(const std::string& nameSpace, const std::string& key) = 0;
	virtual DptfBuffer readConfigurationBinary(const std::string& nameSpace, const std::string& key) = 0;
	virtual void writeConfigurationBinary(
		void* bufferPtr,
		UInt32 bufferLength,
		UInt32 dataLength,
		const std::string&,
		const std::string&) = 0;
	virtual void deleteConfigurationBinary(const std::string& nameSpace, const std::string& elementPath) = 0;
	virtual eEsifError sendCommand(UInt32 argc, const std::string& argv) = 0;

	virtual TimeSpan getMinimumAllowableSamplePeriod() = 0;

	// FIXME:  ESIF Primitives
	virtual DptfBuffer getActiveRelationshipTable() = 0;
	virtual void setActiveRelationshipTable(const DptfBuffer& data) = 0;
	virtual DptfBuffer getThermalRelationshipTable() = 0;
	virtual void setThermalRelationshipTable(const DptfBuffer& data) = 0;
	virtual DptfBuffer getPassiveTable() = 0;
	virtual void setPassiveTable(const DptfBuffer& data) = 0;
	virtual DptfBuffer getAdaptivePerformanceActionsTable(const std::string& uuid) = 0;
	virtual DptfBuffer getAdaptivePerformanceConditionsTable(const std::string& uuid) = 0;
	virtual DptfBuffer getOemVariables() = 0;
	virtual DptfBuffer getSwOemVariables() = 0;
	virtual void setSwOemVariables(const DptfBuffer& data) = 0;
	virtual UInt32 getProcessorConfigTdpControl(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Power getProcessorConfigTdpLevel(UIntN participantIndex, UIntN domainIndex, UIntN configTdpControl) = 0;
	virtual UInt32 getProcessorConfigTdpLock(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Power getProcessorTdp(UIntN participantIndex, UIntN domainIndex) const = 0;
	virtual Temperature getProcessorTjMax(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual DptfBuffer getPowerBossConditionsTable() = 0;
	virtual DptfBuffer getPowerBossActionsTable() = 0;
	virtual DptfBuffer getPowerBossMathTable() = 0;
	virtual DptfBuffer getVoltageThresholdMathTable() = 0;
	virtual DptfBuffer getEmergencyCallModeTable() = 0;
	virtual DptfBuffer getPidAlgorithmTable() = 0;
	virtual Bool getDisplayRequired() = 0;
	virtual TimeSpan getExpectedBatteryLife() = 0;
	virtual UInt32 getAggressivenessLevel() = 0;
	virtual DptfBuffer getDdrfTable() = 0;
	virtual DptfBuffer getRfimTable() = 0;
	virtual DptfBuffer getAggregateDisplayInformation() = 0;
	virtual DptfBuffer getEnergyPerformanceOptimizerTable() = 0;
	virtual void setEnergyPerformanceOptimizerTable(const DptfBuffer& data) = 0;
	virtual DptfBuffer getTpgaTable() = 0;
	virtual void setTpgaTable(const DptfBuffer& data) = 0;
	virtual DptfBuffer getOpbtTable() = 0;
	virtual void setOpbtTable(const DptfBuffer& data) = 0;
	virtual void setPidAlgorithmTable(const DptfBuffer& data) = 0;
	virtual DptfBuffer getActiveControlPointRelationshipTable() = 0;
	virtual void setActiveControlPointRelationshipTable(const DptfBuffer& data) = 0;
	virtual DptfBuffer getPowerShareAlgorithmTable() = 0;
	virtual void setPowerShareAlgorithmTable(const DptfBuffer& data) = 0;
	virtual DptfBuffer getPowerShareAlgorithmTable2() = 0;
	virtual void setPowerShareAlgorithmTable2(const DptfBuffer& data) = 0;
	virtual DptfBuffer getIntelligentThermalManagementTable() = 0;
	virtual void setIntelligentThermalManagementTable(const DptfBuffer& data) = 0;
	virtual DptfBuffer getIntelligentThermalManagementTable3() = 0;
	virtual void setIntelligentThermalManagementTable3(const DptfBuffer& data) = 0;
	virtual void setPpmPackage(const DptfBuffer& data) = 0;
	virtual void setPpmPackageForNonBalancedSchemePersonality(const DptfBuffer& data) = 0;
	virtual DptfBuffer getPpmPackage(const DptfBuffer& data) = 0;
	virtual void setActivePowerScheme() = 0;
	virtual void setPowerSchemeEpp(UInt32 value) = 0;
	virtual void setForegroundAppRatioPeriod(UInt32 value) = 0;
	virtual void setProcessAffinityMask(const std::string& processName, UInt32 maskValue) = 0;
	virtual void setApplicationCompatibility(const DptfBuffer& data) = 0;
	virtual void deleteApplicationCompatibility() = 0;
	virtual UInt32 getDynamicBoostState(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void setDynamicBoostState(UIntN participantIndex, UIntN domainIndex, UInt32 value) = 0;
	virtual UInt32 getTpgPowerStateWithoutCache(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual EnvironmentProfile getEnvironmentProfile() const = 0;
	virtual void clearPpmPackageSettings() = 0;
	virtual UInt32 getLogicalProcessorCount(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt32 getPhysicalCoreCount(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual DptfBuffer getSystemConfigurationFeatureTable() = 0;
	virtual std::string getActiveTableName(TableObjectType::Type tableObjectType) = 0;
};
