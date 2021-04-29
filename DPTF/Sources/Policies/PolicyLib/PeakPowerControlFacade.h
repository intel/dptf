/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "PeakPowerControlFacadeInterface.h"
#include "CachedValue.h"

class dptf_export PeakPowerControlFacade : public PeakPowerControlFacadeInterface
{
public:
	PeakPowerControlFacade(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~PeakPowerControlFacade(void);

	virtual Bool supportsPeakPowerControl(void) override;

	virtual Power getACPeakPower(void) override;
	virtual Power getDCPeakPower(void) override;

	virtual void setACPeakPower(const Power& acPeakPower) override;
	virtual void setDCPeakPower(const Power& dcPeakPower) override;

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;

	// control properties
	DomainProperties m_domainProperties;
	UIntN m_participantIndex;
	UIntN m_domainIndex;

	// cached values
	CachedValue<Power> m_acPeakPower;
	CachedValue<Power> m_dcPeakPower;
};