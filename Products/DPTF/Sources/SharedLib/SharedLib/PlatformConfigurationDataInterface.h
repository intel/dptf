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

#include "Dptf.h"
#include "ActiveRelationshipTable.h"
#include "ThermalRelationshipTable.h"
#include "LpmTable.h"
#include <string>

class PlatformConfigurationDataInterface
{
public:

    virtual ~PlatformConfigurationDataInterface()
    {
    };

    virtual UInt32 readConfigurationUInt32(const std::string& key) = 0;
    virtual void writeConfigurationUInt32(const std::string& key, UInt32 data) = 0;
    virtual std::string readConfigurationString(const std::string& key) = 0;

    //FIXME:  ESIF Primitives
    virtual ActiveRelationshipTable getActiveRelationshipTable(void) = 0;
    virtual ThermalRelationshipTable getThermalRelationshipTable(void) = 0;
    virtual LpmTable getLpmTable(void) = 0;
    virtual UInt32 getLpmMode(void) = 0;
};