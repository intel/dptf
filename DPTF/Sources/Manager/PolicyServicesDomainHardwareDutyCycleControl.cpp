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

#include "PolicyServicesDomainHardwareDutyCycleControl.h"
#include "ParticipantManager.h"

PolicyServicesDomainHardwareDutyCycleControl::PolicyServicesDomainHardwareDutyCycleControl(
    DptfManagerInterface* dptfManager, UIntN policyIndex) :
    PolicyServices(dptfManager, policyIndex)
{

}

DptfBuffer PolicyServicesDomainHardwareDutyCycleControl::getHardwareDutyCycleUtilizationSet(
    UIntN participantIndex, UIntN domainIndex) const
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getHardwareDutyCycleUtilizationSet(domainIndex);
}

Bool PolicyServicesDomainHardwareDutyCycleControl::isEnabledByPlatform(
    UIntN participantIndex, UIntN domainIndex) const
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->isEnabledByPlatform(domainIndex);
}

Bool PolicyServicesDomainHardwareDutyCycleControl::isSupportedByPlatform(
    UIntN participantIndex, UIntN domainIndex) const
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->isSupportedByPlatform(domainIndex);
}

Bool PolicyServicesDomainHardwareDutyCycleControl::isEnabledByOperatingSystem(
    UIntN participantIndex, UIntN domainIndex) const
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->isEnabledByOperatingSystem(domainIndex);
}

Bool PolicyServicesDomainHardwareDutyCycleControl::isSupportedByOperatingSystem(
    UIntN participantIndex, UIntN domainIndex) const
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->isSupportedByOperatingSystem(domainIndex);
}

Bool PolicyServicesDomainHardwareDutyCycleControl::isHdcOobEnabled(
    UIntN participantIndex, UIntN domainIndex) const
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->isHdcOobEnabled(domainIndex);
}

void PolicyServicesDomainHardwareDutyCycleControl::setHdcOobEnable(
    UIntN participantIndex, UIntN domainIndex, const UInt8& hdcOobEnable)
{
    throwIfNotWorkItemThread();
    getParticipantManager()->getParticipantPtr(participantIndex)->setHdcOobEnable(domainIndex, hdcOobEnable);
}

void PolicyServicesDomainHardwareDutyCycleControl::setHardwareDutyCycle(
    UIntN participantIndex, UIntN domainIndex, const Percentage& dutyCycle)
{
    throwIfNotWorkItemThread();
    getParticipantManager()->getParticipantPtr(participantIndex)->setHardwareDutyCycle(domainIndex, dutyCycle);
}

Percentage PolicyServicesDomainHardwareDutyCycleControl::getHardwareDutyCycle(
    UIntN participantIndex, UIntN domainIndex) const
{
    throwIfNotWorkItemThread();
    return getParticipantManager()->getParticipantPtr(participantIndex)->getHardwareDutyCycle(domainIndex);
}