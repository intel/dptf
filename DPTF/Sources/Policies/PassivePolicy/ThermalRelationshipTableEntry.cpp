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

#include "ThermalRelationshipTableEntry.h"
#include "StatusFormat.h"
using namespace StatusFormat;

ThermalRelationshipTableEntry::ThermalRelationshipTableEntry(
    const std::string& sourceDeviceAcpiScope,
    const std::string& targetDeviceAcpiScope,
    UInt32 thermalInfluence,
    UInt32 thermalSamplingPeriod)
    : RelationshipTableEntryBase(sourceDeviceAcpiScope, targetDeviceAcpiScope),
    m_thermalInfluence(thermalInfluence),
    m_thermalSamplingPeriod(thermalSamplingPeriod)
{
}

const UInt32& ThermalRelationshipTableEntry::thermalInfluence() const
{
    return m_thermalInfluence;
}

const UInt32& ThermalRelationshipTableEntry::thermalSamplingPeriod() const
{
    return m_thermalSamplingPeriod;
}

std::shared_ptr<XmlNode> ThermalRelationshipTableEntry::getXml()
{
    auto entry = XmlNode::createWrapperElement("trt_entry");
    entry->addChild(XmlNode::createDataElement("target_index", friendlyValue(getTargetDeviceIndex())));
    entry->addChild(XmlNode::createDataElement("target_acpi_scope", getTargetDeviceAcpiScope()));
    entry->addChild(XmlNode::createDataElement("source_index", friendlyValue(getSourceDeviceIndex())));
    entry->addChild(XmlNode::createDataElement("source_acpi_scope", getSourceDeviceAcpiScope()));
    entry->addChild(XmlNode::createDataElement("influence", friendlyValue(m_thermalInfluence)));
    entry->addChild(XmlNode::createDataElement("sampling_period", friendlyValue(m_thermalSamplingPeriod / 10)));
    return entry;
}

Bool ThermalRelationshipTableEntry::isSameAs(const ThermalRelationshipTableEntry& trtEntry) const
{
    return ((RelationshipTableEntryBase)*this) == ((RelationshipTableEntryBase)trtEntry);
}

Bool ThermalRelationshipTableEntry::operator==(const ThermalRelationshipTableEntry& trtEntry) const
{
    return ((m_thermalInfluence == trtEntry.m_thermalInfluence) && (m_thermalSamplingPeriod == trtEntry.m_thermalSamplingPeriod)
        && ((RelationshipTableEntryBase)*this) == ((RelationshipTableEntryBase)trtEntry));
}
