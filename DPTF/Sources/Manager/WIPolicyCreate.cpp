/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#include "WIPolicyCreate.h"
#include "DptfManager.h"
#include "PolicyManager.h"
#include "EsifServices.h"

WIPolicyCreate::WIPolicyCreate(DptfManager* dptfManager, const std::string& policyFileName) :
    WorkItem(dptfManager, FrameworkEvent::PolicyCreate), m_policyFileName(policyFileName)
{
}

WIPolicyCreate::~WIPolicyCreate(void)
{
}

void WIPolicyCreate::execute(void)
{
    WriteWorkItemStartingInfoMessage();

    try
    {
        getDptfManager()->getPolicyManager()->createPolicy(m_policyFileName);
    }
    catch (std::exception ex)
    {
        WriteWorkItemErrorMessage_Function_MessageKey_MessageValue("PolicyManager::createPolicy",
            "Policy File Name", m_policyFileName);
    }
}