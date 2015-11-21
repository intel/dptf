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

#include "DomainPlatformPowerControlBase.h"

DomainPlatformPowerControlBase::DomainPlatformPowerControlBase(UIntN participantIndex, UIntN domainIndex,
    ParticipantServicesInterface* participantServicesInterface)
    : ControlBase(participantIndex, domainIndex, participantServicesInterface)
{

}

DomainPlatformPowerControlBase::~DomainPlatformPowerControlBase()
{

}

Bool DomainPlatformPowerControlBase::checkEnabled(PlatformPowerLimitType::Type limitType)
{
    try
    {
        UInt32 plEnabled = getParticipantServices()->primitiveExecuteGetAsUInt32(
            GET_PLATFORM_POWER_LIMIT_ENABLE, getDomainIndex(), (UInt8)limitType);
        return plEnabled > 0 ? true : false;
    }
    catch (...)
    {
        return false;
    }
}

void DomainPlatformPowerControlBase::setEnabled(PlatformPowerLimitType::Type limitType, Bool enable)
{
    try
    {
        getParticipantServices()->primitiveExecuteSetAsUInt32(
            SET_PLATFORM_POWER_LIMIT_ENABLE, enable ? (UInt32)1 : (UInt32)0, getDomainIndex(), (UInt8)limitType);
    }
    catch (...)
    {

    }
}