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

#include "PolicyServicesPolicyInitiatedCallback.h"
#include "WIPolicyInitiatedCallback.h"
#include "WorkItemQueueManager.h"
#include "DptfManager.h"

PolicyServicesPolicyInitiatedCallback::PolicyServicesPolicyInitiatedCallback(
    DptfManager* dptfManager, UIntN policyIndex) : PolicyServices(dptfManager, policyIndex)
{
}

UInt64 PolicyServicesPolicyInitiatedCallback::createPolicyInitiatedImmediateCallback(
    UInt64 policyDefinedEventCode, UInt64 param1, void* param2)
{
    // This can be called from any thread
    WorkItem* workItem = new WIPolicyInitiatedCallback(getDptfManager(), getPolicyIndex(), policyDefinedEventCode, param1, param2);
    UInt64 workItemUniqueId = workItem->getUniqueId();
    getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(workItem);

    return workItemUniqueId;
}

UInt64 PolicyServicesPolicyInitiatedCallback::createPolicyInitiatedDeferredCallback(
    UInt64 policyDefinedEventCode, UInt64 param1, void* param2, UInt64 timeDeltaInMilliSeconds)
{
    // This can be called from any thread
    WorkItem* workItem = new WIPolicyInitiatedCallback(getDptfManager(), getPolicyIndex(), policyDefinedEventCode, param1, param2);
    UInt64 workItemUniqueId = workItem->getUniqueId();
    getWorkItemQueueManager()->enqueueDeferredWorkItem(workItem, timeDeltaInMilliSeconds);

    return workItemUniqueId;
}

Bool PolicyServicesPolicyInitiatedCallback::removePolicyInitiatedCallback(UInt64 callbackHandle)
{
    // This can be called from any thread
    WorkItemMatchCriteria matchCriteria;
    matchCriteria.addPolicyIndexToMatchList(getPolicyIndex());
    matchCriteria.addUniqueIdToMatchList(callbackHandle);

    UIntN numRemoved = getWorkItemQueueManager()->removeIfMatches(matchCriteria);

    return (numRemoved > 0);
}