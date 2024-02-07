/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "ParticipantProperties.h"
#include "XmlNode.h"
using namespace std;

ParticipantProperties::ParticipantProperties()
	: m_guid(Guid())
	, m_name(""s)
	, m_description(""s)
	, m_busType(BusType::Acpi)
	, m_pciInfo(PciInfo())
	, m_acpiInfo(AcpiInfo())
{
}

ParticipantProperties::ParticipantProperties(
	const Guid& guid,
	const std::string& name,
	const std::string& description,
	BusType::Type busType,
	const PciInfo& pciInfo,
	const AcpiInfo& acpiInfo)
	: m_guid(guid)
	, m_name(name)
	, m_description(description)
	, m_busType(busType)
	, m_pciInfo(pciInfo)
	, m_acpiInfo(acpiInfo)
{
}

Guid ParticipantProperties::getGuid(void) const
{
	return m_guid;
}

std::string ParticipantProperties::getName(void) const
{
	return m_name;
}

std::string ParticipantProperties::getDescription(void) const
{
	return m_description;
}

BusType::Type ParticipantProperties::getBusType(void) const
{
	return m_busType;
}

PciInfo ParticipantProperties::getPciInfo(void) const
{
	return m_pciInfo;
}

AcpiInfo ParticipantProperties::getAcpiInfo(void) const
{
	return m_acpiInfo;
}

std::shared_ptr<XmlNode> ParticipantProperties::getXml(void) const
{
	auto root = XmlNode::createWrapperElement("participant_properties");

	root->addChild(XmlNode::createDataElement("name", m_name));
	root->addChild(XmlNode::createDataElement("description", m_description));
	root->addChild(XmlNode::createDataElement("bus_type", BusType::ToString(m_busType)));
	root->addChild(XmlNode::createDataElement("acpi_device", m_acpiInfo.getAcpiDevice()));
	root->addChild(XmlNode::createDataElement("acpi_scope", m_acpiInfo.getAcpiScope()));

	return root;
}
