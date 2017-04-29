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
#include "PlatformPowerControlFacadeInterface.h"
#include "CachedValue.h"

class dptf_export PlatformPowerControlFacade : public PlatformPowerControlFacadeInterface
{
public:
	PlatformPowerControlFacade(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~PlatformPowerControlFacade(void);

	virtual Bool isPl1PowerLimitEnabled(void) override;
	virtual Bool isPl2PowerLimitEnabled(void) override;
	virtual Bool isPl3PowerLimitEnabled(void) override;

	virtual Power getPl1PowerLimit(void) override;
	virtual Power getPl2PowerLimit(void) override;
	virtual Power getPl3PowerLimit(void) override;
	virtual TimeSpan getPl1TimeWindow(void) override;
	virtual TimeSpan getPl3TimeWindow(void) override;
	virtual Percentage getPl3DutyCycle(void) override;

	virtual void setPl1PowerLimit(const Power& powerLimit) override;
	virtual void setPl2PowerLimit(const Power& powerLimit) override;
	virtual void setPl3PowerLimit(const Power& powerLimit) override;
	virtual void setPl1TimeWindow(const TimeSpan& timeWindow) override;
	virtual void setPl3TimeWindow(const TimeSpan& timeWindow) override;
	virtual void setPl3DutyCycle(const Percentage& dutyCycle) override;

	std::shared_ptr<XmlNode> getXml() const;

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;

	// control properties
	DomainProperties m_domainProperties;
	UIntN m_participantIndex;
	UIntN m_domainIndex;

	// cached values
	CachedValue<Bool> m_pl1PowerLimitEnabled;
	CachedValue<Bool> m_pl2PowerLimitEnabled;
	CachedValue<Bool> m_pl3PowerLimitEnabled;
	CachedValue<Power> m_pl1PowerLimit;
	CachedValue<Power> m_pl2PowerLimit;
	CachedValue<Power> m_pl3PowerLimit;
	CachedValue<TimeSpan> m_pl1TimeWindow;
	CachedValue<TimeSpan> m_pl3TimeWindow;
	CachedValue<Percentage> m_pl3DutyCycle;

	std::shared_ptr<XmlNode> createPl1XmlData() const;
	std::shared_ptr<XmlNode> createPl2XmlData() const;
	std::shared_ptr<XmlNode> createPl3XmlData() const;
};
