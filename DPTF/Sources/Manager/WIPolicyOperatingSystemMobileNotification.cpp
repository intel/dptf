/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "WIPolicyOperatingSystemMobileNotification.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"
#include "OsMobileNotificationType.h"

WIPolicyOperatingSystemMobileNotification::WIPolicyOperatingSystemMobileNotification(
	DptfManagerInterface* dptfManager,
	UIntN mobileNotification)
	: WorkItem(dptfManager, FrameworkEvent::PolicyOperatingSystemMobileNotification)
	, m_mobileNotification(mobileNotification)
{
}

WIPolicyOperatingSystemMobileNotification::~WIPolicyOperatingSystemMobileNotification(void)
{
}

void WIPolicyOperatingSystemMobileNotification::onExecute(void)
{
	writeWorkItemStartingInfoMessage();

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		std::string functionName = "";
		try
		{
			auto policy = policyManager->getPolicyPtr(*i);

			OsMobileNotificationType::Type notificationType =
				(OsMobileNotificationType::Type)(((UInt32)m_mobileNotification & 0xFFFF0000) >> 16);
			UInt32 notificationValue = (UInt32)m_mobileNotification & 0xFFFF;

			switch (notificationType)
			{
			case OsMobileNotificationType::EmergencyCallMode:
				getDptfManager()->getEventCache()->emergencyCallModeState.set(notificationValue);
				functionName = "Policy::executePolicyOperatingSystemEmergencyCallModeStateChanged";
				policy->executePolicyOperatingSystemEmergencyCallModeStateChanged((OnOffToggle::Type)notificationValue);
				break;

			case OsMobileNotificationType::ScreenState:
			{
				OnOffToggle::Type screenState = OnOffToggle::toType(notificationValue);
				getDptfManager()->getEventCache()->screenState.set(screenState);
				functionName = "Policy::executePolicyOperatingSystemScreenStateChanged";
				policy->executePolicyOperatingSystemScreenStateChanged(screenState);
			}
				break;

			default:
				functionName = "Policy::executePolicyOperatingSystemMobileNotification";
				policy->executePolicyOperatingSystemMobileNotification(notificationType, notificationValue);
				break;
			}
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, functionName, *i);
		}
	}
}
