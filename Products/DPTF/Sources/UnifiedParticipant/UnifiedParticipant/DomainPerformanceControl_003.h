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

class DomainPerformanceControl_003 final : public DomainPerformanceControlInterface,
    public ComponentExtendedInterface, public ConfigTdpDataSyncInterface
{
public:

    DomainPerformanceControl_003(ParticipantServicesInterface* participantServicesInterface);
    ~DomainPerformanceControl_003(void);

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
    DomainPerformanceControl_003(const DomainPerformanceControl_003& rhs);
    DomainPerformanceControl_003& operator=(const DomainPerformanceControl_003& rhs);

    ParticipantServicesInterface* m_participantServicesInterface;

    void createPerformanceControlSet(UIntN domainIndex);
    void verifyPerformanceControlIndex(UIntN performanceControlIndex);
    void checkAndCreateControlStructures(UIntN domainIndex);
    void createPerformanceControlDynamicCaps(UIntN domainIndex);
    void createPerformanceControlStaticCaps(void);

    PerformanceControlSet* m_performanceControlSet;
    PerformanceControlDynamicCaps* m_performanceControlDynamicCaps;
    PerformanceControlStaticCaps* m_performanceControlStaticCaps;
    UIntN m_currentPerformanceControlIndex;
};
