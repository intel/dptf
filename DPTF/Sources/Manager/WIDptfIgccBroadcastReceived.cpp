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

#include "WIDptfIgccBroadcastReceived.h"
#include "PolicyManagerInterface.h"
#include "SystemModeManager.h"
#include "StatusFormat.h"
#include "EventPayloadEnduranceGamingRequest.h"

WIDptfIgccBroadcastReceived::WIDptfIgccBroadcastReceived(
	DptfManagerInterface* dptfManager,
	IgccBroadcastData::IgccToDttNotificationPackage igccNotificationData)
	: WorkItem(dptfManager, FrameworkEvent::DptfAppBroadcastUnprivileged)
	, m_igccNotificationData(igccNotificationData)
{
}

void WIDptfIgccBroadcastReceived::onExecute()
{
	writeWorkItemStartingInfoMessage();
	const Guid broadcastGuid(m_igccNotificationData.uuid);
	
	if (IGCC_BROADCAST_GUID == broadcastGuid)
	{
		getDptfManager()->getEventCache()->igccAppBroadcastNotificationData.set(m_igccNotificationData);
		getDptfManager()->getEventNotifier()->notify(
			getFrameworkEventType(), 
			EventPayloadEnduranceGamingRequest(m_igccNotificationData.enduranceGamingStatus));

		const auto policyManager = getPolicyManager();
		const auto policyIndexes = policyManager->getPolicyIndexes();
		for (const unsigned int policyId : policyIndexes)
		{
			try
			{

				const auto policy = policyManager->getPolicyPtr(policyId);
				policy->executePolicyIgccBroadcastReceived(m_igccNotificationData);
			}
			catch (policy_index_invalid&)
			{
				// do nothing.  No item in the policy list at this index.
			}
			catch (std::exception& ex)
			{
				writeWorkItemErrorMessagePolicy(
					ex, "Policy::executePolicyIgccBroadcastReceived", policyId);
			}
		}
	}
}


