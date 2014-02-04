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

#include "LpmConfigurationReader.h"
#include "LpmConfiguration.h"
#include "LpmMode.h"
#include "ParticipantProxy.h"
#include "LpmConfigurationHelper.h"


class dptf_export LpmConfigurationProxy final : public LpmConfigurationHelper
{
public:
    LpmConfigurationProxy(void);
    ~LpmConfigurationProxy(void);

    void setForegroundApp(string appName);
    LpmConfigurationVersion::Type version(void) const;
    void throwIfNotValidVersion(LpmConfigurationVersion::Type version) const;
    void throwIfNotValidVersion() const;
    LpmMode::Type lpmMode(void) const;
    void setLpmMode(UIntN);
    void setLpmModeOsControlled(UIntN value);
    LpmMode::Boss lpmModeBoss(void) const;
    vector<LpmEntry> getLpmEntries(void) const;
    vector<LpmEntry> getLpmEntriesForTarget(ParticipantProxy& participant) const;
    Bool currentLpmEntriesValid(void);
    void createLpmConfiguration(void);
    void updateCurrentLpmEntries(void);
    void updateCurrentLpmEntriesStandardMode(void);
    void updateCurrentLpmEntriesDppeMode(void);
    void updateCurrentLpmEntriesAppMode(void);
    void associateDomainWithLpmConfiguration(ParticipantProxy& participant);
    void associateDomainWithLpmConfiguration(ParticipantProxy& participant, UIntN domainIndex);
    void disassociateDomainFromLpmConfiguration(UIntN participantIndex, UIntN domainIndex);
    void associateParticipantWithLpmConfiguration(ParticipantProxy& participant);
    void disassociateParticipantFromLpmConfiguration(ParticipantProxy& participant);
    void disassociateDomainFromLpmConfiguration(ParticipantProxy& participant, UIntN domainIndex);
    Bool isParticipantTargetDevice(UIntN participantIndex);
    void setLpmSetIndex(UIntN lpmSetIndex);
    UIntN getLpmSetIndex(void) const;
    Bool isBiosControlled(void) const;
    Bool isDppeControlled(void) const;
    Bool isAppSpecificMode(void) const;
    Bool isStandardMode(void) const;
    void updateLpmMode(void);
    string foregroundAppName(void) const;
    vector<LpmEntry> getLpmStdConfiguration() const;
    vector<LpmSet> getLpmSets() const;
    vector<AppSpecificEntry> getAppSpecificEntries() const;
    void setAppliedControl(LpmEntry lpmEntry, UIntN value, string units);

private:
    // Make the '=' operator and conpy constructor private.
    LpmConfigurationProxy& operator=(const LpmConfigurationProxy& rhs);
    LpmConfigurationProxy(const LpmConfigurationProxy& rhs);

    vector<LpmEntry>                        m_currentLpmEntries;
    string                                  m_currentForegroundApp;
    UIntN                                   m_currentLpmSetIndex; // Only set by Dppe mode.
    LpmConfigurationReader*                 m_configReader;
    LpmConfiguration                        m_lpmConfiguration;
    LpmMode::Boss                           m_lpmModeBoss;
    LpmMode::Type                           m_lpmMode[LpmMode::MaxBoss];

    void allocateConfigReader(LpmConfigurationVersion::Type version);
    void destroyConfigReader(void);
    void cleanUp();
};


