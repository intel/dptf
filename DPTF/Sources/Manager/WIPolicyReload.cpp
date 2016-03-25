/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "WIPolicyReload.h"
#include "ParticipantManager.h"
#include "Participant.h"
#include "PolicyManager.h"
#include "EsifServices.h"

WIPolicyReload::WIPolicyReload(DptfManagerInterface* dptfManager) :
    WorkItem(dptfManager, FrameworkEvent::ParticipantDestroy)
{
}

WIPolicyReload::~WIPolicyReload(void)
{
}

void WIPolicyReload::execute(void)
{
    WriteWorkItemStartingInfoMessage();

    // Iterate through the list of policies and unbind all participant and domains from them
    
    DptfManagerInterface* dptfManager = getDptfManager();
    PolicyManager* policyManager = getPolicyManager();
    UIntN participantCount = getParticipantManager()->getParticipantListCount();

    // Unbind every participant and domain from every policy
    for (UIntN participantIndex = 0; participantIndex < participantCount; participantIndex++)
    {
            dptfManager->unbindDomainsFromPolicies(participantIndex);
            dptfManager->unbindParticipantFromPolicies(participantIndex);
    }

    // No try-catch here because destroyAllPolicies' calling tree catches all exceptions.
    policyManager->destroyAllPolicies();

    try
    {
        policyManager->reloadAllPolicies(dptfManager->getDptfPolicyDirectoryPath());
    }
    catch (dptf_exception ex)
    {
        WriteWorkItemErrorMessage_Function("policyManager::reloadAllPolicies");
    }

    // Bind every participant and domain to every policy
    for (UIntN participantIndex = 0; participantIndex < participantCount; participantIndex++)
    {
            dptfManager->bindParticipantToPolicies(participantIndex);
            dptfManager->bindDomainsToPolicies(participantIndex);
    }
}