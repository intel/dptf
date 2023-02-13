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

#include "WIPolicyCreateAll.h"
#include "DptfManager.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"
#include "EsifFileEnumerator.h"

WIPolicyCreateAll::WIPolicyCreateAll(DptfManagerInterface* dptfManager, const std::string dptfPolicyDirectoryPath)
	: WorkItem(dptfManager, FrameworkEvent::PolicyCreate)
	, m_policyDirectoryPath(dptfPolicyDirectoryPath)
{
}

WIPolicyCreateAll::~WIPolicyCreateAll(void)
{
}

void WIPolicyCreateAll::onExecute(void)
{
	writeWorkItemStartingInfoMessage();

	try
	{
		EsifFileEnumerator fileEnumerator(m_policyDirectoryPath, "DptfPolicy*" ESIF_LIB_EXT);
		std::string policyFileName = fileEnumerator.getFirstFile();

		while (policyFileName.length() > 0)
		{
			std::string policyFilePath = m_policyDirectoryPath + policyFileName;
			try
			{
				getDptfManager()->getPolicyManager()->createPolicy(policyFilePath);
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

			try
			{
				if (getDptfManager()->getPolicyManager()->IsDynamicPolicyTemplateFileName(policyFileName))
				{
					auto supportedDynamicPolicyList =
						getDptfManager()->getPolicyManager()->getSupportedDynamicPolicyList();
					for (UIntN i = 0; i < supportedDynamicPolicyList->getCount(); i++)
					{
						auto dynamicPolicyUuid = supportedDynamicPolicyList->get(i).getUuid();
						auto supportedPolicyList = getDptfManager()->getPolicyManager()->getSupportedPolicyList();

						if (supportedPolicyList->isPolicySupported(dynamicPolicyUuid))
						{
							auto dynamicPolicyTemplateGuid = supportedDynamicPolicyList->get(i).getTemplateGuid();
							auto dynamicPolicyName = supportedDynamicPolicyList->get(i).getName();
							auto dynamicPolicyUuidString = supportedDynamicPolicyList->get(i).getUuidString();

							getDptfManager()->getPolicyManager()->createDynamicPolicy(
								policyFilePath,
								dynamicPolicyUuid,
								dynamicPolicyTemplateGuid,
								dynamicPolicyName,
								dynamicPolicyUuidString);
						}
					}
				}
			}
			catch (std::exception& ex)
			{
				writeWorkItemWarningMessage(
					ex, "PolicyManager::createDynamicPolicy", "Policy File Name", policyFileName);
			}
			catch (...)
			{
				dptf_exception ex("Unknown exception type caught when attempting to create a dynamic policy.");
				writeWorkItemWarningMessage(
					ex, "PolicyManager::createDynamicPolicy", "Policy File Name", policyFileName);
			}

			policyFileName = fileEnumerator.getNextFile();
		}
	}
	catch (std::exception& ex)
	{
		writeWorkItemErrorMessage(ex, "PolicyManager::createAllPolicies");
	}
	catch (...)
	{
		dptf_exception ex("Unknown exception type caught when attempting to create all policies.");
		writeWorkItemErrorMessage(ex, "PolicyManager::createAllPolicies");
	}
}
