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
#include "DomainBatteryStatusBase.h"
#include "CachedValue.h"

class DomainBatteryStatus_001 : public DomainBatteryStatusBase
{
public:
	DomainBatteryStatus_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainBatteryStatus_001(void);

	// DomainBatteryStatusInterface
	virtual Power getMaxBatteryPower() override;
	virtual DptfBuffer getBatteryStatus() override;
	virtual DptfBuffer getBatteryInformation() override;
	virtual ChargerType::Type getChargerType() override;
	virtual Power getPlatformBatterySteadyState() override;
	virtual UInt32 getBatteryHighFrequencyImpedance() override;
	virtual UInt32 getBatteryNoLoadVoltage() override;
	virtual UInt32 getBatteryMaxPeakCurrent() override;
	virtual Percentage getBatteryPercentage() override;
	virtual void setBatteryPercentage(Percentage batteryPercentage) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	// hide the copy constructor and = operator
	DomainBatteryStatus_001(const DomainBatteryStatus_001& rhs);
	DomainBatteryStatus_001& operator=(const DomainBatteryStatus_001& rhs);
};
