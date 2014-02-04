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

#include "DomainCoreControlInterface.h"
#include "ComponentExtendedInterface.h"
#include "ParticipantServicesInterface.h"
#include "BinaryParse.h"
#include "DptfMemory.h"

//
// Core Controls (Processor)
//

class DomainCoreControl_001 final : public DomainCoreControlInterface,
    public ComponentExtendedInterface
{
public:

    DomainCoreControl_001(ParticipantServicesInterface* participantServicesInterface);
    ~DomainCoreControl_001(void);

    // DomainCoreControlInterface
    virtual CoreControlStaticCaps getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual CoreControlLpoPreference getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex) override final;
    virtual CoreControlStatus getCoreControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setActiveCoreControl(UIntN participantIndex, UIntN domainIndex, const CoreControlStatus& coreControlStatus) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;

private:

    // hide the copy constructor and = operator
    DomainCoreControl_001(const DomainCoreControl_001& rhs);
    DomainCoreControl_001& operator=(const DomainCoreControl_001& rhs);

    ParticipantServicesInterface* m_participantServicesInterface;

    // Functions
    void createCoreControlStaticCapsIfNeeded(UIntN domainIndex);
    void createCoreControlDynamicCapsIfNeeded(UIntN domainIndex);
    void createCoreControlLpoPreferenceIfNeeded(UIntN domainIndex);
    void verifyCoreControlStatus(UIntN domainIndex, const CoreControlStatus& coreControlStatus);

    // Vars (external)
    CoreControlStaticCaps* m_coreControlStaticCaps;
    CoreControlDynamicCaps* m_coreControlDynamicCaps;
    CoreControlLpoPreference* m_coreControlLpoPreference;
    CoreControlStatus* m_coreControlStatus;
};