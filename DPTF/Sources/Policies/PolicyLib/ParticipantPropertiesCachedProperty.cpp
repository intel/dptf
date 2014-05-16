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

#include "ParticipantPropertiesCachedProperty.h"
using namespace std;

static const Guid DummyGuid;

ParticipantPropertiesCachedProperty::ParticipantPropertiesCachedProperty(PolicyServicesInterfaceContainer policyServices,
    UIntN participantIndex)
    : CachedProperty(), ParticipantProperty(participantIndex, policyServices),
    m_participantProperties(DummyGuid, "", "", BusType::None, PciInfo(), AcpiInfo())
{
}

ParticipantPropertiesCachedProperty::~ParticipantPropertiesCachedProperty(void)
{
}

void ParticipantPropertiesCachedProperty::refreshData(void)
{
    m_participantProperties = getPolicyServices().participantProperties->getParticipantProperties(
        getParticipantIndex());
}

const ParticipantProperties& ParticipantPropertiesCachedProperty::getParticipantProperties()
{
    if (isCacheValid() == false)
    {
        refresh();
    }
    return m_participantProperties;
}

Bool ParticipantPropertiesCachedProperty::supportsProperty(void)
{
    return true;
}