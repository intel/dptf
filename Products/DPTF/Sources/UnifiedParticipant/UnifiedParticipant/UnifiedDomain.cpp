/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

UnifiedDomain::UnifiedDomain(const Guid& guid, UIntN participantIndex, UIntN domainIndex, Bool domainEnabled, DomainType::Type domainType, std::string domainName,
    std::string domainDescription, DomainFunctionalityVersions domainFunctionalityVersions,
    const ClassFactories& classFactories, ParticipantServicesInterface* participantServicesInterface) : m_guid(guid),
    m_participantIndex(participantIndex), m_domainIndex(domainIndex), m_enabled(domainEnabled), 
    m_domainType(domainType), m_name(domainName), m_description(domainDescription), 
    m_domainFunctionalityVersions(domainFunctionalityVersions), 
    m_participantServicesInterface(participantServicesInterface)
{
    // if an error is thrown we don't want to catch it as the domain can't be created anyway.
    createActiveControlObject(classFactories, participantServicesInterface);
    createConfigTdpControlObject(classFactories, participantServicesInterface);
    createCoreControlObject(classFactories, participantServicesInterface);
    createDisplayControlObject(classFactories, participantServicesInterface);
    createDomainPriorityObject(classFactories, participantServicesInterface);
    createPerformanceControlObject(classFactories, participantServicesInterface);
    createPowerControlObject(classFactories, participantServicesInterface);
    createPowerStatusObject(classFactories, participantServicesInterface);
    createTemperatureObject(classFactories, participantServicesInterface);
    createUtilizationObject(classFactories, participantServicesInterface);
}

UnifiedDomain::~UnifiedDomain(void)
{
    // destroy all objects created in the constructor
    delete m_activeControl;
    delete m_configTdpControl;
    delete m_coreControl;
    delete m_displayControl;
    delete m_domainPriority;
    delete m_performanceControl;
    delete m_powerControl;
    delete m_powerStatus;
    delete m_temperature;
    delete m_utilization;
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

XmlNode* UnifiedDomain::getXml()
{
    XmlNode* domain = XmlNode::createWrapperElement("domain");

    domain->addChild(XmlNode::createDataElement("index", StatusFormat::friendlyValue(m_domainIndex)));
    domain->addChild(XmlNode::createDataElement("name", getName()));
    domain->addChild(XmlNode::createDataElement("description", getDescription()));

    // Active Control XML Status
    try
    {
        domain->addChild(getActiveControlInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get active control XML status!"));
    }

    // cTDP Control XML Status
    try
    {
        domain->addChild(getConfigTdpControlInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get cTDP control XML status!"));
    }

    // Core Control XML Status
    try
    {
        domain->addChild(getCoreControlInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get core control XML status!"));
    }

    // Display Control XML Status
    try
    {
        domain->addChild(getDisplayControlInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get display control XML status!"));
    }

    // Domain Priority XML Status
    try
    {
        domain->addChild(getDomainPriorityInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get domain priority XML status!"));
    }

    // Performance Control XML Status
    try
    {
        domain->addChild(getPerformanceControlInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get performance control XML status!"));
    }

    // Power Control XML Status
    try
    {
        domain->addChild(getPowerControlInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get power control XML status!"));
    }

    // Power Status XML Status
    try
    {
        domain->addChild(getPowerStatusInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get power reporting XML status!"));
    }

    // Temperature XML Status
    try
    {
        domain->addChild(getTemperatureInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get temperature XML status!"));
    }

    // Utilization XML Status
    try
    {
        domain->addChild(getUtilizationInterfaceExPtr()->getXml(m_domainIndex));
    }
    catch (not_implemented)
    {
    }
    catch (...)
    {
        // Write message log error
        m_participantServicesInterface->writeMessageError(ParticipantMessage(FLF, "Unable to get utilization XML status!"));
    }

    return domain;
}

void UnifiedDomain::clearAllCachedData(void)
{
    getActiveControlInterfaceExPtr()->clearCachedData();
    getConfigTdpControlInterfaceExPtr()->clearCachedData();
    getCoreControlInterfaceExPtr()->clearCachedData();
    getDisplayControlInterfaceExPtr()->clearCachedData();
    getDomainPriorityInterfaceExPtr()->clearCachedData();
    getPerformanceControlInterfaceExPtr()->clearCachedData();
    getPowerControlInterfaceExPtr()->clearCachedData();
    getPowerStatusInterfaceExPtr()->clearCachedData();
    getTemperatureInterfaceExPtr()->clearCachedData();
    getUtilizationInterfaceExPtr()->clearCachedData();
}

DomainActiveControlInterface* UnifiedDomain::getActiveControlInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_activeControl;
}

ComponentExtendedInterface* UnifiedDomain::getActiveControlInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_activeControl);
}

DomainConfigTdpControlInterface* UnifiedDomain::getConfigTdpControlInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_configTdpControl;
}

ComponentExtendedInterface* UnifiedDomain::getConfigTdpControlInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_configTdpControl);
}

DomainCoreControlInterface* UnifiedDomain::getCoreControlInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_coreControl;
}

ComponentExtendedInterface* UnifiedDomain::getCoreControlInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_coreControl);
}

DomainDisplayControlInterface* UnifiedDomain::getDisplayControlInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_displayControl;
}

