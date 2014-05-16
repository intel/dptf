/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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
#include "DptfMemory.h"
#include "EsifServices.h"
#include "BinaryParse.h"

PolicyServicesPlatformConfigurationData::PolicyServicesPlatformConfigurationData(
    DptfManager* dptfManager, UIntN policyIndex) : PolicyServices(dptfManager, policyIndex)
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

ActiveRelationshipTable PolicyServicesPlatformConfigurationData::getActiveRelationshipTable(void)
{
    throwIfNotWorkItemThread();

    UInt32 dataLength = 0;
    DptfMemory binaryData;
    binaryData.allocate(Constants::DefaultBufferSize, true);

    getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_ACTIVE_RELATIONSHIP_TABLE,
        ESIF_DATA_BINARY,
        binaryData,
        binaryData.getSize(),
        &dataLength);

    ActiveRelationshipTable art(BinaryParse::activeArtObject(dataLength, binaryData));

    binaryData.deallocate();

    return art;
}

ThermalRelationshipTable PolicyServicesPlatformConfigurationData::getThermalRelationshipTable(void)
{
    throwIfNotWorkItemThread();

    UInt32 dataLength = 0;
    DptfMemory binaryData;
    binaryData.allocate(Constants::DefaultBufferSize, true);

    getEsifServices()->primitiveExecuteGet(
        esif_primitive_type::GET_THERMAL_RELATIONSHIP_TABLE,
        ESIF_DATA_BINARY,
        binaryData,
        binaryData.getSize(),
        &dataLength);

    ThermalRelationshipTable trt(BinaryParse::passiveTrtObject(dataLength, binaryData));

    binaryData.deallocate();

    return trt;
}

UInt32 PolicyServicesPlatformConfigurationData::getLpmMode(void)
{
    throwIfNotWorkItemThread();

    return getEsifServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_CURRENT_LOW_POWER_MODE
        );
}

LpmTable PolicyServicesPlatformConfigurationData::getLpmTable(void)
{
    throwIfNotWorkItemThread();

    UInt32 dataLength = 0;
    DptfMemory binaryData;
    binaryData.allocate(Constants::DefaultBufferSize, true);
    LpmTable lpmTable(LpmConfigurationVersion::vInvalid, vector<LpmEntry>());

    try
    {
        getEsifServices()->primitiveExecuteGet(
            esif_primitive_type::GET_LPM_TABLE,
            ESIF_DATA_BINARY,
            binaryData,
            binaryData.getSize(),
            &dataLength
            );

        lpmTable = BinaryParse::lpmTableObject(dataLength, binaryData);
        binaryData.deallocate();
    }
    catch (...)
    {
        // TODO: This is still under discussion so leaving the v0 part commented.

        // We assume version:v0 if we come here.
        //lpmTable = LpmTable(LpmConfigurationVersion::v0, vector<LpmEntry>());
        lpmTable = LpmTable(LpmConfigurationVersion::v1, vector<LpmEntry>());
    }

    return (lpmTable);
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
