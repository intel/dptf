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

#include "ThermalRelationshipTable.h"

ThermalRelationshipTable::ThermalRelationshipTable(const std::vector<ThermalRelationshipTableEntry>& entries)
    : RelationshipTableBase(),
    m_entries(entries)
{
}

ThermalRelationshipTable::~ThermalRelationshipTable()
{
}

UIntN ThermalRelationshipTable::getNumberOfEntries(void) const
{
    return (UIntN)m_entries.size();
}

const ThermalRelationshipTableEntry& ThermalRelationshipTable::operator[](UIntN rowIndex) const
{
    return m_entries.at(rowIndex);
}

std::vector<ThermalRelationshipTableEntry> ThermalRelationshipTable::getEntriesForTarget(UIntN targetIndex)
{
    std::vector<ThermalRelationshipTableEntry> entries;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].targetDeviceIndex() == targetIndex)
        {
            entries.push_back(m_entries[row]);
        }
    }
    return entries;
}

std::vector<ThermalRelationshipTableEntry> ThermalRelationshipTable::getEntriesForSource(UIntN sourceIndex)
{
    std::vector<ThermalRelationshipTableEntry> entries;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].sourceDeviceIndex() == sourceIndex)
        {
            entries.push_back(m_entries[row]);
        }
    }
    return entries;
}

const ThermalRelationshipTableEntry& ThermalRelationshipTable::getEntryForTargetAndSource(
    UIntN targetIndex, UIntN sourceIndex) const
{
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if ((m_entries[row].targetDeviceIndex() == targetIndex) &&
            (m_entries[row].sourceDeviceIndex() == sourceIndex))
        {
            return m_entries[row];
        }
    }
    throw dptf_exception("No match found for target and source in TRT.");
}

RelationshipTableEntryBase* ThermalRelationshipTable::getEntry(UIntN index) const
{
    return (RelationshipTableEntryBase*)(&m_entries.at(index));
}

UInt32 ThermalRelationshipTable::getMinimumSamplePeriodForSource(UIntN sourceIndex)
{
    UInt32 minimumSamplePeriod = Constants::Invalid;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].sourceDeviceIndex() == sourceIndex)
        {
            if (m_entries[row].thermalSamplingPeriod() < minimumSamplePeriod)
            {
                minimumSamplePeriod = m_entries[row].thermalSamplingPeriod();
            }
        }
    }
    return minimumSamplePeriod;
}

UInt32 ThermalRelationshipTable::getShortestSamplePeriodForTarget(UIntN target)
{
    UInt32 shortestSamplePeriod(Constants::Invalid);
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].targetDeviceIndex() == target)
        {
            if (m_entries[row].thermalSamplingPeriod() < shortestSamplePeriod)
            {
                shortestSamplePeriod = m_entries[row].thermalSamplingPeriod();
            }
        }
    }

    return shortestSamplePeriod;
}

XmlNode* ThermalRelationshipTable::getXml()
{
    XmlNode* status = XmlNode::createWrapperElement("trt");
    for (auto entry = m_entries.begin(); entry != m_entries.end(); entry++)
    {
        status->addChild(entry->getXml());
    }
    return status;
}