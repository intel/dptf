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

#include "Dptf.h"
#include "DptfManager.h"

class dptf_export ISupportedPolicyList
{
public:
	
	virtual Bool isPolicySupported(const Guid& guid) const = 0;
	virtual void update(void) = 0;
	virtual std::set<Guid> getGuids() const = 0;
	virtual UInt64 getCount() const = 0;
	virtual ~ISupportedPolicyList() = default;
};

class dptf_export SupportedPolicyList : public ISupportedPolicyList
{
public:
	SupportedPolicyList(DptfManagerInterface* dptfManager, const std::set<Guid>& defaultGuids);
	virtual ~SupportedPolicyList() = default;

	Bool isPolicySupported(const Guid& guid) const override;
	void update(void) override;
	std::set<Guid> getGuids() const override;
	UInt64 getCount() const override;

private:
	DptfManagerInterface* m_dptfManager;
	std::set<Guid> m_supportedPolicies;
	std::set<Guid> m_defaultGuids;

	void addDefaultPoliciesIfEmpty(std::set<Guid>& supportedPolicies) const;
	void addSystemPolicy(std::set<Guid>& supportedPolicies) const;
	static Bool isIdspDataValid(const DptfBuffer& buffer);
	void throwIfIdspDataValid(const DptfBuffer& buffer) const;
	std::set<Guid> readPoliciesFromIdsp() const;
	void postMessageWithSupportedGuids() const;
	EsifServicesInterface* getEsifServices() const;
};
