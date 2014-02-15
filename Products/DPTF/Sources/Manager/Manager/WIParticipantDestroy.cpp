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

#include "WIParticipantDestroy.h"
#include "DptfManager.h"
#include "WorkItemQueueManager.h"
#include "PolicyManager.h"
#include "ParticipantManager.h"
#include "WorkItemMatchCriteria.h"
#include "EsifServices.h"

WIParticipantDestroy::WIParticipantDestroy(DptfManager* dptfManager, UIntN participantIndex) :
    ParticipantWorkItem(dptfManager, FrameworkEvent::ParticipantDestroy, participantIndex)
{
}

WIParticipantDestroy::~WIParticipantDestroy(void)
{
}

void WIParticipantDestroy::execute(void)
{
    WriteParticipantWorkItemStartingInfoMessage();

    // Call unbind participant for each policy before we actually destroy the participant.

    PolicyManager* policyManager = getPolicyManager();
    UIntN policyListCount = policyManager->getPolicyListCount();

    for (UIntN i = 0; i < policyListCount; i++)
    {
        try
        {
            Policy* policy = policyManager->getPolicyPtr(i);
            policy->unbindParticipant(getParticipantIndex());
        }
        catch (policy_index_invalid ex)
        {
            // do nothing.  No item in the policy list at this index.
        }
        catch (std::exception ex)
        {
            WriteParticipantWorkItemErrorMessage_Function_Policy("Policy::unbindParticipant", i);
        }
    }

    // Remove any work items from the queues that were submitted for this participant

    try
    {
        WorkItemMatchCriteria workItemMatchCriteria;
        workItemMatchCriteria.addParticipantIndexToMatchList(getParticipantIndex());
        getDptfManager()->getWorkItemQueueManager()->removeIfMatches(workItemMatchCriteria);
    }
    catch (std::exception ex)
    {
        WriteParticipantWorkItemErrorMessage_Function("WorkItemQueueManager::removeIfMatches");
    }

    // Now let the participant manager destroy the participant

    try
    {
        getParticipantManager()->destroyParticipant(getParticipantIndex());
    }
    catch (std::exception ex)
    {
        WriteParticipantWorkItemErrorMessage_Function("ParticipantManager::destroyParticipant");
    }
}