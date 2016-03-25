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

#include "DomainPlatformPowerControl_000.h"

DomainPlatformPowerControl_000::DomainPlatformPowerControl_000(UIntN participantIndex, UIntN domainIndex,
    ParticipantServicesInterface* participantServicesInterface)
    : DomainPlatformPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
{
    // Do nothing.  Not an error.
}

void DomainPlatformPowerControl_000::clearCachedData(void)
{
    // Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainPlatformPowerControl_000::getXml(UIntN domainIndex)
{
    throw not_implemented();
}

Bool DomainPlatformPowerControl_000::isPlatformPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType)
{
    throw not_implemented();
}

Power DomainPlatformPowerControl_000::getPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex,
    PlatformPowerLimitType::Type limitType)
{
    throw not_implemented();
}

void DomainPlatformPowerControl_000::setPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType, const Power& limit)
{
    throw not_implemented();
}

TimeSpan DomainPlatformPowerControl_000::getPlatformPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType)
{
    throw not_implemented();
}

void DomainPlatformPowerControl_000::setPlatformPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType, const TimeSpan& timeWindow)
{
    throw not_implemented();
}

Percentage DomainPlatformPowerControl_000::getPlatformPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType)
{
    throw not_implemented();
}

void DomainPlatformPowerControl_000::setPlatformPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType, const Percentage& dutyCycle)
{
    throw not_implemented();
}

std::string DomainPlatformPowerControl_000::getName(void)
{
    return "Platform Power Control (Version 0)";
}
