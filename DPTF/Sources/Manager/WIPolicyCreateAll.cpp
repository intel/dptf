/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#include "PolicyManager.h"
#include "EsifServices.h"
#include "EsifFileEnumerator.h"

WIPolicyCreateAll::WIPolicyCreateAll(DptfManagerInterface* dptfManager, const std::string dptfPolicyDirectoryPath) :
WorkItem(dptfManager, FrameworkEvent::PolicyCreate), 
m_policyDirectoryPath(dptfPolicyDirectoryPath)
{
}

WIPolicyCreateAll::~WIPolicyCreateAll(void)
{
}

void WIPolicyCreateAll::execute(void)
{
    WriteWorkItemStartingInfoMessage();

    try
    {
        EsifFileEnumerator fileEnumerator(m_policyDirectoryPath, "DptfPolicy*" ESIF_LIB_EXT);
        std::string policyFileName = fileEnumerator.getFirstFile();

        while (policyFileName.length() > 0)
        {
            try
            {
                std::string policyFilePath = m_policyDirectoryPath + policyFileName;
                if (getDptfManager()->isDptfPolicyLoadNameOnly())
                {
                    policyFilePath.erase(0, m_policyDirectoryPath.length());
                }

                getDptfManager()->getPolicyManager()->createPolicy(policyFilePath);
            }
            catch (std::exception ex)
            {
                WriteWorkItemErrorMessage_Function_MessageKey_MessageValue("PolicyManager::createPolicy",
                    "Policy File Name", policyFileName);
            }

            policyFileName = fileEnumerator.getNextFile();
        }
    }
    catch (std::exception ex)
    {
        WriteWorkItemErrorMessage_Function("PolicyManager::createAllPolicies");
    }
}