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
#include "PolicyServices.h"
#include "DomainEnergyControlInterface.h"
#include "EnergyCounterInfo.h"

class PolicyServicesDomainEnergyControl : public PolicyServices, public DomainEnergyControlInterface
{
public:
	PolicyServicesDomainEnergyControl(DptfManagerInterface* dptfManager, UIntN policyIndex);

	virtual UInt32 getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex) override final;
	virtual EnergyCounterInfo getRaplEnergyCounterInfo(UIntN participantIndex, UIntN domainIndex) override final;
	virtual double getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex) override final;
	virtual UInt32 getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex) override final;
	virtual Power getInstantaneousPower(UIntN participantIndex, UIntN domainIndex) override final;
	virtual UInt32 getEnergyThreshold(UIntN participantIndex, UIntN domainIndex) override final;
	virtual void setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold) override final;
	virtual void setEnergyThresholdInterruptDisable(
		UIntN participantIndex,
		UIntN domainIndex) override final;
};
