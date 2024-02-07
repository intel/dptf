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

#include "WIDptfApplicationOptimizerBroadcastReceived.h"
#include "PolicyManagerInterface.h"
#include "SystemModeManager.h"
#include "EsifServicesInterface.h"
#include "StatusFormat.h"

WIDptfApplicationOptimizerBroadcastReceived::WIDptfApplicationOptimizerBroadcastReceived(
	DptfManagerInterface* dptfManager,
	const DptfBuffer& iaoNotificationData)
	: WorkItem(dptfManager, FrameworkEvent::DptfAppBroadcastUnprivileged)
	, m_iaoNotificationData(iaoNotificationData)
{
}

WIDptfApplicationOptimizerBroadcastReceived::~WIDptfApplicationOptimizerBroadcastReceived(void)
{
}

void WIDptfApplicationOptimizerBroadcastReceived::onExecute(void)
{
	writeWorkItemStartingInfoMessage();
	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (const auto index : policyIndexes)
	{
		try
		{
			auto policy = policyManager->getPolicyPtr(index);
			policy->executeIaoBroadcastReceived(m_iaoNotificationData);
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executeApplicationOptimizerBroadcastReceived", index);
		}
	}
}