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

#include "LpmConfigurationProxy.h"
#include "LpmConfigurationReaderV0.h"
#include "LpmConfigurationReaderV1.h"

LpmConfigurationProxy::LpmConfigurationProxy(void)
    :m_lpmModeBoss(LpmMode::Bios),
     m_configReader(NULL),
     m_currentForegroundApp(""),
     m_currentLpmSetIndex(Constants::Invalid),
     m_lpmConfiguration(LpmConfigurationVersion::vInvalid,
     vector<LpmEntry>(), vector<LpmSet>(), vector<AppSpecificEntry>())
{
    for (UIntN index = 0; index < LpmMode::MaxBoss; index++)
    {
        m_lpmMode[index] = LpmMode::Disabled;
    }
}

LpmConfigurationProxy::~LpmConfigurationProxy(void)
{
    destroyConfigReader();
}

void LpmConfigurationProxy::setForegroundApp(string appName)
{
    m_currentForegroundApp = appName;
}

LpmConfigurationVersion::Type LpmConfigurationProxy::version(void) const
{
    return m_lpmConfiguration.version();
}

void LpmConfigurationProxy::throwIfNotValidVersion(LpmConfigurationVersion::Type version) const
{
    m_lpmConfiguration.throwIfNotValidVersion(version);
}

void LpmConfigurationProxy::throwIfNotValidVersion() const
{
    m_lpmConfiguration.throwIfNotValidVersion();
}

Bool LpmConfigurationProxy::currentLpmEntriesValid(void)
{
    return (m_currentLpmEntries.empty() == false);
}

LpmMode::Type LpmConfigurationProxy::lpmMode(void) const
{
    return m_lpmMode[m_lpmModeBoss];
}

vector<LpmEntry> LpmConfigurationProxy::getLpmEntries(void) const
{
    return m_currentLpmEntries;
}

vector<LpmEntry> LpmConfigurationProxy::getLpmEntriesForTarget(ParticipantProxy& participant) const
{
    vector<LpmEntry> lpmEntriesForTarget;
    vector<UIntN> domainIndexes = participant.getDomainIndexes();

    for (UIntN row = 0; row < domainIndexes.size(); row++)
    {
        for (auto entry = m_currentLpmEntries.begin(); entry != m_currentLpmEntries.end(); entry++)
        {
            if ((entry->targetDeviceIndex() == participant.getIndex()) &&
                (entry->domainIndex() == domainIndexes[row]))
            {
                lpmEntriesForTarget.push_back(*entry);
            }
        }
    }

    return lpmEntriesForTarget;
}

void LpmConfigurationProxy::updateLpmMode()
{
    UInt32 cLpmSetting;

    //
    // Get the current Low Power Mode Setting. It will return one of the following:
    //
    // 0 - Disable LPM
    // 1 - Enable LPM in standard mode
    // 2 - Enable LPM in application specific mode
    // 3 - Use OS provided mechanism to get values.
    //
    // If this isn't available we will not load the policy.
    //

    cLpmSetting = getPolicyServices().platformConfigurationData->getLpmMode();
    if (cLpmSetting < 3)
    {
        m_lpmModeBoss = LpmMode::Bios;
        m_lpmMode[m_lpmModeBoss] = (LpmMode::Type)cLpmSetting;
    }
    else if (cLpmSetting == 3)
    {
        m_lpmModeBoss = LpmMode::Dppe;
    }
    else
    {
        throw dptf_exception("Invalid cLPM value (" + to_string(cLpmSetting));
    }
}

