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
#include "DomainPowerControlBase.h"
#include "CachedValue.h"
#include "PowerControlState.h"

class DomainPowerControl_002 : public DomainPowerControlBase
{
public:
	DomainPowerControl_002(
		UIntN participantIndex,
		UIntN domainIndex,
		const std::shared_ptr<ParticipantServicesInterface>& participantServicesInterface);
	~DomainPowerControl_002() override;

	// Don't allow this class to be copied
	DomainPowerControl_002(const DomainPowerControl_002& rhs) = delete;
	DomainPowerControl_002& operator=(const DomainPowerControl_002& rhs) = delete;

	// DomainPowerControlInterface
	Bool isPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
		override;
	Power getPowerLimit(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType) override;
	Power getPowerLimitWithoutCache(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	Bool isSocPowerFloorEnabled(UIntN participantIndex, UIntN domainIndex) override;
	Bool isSocPowerFloorSupported(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getSocPowerFloorState(UIntN participantIndex, UIntN domainIndex) override;
	void setPowerLimitMin(
		UIntN participantIndex, 
		UIntN domainIndex, 
		PowerControlType::Type controlType, 
		const Power& powerLimit) override;
	void setPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	void setPowerLimitWithoutUpdatingEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	void setPowerLimitIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	TimeSpan getPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	void setPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	void setPowerLimitTimeWindowWithoutUpdatingEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	void setPowerLimitTimeWindowIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	Percentage getPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	void setPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Percentage& dutyCycle) override;
	void setSocPowerFloorState(
		UIntN participantIndex,
		UIntN domainIndex,
		Bool socPowerFloorState) override;
	void clearPowerLimitMin(UIntN participantIndex, UIntN domainIndex) override;
	void clearPowerLimit(UIntN participantIndex, UIntN domainIndex) override;
	void clearCachedPowerLimits(UIntN participantIndex, UIntN domainIndex) override;

	PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
		override;
	void setPowerControlDynamicCapsSet(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlDynamicCapsSet capsSet) override;
	void setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;
	TimeSpan getPowerSharePowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex) override;

	Bool isPowerShareControl(UIntN participantIndex, UIntN domainIndex) override;
	double getPidKpTerm(UIntN participantIndex, UIntN domainIndex) override;
	double getPidKiTerm(UIntN participantIndex, UIntN domainIndex) override;
	TimeSpan getAlpha(UIntN participantIndex, UIntN domainIndex) override;
	TimeSpan getFastPollTime(UIntN participantIndex, UIntN domainIndex) override;
	TimeSpan getSlowPollTime(UIntN participantIndex, UIntN domainIndex) override;
	TimeSpan getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex) override;
	Power getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex) override;
	Power getThermalDesignPower(UIntN participantIndex, UIntN domainIndex) override;
	void removePowerLimitPolicyRequest(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	void setPowerSharePolicyPower(
		UIntN participantIndex, 
		UIntN domainIndex, 
		const Power& powerSharePolicyPower) override;
	void setPowerShareEffectiveBias(
		UIntN participantIndex, 
		UIntN domainIndex, 
		UInt32 powerShareEffectiveBias) override;

	// DomainPowerControl
	void updateSocPowerFloorState(UInt32 socPowerFloorState) override;

	// ParticipantActivityLoggingInterface
	void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	void onClearCachedData() override;
	std::string getName() override;
	std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

protected:
	void capture() override;
	void restore() override;

private:
	PowerControlDynamicCapsSet getDynamicCapabilities() const;
	void setAndUpdateEnabled(PowerControlType::Type controlType);

	void throwIfLimitNotEnabled(PowerControlType::Type controlType) const;
	static void throwIfTypeInvalidForPowerLimit(PowerControlType::Type controlType);
	void throwIfPowerLimitIsOutsideCapabilityRange(PowerControlType::Type controlType, const Power& powerLimit);
	static void throwIfDynamicCapabilitiesAreWrong(const PowerControlDynamicCapsSet& capabilities);

	std::shared_ptr<XmlNode> createStatusNode(PowerControlType::Type controlType);
	std::string createStatusStringForEnabled(PowerControlType::Type controlType) const;
	std::string createStatusStringForLimitValue(PowerControlType::Type controlType);

	CachedValue<Power> getRaplPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		CachedValue<Power> plLimit);

	CachedValue<PowerControlDynamicCapsSet> m_powerControlDynamicCaps;
	PowerControlState m_initialState;
	Bool m_capabilitiesLocked;
	CachedValue<Power> m_pl1Limit;
	CachedValue<Power> m_pl2Limit;
	CachedValue<Power> m_pl4Limit;
	Power m_tdp;
};
