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
#include "DomainSystemPowerControlBase.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//

class DomainSystemPowerControl_000 : public DomainSystemPowerControlBase
{
public:
	DomainSystemPowerControl_000(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);

	// DomainSystemPowerControlInterface
	virtual Bool isSystemPowerLimitEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) override;
	virtual Power getSystemPowerLimit(UIntN participantIndex, UIntN domainIndex, PsysPowerLimitType::Type limitType)
		override;
	virtual void setSystemPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType,
		const Power& limit) override;
	virtual TimeSpan getSystemPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) override;
	virtual void setSystemPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType,
		const TimeSpan& timeWindow) override;
	virtual Percentage getSystemPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) override;
	virtual void setSystemPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType,
		const Percentage& dutyCycle) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

	void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;
};