void LpmConfigurationProxy::createLpmConfiguration(void)
{
    vector<LpmEntry> lpmEntriesStandardConfiguration;
    LpmConfigurationVersion::Type version = LpmConfigurationVersion::vInvalid;

    // TODO: Check if we really need the cleanup. The UI shows the old value LPMSet
    // value. But we still need that value.
    //cleanUp();
    try
    {
        LpmTable lpmTable = getPolicyServices().platformConfigurationData->getLpmTable();
        version = lpmTable.version();
        allocateConfigReader(version);

        if (version == LpmConfigurationVersion::v0)
        {
            lpmEntriesStandardConfiguration = m_configReader->readStandardEntries();
        }
        else
        {
            lpmEntriesStandardConfiguration = lpmTable.lpmEntries();
        }
    }
    catch(dptf_exception& e)
    {
        string errorMsg = e.what();
        m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF, e.what(), Constants::Invalid));
        return;
    }
    catch(...)
    {
        m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF,"Allocation failed for config reader",
            Constants::Invalid));
        return;
    }

    LpmConfiguration lpmConfiguration(
        version,
        lpmEntriesStandardConfiguration,
        m_configReader->readLpmSets(),
        m_configReader->readAppSpecificEntries());
    m_lpmConfiguration = lpmConfiguration;
}


void LpmConfigurationProxy::updateCurrentLpmEntriesStandardMode(void)
{
    throwIfNotValidVersion();

    if (isStandardMode() == false)
    {
        m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF,
            "Invalid LPM Mode (" + to_string(lpmMode()) + ") expecting Std mode",
            Constants::Invalid));
        m_currentLpmEntries.clear();
        return;
    }

    m_currentLpmEntries = m_lpmConfiguration.getLpmEntriesForStandardConfiguration();
    if (m_currentLpmEntries.empty())
    {
        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "Got zero LPM enrties in updateCurrentLpmEntriesStandardMode()",
            Constants::Invalid));
    }

    return;
}

void LpmConfigurationProxy::updateCurrentLpmEntriesDppeMode(void)
{
    throwIfNotValidVersion();

    if (m_lpmConfiguration.version() == LpmConfigurationVersion::v0)
    {
        // Update the current entries with the standard configuration entries.
        m_currentLpmEntries = m_lpmConfiguration.getLpmEntriesForStandardConfiguration();
    }
    else
    {
        m_currentLpmEntries = m_lpmConfiguration.getLpmEntriesForLpmSetIndex(m_currentLpmSetIndex);
    }

    return;
}

void LpmConfigurationProxy::updateCurrentLpmEntriesAppMode(void)
{
    throwIfNotValidVersion();

    if (isAppSpecificMode() == false)
    {
       m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF,
            "Invalid LPM Mode (" + to_string(lpmMode()) + ") expecting App Specific",
            Constants::Invalid));
        m_currentLpmEntries.clear();
        return;
    }

    if (m_currentForegroundApp.empty())
    {
        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF, "Empty App name...", Constants::Invalid));
        m_currentLpmEntries.clear();
        return;
    }

    m_currentLpmEntries = m_lpmConfiguration.getLpmEntriesForAppName(m_currentForegroundApp);

    return;
}

void LpmConfigurationProxy::associateParticipantWithLpmConfiguration(ParticipantProxy& participant)
{
    throwIfNotValidVersion();

    if (m_lpmConfiguration.version() == LpmConfigurationVersion::v0)
    {
        vector<UIntN> domainIndexes = participant.getDomainIndexes();
        for (UIntN row = 0; row < domainIndexes.size(); row++)
        {
            m_lpmConfiguration.associateParticipantWithDomainType(
                participant[domainIndexes[row]].getDomainProperties().getDomainType(),
                participant.getIndex());

            // Associate current entries too if they are valid.
            vector<UIntN> lpmEntryRows = findLpmEntriesWithDomainType(
                participant[domainIndexes[row]].getDomainProperties().getDomainType(),
                getLpmEntries());
            for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
            {
                UIntN lpmRow = lpmEntryRows[lpmEntryRow];
                m_currentLpmEntries[lpmRow].associateParticipantWithDomainType(
                    participant[domainIndexes[row]].getDomainProperties().getDomainType(),
                    participant.getIndex());
            }
        }
    }
    else
    {
        m_lpmConfiguration.associateParticipantWithAcpiScope(
            participant.getParticipantProperties().getAcpiInfo().getAcpiScope(),
            participant.getIndex());

        // Associate current entries too if they are valid.
        vector<UIntN> lpmEntryRows = findLpmEntriesWithAcpiScope(
            participant.getParticipantProperties().getAcpiInfo().getAcpiScope(),
            getLpmEntries());
        for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
        {
            UIntN lpmRow = lpmEntryRows[lpmEntryRow];
            m_currentLpmEntries[lpmRow].associateParticipantWithAcpiScope(
                participant.getParticipantProperties().getAcpiInfo().getAcpiScope(),
                participant.getIndex());
        }
    }
}

