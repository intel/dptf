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
#include "Dptf.h"
#include <vector>
#include "LpmAppSpecificEntry.h"
#include "LpmSet.h"
#include "LpmConfigurationHelper.h"

using namespace std;

class dptf_export LpmConfiguration final : public LpmConfigurationHelper
{
public:

    LpmConfiguration(
        LpmConfigurationVersion::Type version,
        const vector<LpmEntry>& lpmEntriesStandardConfiguration,
        const vector<LpmSet>& lpmSets,
        const vector<AppSpecificEntry>& appSpecificEntries);
    virtual ~LpmConfiguration();

    LpmConfigurationVersion::Type version(void) const;
    void throwIfNotValidVersion(LpmConfigurationVersion::Type version) const;
    void throwIfNotValidVersion() const;

    std::vector<LpmEntry> getLpmEntriesForStandardConfiguration(void) const;
    std::vector<AppSpecificEntry> getAppSpecificEntries(void) const;
    std::vector<LpmSet> getLpmSets(void) const;
    std::vector<LpmEntry> getLpmEntriesForAppName(string appName) const;
    std::vector<LpmEntry> getLpmEntriesForLpmSetIndex(UIntN lpmSetIndex) const;

    void associateDomain(UIntN participantIndex, UIntN domainIndex, DomainType::Type domainType);
    void disassociateDomain(UIntN participantIndex, UIntN domainIndex);

    void associateParticipantWithAcpiScope(std::string acpiScope, UIntN participantIndex);
    void associateParticipantWithDomainType(DomainType::Type domainType, UIntN participantIndex);
    void disassociateParticipant(UIntN participantIndex);

private:
    LpmConfigurationVersion::Type m_version;
    vector<LpmEntry> m_lpmEntriesStandardConfiguration;
    vector<LpmSet> m_lpmSets;
    vector<AppSpecificEntry> m_appSpecificEntries;

    UIntN getLpmSetIndex(string appName) const;
    LpmSet getLpmSet(string appName) const;
    LpmSet getLpmSetFromIndex(UIntN lpmSetIndex) const;


};