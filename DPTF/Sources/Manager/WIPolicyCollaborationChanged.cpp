/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "WIPolicyCollaborationChanged.h"
#include "PolicyManagerInterface.h"
#include "EventPayloadOnAndOffToggle.h"

WIPolicyCollaborationChanged::WIPolicyCollaborationChanged(
	DptfManagerInterface* dptfManager,
	OnOffToggle::Type state)
	: WorkItem(dptfManager, FrameworkEvent::PolicyCollaborationChanged)
	, m_collaborationState(state)
{
}

void WIPolicyCollaborationChanged::onExecute()
{
	writeWorkItemStartingInfoMessage();
	getDptfManager()->getEventNotifier()->notify(
		getFrameworkEventType(), EventPayloadOnAndOffToggle(m_collaborationState));

	const auto policyManager = getPolicyManager();
	const auto policyIndexes = policyManager->getPolicyIndexes();
	for (const auto policyId : policyIndexes)
	{
		try
		{
			getDptfManager()->getEventCache()->collaborationMode.set(m_collaborationState);
			const auto policy = policyManager->getPolicyPtr(policyId);
			policy->executePolicyCollaborationChanged(m_collaborationState);
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executePolicyCollaborationChanged", policyId);
		}
	}
}
