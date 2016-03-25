/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "RelationshipTableBase.h"
#include "ActiveRelationshipTableEntry.h"
#include "XmlNode.h"
#include "EsifDataBinaryArtPackage.h"
#include "DptfBuffer.h"

class dptf_export ActiveRelationshipTable final : public RelationshipTableBase
{
public:

    ActiveRelationshipTable();
    ActiveRelationshipTable(const std::vector<ActiveRelationshipTableEntry>& entries);
    virtual ~ActiveRelationshipTable();

    static ActiveRelationshipTable createArtFromDptfBuffer(const DptfBuffer& buffer);
    DptfBuffer toArtBinary() const;
    virtual UIntN getNumberOfEntries(void) const override;
    const ActiveRelationshipTableEntry& operator[](UIntN index) const;
    std::vector<UIntN> getAllSources(void) const;
    std::vector<UIntN> getAllTargets(void) const;
    std::vector<ActiveRelationshipTableEntry> getEntriesForTarget(UIntN target);
    std::vector<ActiveRelationshipTableEntry> getEntriesForSource(UIntN source);
    std::shared_ptr<XmlNode> getXml();
    Bool operator==(const ActiveRelationshipTable& art) const;

protected:

    virtual RelationshipTableEntryBase* getEntry(UIntN index) const override;

private:

    std::vector<ActiveRelationshipTableEntry> m_entries;
    static UIntN countArtRows(UInt32 size, UInt8* data);
    static void throwIfOutOfRange(IntN bytesRemaining);
};