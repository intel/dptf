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

#include "Dptf.h"
#include "Policy.h"
#include "SupportedPolicyList.h"

class dptf_export PolicyManagerInterface
{
public:
	virtual ~PolicyManagerInterface(){};

	virtual void createAllPolicies(const std::string& dptfHomeDirectoryPath) = 0;
	virtual void createPolicy(const std::string& policyFileName) = 0;

	virtual void reloadAllPolicies(const std::string& dptfHomeDirectoryPath) = 0;

	virtual void destroyAllPolicies(void) = 0;
	virtual void destroyPolicy(UIntN policyIndex) = 0;

	virtual UIntN getPolicyListCount(void) const = 0;
	virtual Policy* getPolicyPtr(UIntN policyIndex) = 0;

	virtual void registerEvent(UIntN policyIndex, PolicyEvent::Type policyEvent) = 0;
	virtual void unregisterEvent(UIntN policyIndex, PolicyEvent::Type policyEvent) = 0;

	virtual std::shared_ptr<XmlNode> getStatusAsXml(void) = 0;
};
