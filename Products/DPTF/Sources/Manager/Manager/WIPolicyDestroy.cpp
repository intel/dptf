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

#include "WIPolicyDestroy.h"
#include "DptfManager.h"
#include "PolicyManager.h"
#include "ParticipantManager.h"
#include "EsifServices.h"

WIPolicyDestroy::WIPolicyDestroy(DptfManager* dptfManager, UIntN policyIndex) :
    WorkItem(dptfManager, FrameworkEvent::PolicyDestroy), m_policyIndex(policyIndex)
{
}

WIPolicyDestroy::~WIPolicyDestroy(void)
{
}

void WIPolicyDestroy::execute(void)
{
    WriteWorkItemStartingInfoMessage();

    // First let the policy manager know to remove the policy

    try
    {
        getDptfManager()->getPolicyManager()->destroyPolicy(m_policyIndex);
    }
    catch (std::exception ex)
    {
        WriteWorkItemErrorMessage_Function("PolicyManager::destroyPolicy");
    }

    // Now let the participants know so they can clear all arbitration data that
    // is stored for this policy

    ParticipantManager* participantManager = getParticipantManager();
    UIntN participantListCount = participantManager->getParticipantListCount();

    for (UIntN i = 0; i < participantListCount; i++)
    {
        try
        {
            Participant* participant = participantManager->getParticipantPtr(i);
            participant->clearArbitrationDataForPolicy(m_policyIndex);
        }
        catch (participant_index_invalid ex)
        {
            // do nothing.  No item in the participant list at this index.
        }
        catch (std::exception ex)
        {
            WriteWorkItemErrorMessage_Function_Participant("Participant::clearArbitrationDataForPolicy", i);
        }
    }
}

Bool WIPolicyDestroy::matches(const WorkItemMatchCriteria& matchCriteria) const
{
    return matchCriteria.testAgainstMatchList(getFrameworkEventType(), getUniqueId(),
        Constants::Invalid, Constants::Invalid, m_policyIndex);
}