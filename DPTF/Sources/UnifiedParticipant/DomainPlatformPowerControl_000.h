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

#pragma once

#include "Dptf.h"
#include "DomainPlatformPowerControlBase.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//

class DomainPlatformPowerControl_000 : public DomainPlatformPowerControlBase
{
public:

    DomainPlatformPowerControl_000(UIntN participantIndex, UIntN domainIndex,
        ParticipantServicesInterface* participantServicesInterface);

    // DomainPlatformPowerControlInterface
    virtual Bool isPlatformPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType) override;
    virtual Power getPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType) override;
    virtual void setPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType, const Power& limit) override;
    virtual TimeSpan getPlatformPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType) override;
    virtual void setPlatformPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType, const TimeSpan& timeWindow) override;
    virtual Percentage getPlatformPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType) override;
    virtual void setPlatformPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType, const Percentage& dutyCycle) override;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override;
    virtual std::string getName(void) override;
    virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;
};