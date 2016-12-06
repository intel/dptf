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
#include "PlatformSettingType.h"
#include "SensorOrientation.h"
#include "OnOffToggle.h"
#include "SensorSpatialOrientation.h"

class PlatformConfigurationDataInterface
{
public:

    virtual ~PlatformConfigurationDataInterface()
    {
    };

    virtual UInt32 readConfigurationUInt32(const std::string& key) = 0;
    virtual void writeConfigurationUInt32(const std::string& key, UInt32 data) = 0;
    virtual std::string readConfigurationString(const std::string& key) = 0;
    virtual DptfBuffer readConfigurationBinary(const std::string& key) = 0;

    virtual std::string readPlatformSettingValue(PlatformSettingType::Type platformSettingType, UInt8 index) = 0;
    virtual void writePlatformSettingValue(PlatformSettingType::Type platformSettingType, UInt8 index, 
        const std::string& stringValue) = 0;
    virtual void clearPlatformSettings(PlatformSettingType::Type platformSettingType) = 0;

    virtual TimeSpan getMinimumAllowableSamplePeriod(void) = 0;

    //FIXME:  ESIF Primitives
    virtual DptfBuffer getActiveRelationshipTable(void) = 0;
    virtual void setActiveRelationshipTable(DptfBuffer data) = 0;
    virtual DptfBuffer getThermalRelationshipTable(void) = 0;
    virtual void setThermalRelationshipTable(DptfBuffer data) = 0;
    virtual DptfBuffer getPassiveTable(void) = 0;
    virtual DptfBuffer getAdaptivePerformanceConditionsTable(void) = 0;
    virtual DptfBuffer getAdaptivePerformanceParticipantConditionTable(void) = 0;
    virtual void setPassiveTable(DptfBuffer data) = 0;
    virtual DptfBuffer getAdaptivePerformanceActionsTable(void) = 0;
    virtual SensorOrientation::Type getSensorOrientation(void) = 0;
    virtual OnOffToggle::Type getSensorMotion(void) = 0;
    virtual SensorSpatialOrientation::Type getSensorSpatialOrientation(void) = 0;
    virtual DptfBuffer getOemVariables(void) = 0;
    virtual DptfBuffer getPowerBossConditionsTable(void) = 0;
    virtual DptfBuffer getPowerBossActionsTable(void) = 0;
    virtual DptfBuffer getPowerBossMathTable(void) = 0;
    virtual DptfBuffer getEmergencyCallModeTable(void) = 0;
    virtual DptfBuffer getPidAlgorithmTable(void) = 0;
    virtual void setPidAlgorithmTable(DptfBuffer data) = 0;
    virtual DptfBuffer getActiveControlPointRelationshipTable(void) = 0;
};