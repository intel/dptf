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

#include "DomainPerformanceControlInterface.h"
#include "ComponentExtendedInterface.h"
#include "ConfigTdpDataSyncInterface.h"
#include "ParticipantServicesInterface.h"
#include "BinaryParse.h"
#include "DptfMemory.h"

// Generic Participant Performance Controls

class DomainPerformanceControl_001 final : public DomainPerformanceControlInterface,
    public ComponentExtendedInterface, public ConfigTdpDataSyncInterface
{
public:

    DomainPerformanceControl_001(ParticipantServicesInterface* participantServicesInterface);
    ~DomainPerformanceControl_001(void);

    // DomainPerformanceControlInterface
    virtual PerformanceControlStaticCaps getPerformanceControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PerformanceControlStatus getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PerformanceControlSet getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;

    // ConfigTdpDataSyncInterface
    virtual void updateBasedOnConfigTdpInformation(UIntN participantIndex, UIntN domainIndex,
        ConfigTdpControlSet configTdpControlSet, ConfigTdpControlStatus configTdpControlStatus);

private:

    // hide the copy constructor and = operator
    DomainPerformanceControl_001(const DomainPerformanceControl_001& rhs);
    DomainPerformanceControl_001& operator=(const DomainPerformanceControl_001& rhs);

    void createPerformanceControlStaticCapsIfNeeded();
    void createPerformanceControlDynamicCapsIfNeeded(UIntN domainIndex);
    void createPerformanceControlSetIfNeeded(UIntN domainIndex);
    void verifyPerformanceControlIndex(UIntN performanceControlIndex);
    void checkAndCreateControlStructures(UIntN domainIndex);

    ParticipantServicesInterface* m_participantServicesInterface;
    UIntN m_currentPerformanceControlIndex;
    PerformanceControlStaticCaps* m_performanceControlStaticCaps;
    PerformanceControlDynamicCaps* m_performanceControlDynamicCaps;
    PerformanceControlSet* m_performanceControlSet;
};
