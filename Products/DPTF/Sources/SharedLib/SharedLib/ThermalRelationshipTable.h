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
#include "RelationshipTableBase.h"
#include "ThermalRelationshipTableEntry.h"
#include "XmlNode.h"

class ThermalRelationshipTable final : public RelationshipTableBase
{
public:

    ThermalRelationshipTable(const std::vector<ThermalRelationshipTableEntry>& entries);
    virtual ~ThermalRelationshipTable();

    virtual UIntN getNumberOfEntries(void) const override;
    const ThermalRelationshipTableEntry& operator[](UIntN rowIndex) const;

    std::vector<ThermalRelationshipTableEntry> getEntriesForTarget(UIntN targetIndex);
    std::vector<ThermalRelationshipTableEntry> getEntriesForSource(UIntN sourceIndex);
    const ThermalRelationshipTableEntry& getEntryForTargetAndSource(UIntN targetIndex, UIntN sourceIndex) const;
    UInt32 getMinimumSamplePeriodForSource(UIntN sourceIndex);
    UInt32 getShortestSamplePeriodForTarget(UIntN target);

    XmlNode* getXml();

protected:

    virtual RelationshipTableEntryBase* getEntry(UIntN index) const override;

private:

    std::vector<ThermalRelationshipTableEntry> m_entries;
};