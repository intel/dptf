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

#include "WIPolicyDttGamingModeChanged.h"
#include "EsifServicesInterface.h"
#include "PolicyManagerInterface.h"

WIPolicyDttGamingModeChanged::WIPolicyDttGamingModeChanged(DptfManagerInterface* dptfManager, DttGamingMode::Type gamingMode)
	: WorkItem(dptfManager, FrameworkEvent::PolicyDttGamingModeChanged)
	, m_gamingMode(gamingMode)
{
}

void WIPolicyDttGamingModeChanged::onExecute()
{
	const auto policyManager = getPolicyManager();
	const auto policyIndexes = policyManager->getPolicyIndexes();

	getDptfManager()->getEventCache()->dttGamingMode.set(m_gamingMode);

	for (const unsigned int policyIndex : policyIndexes)
	{
		try
		{
			const auto policy = policyManager->getPolicyPtr(policyIndex);
			policy->executePolicyDttGamingModeChanged(m_gamingMode);
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executePolicyGamingModeChanged", policyIndex);
		}
	}
}