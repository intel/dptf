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

#include "UnifiedDomain.h"
#include "XmlNode.h"
#include "StatusFormat.h"
#include "DomainActiveControlInterface.h"
using namespace std;

UnifiedDomain::UnifiedDomain(const Guid& guid, UIntN participantIndex, UIntN domainIndex, 
    Bool domainEnabled, DomainType::Type domainType, std::string domainName,
    std::string domainDescription, DomainFunctionalityVersions domainFunctionalityVersions,
    const ControlFactoryList& classFactories, ParticipantServicesInterface* participantServicesInterface) : m_guid(guid),
    m_participantIndex(participantIndex), m_domainIndex(domainIndex), m_enabled(domainEnabled),
    m_domainType(domainType), m_name(domainName), m_description(domainDescription),
    m_domainFunctionalityVersions(domainFunctionalityVersions),
    m_participantServicesInterface(participantServicesInterface)
{
    m_domainControls = make_shared<DomainControlList>(m_participantIndex, m_domainIndex, domainFunctionalityVersions, 
        classFactories, m_participantServicesInterface);
}

UnifiedDomain::~UnifiedDomain(void)
{

}

Guid UnifiedDomain::getGuid(void)
{
    return m_guid;
}

Bool UnifiedDomain::isEnabled(void)
{
    return m_enabled;
}

void UnifiedDomain::enable(void)
{
    m_enabled = true;
}

void UnifiedDomain::disable(void)
{
    clearAllCachedData();
    m_enabled = false;
}

DomainType::Type UnifiedDomain::getDomainType(void)
{
    return m_domainType;
}

std::string UnifiedDomain::getName(void)
{
    return m_name;
}

std::string UnifiedDomain::getDescription(void)
{
    return m_description;
}

DomainFunctionalityVersions UnifiedDomain::getDomainFunctionalityVersions(void)
{
    return m_domainFunctionalityVersions;
}

std::shared_ptr<XmlNode> UnifiedDomain::getXml()
{
    auto domain = XmlNode::createWrapperElement("domain");
    domain->addChild(XmlNode::createDataElement("index", StatusFormat::friendlyValue(m_domainIndex)));
    domain->addChild(XmlNode::createDataElement("name", getName()));
    domain->addChild(XmlNode::createDataElement("description", getDescription()));
    domain->addChild(m_domainControls->getXml());
    return domain;
}

void UnifiedDomain::clearAllCachedData(void)
{
    m_domainControls->clearAllCachedData();
}

std::shared_ptr<DomainActiveControlBase> UnifiedDomain::getActiveControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getActiveControl();
}

std::shared_ptr<DomainConfigTdpControlBase> UnifiedDomain::getConfigTdpControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getConfigTdpControl();
}

std::shared_ptr<DomainCoreControlBase> UnifiedDomain::getCoreControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getCoreControl();
}

std::shared_ptr<DomainDisplayControlBase> UnifiedDomain::getDisplayControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getDisplayControl();
}

std::shared_ptr<DomainPerformanceControlBase> UnifiedDomain::getPerformanceControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getPerformanceControl();
}

std::shared_ptr<DomainPixelClockControlBase> UnifiedDomain::getPixelClockControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getPixelClockControl();
}

std::shared_ptr<DomainPixelClockStatusBase> UnifiedDomain::getPixelClockStatusControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getPixelClockStatusControl();
}

std::shared_ptr<DomainPowerControlBase> UnifiedDomain::getPowerControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getPowerControl();
}

std::shared_ptr<DomainPowerStatusBase> UnifiedDomain::getPowerStatusControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getPowerStatusControl();
}

std::shared_ptr<DomainPlatformPowerControlBase> UnifiedDomain::getPlatformPowerControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getPlatformPowerControl();
}

std::shared_ptr<DomainPlatformPowerStatusBase> UnifiedDomain::getPlatformPowerStatusControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getPlatformPowerStatusControl();
}

std::shared_ptr<DomainPriorityBase> UnifiedDomain::getDomainPriorityControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getDomainPriorityControl();
}

std::shared_ptr<DomainRfProfileControlBase> UnifiedDomain::getRfProfileControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getRfProfileControl();
}

std::shared_ptr<DomainRfProfileStatusBase> UnifiedDomain::getRfProfileStatusControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getRfProfileStatusControl();
}

std::shared_ptr<DomainTemperatureBase> UnifiedDomain::getTemperatureControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getTemperatureControl();
}

std::shared_ptr<DomainUtilizationBase> UnifiedDomain::getUtilizationControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getUtilizationControl();
}

std::shared_ptr<DomainHardwareDutyCycleControlBase> UnifiedDomain::getHardareDutyCycleControl(void)
{
    throwIfDomainNotEnabled();
    return m_domainControls->getHardareDutyCycleControl();
}

void UnifiedDomain::throwIfDomainNotEnabled(void)
{
    if (m_enabled == false)
    {
        throw domain_not_enabled();
    }
}