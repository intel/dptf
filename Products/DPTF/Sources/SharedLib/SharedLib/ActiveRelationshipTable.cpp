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

#include "ActiveRelationshipTable.h"
#include <set>


ActiveRelationshipTable::ActiveRelationshipTable(const std::vector<ActiveRelationshipTableEntry>& entries)
    : RelationshipTableBase(),
    m_entries(entries)
{
}

ActiveRelationshipTable::~ActiveRelationshipTable()
{
}

UIntN ActiveRelationshipTable::getNumberOfEntries(void) const
{
    return (UIntN)m_entries.size();
}

RelationshipTableEntryBase* ActiveRelationshipTable::getEntry(UIntN index) const
{
    return (RelationshipTableEntryBase*)(&m_entries.at(index));
}

const ActiveRelationshipTableEntry& ActiveRelationshipTable::operator[](UIntN index) const
{
    return m_entries.at(index);
}

std::vector<ActiveRelationshipTableEntry> ActiveRelationshipTable::getEntriesForTarget(UIntN target)
{
    std::vector<ActiveRelationshipTableEntry> entries;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].targetDeviceIndex() == target)
        {
            entries.push_back(m_entries[row]);
        }
    }
    return entries;
}

const ActiveRelationshipTableEntry& ActiveRelationshipTable::getEntryForTargetAndSource(
    UIntN target, UIntN source) const
{
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if ((m_entries[row].targetDeviceIndex() == target) &&
            (m_entries[row].sourceDeviceIndex() == source))
        {
            return m_entries[row];
        }
    }
    throw dptf_exception("No match found for target and source in ART.");
}

std::vector<ActiveRelationshipTableEntry> ActiveRelationshipTable::getEntriesForSource(UIntN source)
{
    std::vector<ActiveRelationshipTableEntry> entries;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].sourceDeviceIndex() == source)
        {
            entries.push_back(m_entries[row]);
        }
    }
    return entries;
}

std::vector<UIntN> ActiveRelationshipTable::getAllSources(void) const
{
    std::set<UIntN> sources;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].sourceDeviceIndex() != Constants::Invalid)
        {
            sources.insert(m_entries[row].sourceDeviceIndex());
        }
    }
    return std::vector<UIntN>(sources.begin(), sources.end());
}

std::vector<UIntN> ActiveRelationshipTable::getAllTargets(void) const
{
    std::set<UIntN> targets;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].targetDeviceIndex() != Constants::Invalid)
        {
            targets.insert(m_entries[row].targetDeviceIndex());
        }
    }
    return std::vector<UIntN>(targets.begin(), targets.end());
}

XmlNode* ActiveRelationshipTable::getXml()
{
    XmlNode* status = XmlNode::createWrapperElement("art");
    for (auto entry = m_entries.begin(); entry != m_entries.end(); entry++)
    {
        status->addChild(entry->getXml());
    }
    return status;
}