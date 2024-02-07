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

#include "WIPolicyOperatingSystemPowerSchemePersonalityChanged.h"
#include "PolicyManagerInterface.h"
#include "SystemModeManager.h"
#include "EsifServicesInterface.h"

WIPolicyOperatingSystemPowerSchemePersonalityChanged::WIPolicyOperatingSystemPowerSchemePersonalityChanged(
	DptfManagerInterface* dptfManager,
	OsPowerSchemePersonality::Type powerSchemePersonality)
	: WorkItem(dptfManager, FrameworkEvent::PolicyOperatingSystemPowerSchemePersonalityChanged)
	, m_powerSchemePersonality(powerSchemePersonality)
{
}

WIPolicyOperatingSystemPowerSchemePersonalityChanged::~WIPolicyOperatingSystemPowerSchemePersonalityChanged(void)
{
}

void WIPolicyOperatingSystemPowerSchemePersonalityChanged::onExecute(void)
{
	writeWorkItemStartingInfoMessage();

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	getDptfManager()->getEventCache()->powerSchemePersonality.set(m_powerSchemePersonality);

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			auto policy = policyManager->getPolicyPtr(*i);
			policy->executePolicyOperatingSystemPowerSchemePersonalityChanged(m_powerSchemePersonality);
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(
				ex, "Policy::executePolicyOperatingSystemPowerSchemePersonalityChanged", *i);
		}
	}

	try
	{
		auto systemModeManager = getDptfManager()->getSystemModeManager();
		if (systemModeManager != nullptr)
		{
			systemModeManager->executeOperatingSystemPowerSchemePersonalityChanged();
		}
	}
	catch (std::exception& ex)
	{
		writeWorkItemErrorMessage(ex, "SystemModeManager::executeOperatingSystemPowerSchemePersonalityChanged");
	}
}
