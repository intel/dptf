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

#include "ActiveTripPointsCachedProperty.h"
#include "XmlCommon.h"
using namespace std;

ActiveTripPointsCachedProperty::ActiveTripPointsCachedProperty(PolicyServicesInterfaceContainer policyServices, UIntN participantIndex)
    : CachedProperty(), ParticipantProperty(participantIndex, policyServices),
    m_activeTripPoints(map<ParticipantSpecificInfoKey::Type, UIntN>())
{
}

ActiveTripPointsCachedProperty::~ActiveTripPointsCachedProperty(void)
{
}

void ActiveTripPointsCachedProperty::refreshData(void)
{
    vector<ParticipantSpecificInfoKey::Type> specificInfoList;
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC0);
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC1);
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC2);
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC3);
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC4);
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC5);
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC6);
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC7);
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC8);
    specificInfoList.push_back(ParticipantSpecificInfoKey::AC9);
    m_activeTripPoints =
        SpecificInfo(getPolicyServices().participantGetSpecificInfo->getParticipantSpecificInfo(
            getParticipantIndex(),
            specificInfoList));
}

const SpecificInfo& ActiveTripPointsCachedProperty::getTripPoints()
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return m_activeTripPoints;
}

Bool ActiveTripPointsCachedProperty::supportsProperty(void)
{
    // make sure the active trip points contain at least one of {AC0-AC9}
    auto activeTripPoints = getTripPoints();
    for (Int32 key = ParticipantSpecificInfoKey::AC0;
         key <= ParticipantSpecificInfoKey::AC9;
         key++)
    {
        if (activeTripPoints.hasItem((ParticipantSpecificInfoKey::Type)key))
        {
            return true;
        }
    }

    return false;
}

XmlNode* ActiveTripPointsCachedProperty::getXml()
{
    return getTripPoints().getXml();
}