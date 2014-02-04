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

#include "DomainDisplayControlInterface.h"
#include "ComponentExtendedInterface.h"
#include "ParticipantServicesInterface.h"
#include "DptfMemory.h"
#include "BinaryParse.h"

//
// Implements the regular display controls, v001.
//

class DomainDisplayControl_001 final : public DomainDisplayControlInterface,
    public ComponentExtendedInterface
{
public:

    DomainDisplayControl_001(ParticipantServicesInterface* participantServicesInterface);
    ~DomainDisplayControl_001(void);

    virtual DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual DisplayControlStatus getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual DisplayControlSet getDisplayControlSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex,
        Bool isOverridable) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;

private:

    // hide the copy constructor and = operator
    DomainDisplayControl_001(const DomainDisplayControl_001& rhs);
    DomainDisplayControl_001& operator=(const DomainDisplayControl_001& rhs);

    ParticipantServicesInterface* m_participantServicesInterface;

    // Functions
    void initializeDataStructures(void);
    void createDisplayControlDynamicCaps(UIntN domainIndex);
    void createDisplayControlSet(UIntN domainIndex);
    void verifyDisplayControlIndex(UIntN displayControlIndex);
    void checkAndCreateControlStructures(UIntN domainIndex);

    // Vars (external)
    DisplayControlDynamicCaps* m_displayControlDynamicCaps;
    DisplayControlSet* m_displayControlSet;
    DisplayControlStatus* m_displayControlStatus;

    // Vars (internal)
    UIntN m_currentDisplayControlIndex;
    std::vector<DisplayControl> processDisplayControlSetData(std::vector<DisplayControl> controls);
};