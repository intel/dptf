/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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


DomainPowerControl_000::DomainPowerControl_000(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface)
    : DomainPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
{
    // Do nothing.  Not an error.
}

PowerControlDynamicCapsSet DomainPowerControl_000::getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

void DomainPowerControl_000::clearCachedData(void)
{
    // Do nothing.  Not an error.
}

XmlNode* DomainPowerControl_000::getXml(UIntN domainIndex)
{
    throw not_implemented();
}

Bool DomainPowerControl_000::isPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
{
    throw not_implemented();
}

Power DomainPowerControl_000::getPowerLimit(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
{
    throw not_implemented();
}

void DomainPowerControl_000::setPowerLimit(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType, const Power& powerLimit)
{
    throw not_implemented();
}

TimeSpan DomainPowerControl_000::getPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
{
    throw not_implemented();
}

void DomainPowerControl_000::setPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType, const TimeSpan& timeWindow)
{
    throw not_implemented();
}

Percentage DomainPowerControl_000::getPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
{
    throw not_implemented();
}

void DomainPowerControl_000::setPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType, const Percentage& dutyCycle)
{
    throw not_implemented();
}

void DomainPowerControl_000::setPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex, PowerControlDynamicCapsSet capsSet)
{
    throw not_implemented();
}

std::string DomainPowerControl_000::getName(void)
{
    return "Power Control (Version 0)";
}
