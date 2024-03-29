/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "WIPolicyProcessLoaded.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"
#include "EsifDataString.h"
#include "StringParser.h"

WIPolicyProcessLoaded::WIPolicyProcessLoaded(
	DptfManagerInterface* dptfManager,
	esif_data_process_notification* processNotification)
	: WorkItem(dptfManager, FrameworkEvent::PolicyProcessLoadNotification)
{
	std::string processNamePath = EsifDataString(processNotification->image_path);
	std::vector<std::string> keyComponents = StringParser::split(processNamePath, '\\');
	std::string processName = keyComponents[keyComponents.size() - 1];

	m_processName = processName;
}

WIPolicyProcessLoaded::~WIPolicyProcessLoaded(void)
{
}

void WIPolicyProcessLoaded::onExecute(void)
{
	writeWorkItemStartingInfoMessage();

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			auto policy = policyManager->getPolicyPtr(*i);
			policy->executePolicyProcessLoaded(m_processName);
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executePolicyProcessLoaded", *i);
		}
	}
}
