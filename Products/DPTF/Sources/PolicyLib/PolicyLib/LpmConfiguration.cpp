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

#include "LpmConfiguration.h"

LpmConfiguration::LpmConfiguration(
    LpmConfigurationVersion::Type version,
    const vector<LpmEntry>& lpmEntriesStandardConfiguration,
    const vector<LpmSet>& lpmSets,
    const vector<AppSpecificEntry>& appSpecificEntries) 
    :m_version(version),
    m_lpmEntriesStandardConfiguration(lpmEntriesStandardConfiguration),
    m_lpmSets(lpmSets),
    m_appSpecificEntries(appSpecificEntries)
{

}

LpmConfiguration::~LpmConfiguration()
{

}

void LpmConfiguration::throwIfNotValidVersion(LpmConfigurationVersion::Type version) const
{
    if (m_version != version)
    {
        throw dptf_exception("Invalid LPM table version");
    }
}

void LpmConfiguration::throwIfNotValidVersion() const
{
    if ((m_version != LpmConfigurationVersion::v0) &&
        (m_version != LpmConfigurationVersion::v1))
    {
        throw dptf_exception("Invalid LPM table version");
    }
}

void LpmConfiguration::associateDomain(
    UIntN participantIndex,
    UIntN domainIndex,
    DomainType::Type domainType )
{
    throwIfNotValidVersion();

    // Associate for all the standard config entries if they exist.
    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithParticipantIndex(participantIndex, m_lpmEntriesStandardConfiguration);
    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntriesStandardConfiguration[row].associateDomain(
            participantIndex,
            domainIndex,
            domainType);
    }

    // Associate for all the LPM set entries if they exist.
    for (UIntN lpmSetRow = 0; lpmSetRow < m_lpmSets.size(); lpmSetRow++)
    {
         m_lpmSets[lpmSetRow].associateDomain(participantIndex, domainIndex, domainType);
    }
}

void LpmConfiguration::disassociateDomain(UIntN participantIndex, UIntN domainIndex)
{
    throwIfNotValidVersion();

    // Disassociate for all the standard config entries if they exist.
    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithParticipantIndex(participantIndex, m_lpmEntriesStandardConfiguration);
    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntriesStandardConfiguration[row].disassociateDomain(participantIndex, domainIndex);
    }

    // Disassociate for all the LPM set entries if they exist.
    for (UIntN lpmSetRow = 0; lpmSetRow < m_lpmSets.size(); lpmSetRow++)
    {
        m_lpmSets[lpmSetRow].disassociateDomain(participantIndex, domainIndex);
    }
}

void LpmConfiguration::associateParticipantWithAcpiScope(
    std::string acpiScope,
    UIntN participantIndex)
{
    throwIfNotValidVersion(LpmConfigurationVersion::v1);

    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithAcpiScope(acpiScope, m_lpmEntriesStandardConfiguration);

    // Associate for all the standard config entries if they exist.
    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntriesStandardConfiguration[row].associateParticipantWithAcpiScope(
            acpiScope, participantIndex);
    }

    // Associate for all the LPM set entries if they exist.
    for (UIntN lpmSetRow = 0; lpmSetRow < m_lpmSets.size(); lpmSetRow++)
    {
         m_lpmSets[lpmSetRow].associateParticipantWithAcpiScope(acpiScope, participantIndex);
    }
}

void LpmConfiguration::associateParticipantWithDomainType(
    DomainType::Type domainType,
    UIntN participantIndex )
{
    // This should only be called in v0. This might not work properly if called for v1.
    throwIfNotValidVersion(LpmConfigurationVersion::v0);

    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithDomainType(domainType, m_lpmEntriesStandardConfiguration);

    // Associate for all the standard config entries if they exist.
    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntriesStandardConfiguration[row].associateParticipantWithDomainType(
            domainType, participantIndex);
    }

    // Associate for all the LPM set entries if they exist.
    for (UIntN lpmSetRow = 0; lpmSetRow < m_lpmSets.size(); lpmSetRow++)
    {
        m_lpmSets[lpmSetRow].associateParticipantWithDomainType(domainType, participantIndex);
    }

}

void LpmConfiguration::disassociateParticipant(UIntN participantIndex)
{
    throwIfNotValidVersion();

    // Disassociate for all the standard config entries if they exist.
    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithParticipantIndex(participantIndex, m_lpmEntriesStandardConfiguration);
    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntriesStandardConfiguration[row].disassociateParticipant(participantIndex);
    }

    // Disassociate for all the LPM set entries if they exist.
    for (UIntN lpmSetRow = 0; lpmSetRow < m_lpmSets.size(); lpmSetRow++)
    {
         m_lpmSets[lpmSetRow].disassociateParticipant(participantIndex);
    }
}


UIntN LpmConfiguration::getLpmSetIndex(string appName) const
{
    for (auto entry = m_appSpecificEntries.begin(); entry != m_appSpecificEntries.end(); entry++)
    {
        if (entry->containsAppName(appName) == true)
        {
            return (entry->lpmSetIndex());
        }
    }

    return Constants::Invalid;
}

LpmSet LpmConfiguration::getLpmSet(string appName) const
{
    UIntN lpmSetIndex = getLpmSetIndex(appName);

    return getLpmSetFromIndex(lpmSetIndex);
}

std::vector<LpmEntry> LpmConfiguration::getLpmEntriesForAppName(string appName) const
{
    LpmSet lpmSet = getLpmSet(appName);

    return (lpmSet.lpmEntries());
}

std::vector<LpmEntry> LpmConfiguration::getLpmEntriesForStandardConfiguration(void) const
{
    return (m_lpmEntriesStandardConfiguration);
}

std::vector<LpmEntry> LpmConfiguration::getLpmEntriesForLpmSetIndex(UIntN lpmSetIndex) const
{
    LpmSet lpmSet = getLpmSetFromIndex(lpmSetIndex);

    return (lpmSet.lpmEntries());
}

LpmConfigurationVersion::Type LpmConfiguration::version(void) const
{
    return m_version;
}

LpmSet LpmConfiguration::getLpmSetFromIndex(UIntN lpmSetIndex) const
{
    for (auto entry = m_lpmSets.begin(); entry != m_lpmSets.end(); entry++)
    {
        if (entry->lpmSetIndex() == lpmSetIndex)
        {
            return *entry;
        }
    }

    return LpmSet(Constants::Invalid, vector<LpmEntry>());
}

std::vector<AppSpecificEntry> LpmConfiguration::getAppSpecificEntries(void) const
{
    return m_appSpecificEntries;
}

std::vector<LpmSet> LpmConfiguration::getLpmSets(void) const
{
    return m_lpmSets;
}
