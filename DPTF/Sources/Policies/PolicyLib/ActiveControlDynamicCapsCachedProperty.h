/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "CachedProperty.h"
#include "DomainProperty.h"
#include "PolicyServicesInterfaceContainer.h"
#include "DomainSetCachedProperty.h"
#include "ActiveControlDynamicCaps.h"

class dptf_export ActiveControlDynamicCapsCachedProperty : public CachedProperty, DomainProperty
{
public:
	ActiveControlDynamicCapsCachedProperty(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~ActiveControlDynamicCapsCachedProperty();

	virtual Bool supportsProperty() override;
	const ActiveControlDynamicCaps& getCapabilities();

protected:
	virtual void refreshData() override;

private:
	ActiveControlDynamicCaps m_capabilities;
};
