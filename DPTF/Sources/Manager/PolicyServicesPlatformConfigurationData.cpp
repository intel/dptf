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

#include "PolicyServicesPlatformConfigurationData.h"
#include "EsifServicesInterface.h"
#include "esif_sdk_data_misc.h"

PolicyServicesPlatformConfigurationData::PolicyServicesPlatformConfigurationData(
    DptfManagerInterface* dptfManager, UIntN policyIndex) 
    : PolicyServices(dptfManager, policyIndex), m_defaultSamplePeriod(TimeSpan::createInvalid())
{
}

UInt32 PolicyServicesPlatformConfigurationData::readConfigurationUInt32(const std::string& key)
{
    throwIfNotWorkItemThread();
    UInt32 value = getEsifServices()->readConfigurationUInt32(key);
    return value;
}

void PolicyServicesPlatformConfigurationData::writeConfigurationUInt32(const std::string& key, UInt32 data)
{
    throwIfNotWorkItemThread();
    getEsifServices()->writeConfigurationUInt32(key, data);
}

std::string PolicyServicesPlatformConfigurationData::readConfigurationString(const std::string& key)
{
    throwIfNotWorkItemThread();
    std::string value = getEsifServices()->readConfigurationString(key);
    return value;
}

DptfBuffer PolicyServicesPlatformConfigurationData::readConfigurationBinary(const std::string& key)
{
    throwIfNotWorkItemThread();
    return getEsifServices()->readConfigurationBinary(key);
}

TimeSpan PolicyServicesPlatformConfigurationData::getMinimumAllowableSamplePeriod(void)
{
    throwIfNotWorkItemThread();
    
    if(m_defaultSamplePeriod.isInvalid())
    {
        m_defaultSamplePeriod = getEsifServices()->primitiveExecuteGetAsTimeInMilliseconds(
            esif_primitive_type::GET_MINIMUM_SAMPLE_PERIOD);
    }

    return m_defaultSamplePeriod;
}