ComponentExtendedInterface* UnifiedDomain::getDisplayControlInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_displayControl);
}

DomainPriorityInterface* UnifiedDomain::getDomainPriorityInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_domainPriority;
}

ComponentExtendedInterface* UnifiedDomain::getDomainPriorityInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_domainPriority);
}

DomainPerformanceControlInterface* UnifiedDomain::getPerformanceControlInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_performanceControl;
}

ComponentExtendedInterface* UnifiedDomain::getPerformanceControlInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_performanceControl);
}

ConfigTdpDataSyncInterface* UnifiedDomain::getPerformanceControlConfigTdpSyncInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ConfigTdpDataSyncInterface*>(m_performanceControl);
}

DomainPowerControlInterface* UnifiedDomain::getPowerControlInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_powerControl;
}

ComponentExtendedInterface* UnifiedDomain::getPowerControlInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_powerControl);
}

ConfigTdpDataSyncInterface* UnifiedDomain::getPowerControlConfigTdpSyncInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ConfigTdpDataSyncInterface*>(m_powerControl);
}

DomainPowerStatusInterface* UnifiedDomain::getPowerStatusInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_powerStatus;
}

ComponentExtendedInterface* UnifiedDomain::getPowerStatusInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_powerStatus);
}

DomainTemperatureInterface* UnifiedDomain::getTemperatureInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_temperature;
}

ComponentExtendedInterface* UnifiedDomain::getTemperatureInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_temperature);
}

DomainUtilizationInterface* UnifiedDomain::getUtilizationInterfacePtr(void)
{
    throwIfDomainNotEnabled();
    return m_utilization;
}

ComponentExtendedInterface* UnifiedDomain::getUtilizationInterfaceExPtr(void)
{
    throwIfDomainNotEnabled();
    return dynamic_cast<ComponentExtendedInterface*>(m_utilization);
}

void UnifiedDomain::createActiveControlObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_activeControl = classFactories.domainActiveControlFactory->createDomainActiveControlObject(
        m_domainFunctionalityVersions.activeControlVersion, participantServicesInterface);
}

void UnifiedDomain::createConfigTdpControlObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_configTdpControl = classFactories.domainConfigTdpControlFactory->createDomainConfigTdpControlObject(
        m_domainFunctionalityVersions.configTdpControlVersion, participantServicesInterface);
}

void UnifiedDomain::createCoreControlObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_coreControl = classFactories.domainCoreControlFactory->createDomainCoreControlObject(
        m_domainFunctionalityVersions.coreControlVersion, participantServicesInterface);
}

void UnifiedDomain::createDisplayControlObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_displayControl = classFactories.domainDisplayControlFactory->createDomainDisplayControlObject(
        m_domainFunctionalityVersions.displayControlVersion, participantServicesInterface);
}

void UnifiedDomain::createDomainPriorityObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_domainPriority = classFactories.domainPriorityFactory->createDomainPriorityObject(
        m_domainFunctionalityVersions.domainPriorityVersion, participantServicesInterface);
}

void UnifiedDomain::createPerformanceControlObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_performanceControl = classFactories.domainPerformanceControlFactory->createDomainPerformanceControlObject(
        m_domainFunctionalityVersions.performanceControlVersion, participantServicesInterface);
}

void UnifiedDomain::createPowerControlObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_powerControl = classFactories.domainPowerControlFactory->createDomainPowerControlObject(
        m_domainFunctionalityVersions.powerControlVersion, participantServicesInterface);
}

void UnifiedDomain::createPowerStatusObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_powerStatus = classFactories.domainPowerStatusFactory->createDomainPowerStatusObject(
        m_domainFunctionalityVersions.powerStatusVersion, participantServicesInterface);
}

void UnifiedDomain::createTemperatureObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_temperature = classFactories.domainTemperatureFactory->createDomainTemperatureObject(
        m_domainFunctionalityVersions.temperatureVersion, participantServicesInterface);
}

void UnifiedDomain::createUtilizationObject(const ClassFactories& classFactories,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_utilization = classFactories.domainUtilizationFactory->createDomainUtilizationObject(
        m_domainFunctionalityVersions.utilizationVersion, participantServicesInterface);
}

void UnifiedDomain::throwIfDomainNotEnabled(void)
{
    if (m_enabled == false)
    {
        throw domain_not_enabled();
    }
}