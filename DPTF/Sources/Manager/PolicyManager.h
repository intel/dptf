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

#pragma once

#include "Dptf.h"
#include "Policy.h"
#include "SupportedPolicyList.h"

class PolicyManager
{
public:

    PolicyManager(DptfManager* dptfManager);
    ~PolicyManager(void);

    // Create policies
    void createAllPolicies(const std::string& dptfHomeDirectoryPath);
    void createPolicy(const std::string& policyFileName);

    // Destroy policies
    void destroyAllPolicies(void);
    void destroyPolicy(UIntN policyIndex);

    // Allows the work items to iterate through the list of policies.
    UIntN getPolicyListCount(void) const;
    Policy* getPolicyPtr(UIntN policyIndex);

    // Policy manager handles registering and unregistering events since more than one policy can
    // register or unregister, and we don't want one policy to unregister the event while another
    // still needs it.
    void registerEvent(UIntN policyIndex, PolicyEvent::Type policyEvent);
    void unregisterEvent(UIntN policyIndex, PolicyEvent::Type policyEvent);

    std::string GetStatusAsXml(void);

private:

    // hide the copy constructor and assignment operator.
    PolicyManager(const PolicyManager& rhs);
    PolicyManager& operator=(const PolicyManager& rhs);

    DptfManager* m_dptfManager;
    std::vector<Policy*> m_policy;
    SupportedPolicyList m_supportedPolicyList;

    // tracks the overall events registered by one or more policies
    std::bitset<PolicyEvent::Max> m_registeredEvents;

    Bool isAnyPolicyRegisteredForEvent(PolicyEvent::Type policyEvent);
    UIntN getPolicyCount(void);
};