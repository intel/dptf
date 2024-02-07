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

#include "WIDptfExtendedWorkloadPredictionEventRegistrationChanged.h"
#include "EsifServicesInterface.h"
#include "PolicyManagerInterface.h"

WIDptfExtendedWorkloadPredictionEventRegistrationChanged::WIDptfExtendedWorkloadPredictionEventRegistrationChanged(
	DptfManagerInterface* dptfManager,
	UInt32 consumerCount)
	: WorkItem(dptfManager, FrameworkEvent::DptfExtendedWorkloadPredictionEventRegistrationChanged)
	, m_consumerCount(consumerCount)
{
}

void WIDptfExtendedWorkloadPredictionEventRegistrationChanged::onExecute()
{
	writeWorkItemStartingInfoMessage();

	const auto policyManager = getPolicyManager();
	const auto policyIndexes = policyManager->getPolicyIndexes();
	for (const auto policyIndex : policyIndexes)
	{
		try
		{
			const auto policy = policyManager->getPolicyPtr(policyIndex);
			policy->executeDptfExtendedWorkloadPredictionEventRegistrationChanged(m_consumerCount);
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executeDptfExtendedWorkloadPredictionEventRegistrationChanged", policyIndex);
		}
	}
}
