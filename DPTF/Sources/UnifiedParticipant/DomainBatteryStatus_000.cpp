/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "DomainBatteryStatus_000.h"

DomainBatteryStatus_000::DomainBatteryStatus_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainBatteryStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

Power DomainBatteryStatus_000::getMaxBatteryPower()
{
	throw not_implemented();
}

DptfBuffer DomainBatteryStatus_000::getBatteryStatus()
{
	throw not_implemented();
}

DptfBuffer DomainBatteryStatus_000::getBatteryInformation()
{
	throw not_implemented();
}

ChargerType::Type DomainBatteryStatus_000::getChargerType()
{
	throw not_implemented();
}

Power DomainBatteryStatus_000::getPlatformBatterySteadyState()
{
	throw not_implemented();
}

UInt32 DomainBatteryStatus_000::getBatteryHighFrequencyImpedance()
{
	throw not_implemented();
}

UInt32 DomainBatteryStatus_000::getBatteryNoLoadVoltage()
{
	throw not_implemented();
}

UInt32 DomainBatteryStatus_000::getBatteryMaxPeakCurrent()
{
	throw not_implemented();
}

Percentage DomainBatteryStatus_000::getBatteryPercentage()
{
	throw not_implemented();
}

void DomainBatteryStatus_000::setBatteryPercentage(Percentage batteryPercentage)
{
	throw not_implemented();
}

void DomainBatteryStatus_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainBatteryStatus_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainBatteryStatus_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string DomainBatteryStatus_000::getName(void)
{
	return "Battery Status (Version 0)";
}
