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

#include "DomainProperties.h"
#include "StatusFormat.h"
using namespace StatusFormat;

DomainProperties::DomainProperties(Guid guid, UIntN domainIndex, Bool domainEnabled,
    DomainType::Type domainType, std::string domainName, std::string domainDescription,
    DomainFunctionalityVersions domainFunctionalityVersions) : m_guid(guid), m_domainIndex(domainIndex),
    m_enabled(domainEnabled), m_domainType(domainType), m_name(domainName), m_description(domainDescription),
    m_domainFunctionalityVersions(domainFunctionalityVersions)
{
}

Guid DomainProperties::getGuid(void) const
{
    return m_guid;
}

UIntN DomainProperties::getDomainIndex(void) const
{
    return m_domainIndex;
}

Bool DomainProperties::isEnabled(void) const
{
    return m_enabled;
}

DomainType::Type DomainProperties::getDomainType(void) const
{
    return m_domainType;
}

std::string DomainProperties::getName(void) const
{
    return m_name;
}

std::string DomainProperties::getDescription(void) const
{
    return m_description;
}

Bool DomainProperties::implementsActiveControlInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.activeControlVersion);
}

Bool DomainProperties::implementsConfigTdpControlInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.configTdpControlVersion);
}

Bool DomainProperties::implementsCoreControlInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.coreControlVersion);
}

Bool DomainProperties::implementsDisplayControlInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.displayControlVersion);
}

Bool DomainProperties::implementsPerformanceControlInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.performanceControlVersion);
}

Bool DomainProperties::implementsPixelClockControlInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.pixelClockControlVersion);
}

Bool DomainProperties::implementsPixelClockStatusInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.pixelClockStatusVersion);
}

Bool DomainProperties::implementsPowerControlInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.powerControlVersion);
}

Bool DomainProperties::implementsPowerStatusInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.powerStatusVersion);
}

Bool DomainProperties::implementsDomainPriorityInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.domainPriorityVersion);
}

Bool DomainProperties::implementsRfProfileControlInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.rfProfileControlVersion);
}

Bool DomainProperties::implementsRfProfileStatusInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.rfProfileStatusVersion);
}

Bool DomainProperties::implementsTemperatureInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.temperatureVersion);
}

Bool DomainProperties::implementsUtilizationInterface(void) const
{
    return isInterfaceImplemented(m_domainFunctionalityVersions.utilizationVersion);
}

XmlNode* DomainProperties::getXml()
{
    XmlNode* properties = XmlNode::createWrapperElement("domain_properties");
    properties->addChild(XmlNode::createDataElement("guid", m_guid.toString()));
    properties->addChild(XmlNode::createDataElement("index", friendlyValue(m_domainIndex)));
    properties->addChild(XmlNode::createDataElement("enabled", friendlyValue(m_enabled)));
    properties->addChild(XmlNode::createDataElement("type", DomainType::ToString(m_domainType)));
    properties->addChild(XmlNode::createDataElement("name", m_name));
    properties->addChild(XmlNode::createDataElement("description", m_description));
    return properties;
}

Bool DomainProperties::isInterfaceImplemented(UInt8 version) const
{
    return (version != 0);
}