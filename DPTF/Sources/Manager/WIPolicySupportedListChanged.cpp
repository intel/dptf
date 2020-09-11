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

#include "WIPolicySupportedListChanged.h"
#include "ParticipantManagerInterface.h"
#include "Participant.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"
#include "EsifFileEnumerator.h"
#include "WIPolicyDestroy.h"

WIPolicySupportedListChanged::WIPolicySupportedListChanged(DptfManagerInterface* dptfManager)
	: WorkItem(dptfManager, FrameworkEvent::DptfSupportedPoliciesChanged)
{
}

WIPolicySupportedListChanged::~WIPolicySupportedListChanged(void)
{
}

void WIPolicySupportedListChanged::onExecute(void)
{
	writeWorkItemStartingInfoMessage();
	auto dptfManager = getDptfManager();
	auto policyManager = dptfManager->getPolicyManager();

	try
	{
		auto supportedPolicyList = policyManager->getSupportedPolicyList();
		supportedPolicyList->update();
		auto policyIndexes = policyManager->getPolicyIndexes();
		for (auto policyIndex = policyIndexes.begin(); policyIndex != policyIndexes.end(); ++policyIndex)
		{
			auto policyGuid = policyManager->getPolicyPtr(*policyIndex)->getGuid();
			if (!supportedPolicyList->isPolicySupported(policyGuid))
			{
				auto policyDestroyWorkItem = std::make_shared<WIPolicyDestroy>(dptfManager, *policyIndex);
				policyDestroyWorkItem->onExecute();
			}
		}

		auto policyDirectoryPath = dptfManager->getDptfPolicyDirectoryPath();
		EsifFileEnumerator fileEnumerator(policyDirectoryPath, "DptfPolicy*" ESIF_LIB_EXT);
		std::string policyFileName = fileEnumerator.getFirstFile();

		while (policyFileName.length() > 0)
		{
			try
			{
				std::string policyFilePath = policyDirectoryPath + policyFileName;
				if (dptfManager->isDptfPolicyLoadNameOnly())
				{
					policyFilePath.erase(0, policyDirectoryPath.length());
				}

				UIntN policyIndex = policyManager->createPolicy(policyFilePath);

				// Bind every participant and domain to created policy
				dptfManager->bindAllParticipantsToPolicy(policyIndex);
			}
			catch (policy_already_exists&)
			{
				// Ignore duplicate policy instance
			}
			catch (std::exception& ex)
			{
				writeWorkItemWarningMessage(ex, "PolicyManager::createPolicy", "Policy File Name", policyFileName);
			}
			catch (...)
			{
				dptf_exception ex("Unknown exception type caught when attempting to create a policy.");
				writeWorkItemWarningMessage(ex, "PolicyManager::createPolicy", "Policy File Name", policyFileName);
			}

			policyFileName = fileEnumerator.getNextFile();
		}
	}
	catch (std::exception& ex)
	{
		writeWorkItemErrorMessage(ex, "PolicySupportedListChanged");
	}
	catch (...)
	{
		dptf_exception ex("Unknown exception type caught when attempting to update supported policy list.");
		writeWorkItemErrorMessage(ex, "PolicySupportedListChanged");
	}
}
