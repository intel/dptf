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

#include "DomainProperties.h"
#include "StatusFormat.h"
using namespace StatusFormat;

DomainProperties::DomainProperties(
	const Guid& guid,
	UIntN domainIndex,
	Bool domainEnabled,
	DomainType::Type domainType,
	const std::string& domainName,
	const std::string& domainDescription,
	const DomainFunctionalityVersions& domainFunctionalityVersions)
	: m_guid(guid)
	, m_domainIndex(domainIndex)
	, m_enabled(domainEnabled)
	, m_domainType(domainType)
	, m_name(domainName)
	, m_description(domainDescription)
	, m_domainFunctionalityVersions(domainFunctionalityVersions)
{
}

Guid DomainProperties::getGuid() const
{
	return m_guid;
}

UIntN DomainProperties::getDomainIndex() const
{
	return m_domainIndex;
}

Bool DomainProperties::isEnabled() const
{
	return m_enabled;
}

DomainType::Type DomainProperties::getDomainType() const
{
	return m_domainType;
}

std::string DomainProperties::getName() const
{
	return m_name;
}

std::string DomainProperties::getDescription() const
{
	return m_description;
}

DomainFunctionalityVersions DomainProperties::getDomainFunctionalityVersions() const
{
	return m_domainFunctionalityVersions;
}

Bool DomainProperties::implementsActiveControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.activeControlVersion);
}

Bool DomainProperties::implementsActivityStatusInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.activityStatusVersion);
}

Bool DomainProperties::implementsCoreControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.coreControlVersion);
}

Bool DomainProperties::implementsDisplayControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.displayControlVersion);
}

Bool DomainProperties::implementsEnergyControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.energyControlVersion);
}

Bool DomainProperties::implementsPeakPowerControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.peakPowerControlVersion);
}

Bool DomainProperties::implementsPerformanceControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.performanceControlVersion);
}

Bool DomainProperties::implementsPowerControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.powerControlVersion);
}

Bool DomainProperties::implementsPowerStatusInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.powerStatusVersion);
}

Bool DomainProperties::implementsSystemPowerControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.systemPowerControlVersion);
}

Bool DomainProperties::implementsBatteryStatusInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.batteryStatusVersion);
}

Bool DomainProperties::implementsPlatformPowerStatusInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.platformPowerStatusVersion);
}

Bool DomainProperties::implementsDomainPriorityInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.domainPriorityVersion);
}

Bool DomainProperties::implementsRfProfileControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.rfProfileControlVersion);
}

Bool DomainProperties::implementsRfProfileStatusInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.rfProfileStatusVersion);
}

Bool DomainProperties::implementsTemperatureInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.temperatureVersion);
}

Bool DomainProperties::implementsTemperatureThresholdInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.temperatureThresholdVersion);
}

Bool DomainProperties::implementsProcessorControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.processorControlVersion);
}

Bool DomainProperties::implementsUtilizationInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.utilizationVersion);
}

Bool DomainProperties::implementsSocWorkloadClassificationInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.socWorkloadClassificationVersion);
}

Bool DomainProperties::implementsDynamicEppInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.dynamicEppVersion);
}

Bool DomainProperties::implementsBiasControlInterface() const
{
	return isInterfaceImplemented(m_domainFunctionalityVersions.biasControlVersion);
}

std::shared_ptr<XmlNode> DomainProperties::getXml() const
{
	auto properties = XmlNode::createWrapperElement("domain_properties");
	properties->addChild(XmlNode::createDataElement("guid", m_guid.toString()));
	properties->addChild(XmlNode::createDataElement("index", friendlyValue(m_domainIndex)));
	properties->addChild(XmlNode::createDataElement("enabled", friendlyValue(m_enabled)));
	properties->addChild(XmlNode::createDataElement("type", DomainType::toString(m_domainType)));
	properties->addChild(XmlNode::createDataElement("name", m_name));
	properties->addChild(XmlNode::createDataElement("description", m_description));
	return properties;
}

Bool DomainProperties::isInterfaceImplemented(UInt8 version)
{
	return (version != 0);
}

Bool DomainProperties::operator==(const DomainProperties& domain)
{
	return (
		(m_guid == domain.m_guid) && (m_domainIndex == domain.m_domainIndex) && (m_enabled == domain.m_enabled)
		&& (m_domainType == domain.m_domainType) && (m_name == domain.m_name) && (m_description == domain.m_description)
		&& (m_domainFunctionalityVersions == domain.m_domainFunctionalityVersions));
}

Bool DomainProperties::operator!=(const DomainProperties& domain)
{
	return !(*this == domain);
}
