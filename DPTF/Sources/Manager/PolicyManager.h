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

#pragma once

#include "PolicyManagerInterface.h"

class dptf_export PolicyManager : public PolicyManagerInterface
{
public:
	PolicyManager(DptfManagerInterface* dptfManager, const std::set<Guid>& defaultPolicies);
	~PolicyManager() override;

	// Create policies
	void createAllPolicies(const std::string& dptfPolicyDirectoryPath) override;
	UIntN createPolicy(const std::string& policyFileName) override;
	UIntN createDynamicPolicy(
		const std::string& policyFileName,
		Guid dynamicPolicyUuid,
		Guid dynamicPolicyTemplateGuid,
		const std::string& dynamicPolicyName,
		const std::string& dynamicPolicyUuidString) override;

	// Destroy policies
	void destroyAllPolicies() override;
	void destroyPolicy(UIntN policyIndex) override;
	void reloadPolicy(const std::string& policyName) override;

	// Allows the work items to iterate through the list of policies.
	[[nodiscard]] std::set<UIntN> getPolicyIndexes() const override;
	[[nodiscard]] std::shared_ptr<ISupportedPolicyList> getSupportedPolicyList() const override;
	[[nodiscard]] std::shared_ptr<ISupportedDynamicPolicyList> getSupportedDynamicPolicyList() const override;
	IPolicy* getPolicyPtr(UIntN policyIndex) override;
	[[nodiscard]] std::shared_ptr<IPolicy> getPolicy(const std::string& policyName) const override;
	[[nodiscard]] Bool policyExists(const std::string& policyName) const override;
	[[nodiscard]] Bool IsDynamicPolicyTemplateFileName(const std::string& policyName) const override;
	// Policy manager handles registering and unregistering events since more than one policy can
	// register or unregister, and we don't want one policy to unregister the event while another
	// still needs it.
	void registerEvent(UIntN policyIndex, PolicyEvent::Type policyEvent) override;
	void unregisterEvent(UIntN policyIndex, PolicyEvent::Type policyEvent) override;

	std::shared_ptr<XmlNode> getStatusAsXml() override;
	std::string getDiagnosticsAsXml() override;

private:
	// hide the copy constructor and assignment operator.
	PolicyManager(const PolicyManager& rhs);
	PolicyManager& operator=(const PolicyManager& rhs);

	DptfManagerInterface* m_dptfManager;
	std::map<UIntN, std::shared_ptr<IPolicy>> m_policies;
	std::shared_ptr<SupportedPolicyList> m_supportedPolicyList;
	std::shared_ptr<SupportedDynamicPolicyList> m_supportedDynamicPolicyList;

	// tracks the overall events registered by one or more policies
	std::bitset<PolicyEvent::Max> m_registeredEvents;

	void throwIfPolicyAlreadyExists(const std::string& policyFileName);
	void throwIfDynamicPolicyAlreadyExists(const std::string& policyFileName, const std::string& policyName);
	[[nodiscard]] Bool isAnyPolicyRegisteredForEvent(PolicyEvent::Type policyEvent) const;
	[[nodiscard]] UIntN getPolicyCount(void) const;
	std::shared_ptr<XmlNode> getEventsXmlForPolicy(UIntN policyIndex);
	static std::shared_ptr<XmlNode> getEventsInXml();
	[[nodiscard]] EsifServicesInterface* getEsifServices() const;
};
