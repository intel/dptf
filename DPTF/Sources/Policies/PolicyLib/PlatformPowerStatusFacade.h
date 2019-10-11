/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "PlatformPowerStatusFacadeInterface.h"

class dptf_export PlatformPowerStatusFacade : public PlatformPowerStatusFacadeInterface
{
public:
	PlatformPowerStatusFacade(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~PlatformPowerStatusFacade(void);

	virtual Power getPlatformRestOfPower(void) override;
	virtual Power getAdapterPowerRating(void) override;
	virtual PlatformPowerSource::Type getPlatformPowerSource(void) override;
	virtual UInt32 getACNominalVoltage(void) override;
	virtual UInt32 getACOperationalCurrent(void) override;
	virtual Percentage getAC1msPercentageOverload(void) override;
	virtual Percentage getAC2msPercentageOverload(void) override;
	virtual Percentage getAC10msPercentageOverload(void) override;
	virtual void notifyForProchotDeassertion(void) override;

	std::shared_ptr<XmlNode> getXml() const;

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;

	// control properties
	DomainProperties m_domainProperties;
	UIntN m_participantIndex;
	UIntN m_domainIndex;

	Power m_platformRestOfPower;
	Power m_adapterPowerRating;
	PlatformPowerSource::Type m_platformPowerSource;
	UInt32 m_acNominalVoltage;
	UInt32 m_acOperationalCurrent;
	Percentage m_ac1msPercentageOverload;
	Percentage m_ac2msPercentageOverload;
	Percentage m_ac10msPercentageOverload;
};
