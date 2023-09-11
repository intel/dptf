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
#include "ParticipantProperty.h"
#include "DomainProperties.h"

// base class for a property associated with a domain.  includes participant index.
class dptf_export DomainProperty : public ParticipantProperty
{
public:
	DomainProperty(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~DomainProperty() override = default;

	DomainProperty(const DomainProperty& other) = default;
	DomainProperty& operator=(const DomainProperty& other) = default;
	DomainProperty(DomainProperty&& other) = default;
	DomainProperty& operator=(DomainProperty&& other) = default;

protected:
	UIntN getDomainIndex() const;
	DomainProperties getDomainProperties();

private:
	UIntN m_domainIndex;
	DomainProperties m_domainProperties;
};
