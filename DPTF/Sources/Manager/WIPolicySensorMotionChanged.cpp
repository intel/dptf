/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "WIPolicySensorMotionChanged.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"

WIPolicySensorMotionChanged::WIPolicySensorMotionChanged(
	DptfManagerInterface* dptfManager,
	OnOffToggle::Type sensorMotion)
	: WorkItem(dptfManager, FrameworkEvent::PolicySensorMotionChanged)
	, m_sensorMotion(sensorMotion)
{
}

WIPolicySensorMotionChanged::~WIPolicySensorMotionChanged(void)
{
}

void WIPolicySensorMotionChanged::onExecute(void)
{
	writeWorkItemStartingInfoMessage();

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			getDptfManager()->getEventCache()->sensorMotion.set(m_sensorMotion);
			auto policy = policyManager->getPolicyPtr(*i);
			policy->executePolicySensorMotionChanged(m_sensorMotion);
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executePolicySensorMotionChanged", *i);
		}
	}
}
