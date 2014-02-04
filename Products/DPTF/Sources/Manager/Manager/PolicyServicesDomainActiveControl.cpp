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

#include "PolicyServicesDomainActiveControl.h"
#include "ParticipantManager.h"

PolicyServicesDomainActiveControl::PolicyServicesDomainActiveControl(DptfManager* dptfManager, UIntN policyIndex) :
    PolicyServices(dptfManager, policyIndex)
{
}

ActiveControlStaticCaps PolicyServicesDomainActiveControl::getActiveControlStaticCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getActiveControlStaticCaps(domainIndex);
}

ActiveControlStatus PolicyServicesDomainActiveControl::getActiveControlStatus(UIntN participantIndex,
    UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getActiveControlStatus(domainIndex);
}

ActiveControlSet PolicyServicesDomainActiveControl::getActiveControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getActiveControlSet(domainIndex);
}

void PolicyServicesDomainActiveControl::setActiveControl(UIntN participantIndex, UIntN domainIndex,
    UIntN controlIndex)
{
    throwIfNotWorkItemThread();
    getParticipantManager()->getParticipantPtr(participantIndex)->setActiveControl(domainIndex,
        getPolicyIndex(), controlIndex);
}

void PolicyServicesDomainActiveControl::setActiveControl(UIntN participantIndex, UIntN domainIndex,
    const Percentage& fanSpeed)
{
    throwIfNotWorkItemThread();
    getParticipantManager()->getParticipantPtr(participantIndex)->setActiveControl(domainIndex,
        getPolicyIndex(), fanSpeed);
}