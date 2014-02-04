/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#include "DomainPowerControlInterface.h"
#include "ComponentExtendedInterface.h"
#include "ParticipantServicesInterface.h"
#include "BinaryParse.h"
#include <sstream>
#include "DptfMemory.h"
#include "ConfigTdpDataSyncInterface.h"

class DomainPowerControl_001 final : public DomainPowerControlInterface,
    public ComponentExtendedInterface
{
public:

    DomainPowerControl_001(ParticipantServicesInterface* participantServicesInterface);
    ~DomainPowerControl_001(void);

    // DomainPowerControlInterface
    virtual PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PowerControlStatusSet getPowerControlStatusSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setPowerControl(UIntN participantIndex, UIntN domainIndex, const PowerControlStatusSet& powerControlStatusSet) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;

private:

    // Don't allow this class to be copied
    DomainPowerControl_001(const DomainPowerControl_001& rhs);
    DomainPowerControl_001& operator=(const DomainPowerControl_001& rhs);

    void createPowerControlDynamicCapsSet(UIntN domainIndex);
    void validatePowerControlDynamicCapsSet();
    void determinePowerControlProgrammability();
    void programPowerControl(const PowerControlStatusSet& powerControlStatusSet, UIntN domainIndex);
    void validatePowerControlStatus(const PowerControlStatusSet& powerControlStatusSet, UIntN domainIndex);
    void initializePowerControlDynamicCapsSetIfNull(UIntN domainIndex);
    PowerControlDynamicCapsSet getAdjustedDynamicCapsBasedOnConfigTdpMaxLimit(
        const PowerControlDynamicCapsSet& capsSet);

    ParticipantServicesInterface* m_participantServicesInterface;
    PowerControlDynamicCapsSet* m_powerControlDynamicCaps;
    PowerControlStatusSet* m_powerControlStatusSet;
    std::map<PowerControlType::Type, Bool> m_canProgramPowerLimit;
    std::map<PowerControlType::Type, Bool> m_canProgramTimeWindow;
};
