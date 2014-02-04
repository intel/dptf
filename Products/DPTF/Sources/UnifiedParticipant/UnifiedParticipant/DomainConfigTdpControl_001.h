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

#include "DomainConfigTdpControlInterface.h"
#include "ComponentExtendedInterface.h"
#include "ParticipantServicesInterface.h"
#include "PerformanceControl.h"
#include "BinaryParse.h"
#include <sstream>
#include "DptfMemory.h"

class DomainConfigTdpControl_001 final : public DomainConfigTdpControlInterface,
    public ComponentExtendedInterface
{
public:

    DomainConfigTdpControl_001(ParticipantServicesInterface* participantServicesInterface);
    ~DomainConfigTdpControl_001(void);

    // DomainConfigTdpControlInterface
    virtual ConfigTdpControlDynamicCaps getConfigTdpControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual ConfigTdpControlStatus getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual ConfigTdpControlSet getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setConfigTdpControl(UIntN participantIndex, UIntN domainIndex, UIntN configTdpControlIndex) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;

private:

    // hide the copy constructor and = operator
    DomainConfigTdpControl_001(const DomainConfigTdpControl_001& rhs);
    DomainConfigTdpControl_001& operator=(const DomainConfigTdpControl_001& rhs);

    ParticipantServicesInterface* m_participantServicesInterface;

    // Functions
    void checkHWConfigTdpSupport(std::vector<ConfigTdpControl> controls, UIntN domainIndex);
    Bool isLockBitSet(UIntN domainIndex);
    UIntN getLevelCount(UIntN domainIndex);

    void createConfigTdpControlSet(UIntN domainIndex);
    void createConfigTdpControlDynamicCaps(UIntN domainIndex);
    //UInt32 translateTdpRatioToPstate(UInt32 maxNonTurboRatio, UIntN domainIndex);
    void verifyConfigTdpControlIndex(UIntN configTdpControlIndex);
    void checkAndCreateControlStructures(UIntN domainIndex);

    // Vars (external)
    ConfigTdpControlDynamicCaps* m_configTdpControlDynamicCaps;
    ConfigTdpControlSet* m_configTdpControlSet;
    ConfigTdpControlStatus* m_configTdpControlStatus;

    // Vars (internal)
    UInt32 m_configTdpLevelsAvailable;
    UInt32 m_currentConfigTdpControlId;
    Bool m_configTdpLock;
};