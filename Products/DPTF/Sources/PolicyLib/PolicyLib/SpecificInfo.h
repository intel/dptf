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
#include "ParticipantSpecificInfoKey.h"
#include <vector>
#include <map>
#include "XmlNode.h"

// wraps specific info with functions to sort the data, access items, and generate xml status
class dptf_export SpecificInfo
{
public:

    SpecificInfo(std::map<ParticipantSpecificInfoKey::Type, UIntN> specificInfo);
    ~SpecificInfo(void);

    std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> getSortedByValue();
    std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> getSortedByKey();

    Bool hasItem(ParticipantSpecificInfoKey::Type key);
    UIntN getItem(ParticipantSpecificInfoKey::Type key);

    XmlNode* getXml() const;

private:

    std::map<ParticipantSpecificInfoKey::Type, UIntN> m_specificInfo;
    std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> m_sortedTripPointsByValue;
    Bool m_sortedTripPointsByValueValid;
    std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> m_sortedTripPointsByKey;
    Bool m_sortedTripPointsByKeyValid;
};