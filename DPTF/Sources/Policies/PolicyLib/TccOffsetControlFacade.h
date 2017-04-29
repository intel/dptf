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
#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"
#include "XmlNode.h"
#include "TccOffsetControlFacadeInterface.h"
#include "CachedValue.h"

class dptf_export TccOffsetControlFacade : public TccOffsetControlFacadeInterface
{
public:
	TccOffsetControlFacade(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~TccOffsetControlFacade(void);

	virtual Bool supportsTccOffsetControl(void) override;

	virtual Temperature getTccOffsetTemperature(void) override;
	virtual void setTccOffsetTemperature(const Temperature& tccOffset) override;
	virtual Temperature getMaxTccOffsetTemperature(void) override;
	virtual Temperature getMinTccOffsetTemperature(void) override;

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;

	// control properties
	DomainProperties m_domainProperties;
	UIntN m_participantIndex;
	UIntN m_domainIndex;

	// cached values
	CachedValue<Temperature> m_tccOffset;
};