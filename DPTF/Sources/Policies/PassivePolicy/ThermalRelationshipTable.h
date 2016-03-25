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
#include "ThermalRelationshipTableEntry.h"
#include "XmlNode.h"

class dptf_export ThermalRelationshipTable final : public RelationshipTableBase
{
public:

    ThermalRelationshipTable(void);
    ThermalRelationshipTable(const std::vector<ThermalRelationshipTableEntry>& entries);
    virtual ~ThermalRelationshipTable();

    static ThermalRelationshipTable createTrtFromDptfBuffer(const DptfBuffer& buffer);
    DptfBuffer toTrtBinary(void) const;
    virtual UIntN getNumberOfEntries(void) const override;

    std::vector<ThermalRelationshipTableEntry> getEntriesForTarget(UIntN targetIndex);
    TimeSpan getMinimumActiveSamplePeriodForSource(UIntN sourceIndex, std::set<UIntN> activeTargets);
    TimeSpan getShortestSamplePeriodForTarget(UIntN target);
    TimeSpan getSampleTimeForRelationship(UIntN target, UIntN source) const;

    std::shared_ptr<XmlNode> getXml();
    Bool operator==(const ThermalRelationshipTable& trt) const;
    Bool operator!=(const ThermalRelationshipTable& trt) const;

protected:

    virtual RelationshipTableEntryBase* getEntry(UIntN index) const override;
    
private:

    std::vector<ThermalRelationshipTableEntry> m_entries;
    static UIntN countTrtRows(UInt32 size, UInt8* data);
    static void throwIfOutOfRange(IntN bytesRemaining);
};