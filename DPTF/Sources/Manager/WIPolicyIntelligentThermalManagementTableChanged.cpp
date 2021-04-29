/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "WIPolicyIntelligentThermalManagementTableChanged.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"

WIPolicyIntelligentThermalManagementTableChanged::WIPolicyIntelligentThermalManagementTableChanged(
	DptfManagerInterface* dptfManager)
	: WorkItem(dptfManager, FrameworkEvent::PolicyIntelligentThermalManagementTableChanged)
{
}

WIPolicyIntelligentThermalManagementTableChanged::~WIPolicyIntelligentThermalManagementTableChanged(void)
{
}

void WIPolicyIntelligentThermalManagementTableChanged::onExecute(void)
{
	writeWorkItemStartingInfoMessage();

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			auto policy = policyManager->getPolicyPtr(*i);
			policy->executePolicyIntelligentThermalManagementTableChanged();
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executePolicyIntelligentThermalManagementTableChanged", *i);
		}
	}
}
