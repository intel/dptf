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
#include "DomainEnergyControlBase.h"
#include "EnergyCounterInfo.h"

class DomainEnergyControl_001 : public DomainEnergyControlBase
{
public:
	DomainEnergyControl_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainEnergyControl_001(void);

	// DomainEnergyControlInterface
	virtual UInt32 getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex) override;
	virtual EnergyCounterInfo getRaplEnergyCounterInfo(UIntN participantIndex, UIntN domainIndex) override;
	virtual double getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getInstantaneousPower(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getEnergyThreshold(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold) override;
	virtual void setEnergyThresholdInterruptDisable(UIntN participantIndex, UIntN domainIndex) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	// Don't allow this class to be copied
	DomainEnergyControl_001(const DomainEnergyControl_001& rhs);
	DomainEnergyControl_001& operator=(const DomainEnergyControl_001& rhs);

	double m_raplEnergyUnit;
};
