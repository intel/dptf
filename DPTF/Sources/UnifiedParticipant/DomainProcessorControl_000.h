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
#include "DomainProcessorControlBase.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//

class DomainProcessorControl_000 : public DomainProcessorControlBase
{
public:
	DomainProcessorControl_000(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);

	// DomainProcessorControl
	virtual void updatePcieThrottleRequestState(UInt32 pcieThrottleRequestState) override;

	// DomainProcessorControlInterface
	virtual Temperature getTccOffsetTemperature() override;
	virtual void setTccOffsetTemperature(const Temperature& tccOffset) override;
	virtual Temperature getMaxTccOffsetTemperature() override;
	virtual Temperature getMinTccOffsetTemperature() override;
	virtual void setUnderVoltageThreshold(const UInt32 voltageThreshold) override;
	virtual void setPerfPreferenceMax(const Percentage& cpuMaxRatio) override;
	virtual void setPerfPreferenceMin(const Percentage& cpuMinRatio) override;
	virtual UInt32 getPcieThrottleRequestState() override;
	virtual SocGear::Type getSocGear() const override;
	virtual void setSocGear(SocGear::Type socGear) override;
	virtual SystemUsageMode::Type getSocSystemUsageMode() override;
	virtual void setSocSystemUsageMode(SystemUsageMode::Type mode) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;
};