DptfBuffer PolicyServicesPlatformConfigurationData::getActiveRelationshipTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_ACTIVE_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setActiveRelationshipTable(DptfBuffer data)
{
    throwIfNotWorkItemThread();

    getEsifServices()->primitiveExecuteSet(esif_primitive_type::SET_ACTIVE_RELATIONSHIP_TABLE,
        esif_data_type::ESIF_DATA_BINARY, data.get(), data.size(), data.size(),
        Constants::Esif::NoParticipant, Constants::Esif::NoDomain, Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getThermalRelationshipTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_THERMAL_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setThermalRelationshipTable(DptfBuffer data)
{
    throwIfNotWorkItemThread();

    getEsifServices()->primitiveExecuteSet(esif_primitive_type::SET_THERMAL_RELATIONSHIP_TABLE,
        esif_data_type::ESIF_DATA_BINARY, data.get(), data.size(), data.size(),
        Constants::Esif::NoParticipant, Constants::Esif::NoDomain, Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPassiveTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_PASSIVE_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setPassiveTable(DptfBuffer data)
{
    throwIfNotWorkItemThread();

    getEsifServices()->primitiveExecuteSet(esif_primitive_type::SET_PASSIVE_RELATIONSHIP_TABLE,
        esif_data_type::ESIF_DATA_BINARY, data.get(), data.size(), data.size(),
        Constants::Esif::NoParticipant, Constants::Esif::NoDomain, Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceConditionsTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_ADAPTIVE_PERFORMANCE_CONDITIONS_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceParticipantConditionTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_ADAPTIVE_PERFORMANCE_PARTICIPANT_CONDITION_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceActionsTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_ADAPTIVE_PERFORMANCE_ACTIONS_TABLE, ESIF_DATA_BINARY);
}

SensorOrientation::Type PolicyServicesPlatformConfigurationData::getSensorOrientation(void)
{
    throw not_implemented();

    //
    // FIXME: need to add primitive and test
    //
    //throwIfNotWorkItemThread();
    //
    //UInt32 primitiveValue = getEsifServices()->primitiveExecuteGetAsUInt32(
    //    esif_primitive_type::/*primitive*/);
    //
    //return static_cast<SensorOrientation::Type>(primitiveValue);
}

OnOffToggle::Type PolicyServicesPlatformConfigurationData::getSensorMotion(void)  
{
    throw not_implemented();

    //
    // FIXME: need to add primitive and test
    //
    //throwIfNotWorkItemThread();
    //
    //UInt32 primitiveValue = getEsifServices()->primitiveExecuteGetAsUInt32(
    //    esif_primitive_type::/*primitive*/);
    //
    //return static_cast<SensorMotion::Type>(primitiveValue);
}

SensorSpatialOrientation::Type PolicyServicesPlatformConfigurationData::getSensorSpatialOrientation(void)
{
    throw not_implemented();

    //
    // FIXME: need to add primitive and test
    //
    //throwIfNotWorkItemThread();
    //
    //UInt32 primitiveValue = getEsifServices()->primitiveExecuteGetAsUInt32(
    //    esif_primitive_type::/*primitive*/);
    //
    //return static_cast<SensorSpatialOrientation::Type>(primitiveValue);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getOemVariables(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_OEM_VARS, ESIF_DATA_BINARY);
}

std::string PolicyServicesPlatformConfigurationData::readPlatformSettingValue(
    PlatformSettingType::Type platformSettingType, UInt8 index)  
{
    throwIfNotWorkItemThread();

    switch (platformSettingType)
    {
        case PlatformSettingType::ConfigTdp:
        {
            return getEsifServices()->primitiveExecuteGetAsString(esif_primitive_type::GET_SYSTEM_CONFIGTDP_LEVEL_NAME, 
                Constants::Esif::NoParticipant, Constants::Esif::NoDomain, index);
            break;
        }
        default:
        {
            throw dptf_exception("Invalid platform setting type referenced in call to read platform setting value.");
        }
    }

}

void PolicyServicesPlatformConfigurationData::writePlatformSettingValue(
    PlatformSettingType::Type platformSettingType, UInt8 index, const std::string& stringValue)  
{
    throwIfNotWorkItemThread();

    switch (platformSettingType)
    {
        case PlatformSettingType::ConfigTdp:
        {
            getEsifServices()->primitiveExecuteSetAsString(esif_primitive_type::SET_SYSTEM_CONFIGTDP_LEVEL_NAME, 
                stringValue, Constants::Esif::NoParticipant, Constants::Esif::NoDomain, index);
            break;
        }
        default:
        {
            throw dptf_exception("Invalid platform setting type referenced in call to write platform setting value.");
        }
    }
}

const Guid DptfDppeGroup(0x48, 0xdf, 0x9d, 0x60, 0x4f, 0x68, 0x11, 0xdc, 0x83, 0x14, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66);
void PolicyServicesPlatformConfigurationData::clearPlatformSettings(
    PlatformSettingType::Type platformSettingType)
{
    throwIfNotWorkItemThread();

    switch (platformSettingType)
    {
        case PlatformSettingType::ConfigTdp:
        {
            getEsifServices()->primitiveExecuteSet(esif_primitive_type::SET_SYSTEM_CONFIGTDP_CLEAR_LEVELS, 
                esif_data_type::ESIF_DATA_VOID, nullptr, 0, 0);
            break;
        }
        default:
        {
            throw dptf_exception("Invalid platform setting type referenced in call to clear platform settings.");
        }
    }
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerBossConditionsTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_POWER_BOSS_CONDITIONS_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerBossActionsTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_POWER_BOSS_ACTIONS_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getEmergencyCallModeTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_EMERGENCY_CALL_MODE_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPidAlgorithmTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_PID_ALGORITHM_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setPidAlgorithmTable(DptfBuffer data)
{
    throwIfNotWorkItemThread();

    getEsifServices()->primitiveExecuteSet(esif_primitive_type::SET_PID_ALGORITHM_TABLE,
        esif_data_type::ESIF_DATA_BINARY, data.get(), data.size(), data.size(),
        Constants::Esif::NoParticipant, Constants::Esif::NoDomain, Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerBossMathTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_POWER_BOSS_MATH_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getActiveControlPointRelationshipTable(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_ACTIVE_CONTROL_POINT_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}