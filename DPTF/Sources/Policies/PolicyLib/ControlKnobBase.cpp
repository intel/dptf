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

#include "ControlKnobBase.h"
using namespace std;

ControlKnobBase::ControlKnobBase(
    const PolicyServicesInterfaceContainer& policyServices, UIntN participantIndex, UIntN domainIndex)
    : m_policyServices(policyServices),
    m_participantIndex(participantIndex),
    m_domainIndex(domainIndex)
{
}

ControlKnobBase::~ControlKnobBase(void)
{
}

UIntN ControlKnobBase::getParticipantIndex() const
{
    return m_participantIndex;
}

UIntN ControlKnobBase::getDomainIndex() const
{
    return m_domainIndex;
}

PolicyServicesInterfaceContainer ControlKnobBase::getPolicyServices() const
{
    return m_policyServices;
}