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

#include "WIDptfConnectedStandbyEntry.h"
#include "PolicyManager.h"
#include "ParticipantManager.h"
#include "EsifServices.h"

WIDptfConnectedStandbyEntry::WIDptfConnectedStandbyEntry(DptfManager* dptfManager) :
    WorkItem(dptfManager, FrameworkEvent::DptfConnectedStandbyEntry)
{
}

WIDptfConnectedStandbyEntry::~WIDptfConnectedStandbyEntry(void)
{
}

void WIDptfConnectedStandbyEntry::execute(void)
{
    WriteWorkItemStartingInfoMessage();

    // First let all policies know that we are entering connected standby

    PolicyManager* policyManager = getPolicyManager();
    UIntN policyListCount = policyManager->getPolicyListCount();

    for (UIntN i = 0; i < policyListCount; i++)
    {
        try
        {
            Policy* policy = policyManager->getPolicyPtr(i);
            policy->executeConnectedStandbyEntry();
        }
        catch (policy_index_invalid ex)
        {
            // do nothing.  No item in the policy list at this index.
        }
        catch (std::exception ex)
        {
            WriteWorkItemErrorMessage_Function_Policy("Policy::executeConnectedStandbyEntry", i);
        }
    }

    // Now let all participants know

    ParticipantManager* participantManager = getParticipantManager();
    UIntN participantListCount = participantManager->getParticipantListCount();

    for (UIntN i = 0; i < participantListCount; i++)
    {
        try
        {
            Participant* participant = participantManager->getParticipantPtr(i);
            participant->connectedStandbyEntry();
        }
        catch (participant_index_invalid ex)
        {
            // do nothing.  No item in the participant list at this index.
        }
        catch (std::exception ex)
        {
            WriteWorkItemErrorMessage_Function_Participant("Participant::connectedStandbyEntry", i);
        }
    }
}