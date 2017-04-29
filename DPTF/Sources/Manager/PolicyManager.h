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

#pragma once

#include "PolicyManagerInterface.h"

class dptf_export PolicyManager : public PolicyManagerInterface
{
public:
	PolicyManager(DptfManagerInterface* dptfManager);
	~PolicyManager(void);

	// Create policies
	virtual void createAllPolicies(const std::string& dptfHomeDirectoryPath) override;
	virtual void createPolicy(const std::string& policyFileName) override;

	// ReCreate Policies
	virtual void reloadAllPolicies(const std::string& dptfHomeDirectoryPath) override;

	// Destroy policies
	virtual void destroyAllPolicies(void) override;
	virtual void destroyPolicy(UIntN policyIndex) override;

	// Allows the work items to iterate through the list of policies.
	virtual UIntN getPolicyListCount(void) const override;
	virtual Policy* getPolicyPtr(UIntN policyIndex) override;

	// Policy manager handles registering and unregistering events since more than one policy can
	// register or unregister, and we don't want one policy to unregister the event while another
	// still needs it.
	virtual void registerEvent(UIntN policyIndex, PolicyEvent::Type policyEvent) override;
	virtual void unregisterEvent(UIntN policyIndex, PolicyEvent::Type policyEvent) override;

	virtual std::shared_ptr<XmlNode> getStatusAsXml(void) override;

private:
	// hide the copy constructor and assignment operator.
	PolicyManager(const PolicyManager& rhs);
	PolicyManager& operator=(const PolicyManager& rhs);

	DptfManagerInterface* m_dptfManager;
	std::map<UIntN, std::shared_ptr<Policy>> m_policies;
	SupportedPolicyList m_supportedPolicyList;

	// tracks the overall events registered by one or more policies
	std::bitset<PolicyEvent::Max> m_registeredEvents;

	Bool isAnyPolicyRegisteredForEvent(PolicyEvent::Type policyEvent);
	UIntN getPolicyCount(void);
	std::shared_ptr<XmlNode> getEventsXmlForPolicy(UIntN policyIndex);
	std::shared_ptr<XmlNode> getEventsInXml();
};
