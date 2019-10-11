/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "ActiveCoolingControl.h"
using namespace StatusFormat;

ActiveRelationshipTableEntry::ActiveRelationshipTableEntry(
	const std::string& sourceDeviceScope,
	const std::string& targetDeviceScope,
	UInt32 weight,
	const std::vector<UInt32>& acEntries)
	: RelationshipTableEntryBase(sourceDeviceScope, targetDeviceScope)
	, m_weight(weight)
	, m_acEntries(acEntries)
{
}

ActiveRelationshipTableEntry::~ActiveRelationshipTableEntry()
{
}

const UInt32& ActiveRelationshipTableEntry::ac(UIntN acLevel) const
{
	if (acLevel >= ActiveCoolingControl::FanOffIndex)
	{
		throw dptf_exception("Requested AC level outside of expected range.");
	}
	return m_acEntries[acLevel];
}

UInt32 ActiveRelationshipTableEntry::getWeight() const
{
	return m_weight;
}

std::shared_ptr<XmlNode> ActiveRelationshipTableEntry::getXml()
{
	auto entry = XmlNode::createWrapperElement("art_entry");
	entry->addChild(XmlNode::createDataElement("target_index", friendlyValue(getTargetDeviceIndex())));
	entry->addChild(XmlNode::createDataElement("target_acpi_scope", getTargetDeviceName()));
	entry->addChild(XmlNode::createDataElement("source_index", friendlyValue(getSourceDeviceIndex())));
	entry->addChild(XmlNode::createDataElement("source_acpi_scope", getSourceDeviceName()));
	entry->addChild(XmlNode::createDataElement("weight", friendlyValue(m_weight)));
	for (UIntN acNum = ParticipantSpecificInfoKey::AC0; acNum <= ParticipantSpecificInfoKey::AC9; acNum++)
	{
		UIntN index = acNum - ParticipantSpecificInfoKey::AC0;
		entry->addChild(XmlNode::createDataElement(
			ParticipantSpecificInfoKey::ToString(ParticipantSpecificInfoKey::Type(acNum)), friendlyValue(ac(index))));
	}
	return entry;
}

Bool ActiveRelationshipTableEntry::isSameAs(const ActiveRelationshipTableEntry& artEntry) const
{
	return ((RelationshipTableEntryBase) * this) == ((RelationshipTableEntryBase)artEntry);
}

Bool ActiveRelationshipTableEntry::operator==(const ActiveRelationshipTableEntry& artEntry) const
{
	return (
		(m_weight == artEntry.m_weight) && (m_acEntries == artEntry.m_acEntries)
		&& ((RelationshipTableEntryBase) * this) == ((RelationshipTableEntryBase)artEntry));
}
