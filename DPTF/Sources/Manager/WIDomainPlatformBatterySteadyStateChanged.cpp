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

#include "WIDomainPlatformBatterySteadyStateChanged.h"
#include "Participant.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"

WIDomainPlatformBatterySteadyStateChanged::WIDomainPlatformBatterySteadyStateChanged(
    DptfManagerInterface* dptfManager, UIntN participantIndex, UIntN domainIndex) :
    DomainWorkItem(dptfManager, FrameworkEvent::Type::DomainPlatformBatterySteadyStateChanged, participantIndex, domainIndex)
{
}

WIDomainPlatformBatterySteadyStateChanged::~WIDomainPlatformBatterySteadyStateChanged(void)
{
}

void WIDomainPlatformBatterySteadyStateChanged::execute(void)
{
    writeDomainWorkItemStartingInfoMessage();

    try
    {
        getParticipantPtr()->domainPlatformBatterySteadyStateChanged();
    }
    catch (std::exception& ex)
    {
        writeDomainWorkItemErrorMessage(ex, "Participant::domainPlatformBatterySteadyStateChanged");
    }

    auto policyManager = getPolicyManager();
    UIntN policyListCount = policyManager->getPolicyListCount();

    for (UIntN i = 0; i < policyListCount; i++)
    {
        try
        {
            Policy* policy = policyManager->getPolicyPtr(i);
            policy->executeDomainPlatformBatterySteadyStateChanged(getParticipantIndex());
        }
        catch (policy_index_invalid ex)
        {
            // do nothing.  No item in the policy list at this index.
        }
        catch (std::exception& ex)
        {
            writeDomainWorkItemErrorMessagePolicy(ex, "Policy::executeDomainPlatformBatterySteadyStateChanged", i);
        }
    }
}