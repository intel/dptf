/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "DomainPlatformPowerStatus_000.h"

DomainPlatformPowerStatus_000::DomainPlatformPowerStatus_000(UIntN participantIndex, UIntN domainIndex,
    std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
    : DomainPlatformPowerStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
    // Do nothing.  Not an error.
}

Power DomainPlatformPowerStatus_000::getMaxBatteryPower(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

Power DomainPlatformPowerStatus_000::getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

Power DomainPlatformPowerStatus_000::getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

DptfBuffer DomainPlatformPowerStatus_000::getBatteryStatus(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

DptfBuffer DomainPlatformPowerStatus_000::getBatteryInformation(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

PlatformPowerSource::Type DomainPlatformPowerStatus_000::getPlatformPowerSource(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

ChargerType::Type DomainPlatformPowerStatus_000::getChargerType(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

Power DomainPlatformPowerStatus_000::getACPeakPower(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

TimeSpan DomainPlatformPowerStatus_000::getACPeakTimeWindow(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

Power DomainPlatformPowerStatus_000::getPlatformBatterySteadyState(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

void DomainPlatformPowerStatus_000::clearCachedData(void)
{
    // Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainPlatformPowerStatus_000::getXml(UIntN domainIndex)
{
    throw not_implemented();
}

std::string DomainPlatformPowerStatus_000::getName(void)
{
    return "Platform Power Status (Version 0)";
}