void LpmConfigurationProxy::disassociateParticipantFromLpmConfiguration(ParticipantProxy& participant)
{
    throwIfNotValidVersion();

    m_lpmConfiguration.disassociateParticipant(participant.getIndex());

    // disassociate from the current lpm entries too.
    if (currentLpmEntriesValid())
    {
        for (auto entry = m_currentLpmEntries.begin(); entry != m_currentLpmEntries.end(); entry++)
        {
            entry->disassociateParticipant(participant.getIndex());
        }
    }
}

void LpmConfigurationProxy::disassociateDomainFromLpmConfiguration(
    ParticipantProxy& participant,
    UIntN domainIndex
    )
{
    throwIfNotValidVersion();

    m_lpmConfiguration.disassociateDomain(participant.getIndex(), domainIndex);

    // disassociate from the current lpm entries too.
    if (currentLpmEntriesValid())
    {
        for (auto entry = m_currentLpmEntries.begin(); entry != m_currentLpmEntries.end(); entry++)
        {
            entry->disassociateDomain(participant.getIndex(), domainIndex);
        }
    }
}

void LpmConfigurationProxy::associateDomainWithLpmConfiguration(ParticipantProxy& participant, UIntN domainIndex)
{
    throwIfNotValidVersion();

    if (m_lpmConfiguration.version() == LpmConfigurationVersion::v0)
    {
        //
        // For v0 we have not yet done participant association as the domain information
        // was not present during bindParticipant. So we do it here, the drawback is that 
        // for participants with multiple domains (e.g. proc) the participant association
        // will be done multiple (redundant) times. We can fix this later if it's an issue
        // by having a flag to indicate that the association has already been done.
        //
        associateParticipantWithLpmConfiguration(participant);
    }

    m_lpmConfiguration.associateDomain(
        participant.getIndex(),
        domainIndex,
        participant[domainIndex].getDomainProperties().getDomainType());

    // Associate current entries too if they are valid.
    vector<UIntN> lpmEntryRows =
        findLpmEntriesWithParticipantIndex(participant.getIndex(), getLpmEntries());
    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_currentLpmEntries[row].associateDomain(
            participant.getIndex(),
            domainIndex,
            participant[domainIndex].getDomainProperties().getDomainType());
    }
}

void LpmConfigurationProxy::associateDomainWithLpmConfiguration(ParticipantProxy& participant)
{
    throwIfNotValidVersion();

    // This needs to be called after the corresponding associate participant call.

    vector<UIntN> domainIndexes = participant.getDomainIndexes();
    for (UIntN row = 0; row < domainIndexes.size(); ++row)
    {
        m_lpmConfiguration.associateDomain(
            participant.getIndex(),
            domainIndexes[row],
            participant[domainIndexes[row]].getDomainProperties().getDomainType());

        // Associate current entries too if they are valid.
        vector<UIntN> lpmEntryRows =
            findLpmEntriesWithParticipantIndex(participant.getIndex(), getLpmEntries());
        for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
        {
            UIntN lpmRow = lpmEntryRows[lpmEntryRow];
            m_currentLpmEntries[lpmRow].associateDomain(
                participant.getIndex(),
                domainIndexes[row],
                participant[domainIndexes[row]].getDomainProperties().getDomainType());
        }
    }
}

Bool LpmConfigurationProxy::isParticipantTargetDevice(UIntN participantIndex)
{
    for (auto entry = m_currentLpmEntries.begin(); entry != m_currentLpmEntries.end(); entry++)
    {
        if (entry->targetDeviceIndex() == participantIndex)
        {
            return true;
        }
    }

    return false;
}

