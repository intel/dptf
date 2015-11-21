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

#include "ActiveRelationshipTable.h"
#include "BinaryParse.h"

ActiveRelationshipTable::ActiveRelationshipTable(const std::vector<ActiveRelationshipTableEntry>& entries)
    : RelationshipTableBase(),
    m_entries(entries)
{
}

ActiveRelationshipTable::~ActiveRelationshipTable()
{
}

ActiveRelationshipTable ActiveRelationshipTable::createArtFromDptfBuffer(const DptfBuffer& buffer)
{
    std::vector<ActiveRelationshipTableEntry> entries;

    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryArtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

    if (buffer.size() == 0)
    {
        throw dptf_exception("There is no data to process.");
    }

    UIntN rows = countArtRows(buffer.size(), data);

    // Reset currentRow to point to the beginning of the data block
    data = reinterpret_cast<UInt8*>(buffer.get());
    data += sizeof(esif_data_variant); //Ignore revision field
    currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

    for (UIntN i = 0; i < rows; i++)
    {
        // Since the ART has 2 strings in it, the process for extracting them is:
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
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

        std::string target(
            reinterpret_cast<const char*>(&(currentRow->targetDevice)) + sizeof(union esif_data_variant),
            currentRow->targetDevice.string.length);

        data += currentRow->targetDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

        std::vector <UInt32> acEntries;
        acEntries.push_back(static_cast<UInt32>(currentRow->ac0MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac1MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac2MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac3MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac4MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac5MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac6MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac7MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac8MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac9MaxFanSpeed.integer.value));

        ActiveRelationshipTableEntry temp(
            BinaryParse::normalizeAcpiScope(source),
            BinaryParse::normalizeAcpiScope(target),
            static_cast<UInt32>(currentRow->weight.integer.value),
            acEntries);

        entries.push_back(temp);

        // Since we've already accounted for the strings, we now move the pointer by the size of the structure
        //  to get to the next row.
        data += sizeof(struct EsifDataBinaryArtPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);
    }

    return ActiveRelationshipTable(entries);
}

UIntN ActiveRelationshipTable::countArtRows(UInt32 size, UInt8* data)
{
    IntN bytesRemaining = size;
    UIntN rows = 0;

    //Remove revision field
    data += sizeof(esif_data_variant);
    bytesRemaining -= sizeof(esif_data_variant);

    struct EsifDataBinaryArtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

    while (bytesRemaining > 0)
    {
        bytesRemaining -= sizeof(struct EsifDataBinaryArtPackage);
        bytesRemaining -= currentRow->sourceDevice.string.length;

        data += currentRow->sourceDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

        bytesRemaining -= currentRow->targetDevice.string.length;

        data += currentRow->targetDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

        if (bytesRemaining >= 0)
        {
            // The math done here will vary based on the number of strings in the BIOS object
            rows++;

            data += sizeof(struct EsifDataBinaryArtPackage);
            currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);
        }
        else // Data size mismatch, we went negative
        {
            throw dptf_exception("Expected binary data size mismatch. (ART)");
        }
    }

    return rows;
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
        if (m_entries[row].getTargetDeviceIndex() == target)
        {
            entries.push_back(m_entries[row]);
        }
    }
    return entries;
}

std::vector<ActiveRelationshipTableEntry> ActiveRelationshipTable::getEntriesForSource(UIntN source)
{
    std::vector<ActiveRelationshipTableEntry> entries;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].getSourceDeviceIndex() == source)
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
        if (m_entries[row].getSourceDeviceIndex() != Constants::Invalid)
        {
            sources.insert(m_entries[row].getSourceDeviceIndex());
        }
    }
    return std::vector<UIntN>(sources.begin(), sources.end());
}

std::vector<UIntN> ActiveRelationshipTable::getAllTargets(void) const
{
    std::set<UIntN> targets;
    for (UIntN row = 0; row < m_entries.size(); ++row)
    {
        if (m_entries[row].getTargetDeviceIndex() != Constants::Invalid)
        {
            targets.insert(m_entries[row].getTargetDeviceIndex());
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

Bool ActiveRelationshipTable::operator==(const ActiveRelationshipTable& art) const
{
    return (m_entries == art.m_entries);
}