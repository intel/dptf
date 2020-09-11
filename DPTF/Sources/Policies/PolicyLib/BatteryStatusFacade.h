/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "BatteryStatusFacadeInterface.h"

class dptf_export BatteryStatusFacade : public BatteryStatusFacadeInterface
{
public:
	BatteryStatusFacade(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~BatteryStatusFacade(void);

	virtual Power getMaxBatteryPower(void) override;
	virtual DptfBuffer getBatteryStatus(void) override;
	virtual DptfBuffer getBatteryInformation(void) override;
	virtual ChargerType::Type getChargerType(void) override;
	virtual Power getPlatformBatterySteadyState(void) override;
	virtual UInt32 getBatteryHighFrequencyImpedance(void) override;
	virtual UInt32 getBatteryNoLoadVoltage(void) override;
	virtual UInt32 getBatteryMaxPeakCurrent(void) override;
	virtual Percentage getBatteryPercentage(void) override;
	virtual void setBatteryPercentage(Percentage batteryPercentage) override;

	std::shared_ptr<XmlNode> getXml() const;

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;

	// control properties
	DomainProperties m_domainProperties;
	UIntN m_participantIndex;
	UIntN m_domainIndex;

	Power m_maxBatteryPower;
	ChargerType::Type m_chargerType;
	Power m_batterySteadyState;
	UInt32 m_batteryHighFrequencyImpedance;
	UInt32 m_batteryNoLoadVoltage;
	UInt32 m_batteryMaxPeakCurrent;
	UInt32 m_batteryPercentage;

	const PolicyServicesInterfaceContainer& getPolicyServices() const;
};
