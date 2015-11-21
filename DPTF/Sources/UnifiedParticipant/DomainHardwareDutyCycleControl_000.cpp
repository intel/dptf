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

#include "DomainHardwareDutyCycleControl_000.h"

DomainHardwareDutyCycleControl_000::DomainHardwareDutyCycleControl_000(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface)
    : DomainHardwareDutyCycleControlBase(participantIndex, domainIndex, participantServicesInterface)
{
    
}

void DomainHardwareDutyCycleControl_000::clearCachedData(void)
{

}

XmlNode* DomainHardwareDutyCycleControl_000::getXml(UIntN domainIndex)
{
    throw not_implemented();
}

DptfBuffer DomainHardwareDutyCycleControl_000::getHardwareDutyCycleUtilizationSet(
    UIntN participantIndex, UIntN domainIndex) const
{
    throw not_implemented();
}

Bool DomainHardwareDutyCycleControl_000::isEnabledByPlatform(UIntN participantIndex, UIntN domainIndex) const
{
    throw not_implemented();
}

Bool DomainHardwareDutyCycleControl_000::isSupportedByPlatform(UIntN participantIndex, UIntN domainIndex) const
{
    throw not_implemented();
}

Bool DomainHardwareDutyCycleControl_000::isEnabledByOperatingSystem(UIntN participantIndex, UIntN domainIndex) const
{
    throw not_implemented();
}

Bool DomainHardwareDutyCycleControl_000::isSupportedByOperatingSystem(UIntN participantIndex, UIntN domainIndex) const
{
    throw not_implemented();
}

Bool DomainHardwareDutyCycleControl_000::isHdcOobEnabled(UIntN participantIndex, UIntN domainIndex) const
{
    throw not_implemented();
}

void DomainHardwareDutyCycleControl_000::setHdcOobEnable(UIntN participantIndex, UIntN domainIndex, const UInt8& hdcOobEnable)
{
    throw not_implemented();
}

void DomainHardwareDutyCycleControl_000::setHardwareDutyCycle(
    UIntN participantIndex, UIntN domainIndex, const Percentage& dutyCycle)
{
    throw not_implemented();
}

Percentage DomainHardwareDutyCycleControl_000::getHardwareDutyCycle(UIntN participantIndex, UIntN domainIndex) const
{
    throw not_implemented();
}

std::string DomainHardwareDutyCycleControl_000::getName(void)
{
    return "Hardware Duty Cycle Control (Version 0)";
}
