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

#include "DomainTemperature_000.h"

DomainTemperature_000::DomainTemperature_000(
	UIntN participantIndex,
	UIntN domainIndex,
	Bool areTemperatureThresholdsSupported,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainTemperatureBase(
		participantIndex,
		domainIndex,
		areTemperatureThresholdsSupported,
		participantServicesInterface)
{
	// Do nothing.  Not an error.
}

TemperatureStatus DomainTemperature_000::getTemperatureStatus()
{
	throw not_implemented();
}

TemperatureThresholds DomainTemperature_000::getTemperatureThresholds()
{
	throw not_implemented();
}

void DomainTemperature_000::setTemperatureThresholds(const TemperatureThresholds& temperatureThresholds,
	const TemperatureThresholds& lastSetTemperatureThresholds)
{
	throw not_implemented();
}

Temperature DomainTemperature_000::getPowerShareTemperatureThreshold()
{
	throw not_implemented();
}

DptfBuffer DomainTemperature_000::getCalibrationTable()
{
	throw not_implemented();
}

DptfBuffer DomainTemperature_000::getPollingTable()
{
	throw not_implemented();
}

Bool DomainTemperature_000::isVirtualTemperature()
{
	return false;
}

void DomainTemperature_000::setVirtualTemperature(const Temperature& temperature)
{
	throw not_implemented();
}

void DomainTemperature_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

std::shared_ptr<XmlNode> DomainTemperature_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string DomainTemperature_000::getName(void)
{
	return "Temperature Status (Version 0)";
}

std::shared_ptr<XmlNode> DomainTemperature_000::getArbitratorXml(UIntN policyIndex) const
{
	throw not_implemented();
}
