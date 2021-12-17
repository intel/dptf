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
#include "DomainPowerControlBase.h"
#include "CachedValue.h"
#include "PowerControlState.h"

class DomainPowerControl_001 : public DomainPowerControlBase
{
public:
	DomainPowerControl_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainPowerControl_001(void);

	// DomainPowerControlInterface
	virtual Bool isPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
		override;
	virtual Power getPowerLimit(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType) override;
	virtual Power getPowerLimitWithoutCache(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	virtual Bool isSocPowerFloorEnabled(UIntN participantIndex, UIntN domainIndex) override;
	virtual Bool isSocPowerFloorSupported(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	virtual void setPowerLimitWithoutUpdatingEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	virtual void setPowerLimitIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	virtual TimeSpan getPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	virtual void setPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	virtual void setPowerLimitTimeWindowWithoutUpdatingEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	virtual void setPowerLimitTimeWindowIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	virtual Percentage getPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	virtual void setPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Percentage& dutyCycle) override;
	virtual void setSocPowerFloorState(
		UIntN participantIndex,
		UIntN domainIndex,
		Bool socPowerFloorState) override;

	virtual PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
		override;
	virtual void setPowerControlDynamicCapsSet(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlDynamicCapsSet capsSet) override;
	virtual void setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;
	virtual TimeSpan getPowerSharePowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex) override;

	virtual Bool isPowerShareControl(UIntN participantIndex, UIntN domainIndex) override;
	virtual double getPidKpTerm(UIntN participantIndex, UIntN domainIndex) override;
	virtual double getPidKiTerm(UIntN participantIndex, UIntN domainIndex) override;
	virtual TimeSpan getAlpha(UIntN participantIndex, UIntN domainIndex) override;
	virtual TimeSpan getFastPollTime(UIntN participantIndex, UIntN domainIndex) override;
	virtual TimeSpan getSlowPollTime(UIntN participantIndex, UIntN domainIndex) override;
	virtual TimeSpan getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex) override;
	virtual void removePowerLimitPolicyRequest(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	virtual void setPowerSharePolicyPower(
		UIntN participantIndex,
		UIntN domainIndex,
		const Power& powerSharePolicyPower) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

protected:
	virtual void capture(void) override;
	virtual void restore(void) override;

private:
	// Don't allow this class to be copied
	DomainPowerControl_001(const DomainPowerControl_001& rhs);
	DomainPowerControl_001& operator=(const DomainPowerControl_001& rhs);

	PowerControlDynamicCapsSet getDynamicCapabilities();
	void setAndUpdateEnabled(PowerControlType::Type controlType);

	void throwIfLimitNotEnabled(PowerControlType::Type controlType);
	void throwIfTypeInvalidForPowerLimit(PowerControlType::Type controlType);
	void throwIfTypeInvalidForTimeWindow(PowerControlType::Type controlType);
	void throwIfTypeInvalidForDutyCycle(PowerControlType::Type controlType);
	void throwIfPowerLimitIsOutsideCapabilityRange(PowerControlType::Type controlType, const Power& powerLimit);
	void throwIfTimeWindowIsOutsideCapabilityRange(PowerControlType::Type controlType, const TimeSpan& timeWindow);
	void throwIfDutyCycleIsOutsideCapabilityRange(PowerControlType::Type controlType, const Percentage& dutyCycle);
	void throwIfDynamicCapabilitiesAreEmpty(const PowerControlDynamicCapsSet& capabilities);

	std::shared_ptr<XmlNode> createStatusNode(PowerControlType::Type controlType);
	std::string createStatusStringForEnabled(PowerControlType::Type controlType);
	std::string createStatusStringForLimitValue(PowerControlType::Type controlType);
	std::string createStatusStringForTimeWindow(PowerControlType::Type controlType);
	std::string createStatusStringForDutyCycle(PowerControlType::Type controlType);

	CachedValue<PowerControlDynamicCapsSet> m_powerControlDynamicCaps;
	PowerControlState m_initialState;
	Bool m_capabilitiesLocked;
	CachedValue<Bool> m_socPowerFloorSupported;
};
