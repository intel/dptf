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
#include "RelationshipTableEntryBase.h"
#include "XmlNode.h"
#include <string>

class ActiveRelationshipTableEntry : public RelationshipTableEntryBase
{
public:

    ActiveRelationshipTableEntry(
        const std::string& sourceDeviceAcpiScope,
        const std::string& targetDeviceAcpiScope,
        UInt32 weight,
        const std::vector<UInt32>& acEntries);
    ~ActiveRelationshipTableEntry();

    const UInt32& weight() const;
    const UInt32& ac(UIntN acLevel) const;

    XmlNode* getXml();

    static const UIntN FanOffIndex = 10;

private:

    UInt32 m_weight;
    std::vector<UInt32> m_acEntries;
};