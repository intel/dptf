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
#include "DomainPlatformPowerControlBase.h"
#include "PlatformPowerControlState.h"

class DomainPlatformPowerControl_001 : public DomainPlatformPowerControlBase
{
public:

    DomainPlatformPowerControl_001(UIntN participantIndex, UIntN domainIndex,
        ParticipantServicesInterface* participantServicesInterface);
    ~DomainPlatformPowerControl_001(void);

    // DomainPlatformPowerControlInterface
    virtual Bool isPlatformPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType) override;
    virtual Power getPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType) override;
    virtual void setPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex, 
        PlatformPowerLimitType::Type limitType, const Power& powerLimit) override;
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
    virtual XmlNode* getXml(UIntN domainIndex) override;

private:

    DomainPlatformPowerControl_001(const DomainPlatformPowerControl_001& rhs);
    DomainPlatformPowerControl_001& operator=(const DomainPlatformPowerControl_001& rhs);

    void throwIfLimitNotEnabled(PlatformPowerLimitType::Type limitType);
    void throwIfTypeInvalidForPowerLimit(PlatformPowerLimitType::Type limitType);
    void throwIfTypeInvalidForTimeWindow(PlatformPowerLimitType::Type limitType);
    void throwIfTypeInvalidForDutyCycle(PlatformPowerLimitType::Type limitType);

    XmlNode* createStatusNode(PlatformPowerLimitType::Type limitType);
    std::string createStatusStringForEnabled(PlatformPowerLimitType::Type limitType);
    std::string createStatusStringForLimitValue(PlatformPowerLimitType::Type limitType);
    Bool isEnabled(PlatformPowerLimitType::Type limitType) const;
    std::string createStatusStringForTimeWindow(PlatformPowerLimitType::Type limitType);
    std::string createStatusStringForDutyCycle(PlatformPowerLimitType::Type limitType);

    Bool m_pl1Enabled;
    Bool m_pl2Enabled;
    Bool m_pl3Enabled;

    PlatformPowerControlState m_initialState;
};