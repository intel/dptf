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

#include "WIPolicyOperatingSystemPowerSourceChanged.h"
#include "PolicyManagerInterface.h"
#include "SystemModeManager.h"
#include "EventPayloadOperatingSystemPowerSourceType.h"

WIPolicyOperatingSystemPowerSourceChanged::WIPolicyOperatingSystemPowerSourceChanged(
	DptfManagerInterface* dptfManager,
	OsPowerSource::Type powerSource)
	: WorkItem(dptfManager, FrameworkEvent::PolicyOperatingSystemPowerSourceChanged)
	, m_powerSource(powerSource)
{
}

void WIPolicyOperatingSystemPowerSourceChanged::onExecute()
{
	writeWorkItemStartingInfoMessage();

	const auto policyManager = getPolicyManager();
	const auto policyIndexes = policyManager->getPolicyIndexes();

	getDptfManager()->getEventCache()->powerSource.set(m_powerSource);
	getDptfManager()->getEventNotifier()->notify(
		getFrameworkEventType(), EventPayloadOperatingSystemPowerSourceType(m_powerSource));

	for (const auto policyId : policyIndexes)
	{
		try
		{
			const auto policy = policyManager->getPolicyPtr(policyId);
			policy->executePolicyOperatingSystemPowerSourceChanged(m_powerSource);
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executePolicyOperatingSystemPowerSourceChanged", policyId);
		}
	}

	try
	{
		const auto systemModeManager = getDptfManager()->getSystemModeManager();
		if (systemModeManager != nullptr)
		{
			systemModeManager->executeOperatingSystemPowerSourceChanged(m_powerSource);
		}
	}
	catch (std::exception& ex)
	{
		writeWorkItemErrorMessage(ex, "SystemModeManager::executeOperatingSystemPowerSourceChanged");
	}
}
