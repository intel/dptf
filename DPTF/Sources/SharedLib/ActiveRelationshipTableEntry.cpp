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

#include "ActiveRelationshipTableEntry.h"
#include "StatusFormat.h"
#include "ParticipantSpecificInfoKey.h"
using namespace StatusFormat;

ActiveRelationshipTableEntry::ActiveRelationshipTableEntry(
    const std::string& sourceDeviceAcpiScope,
    const std::string& targetDeviceAcpiScope,
    UInt32 weight,
    const std::vector<UInt32>& acEntries)
    : RelationshipTableEntryBase(sourceDeviceAcpiScope, targetDeviceAcpiScope),
    m_weight(weight),
    m_acEntries(acEntries)
{
}

ActiveRelationshipTableEntry::~ActiveRelationshipTableEntry()
{
}

const UInt32& ActiveRelationshipTableEntry::weight() const
{
    return m_weight;
}

const UInt32& ActiveRelationshipTableEntry::ac(UIntN acLevel) const
{
    if (acLevel >= FanOffIndex)
    {
        throw dptf_exception("Requested AC level outside of expected range.");
    }
    return m_acEntries[acLevel];
}

XmlNode* ActiveRelationshipTableEntry::getXml()
{
    XmlNode* entry = XmlNode::createWrapperElement("art_entry");
    entry->addChild(XmlNode::createDataElement("target_index", friendlyValue(targetDeviceIndex())));
    entry->addChild(XmlNode::createDataElement("target_acpi_scope", targetDeviceAcpiScope()));
    entry->addChild(XmlNode::createDataElement("source_index", friendlyValue(sourceDeviceIndex())));
    entry->addChild(XmlNode::createDataElement("source_acpi_scope", sourceDeviceAcpiScope()));
    entry->addChild(XmlNode::createDataElement("weight", friendlyValue(m_weight)));
    for (UIntN acNum = ParticipantSpecificInfoKey::AC0; acNum <= ParticipantSpecificInfoKey::AC9; acNum++)
    {
        UIntN index = acNum - ParticipantSpecificInfoKey::AC0;
        entry->addChild(XmlNode::createDataElement(
            ParticipantSpecificInfoKey::ToString(ParticipantSpecificInfoKey::Type(acNum)), friendlyValue(ac(index))));
    }
    return entry;
}