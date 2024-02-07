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

#include "DomainActiveControl_000.h"

DomainActiveControl_000::DomainActiveControl_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainActiveControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

DptfBuffer DomainActiveControl_000::getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

DptfBuffer DomainActiveControl_000::getActiveControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

DptfBuffer DomainActiveControl_000::getActiveControlStatus(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

DptfBuffer DomainActiveControl_000::getActiveControlSet(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainActiveControl_000::setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed)
{
	throw not_implemented();
}

void DomainActiveControl_000::setActiveControlFanDirection(UInt32 fanDirection)
{
	throw not_implemented();
}

void DomainActiveControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainActiveControl_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainActiveControl_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string DomainActiveControl_000::getName(void)
{
	return "Active Control (Version 0)";
}

void DomainActiveControl_000::setFanCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	throw not_implemented();
}

void DomainActiveControl_000::setActiveControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	ActiveControlDynamicCaps newCapabilities)
{
	throw not_implemented();
}

void DomainActiveControl_000::setActiveControlFanOperatingMode(UInt32 fanOperatingMode)
{
	throw not_implemented();
}

UInt32 DomainActiveControl_000::getActiveControlFanOperatingMode(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainActiveControl_000::getActiveControlFanCapabilities(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}