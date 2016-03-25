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
#include "BinaryParse.h"
#include "DomainConfigTdpControlBase.h"

class DomainConfigTdpControl_001 : public DomainConfigTdpControlBase
{
public:

    DomainConfigTdpControl_001(UIntN participantIndex, UIntN domainIndex, 
        ParticipantServicesInterface* participantServicesInterface);
    ~DomainConfigTdpControl_001(void);

    // DomainConfigTdpControlInterface
    virtual ConfigTdpControlDynamicCaps getConfigTdpControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
    virtual ConfigTdpControlStatus getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex) override;
    virtual ConfigTdpControlSet getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex) override;
    virtual void setConfigTdpControl(UIntN participantIndex, UIntN domainIndex, UIntN configTdpControlIndex) override;

    // ParticipantActivityLoggingInterface
    virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override;
    virtual std::string getName(void) override;
    virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:

    // hide the copy constructor and = operator
    DomainConfigTdpControl_001(const DomainConfigTdpControl_001& rhs);
    DomainConfigTdpControl_001& operator=(const DomainConfigTdpControl_001& rhs);

    void checkHWConfigTdpSupport(std::vector<ConfigTdpControl> controls, UIntN domainIndex);
    Bool isLockBitSet(UIntN domainIndex);
    UIntN getLevelCount(UIntN domainIndex);
    void createConfigTdpControlSet(UIntN domainIndex);
    void createConfigTdpControlDynamicCaps(UIntN domainIndex);
    void verifyConfigTdpControlIndex(UIntN configTdpControlIndex);
    void checkAndCreateControlStructures(UIntN domainIndex);

    ConfigTdpControlDynamicCaps* m_configTdpControlDynamicCaps;
    ConfigTdpControlSet* m_configTdpControlSet;
    ConfigTdpControlStatus* m_configTdpControlStatus;
    UInt32 m_configTdpLevelsAvailable;
    UInt32 m_currentConfigTdpControlId;
    Bool m_configTdpLock;
};