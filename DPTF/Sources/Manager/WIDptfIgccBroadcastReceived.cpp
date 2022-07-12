/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "WIDptfIgccBroadcastReceived.h"
#include "PolicyManagerInterface.h"
#include "SystemModeManager.h"
#include "EsifServicesInterface.h"
#include "StatusFormat.h"

WIDptfIgccBroadcastReceived::WIDptfIgccBroadcastReceived(
	DptfManagerInterface* dptfManager,
	IgccBroadcastData::IgccToDttNotificationPackage igccNotificationData)
	: WorkItem(dptfManager, FrameworkEvent::PolicyAppBroadcastUnprivileged)
	, m_igccNotificationData(igccNotificationData)
{
}

WIDptfIgccBroadcastReceived::~WIDptfIgccBroadcastReceived(void)
{
}

void WIDptfIgccBroadcastReceived::onExecute(void)
{
	writeWorkItemStartingInfoMessage();
	Guid broadcastGuid(m_igccNotificationData.uuid);
	if (IGCC_BROADCAST_GUID == broadcastGuid)
	{
		auto policyManager = getPolicyManager();
		auto policyIndexes = policyManager->getPolicyIndexes();

		for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
		{
			try
			{
				getDptfManager()->getEventCache()->appBroadcastNotificationData.set(m_igccNotificationData);
				auto policy = policyManager->getPolicyPtr(*i);
				policy->executeIgccBroadcastReceived(m_igccNotificationData);
			}
			catch (policy_index_invalid&)
			{
				// do nothing.  No item in the policy list at this index.
			}
			catch (std::exception& ex)
			{
				writeWorkItemErrorMessagePolicy(ex, "Policy::executeIgccBroadcastReceived", *i);
			}
		}
	}
}