void LpmConfigurationProxy::updateCurrentLpmEntries(void)
{
    switch (lpmMode())
    {
        case LpmMode::Disabled:
            m_currentLpmEntries.clear();
            break;

        case LpmMode::Standard:
            if (lpmModeBoss() == LpmMode::Bios)
            {
                updateCurrentLpmEntriesStandardMode();
            }
            else if (lpmModeBoss() == LpmMode::Dppe)
            {
                updateCurrentLpmEntriesDppeMode();
            }
            else
            {
                m_currentLpmEntries.clear();
                throw dptf_exception("Invalid LPM mode boss - " + to_string(lpmModeBoss()));
            }
            break;

        case LpmMode::AppSpecific:
            updateCurrentLpmEntriesAppMode();
            break;

        default:
            throw dptf_exception("Invalid LPM mode - " + to_string(lpmMode()));
    }

    return;
}

void LpmConfigurationProxy::setLpmSetIndex(UIntN lpmSetIndex)
{
    m_currentLpmSetIndex = lpmSetIndex;
}

UIntN LpmConfigurationProxy::getLpmSetIndex(void) const
{
    return m_currentLpmSetIndex;
}

Bool LpmConfigurationProxy::isBiosControlled(void) const
{
    return (lpmModeBoss() == LpmMode::Bios);
}

Bool LpmConfigurationProxy::isDppeControlled(void) const
{
    return (lpmModeBoss() == LpmMode::Dppe);
}

Bool LpmConfigurationProxy::isAppSpecificMode(void) const
{
    return (lpmMode() == LpmMode::AppSpecific);
}

Bool LpmConfigurationProxy::isStandardMode(void) const
{
    return (lpmMode() == LpmMode::Standard);
}

LpmMode::Boss LpmConfigurationProxy::lpmModeBoss(void) const
{
    return m_lpmModeBoss;
}

std::string LpmConfigurationProxy::foregroundAppName(void) const
{
    return m_currentForegroundApp;
}

void LpmConfigurationProxy::setLpmMode(UIntN value)
{
    m_lpmMode[m_lpmModeBoss] = (LpmMode::Type)value;
}

void LpmConfigurationProxy::allocateConfigReader(LpmConfigurationVersion::Type version)
{
    //
    // TODO: configReader could be changed to a singleton if we do not need to support
    // dynamic version changes. v0 <-> v1
    //

    destroyConfigReader();
    if (version == LpmConfigurationVersion::v0)
    {
        m_configReader = new LpmConfigurationReaderV0(getPolicyServices());
    }
    else if (version == LpmConfigurationVersion::v1)
    {
        m_configReader = new LpmConfigurationReaderV1(getPolicyServices());
    }
    else
    {
        throw dptf_exception("Invalid LPM version - " + to_string(version));
    }
}

void LpmConfigurationProxy::destroyConfigReader(void)
{
    DELETE_MEMORY_TC(m_configReader);
}

vector<LpmEntry> LpmConfigurationProxy::getLpmStdConfiguration() const
{
    return m_lpmConfiguration.getLpmEntriesForStandardConfiguration();
}

vector<LpmSet> LpmConfigurationProxy::getLpmSets() const
{
    return m_lpmConfiguration.getLpmSets();
}

vector<AppSpecificEntry> LpmConfigurationProxy::getAppSpecificEntries() const
{
    return m_lpmConfiguration.getAppSpecificEntries();
}

void LpmConfigurationProxy::setAppliedControl(LpmEntry lpmEntry, UIntN value, string units)
{
    for (UIntN row = 0; row < m_currentLpmEntries.size(); row++)
    {
        if (m_currentLpmEntries[row] == lpmEntry)
        {
            m_currentLpmEntries[row].setAppliedControl(value);
            m_currentLpmEntries[row].setAppliedControlUnits(units);
        }
    }
}

void LpmConfigurationProxy::cleanUp()
{
    setLpmSetIndex(Constants::Invalid);
    setForegroundApp("");
}

void LpmConfigurationProxy::setLpmModeOsControlled(UIntN value)
{
    m_lpmMode[LpmMode::Dppe] = (LpmMode::Type)value;
}
