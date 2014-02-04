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

#include "LpmSet.h"
#include "StatusFormat.h"

using namespace StatusFormat;


LpmSet::LpmSet(UIntN lpmSetIndex, vector<LpmEntry> lpmEntries)
    :m_lpmSetIndex(lpmSetIndex),
     m_lpmEntries(lpmEntries)
{

}

LpmSet::~LpmSet()
{

}

UIntN LpmSet::lpmSetIndex() const
{
    return m_lpmSetIndex;
}

vector<LpmEntry> LpmSet::lpmEntries(void) const
{
    return m_lpmEntries;
}

void LpmSet::associateDomain(
    UIntN participantIndex,
    UIntN domainIndex,
    DomainType::Type domainType
    )
{
    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithParticipantIndex(participantIndex, m_lpmEntries);
    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntries[row].associateDomain(participantIndex, domainIndex, domainType);
    }
}

void LpmSet::disassociateDomain(UIntN participantIndex, UIntN domainIndex)
{
    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithParticipantIndex(participantIndex, m_lpmEntries);
    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntries[row].disassociateDomain(participantIndex, domainIndex);
    }
}

void LpmSet::associateParticipantWithAcpiScope(string acpiScope, UIntN participantIndex)
{
    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithAcpiScope(acpiScope, m_lpmEntries);

    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntries[row].associateParticipantWithAcpiScope(acpiScope, participantIndex);
    }
}

void LpmSet::associateParticipantWithDomainType(
    DomainType::Type domainType,
    UIntN participantIndex
    )
{
    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithDomainType(domainType, m_lpmEntries);

    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntries[row].associateParticipantWithDomainType(domainType, participantIndex);
    }
}

void LpmSet::disassociateParticipant(UIntN participantIndex)
{
    vector<UIntN> lpmEntryRows = 
        findLpmEntriesWithParticipantIndex(participantIndex, m_lpmEntries);
    for (UIntN lpmEntryRow = 0; lpmEntryRow < lpmEntryRows.size(); lpmEntryRow++)
    {
        UIntN row = lpmEntryRows[lpmEntryRow];
        m_lpmEntries[row].disassociateParticipant(participantIndex);
    }
}

XmlNode* LpmSet::getXml() const
{
    XmlNode* lpmSetEntryRoot = XmlNode::createWrapperElement("lpmset_entry");

    lpmSetEntryRoot->addChild(
        XmlNode::createDataElement("lpmset_index", friendlyValue(lpmSetIndex())));

    XmlNode* lpmEntriesNode = XmlNode::createWrapperElement("lpm_entries");
    for (auto entry = m_lpmEntries.begin(); entry != m_lpmEntries.end(); entry++)
    {
        XmlNode* lpmEntryNode = entry->getXml();
        lpmEntriesNode->addChild(lpmEntryNode);
    }
    lpmSetEntryRoot->addChild(lpmEntriesNode);

    return lpmSetEntryRoot;
}
