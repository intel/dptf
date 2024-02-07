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

#include "DomainBiasControl_000.h"

DomainBiasControl_000::DomainBiasControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainBiasControlBase(participantIndex, domainIndex, participantServicesInterface)
{
}

void DomainBiasControl_000::setCpuOpboostEnableAC(Bool enabled)
{
	throw not_implemented();
}

void DomainBiasControl_000::setCpuOpboostEnableDC(Bool enabled)
{
	throw not_implemented();
}

void DomainBiasControl_000::setGpuOpboostEnableAC(Bool enabled)
{
	throw not_implemented();
}

void DomainBiasControl_000::setGpuOpboostEnableDC(Bool enabled)
{
	throw not_implemented();
}

void DomainBiasControl_000::setSplitRatio(const Percentage& splitRatio)
{
	throw not_implemented();
}

void DomainBiasControl_000::setSplitRatioMax(const Percentage& splitRatio)
{
	throw not_implemented();
}

Bool DomainBiasControl_000::getCpuOpboostEnableAC()
{
	throw not_implemented();
}

Bool DomainBiasControl_000::getCpuOpboostEnableDC()
{
	throw not_implemented();
}

Bool DomainBiasControl_000::getGpuOpboostEnableAC()
{
	throw not_implemented();
}

Bool DomainBiasControl_000::getGpuOpboostEnableDC()
{
	throw not_implemented();
}

Percentage DomainBiasControl_000::getSplitRatio()
{
	throw not_implemented();
}

Percentage DomainBiasControl_000::getSplitRatioActive()
{
	throw not_implemented();
}

Percentage DomainBiasControl_000::getSplitRatioMax()
{
	throw not_implemented();
}

Power DomainBiasControl_000::getReservedTgp()
{
	throw not_implemented();
}

OpportunisticBoostMode::Type DomainBiasControl_000::getOppBoostMode()
{
	throw not_implemented();
}

std::string DomainBiasControl_000::getName(void)
{
	return "Bias Control (Version 0)";
}

std::shared_ptr<XmlNode> DomainBiasControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}