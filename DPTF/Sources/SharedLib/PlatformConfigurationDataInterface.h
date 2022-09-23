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
#include "SensorOrientation.h"
#include "OnOffToggle.h"
#include "SensorSpatialOrientation.h"
#include "PpmPackage.h"

class PlatformConfigurationDataInterface
{
public:
	virtual ~PlatformConfigurationDataInterface(){};

	virtual UInt32 readConfigurationUInt32(const std::string& key) = 0;
	virtual UInt32 readConfigurationUInt32(const std::string& nameSpace, const std::string& key) = 0;
	virtual void writeConfigurationUInt32(const std::string& key, UInt32 data) = 0;
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

	virtual TimeSpan getMinimumAllowableSamplePeriod(void) = 0;

	// FIXME:  ESIF Primitives
	virtual DptfBuffer getActiveRelationshipTable(void) = 0;
	virtual void setActiveRelationshipTable(DptfBuffer data) = 0;
	virtual DptfBuffer getThermalRelationshipTable(void) = 0;
	virtual void setThermalRelationshipTable(DptfBuffer data) = 0;
	virtual DptfBuffer getPassiveTable(void) = 0;
	virtual void setPassiveTable(DptfBuffer data) = 0;
	virtual DptfBuffer getAdaptivePerformanceActionsTable(std::string uuid) = 0;
	virtual DptfBuffer getAdaptivePerformanceConditionsTable(std::string uuid) = 0;
	virtual DptfBuffer getOemVariables(void) = 0;
	virtual DptfBuffer getSwOemVariables(void) = 0;
	virtual void setSwOemVariables(const DptfBuffer& swOemVariablesData) = 0;
	virtual UInt64 getHwpfState(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt32 getProcessorConfigTdpControl(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Power getProcessorConfigTdpLevel(UIntN participantIndex, UIntN domainIndex, UIntN configTdpControl) = 0;
	virtual UInt32 getProcessorConfigTdpLock(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Power getProcessorTdp(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Temperature getProcessorTjMax(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual DptfBuffer getPowerBossConditionsTable(void) = 0;
	virtual DptfBuffer getPowerBossActionsTable(void) = 0;
	virtual DptfBuffer getPowerBossMathTable(void) = 0;
	virtual DptfBuffer getVoltageThresholdMathTable(void) = 0;
	virtual DptfBuffer getEmergencyCallModeTable(void) = 0;
	virtual DptfBuffer getPidAlgorithmTable(void) = 0;
	virtual Bool getDisplayRequired(void) = 0;
	virtual TimeSpan getExpectedBatteryLife(void) = 0;
	virtual UInt32 getAggressivenessLevel(void) = 0;
	virtual DptfBuffer getDdrfTable(void) = 0;
	virtual DptfBuffer getAggregateDisplayInformation(void) = 0;
	virtual DptfBuffer getEnergyPerformanceOptimizerTable(void) = 0;
	virtual void setEnergyPerformanceOptimizerTable(DptfBuffer data) = 0;
	virtual DptfBuffer getTpgaTable(void) = 0;
	virtual void setTpgaTable(DptfBuffer data) = 0;
	virtual void setPidAlgorithmTable(DptfBuffer data) = 0;
	virtual DptfBuffer getActiveControlPointRelationshipTable(void) = 0;
	virtual void setActiveControlPointRelationshipTable(DptfBuffer data) = 0;
	virtual DptfBuffer getPowerShareAlgorithmTable(void) = 0;
	virtual void setPowerShareAlgorithmTable(DptfBuffer data) = 0;
	virtual DptfBuffer getPowerShareAlgorithmTable2(void) = 0;
	virtual void setPowerShareAlgorithmTable2(DptfBuffer data) = 0;
	virtual DptfBuffer getIntelligentThermalManagementTable(void) = 0;
	virtual void setIntelligentThermalManagementTable(DptfBuffer data) = 0;
	virtual void setPpmPackage(DptfBuffer package) = 0;
	virtual DptfBuffer getPpmPackage(DptfBuffer package) = 0;
	virtual void setActivePowerScheme() = 0;
	virtual void setPowerSchemeEpp(UInt32 value) = 0;
	virtual void setForegroundAppRatioPeriod(UInt32 value) = 0;
	virtual UInt32 getDynamicBoostState(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void setDynamicBoostState(UIntN participantIndex, UIntN domainIndex, UInt32 value) = 0;
	virtual UInt32 getTpgPowerStateWithoutCache(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt64 getPlatformCpuId() = 0;
	virtual void clearPpmPackageSettings(void) = 0;
	virtual UInt32 getLogicalProcessorCount(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt32 getPhysicalCoreCount(UIntN participantIndex, UIntN domainIndex) = 0;
};
