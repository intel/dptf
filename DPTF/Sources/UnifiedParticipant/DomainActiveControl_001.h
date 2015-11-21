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
#include "BinaryParse.h"
#include "DomainActiveControlBase.h"

class DomainActiveControl_001 : public DomainActiveControlBase
{
public:

    DomainActiveControl_001(UIntN participantIndex, UIntN domainIndex, 
        ParticipantServicesInterface* participantServicesInterface);
    ~DomainActiveControl_001(void);

    // DomainActiveControlInterface
    virtual ActiveControlStaticCaps getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override;
    virtual ActiveControlStatus getActiveControlStatus(UIntN participantIndex, UIntN domainIndex) override;
    virtual ActiveControlSet getActiveControlSet(UIntN participantIndex, UIntN domainIndex) override;
    virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, UIntN controlIndex) override;
    virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed) override;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override;
    virtual std::string getName(void) override;
    virtual XmlNode* getXml(UIntN domainIndex) override;

private:

    // hide the copy constructor and = operator
    DomainActiveControl_001(const DomainActiveControl_001& rhs);
    DomainActiveControl_001& operator=(const DomainActiveControl_001& rhs);

    // Functions
    void createActiveControlSet(UIntN domainIndex);
    void createActiveControlStaticCaps(UIntN domainIndex);
    void createActiveControlStatus(UIntN domainIndex);
    void checkAndCreateControlStructures(UIntN domainIndex);

    // Vars
    ActiveControlStaticCaps* m_activeControlStaticCaps;
    ActiveControlStatus* m_activeControlStatus;
    ActiveControlSet* m_activeControlSet;
};