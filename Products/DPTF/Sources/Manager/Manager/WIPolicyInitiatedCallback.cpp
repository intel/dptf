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

#include "WIPolicyInitiatedCallback.h"
#include "PolicyManager.h"
#include "EsifServices.h"

WIPolicyInitiatedCallback::WIPolicyInitiatedCallback(DptfManager* dptfManager, UIntN policyIndex,
    UInt64 policyDefinedEventCode, UInt64 param1, void* param2) :
    WorkItem(dptfManager, FrameworkEvent::PolicyInitiatedCallback),
    m_policyIndex(policyIndex), m_policyDefinedEventCode(policyDefinedEventCode),
    m_param1(param1), m_param2(param2)
{
}

WIPolicyInitiatedCallback::~WIPolicyInitiatedCallback(void)
{
}

void WIPolicyInitiatedCallback::execute(void)
{
    WriteWorkItemStartingDebugMessage();

    try
    {
        Policy* policy = getPolicyManager()->getPolicyPtr(m_policyIndex);
        policy->executePolicyInitiatedCallback(m_policyDefinedEventCode, m_param1, m_param2);
    }
    catch (std::exception ex)
    {
        WriteWorkItemErrorMessage_Function_Policy("Policy::executePolicyInitiatedCallback", m_policyIndex);
    }
}

Bool WIPolicyInitiatedCallback::matches(const WorkItemMatchCriteria& matchCriteria) const
{
    return matchCriteria.testAgainstMatchList(getFrameworkEventType(), getUniqueId(),
        Constants::Invalid, Constants::Invalid, m_policyIndex);
}