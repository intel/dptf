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

#include "DomainPowerControl_000.h"

DomainPowerControl_000::DomainPowerControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

PowerControlDynamicCapsSet DomainPowerControl_000::getPowerControlDynamicCapsSet(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPowerControl_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainPowerControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

Bool DomainPowerControl_000::isPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throw not_implemented();
}

Power DomainPowerControl_000::getPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throw not_implemented();
}

Power DomainPowerControl_000::getPowerLimitWithoutCache(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throw not_implemented();
}

Bool DomainPowerControl_000::isSocPowerFloorEnabled(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throw not_implemented();
}

Bool DomainPowerControl_000::isSocPowerFloorSupported(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPowerControl_000::setPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throw not_implemented();
}

void DomainPowerControl_000::setPowerLimitWithoutUpdatingEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throw not_implemented();
}

void DomainPowerControl_000::setPowerLimitIgnoringCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throw not_implemented();
}

TimeSpan DomainPowerControl_000::getPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throw not_implemented();
}

void DomainPowerControl_000::setPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throw not_implemented();
}

void DomainPowerControl_000::setPowerLimitTimeWindowWithoutUpdatingEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throw not_implemented();
}

void DomainPowerControl_000::setPowerLimitTimeWindowIgnoringCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throw not_implemented();
}

Percentage DomainPowerControl_000::getPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throw not_implemented();
}

void DomainPowerControl_000::setPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Percentage& dutyCycle)
{
	throw not_implemented();
}

void DomainPowerControl_000::setSocPowerFloorState(
	UIntN participantIndex,
	UIntN domainIndex,
	Bool socPowerFloorState)
{
	throw not_implemented();
}

void DomainPowerControl_000::clearPowerLimit(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPowerControl_000::setPowerControlDynamicCapsSet(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlDynamicCapsSet capsSet)
{
	throw not_implemented();
}

void DomainPowerControl_000::setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	throw not_implemented();
}

TimeSpan DomainPowerControl_000::getPowerSharePowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Bool DomainPowerControl_000::isPowerShareControl(UIntN participantIndex, UIntN domainIndex)
{
	return false;
}

double DomainPowerControl_000::getPidKpTerm(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

double DomainPowerControl_000::getPidKiTerm(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

TimeSpan DomainPowerControl_000::getAlpha(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

TimeSpan DomainPowerControl_000::getFastPollTime(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

TimeSpan DomainPowerControl_000::getSlowPollTime(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

TimeSpan DomainPowerControl_000::getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Power DomainPowerControl_000::getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPowerControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainPowerControl_000::removePowerLimitPolicyRequest(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	// Do nothing.  Not an error.
}

void DomainPowerControl_000::setPowerSharePolicyPower(
	UIntN participantIndex,
	UIntN domainIndex,
	const Power& powerSharePolicyPower)
{
	throw not_implemented();
}

std::string DomainPowerControl_000::getName(void)
{
	return "Power Control (Version 0)";
}
