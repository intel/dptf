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
#include "EnergyCounterInfo.h"

class DomainEnergyControlInterface
{
public:
	virtual ~DomainEnergyControlInterface() {};
	virtual UInt32 getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual EnergyCounterInfo getRaplEnergyCounterInfo(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual double getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt32 getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Power getInstantaneousPower(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UInt32 getEnergyThreshold(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold) = 0;
	virtual void setEnergyThresholdInterruptDisable(
		UIntN participantIndex,
		UIntN domainIndex) = 0;
};
