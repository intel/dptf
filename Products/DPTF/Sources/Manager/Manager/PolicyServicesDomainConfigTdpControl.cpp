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

#include "PolicyServicesDomainConfigTdpControl.h"
#include "ParticipantManager.h"

PolicyServicesDomainConfigTdpControl::PolicyServicesDomainConfigTdpControl(DptfManager* dptfManager, UIntN policyIndex) :
    PolicyServices(dptfManager, policyIndex)
{
}

ConfigTdpControlDynamicCaps PolicyServicesDomainConfigTdpControl::getConfigTdpControlDynamicCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getConfigTdpControlDynamicCaps(domainIndex);
}

ConfigTdpControlStatus PolicyServicesDomainConfigTdpControl::getConfigTdpControlStatus(UIntN participantIndex,
    UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getConfigTdpControlStatus(domainIndex);
}

ConfigTdpControlSet PolicyServicesDomainConfigTdpControl::getConfigTdpControlSet(UIntN participantIndex,
    UIntN domainIndex)
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getConfigTdpControlSet(domainIndex);
}

void PolicyServicesDomainConfigTdpControl::setConfigTdpControl(UIntN participantIndex, UIntN domainIndex,
    UIntN controlIndex)
{
    throwIfNotWorkItemThread();
    getParticipantManager()->getParticipantPtr(participantIndex)->setConfigTdpControl(domainIndex,
        getPolicyIndex(), controlIndex);
}