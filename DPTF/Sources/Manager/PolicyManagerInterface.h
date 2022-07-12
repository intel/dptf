/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "SupportedDynamicPolicyList.h"

class dptf_export PolicyManagerInterface
{
public:
	virtual ~PolicyManagerInterface(){};

	virtual void createAllPolicies(const std::string& dptfHomeDirectoryPath) = 0;
	virtual UIntN createPolicy(const std::string& policyFileName) = 0;
	virtual UIntN createDynamicPolicy(
		const std::string& policyFileName,
		Guid dynamicPolicyUuid,
		Guid dynamicPolicyTemplateGuid,
		const std::string& dynamicPolicyName,
		const std::string& dynamicPolicyUuidString) = 0;

	virtual void destroyAllPolicies(void) = 0;
	virtual void destroyPolicy(UIntN policyIndex) = 0;

	virtual std::set<UIntN> getPolicyIndexes(void) const = 0;
	virtual std::shared_ptr<ISupportedPolicyList> getSupportedPolicyList(void) const = 0;
	virtual std::shared_ptr<ISupportedDynamicPolicyList> getSupportedDynamicPolicyList(void) const = 0;
	virtual IPolicy* getPolicyPtr(UIntN policyIndex) = 0;
	virtual std::shared_ptr<IPolicy> getPolicy(const std::string& policyName) const = 0;
	virtual Bool policyExists(const std::string& policyName) const = 0;
	virtual Bool IsDynamicPolicyTemplateFileName(const std::string& policyName) const = 0;

	virtual void registerEvent(UIntN policyIndex, PolicyEvent::Type policyEvent) = 0;
	virtual void unregisterEvent(UIntN policyIndex, PolicyEvent::Type policyEvent) = 0;

	virtual std::shared_ptr<XmlNode> getStatusAsXml(void) = 0;
	virtual std::string getDiagnosticsAsXml(void) = 0;
};
