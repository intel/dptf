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
#include "LpmEntry.h"
#include "LpmConfigurationHelper.h"
#include "XmlNode.h"

using namespace std;

class dptf_export LpmSet final : public LpmConfigurationHelper
{
public:
    LpmSet(UIntN lpmSetIndex, vector<LpmEntry> lpmEntries);
    ~LpmSet();


    UIntN lpmSetIndex() const;
    vector<LpmEntry> lpmEntries(void) const;
    void associateDomain(
        UIntN participantIndex,
        UIntN domainIndex,
        DomainType::Type domainType
        );
    void disassociateDomain(UIntN participantIndex, UIntN domainIndex);
    void associateParticipantWithAcpiScope(
        std::string acpiScope,
        UIntN participantIndex
        );
    void associateParticipantWithDomainType(
        DomainType::Type domainType,
        UIntN participantIndex 
        );
    void disassociateParticipant(UIntN participantIndex);

    XmlNode* getXml() const;

private:
    vector<LpmEntry> m_lpmEntries;
    UIntN m_lpmSetIndex;

};

