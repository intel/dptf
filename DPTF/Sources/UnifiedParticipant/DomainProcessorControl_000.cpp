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

#include "DomainProcessorControl_000.h"

DomainProcessorControl_000::DomainProcessorControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainProcessorControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

void DomainProcessorControl_000::updatePcieThrottleRequestState(UInt32 pcieThrottleRequestState)
{
	throw not_implemented();
}

Temperature DomainProcessorControl_000::getTccOffsetTemperature()
{
	throw not_implemented();
}

void DomainProcessorControl_000::setTccOffsetTemperature(const Temperature& tccOffset)
{
	throw not_implemented();
}

void DomainProcessorControl_000::setPerfPreferenceMax(const Percentage& cpuMaxRatio)
{
	throw not_implemented();
}

void DomainProcessorControl_000::setPerfPreferenceMin(const Percentage& cpuMinRatio)
{
	throw not_implemented();
}

SocGear::Type DomainProcessorControl_000::getSocGear() const
{
	throw not_implemented();
}

void DomainProcessorControl_000::setSocGear(SocGear::Type gear)
{
	throw not_implemented();
}

SystemUsageMode::Type DomainProcessorControl_000::getSocSystemUsageMode()
{
	throw not_implemented();
}

void DomainProcessorControl_000::setSocSystemUsageMode(SystemUsageMode::Type mode)
{
	throw not_implemented();
}

UInt32 DomainProcessorControl_000::getPcieThrottleRequestState()
{
	throw not_implemented();
}

Temperature DomainProcessorControl_000::getMaxTccOffsetTemperature()
{
	throw not_implemented();
}

Temperature DomainProcessorControl_000::getMinTccOffsetTemperature()
{
	throw not_implemented();
}

void DomainProcessorControl_000::setUnderVoltageThreshold(const UInt32 voltageThreshold)
{
	throw not_implemented();
}

void DomainProcessorControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainProcessorControl_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::string DomainProcessorControl_000::getName(void)
{
	return "No Processor Control (Version 0)";
}

std::shared_ptr<XmlNode> DomainProcessorControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

