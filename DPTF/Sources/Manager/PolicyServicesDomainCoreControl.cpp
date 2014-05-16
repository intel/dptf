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

#include "PolicyServicesDomainCoreControl.h"
#include "ParticipantManager.h"

PolicyServicesDomainCoreControl::PolicyServicesDomainCoreControl(DptfManager* dptfManager, UIntN policyIndex) :
    PolicyServices(dptfManager, policyIndex)
{
}

CoreControlStaticCaps PolicyServicesDomainCoreControl::getCoreControlStaticCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getCoreControlStaticCaps(domainIndex);
}

CoreControlDynamicCaps PolicyServicesDomainCoreControl::getCoreControlDynamicCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getCoreControlDynamicCaps(domainIndex);
}

CoreControlLpoPreference PolicyServicesDomainCoreControl::getCoreControlLpoPreference(UIntN participantIndex,
    UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getCoreControlLpoPreference(domainIndex);
}

CoreControlStatus PolicyServicesDomainCoreControl::getCoreControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getCoreControlStatus(domainIndex);
}

void PolicyServicesDomainCoreControl::setActiveCoreControl(UIntN participantIndex, UIntN domainIndex,
    const CoreControlStatus& coreControlStatus)
{
    throwIfNotWorkItemThread();
    getParticipantManager()->getParticipantPtr(participantIndex)->setActiveCoreControl(domainIndex,
        getPolicyIndex(), coreControlStatus);
}