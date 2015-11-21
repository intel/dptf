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

#include "ThermalRelationshipTable.h"

ThermalRelationshipTable::ThermalRelationshipTable(const std::vector<ThermalRelationshipTableEntry>& entries)
    : RelationshipTableBase(),
    m_entries(entries)
{
}

ThermalRelationshipTable::~ThermalRelationshipTable()
{
}

ThermalRelationshipTable ThermalRelationshipTable::createTrtFromDptfBuffer(const DptfBuffer& buffer)
{
    std::vector<ThermalRelationshipTableEntry> entries;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryTrtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

    if (buffer.size() != 0)
    {
        UIntN rows = countTrtRows(buffer.size(), data);

        // Reset currentRow to point to the beginning of the data block
        data = reinterpret_cast<UInt8*>(buffer.get());
        currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

        for (UIntN i = 0; i < rows; i++)
        {
            // Since the TRT has 2 strings in it, the process for extracting them is:
            //  1. Extract the source at the beginning of the structure
            //  2. Since the actual string data is placed between the source and target, the pointer needs moved
            //  3. Move the pointer past the source string data and set current row
            //  4. Now the targetDevice field will actually point to the right spot
            //  5. Extract target device
            //  6. Move the pointer as before (past the targetDevice string data) and set current row
            //  7. Extract the remaining fields
            //  8. Point data and currentRow to the next row

            std::string source(
                reinterpret_cast<const char*>(&(currentRow->sourceDevice)) + sizeof(union esif_data_variant),
                currentRow->sourceDevice.string.length);

            data += currentRow->sourceDevice.string.length;
            currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

            std::string target(
                reinterpret_cast<const char*>(&(currentRow->targetDevice)) + sizeof(union esif_data_variant),
                currentRow->targetDevice.string.length);

            data += currentRow->targetDevice.string.length;
            currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

            ThermalRelationshipTableEntry temp(
                BinaryParse::normalizeAcpiScope(source),
                BinaryParse::normalizeAcpiScope(target),
                static_cast<UInt32>(currentRow->thermalInfluence.integer.value),
                static_cast<UInt32>(currentRow->thermalSamplingPeriod.integer.value));

            entries.push_back(temp);

            // Since we've already accounted for the strings, we now move the pointer by the size of the structure
            //  to get to the next row.
            data += sizeof(struct EsifDataBinaryTrtPackage);
            currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);
        }
    }
    return ThermalRelationshipTable(entries);
}

UIntN ThermalRelationshipTable::getNumberOfEntries(void) const
{
    return (UIntN)m_entries.size();
}

std::vector<ThermalRelationshipTableEntry> ThermalRelationshipTable::getEntriesForTarget(UIntN targetIndex)
{
    std::vector<ThermalRelationshipTableEntry> entries;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].getTargetDeviceIndex() == targetIndex)
        {
            entries.push_back(m_entries[row]);
        }
    }
    return entries;
}

RelationshipTableEntryBase* ThermalRelationshipTable::getEntry(UIntN index) const
{
    return (RelationshipTableEntryBase*)(&m_entries.at(index));
}

TimeSpan ThermalRelationshipTable::getMinimumActiveSamplePeriodForSource(
    UIntN sourceIndex, std::set<UIntN> activeTargets)
{
    UInt32 minimumSamplePeriod = Constants::Invalid;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if ((m_entries[row].getSourceDeviceIndex() == sourceIndex) &&
            (activeTargets.find(m_entries[row].getTargetDeviceIndex()) != activeTargets.end()))
        {
            if (m_entries[row].thermalSamplingPeriod() < minimumSamplePeriod)
            {
                minimumSamplePeriod = m_entries[row].thermalSamplingPeriod();
            }
        }
    }
    return TimeSpan::createFromTenthSeconds(minimumSamplePeriod);
}

TimeSpan ThermalRelationshipTable::getShortestSamplePeriodForTarget(UIntN target)
{
    UInt32 shortestSamplePeriod(Constants::Invalid);
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].getTargetDeviceIndex() == target)
        {
            if (m_entries[row].thermalSamplingPeriod() < shortestSamplePeriod)
            {
                shortestSamplePeriod = m_entries[row].thermalSamplingPeriod();
            }
        }
    }

    return TimeSpan::createFromTenthSeconds(shortestSamplePeriod);
}

TimeSpan ThermalRelationshipTable::getSampleTimeForRelationship(UIntN target, UIntN source) const
{
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if ((m_entries[row].getTargetDeviceIndex() == target) &&
            (m_entries[row].getSourceDeviceIndex() == source))
        {
            return TimeSpan::createFromTenthSeconds(m_entries[row].thermalSamplingPeriod());
        }
    }
    throw dptf_exception("No match found for target and source in TRT.");
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

UIntN ThermalRelationshipTable::countTrtRows(UInt32 size, UInt8* data)
{
    IntN bytesRemaining = size;
    UIntN rows = 0;

    struct EsifDataBinaryTrtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

    while (bytesRemaining > 0)
    {
        bytesRemaining -= sizeof(struct EsifDataBinaryTrtPackage);
        bytesRemaining -= currentRow->sourceDevice.string.length;

        data += currentRow->sourceDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

        bytesRemaining -= currentRow->targetDevice.string.length;

        data += currentRow->targetDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

        if (bytesRemaining >= 0)
        {
            // The math done here will vary based on the number of strings in the BIOS object
            rows++;

            data += sizeof(struct EsifDataBinaryTrtPackage);
            currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);
        }
        else // Data size mismatch, we went negative
        {
            throw dptf_exception("Expected binary data size mismatch. (TRT)");
        }
    }

    return rows;
}

Bool ThermalRelationshipTable::operator==(const ThermalRelationshipTable& trt) const
{
    return (m_entries == trt.m_entries);
}

Bool ThermalRelationshipTable::operator!=(const ThermalRelationshipTable& trt) const
{
    return (m_entries != trt.m_entries);
}
