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

#include "PassiveTripPointsCachedProperty.h"
#include "XmlCommon.h"
using namespace std;

PassiveTripPointsCachedProperty::PassiveTripPointsCachedProperty(PolicyServicesInterfaceContainer policyServices, UIntN participantIndex)
    : CachedProperty(), ParticipantProperty(participantIndex, policyServices),
    m_passiveTripPoints(map<ParticipantSpecificInfoKey::Type, UIntN>())
{
}

PassiveTripPointsCachedProperty::~PassiveTripPointsCachedProperty(void)
{
}

void PassiveTripPointsCachedProperty::refreshData(void)
{
    vector<ParticipantSpecificInfoKey::Type> specificInfoList;
    specificInfoList.push_back(ParticipantSpecificInfoKey::PSV);
    specificInfoList.push_back(ParticipantSpecificInfoKey::NTT);
    m_passiveTripPoints =
        SpecificInfo(getPolicyServices().participantGetSpecificInfo->getParticipantSpecificInfo(
            getParticipantIndex(),
            specificInfoList));
}

const SpecificInfo& PassiveTripPointsCachedProperty::getTripPoints()
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return m_passiveTripPoints;
}

Bool PassiveTripPointsCachedProperty::supportsProperty(void)
{
    // must contain psv, ntt is optional
    auto passiveTripPoints = getTripPoints();
    return passiveTripPoints.hasItem(ParticipantSpecificInfoKey::PSV);
}

XmlNode* PassiveTripPointsCachedProperty::getXml()
{
    return getTripPoints().getXml();
}