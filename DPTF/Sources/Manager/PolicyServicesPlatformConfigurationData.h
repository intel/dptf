/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

    virtual std::string readPlatformSettingValue(
        PlatformSettingType::Type platformSettingType, UInt8 index) override final;
    virtual void writePlatformSettingValue(
        PlatformSettingType::Type platformSettingType, UInt8 index, const std::string& stringValue) override final;
    virtual void clearPlatformSettings(
        PlatformSettingType::Type platformSettingType) override final;


    virtual TimeSpan getMinimumAllowableSamplePeriod(void) override final;

    virtual DptfBuffer getActiveRelationshipTable(void) override final;
    virtual void setActiveRelationshipTable(DptfBuffer data) override final;
    virtual DptfBuffer getThermalRelationshipTable(void) override final;
    virtual DptfBuffer getPassiveTable(void) override final;
    virtual void setPassiveTable(DptfBuffer data) override final;
    virtual DptfBuffer getAdaptivePerformanceConditionsTable(void) override final;
    virtual DptfBuffer getAdaptivePerformanceParticipantConditionTable(void) override final;
    virtual DptfBuffer getAdaptivePerformanceActionsTable(void) override final;
    virtual SensorOrientation::Type getSensorOrientation(void) override final;
    virtual OnOffToggle::Type getSensorMotion(void) override final;
    virtual SensorSpatialOrientation::Type getSensorSpatialOrientation(void) override final;
    virtual DptfBuffer getOemVariables(void) override final;
    virtual DptfBuffer getPowerBossConditionsTable(void) override final;
    virtual DptfBuffer getPowerBossActionsTable(void) override final;
    virtual DptfBuffer getPowerBossMathTable(void) override final;
    virtual DptfBuffer getEmergencyCallModeTable(void) override final;
    virtual DptfBuffer getPidAlgorithmTable(void) override final;
    virtual DptfBuffer getActiveControlPointRelationshipTable(void) override final;

private:

    TimeSpan m_defaultSamplePeriod;
};