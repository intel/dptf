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

#pragma once

#include "Dptf.h"

class DomainHardwareDutyCycleControlInterface
{
public:

    virtual ~DomainHardwareDutyCycleControlInterface()
    {
    };

    virtual DptfBuffer getHardwareDutyCycleUtilizationSet(
        UIntN participantIndex, UIntN domainIndex) const = 0;
    virtual Bool isEnabledByPlatform(UIntN participantIndex, UIntN domainIndex) const = 0;
    virtual Bool isSupportedByPlatform(UIntN participantIndex, UIntN domainIndex) const = 0;
    virtual Bool isEnabledByOperatingSystem(UIntN participantIndex, UIntN domainIndex) const = 0;
    virtual Bool isSupportedByOperatingSystem(UIntN participantIndex, UIntN domainIndex) const = 0;
    virtual Bool isHdcOobEnabled(UIntN participantIndex, UIntN domainIndex) const = 0;
    virtual void setHdcOobEnable(UIntN participantIndex, UIntN domainIndex, const UInt8& oobHdcEnable) = 0;
    virtual void setHardwareDutyCycle(UIntN participantIndex, UIntN domainIndex, const Percentage& dutyCycle) = 0;
    virtual Percentage getHardwareDutyCycle(UIntN participantIndex, UIntN domainIndex) const = 0;

};