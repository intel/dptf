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

#pragma once

#include "PolicyServices.h"
#include "PlatformConfigurationDataInterface.h"

class PolicyServicesPlatformConfigurationData final : public PolicyServices, public PlatformConfigurationDataInterface
{
public:

    PolicyServicesPlatformConfigurationData(DptfManager* dptfManager, UIntN policyIndex);

    virtual UInt32 readConfigurationUInt32(const std::string& key) override final;
    virtual void writeConfigurationUInt32(const std::string& key, UInt32 data) override final;
    virtual std::string readConfigurationString(const std::string& key) override final;

    virtual std::string readPlatformSettingValue(
        PlatformSettingType::Type platformSettingType, UInt8 index) override final;
    virtual void writePlatformSettingValue(
        PlatformSettingType::Type platformSettingType, UInt8 index, const std::string& stringValue) override final;
    virtual void clearPlatformSettings(
        PlatformSettingType::Type platformSettingType) override final;

    virtual ActiveRelationshipTable getActiveRelationshipTable(void) override final;
    virtual ThermalRelationshipTable getThermalRelationshipTable(void) override final;
    virtual LpmTable getLpmTable(void) override final;
    virtual UInt32 getLpmMode(void) override final;
};