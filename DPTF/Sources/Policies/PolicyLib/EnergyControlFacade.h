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

#include "EnergyControlFacadeInterface.h"
#include "PolicyServicesInterfaceContainer.h"


class dptf_export EnergyControlFacade : public EnergyControlFacadeInterface
{
public:
	EnergyControlFacade(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~EnergyControlFacade() override = default;

	Bool supportsEnergyControl() const override;
	UInt32 getRaplEnergyCounter() const override;
	EnergyCounterInfo getRaplEnergyCounterInfo() override;
	double getRaplEnergyUnit() override;
	UInt32 getRaplEnergyCounterWidth() override;
	Power getInstantaneousPower() override;
	UInt32 getEnergyThreshold() override;
	void setEnergyThreshold(UInt32 energyThreshold) override;
	void setEnergyThresholdInterruptDisable() override;

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;

	// control properties
	UIntN m_participantIndex;
	UIntN m_domainIndex;
	DomainProperties m_domainProperties;

	void throwIfControlUnsupported() const;
};
