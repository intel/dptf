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

#include "CriticalTripPointsCachedProperty.h"
#include "XmlCommon.h"
using namespace std;

CriticalTripPointsCachedProperty::CriticalTripPointsCachedProperty(PolicyServicesInterfaceContainer policyServices, UIntN participantIndex)
    : CachedProperty(), ParticipantProperty(participantIndex, policyServices),
    m_criticalTripPoints(map<ParticipantSpecificInfoKey::Type, UIntN>())
{
}

CriticalTripPointsCachedProperty::~CriticalTripPointsCachedProperty(void)
{
}

void CriticalTripPointsCachedProperty::refreshData(void)
{
    vector<ParticipantSpecificInfoKey::Type> specificInfoList;
    specificInfoList.push_back(ParticipantSpecificInfoKey::Critical);
    specificInfoList.push_back(ParticipantSpecificInfoKey::Hot);
    specificInfoList.push_back(ParticipantSpecificInfoKey::Warm);
    m_criticalTripPoints =
        SpecificInfo(getPolicyServices().participantGetSpecificInfo->getParticipantSpecificInfo(
            getParticipantIndex(),
            specificInfoList));
}

const SpecificInfo& CriticalTripPointsCachedProperty::getTripPoints()
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return m_criticalTripPoints;
}

Bool CriticalTripPointsCachedProperty::supportsProperty(void)
{
    // make sure the critical trip points contain at least one of {warm, hot, critical}
    auto criticalTripPoints = getTripPoints();
    return (criticalTripPoints.hasItem(ParticipantSpecificInfoKey::Warm)
        || criticalTripPoints.hasItem(ParticipantSpecificInfoKey::Hot)
        || criticalTripPoints.hasItem(ParticipantSpecificInfoKey::Critical));
}

XmlNode* CriticalTripPointsCachedProperty::getXml()
{
    return getTripPoints().getXml();
}