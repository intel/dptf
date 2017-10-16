/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

WIPolicySupportedListChanged::WIPolicySupportedListChanged(DptfManagerInterface* dptfManager)
	: WorkItem(dptfManager, FrameworkEvent::DptfSupportedPoliciesChanged)
{
}

WIPolicySupportedListChanged::~WIPolicySupportedListChanged(void)
{
}

void WIPolicySupportedListChanged::execute(void)
{
	writeWorkItemStartingInfoMessage();

	try
	{
		auto supportedPolicyList = getDptfManager()->getPolicyManager()->getSupportedPolicyList();
		supportedPolicyList->update();
		auto policyIndexes = getDptfManager()->getPolicyManager()->getPolicyIndexes();
		for (auto policyIndex = policyIndexes.begin(); policyIndex != policyIndexes.end(); ++policyIndex)
		{
			auto policyGuid = getDptfManager()->getPolicyManager()->getPolicyPtr(*policyIndex)->getGuid();
			if (!supportedPolicyList->isPolicySupported(policyGuid))
			{
				getDptfManager()->getPolicyManager()->destroyPolicy(*policyIndex);
			}
		}

		auto policyDirectoryPath = getDptfManager()->getDptfPolicyDirectoryPath();
		EsifFileEnumerator fileEnumerator(policyDirectoryPath, "DptfPolicy*" ESIF_LIB_EXT);
		std::string policyFileName = fileEnumerator.getFirstFile();

		while (policyFileName.length() > 0)
		{
			try
			{
				std::string policyFilePath = policyDirectoryPath + policyFileName;
				if (getDptfManager()->isDptfPolicyLoadNameOnly())
				{
					policyFilePath.erase(0, policyDirectoryPath.length());
				}

				UIntN policyIndex = getDptfManager()->getPolicyManager()->createPolicy(policyFilePath);

				// Bind every participant and domain to created policy
				getDptfManager()->bindAllParticipantsToPolicy(policyIndex);
			}
			catch (policy_already_exists)
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
