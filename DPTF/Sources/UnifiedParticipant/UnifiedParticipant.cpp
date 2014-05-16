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

#include "UnifiedParticipant.h"
#include "ParticipantGetSpecificInfoFactory.h"
#include "ParticipantSetSpecificInfoFactory.h"
#include "XmlNode.h"
#include "StatusFormat.h"

static const Guid FormatId(0xF0, 0xCB, 0x64, 0x06, 0xE4, 0x2B, 0x46, 0xB5, 0x9C, 0x85, 0x32, 0xD1, 0xA1, 0xB7, 0xCB, 0x68);

UnifiedParticipant::UnifiedParticipant(void)
{
    initialize();

    // In this cases, which is the default, all class factories need to be created.
    createAllMissingClassFactories();
}

UnifiedParticipant::UnifiedParticipant(const ClassFactories& classFactories) :
    m_classFactories(classFactories)
{
    initialize();

    // In this case, which is provided for validation purposes, any class factories that aren't
    // provided via the constructor will be filled in.  This allows test code to be written
    // that only sends in the 'fake' factories as needed.
    createAllMissingClassFactories();
}

void UnifiedParticipant::initialize(void)
{
    m_guid = Guid();
    m_participantIndex = Constants::Invalid;
    m_enabled = false;
    m_busType = BusType::None;
    m_pciInfo = PciInfo();
    m_acpiInfo = AcpiInfo();
    m_participantServicesInterface = nullptr;
    m_getSpecificInfo = nullptr;
    m_getSpecificInfoEx = nullptr;
    m_setSpecificInfo = nullptr;
    m_setSpecificInfoEx = nullptr;
    m_configTdpEventsRegistered = false;
    m_coreControlEventsRegistered = false;
    m_displayControlEventsRegistered = false;
    m_domainPriorityEventsRegistered = false;
    m_performanceControlEventsRegistered = false;
    m_powerControlEventsRegistered = false;
    m_rfProfileEventsRegistered = false;
    m_temperatureThresholdEventsRegistered = false;
}

void UnifiedParticipant::createAllMissingClassFactories(void)
{
    createDomainActiveControlFactoryIfMissing();
    createDomainConfigTdpControlFactoryIfMissing();
    createDomainCoreControlFactoryIfMissing();
    createDomainDisplayControlFactoryIfMissing();
    createDomainPerformanceControlFactoryIfMissing();
    createDomainPixelClockControlFactoryIfMissing();
    createDomainPixelClockStatusFactoryIfMissing();
    createDomainPowerControlFactoryIfMissing();
    createDomainPowerStatusFactoryIfMissing();
    createDomainPriorityFactoryIfMissing();
    createDomainRfProfileControlFactoryIfMissing();
    createDomainRfProfileStatusFactoryIfMissing();
    createDomainTemperatureFactoryIfMissing();
    createDomainUtilizationFactoryIfMissing();
    createParticipantGetSpecificInfoFactoryIfMissing();
    createParticipantSetSpecificInfoFactoryIfMissing();
}

void UnifiedParticipant::createDomainActiveControlFactoryIfMissing(void)
{
    if (m_classFactories.domainActiveControlFactory == nullptr)
    {
        m_classFactories.domainActiveControlFactory = new DomainActiveControlFactory();
    }
}

void UnifiedParticipant::createDomainConfigTdpControlFactoryIfMissing(void)
{
    if (m_classFactories.domainConfigTdpControlFactory == nullptr)
    {
        m_classFactories.domainConfigTdpControlFactory = new DomainConfigTdpControlFactory();
    }
}

void UnifiedParticipant::createDomainCoreControlFactoryIfMissing(void)
{
    if (m_classFactories.domainCoreControlFactory == nullptr)
    {
        m_classFactories.domainCoreControlFactory = new DomainCoreControlFactory();
    }
}

void UnifiedParticipant::createDomainDisplayControlFactoryIfMissing(void)
{
    if (m_classFactories.domainDisplayControlFactory == nullptr)
    {
        m_classFactories.domainDisplayControlFactory = new DomainDisplayControlFactory();
    }
}

void UnifiedParticipant::createDomainPerformanceControlFactoryIfMissing(void)
{
    if (m_classFactories.domainPerformanceControlFactory == nullptr)
    {
        m_classFactories.domainPerformanceControlFactory = new DomainPerformanceControlFactory();
    }
}

void UnifiedParticipant::createDomainPixelClockControlFactoryIfMissing(void)
{
    if (m_classFactories.domainPixelClockControlFactory == nullptr)
    {
        m_classFactories.domainPixelClockControlFactory = new DomainPixelClockControlFactory();
    }
}

void UnifiedParticipant::createDomainPixelClockStatusFactoryIfMissing(void)
{
    if (m_classFactories.domainPixelClockStatusFactory == nullptr)
    {
        m_classFactories.domainPixelClockStatusFactory = new DomainPixelClockStatusFactory();
    }
}

void UnifiedParticipant::createDomainPowerControlFactoryIfMissing(void)
{
    if (m_classFactories.domainPowerControlFactory == nullptr)
    {
        m_classFactories.domainPowerControlFactory = new DomainPowerControlFactory();
    }
}

void UnifiedParticipant::createDomainPowerStatusFactoryIfMissing(void)
{
    if (m_classFactories.domainPowerStatusFactory == nullptr)
    {
        m_classFactories.domainPowerStatusFactory = new DomainPowerStatusFactory();
    }
}

void UnifiedParticipant::createDomainPriorityFactoryIfMissing(void)
{
    if (m_classFactories.domainPriorityFactory == nullptr)
    {
        m_classFactories.domainPriorityFactory = new DomainPriorityFactory();
    }
}

void UnifiedParticipant::createDomainRfProfileControlFactoryIfMissing(void)
{
    if (m_classFactories.domainRfProfileControlFactory == nullptr)
    {
        m_classFactories.domainRfProfileControlFactory = new DomainRfProfileControlFactory();
    }
}

void UnifiedParticipant::createDomainRfProfileStatusFactoryIfMissing(void)
{
    if (m_classFactories.domainRfProfileStatusFactory == nullptr)
    {
        m_classFactories.domainRfProfileStatusFactory = new DomainRfProfileStatusFactory();
    }
}

void UnifiedParticipant::createDomainTemperatureFactoryIfMissing(void)
{
    if (m_classFactories.domainTemperatureFactory == nullptr)
    {
        m_classFactories.domainTemperatureFactory = new DomainTemperatureFactory();
    }
}

void UnifiedParticipant::createDomainUtilizationFactoryIfMissing(void)
{
    if (m_classFactories.domainUtilizationFactory == nullptr)
    {
        m_classFactories.domainUtilizationFactory = new DomainUtilizationFactory();
    }
}

void UnifiedParticipant::createParticipantGetSpecificInfoFactoryIfMissing(void)
{
    if (m_classFactories.participantGetSpecificInfoFactory == nullptr)
    {
        m_classFactories.participantGetSpecificInfoFactory = new ParticipantGetSpecificInfoFactory();
    }
}

void UnifiedParticipant::createParticipantSetSpecificInfoFactoryIfMissing(void)
{
    if (m_classFactories.participantSetSpecificInfoFactory == nullptr)
    {
        m_classFactories.participantSetSpecificInfoFactory = new ParticipantSetSpecificInfoFactory();
    }
}

UnifiedParticipant::~UnifiedParticipant(void)
{
    m_classFactories.deleteAllFactories();
}

void UnifiedParticipant::createParticipant(const Guid& guid, UIntN participantIndex, Bool enabled,
    const std::string& name, const std::string& description,
    BusType::Type busType, const PciInfo& pciInfo, const AcpiInfo& acpiInfo,
    ParticipantServicesInterface* participantServicesInterface)
{
    m_guid = guid;
    m_participantIndex = participantIndex;
    m_enabled = enabled;
    m_name = name;
    m_description = description;
    m_busType = busType;
    m_pciInfo = pciInfo;
    m_acpiInfo = acpiInfo;
    m_participantServicesInterface = participantServicesInterface;

    m_getSpecificInfo = m_classFactories.participantGetSpecificInfoFactory->createParticipantGetSpecificInfoObject(
        GetSpecificInfoVersionDefault, m_participantServicesInterface);
    m_getSpecificInfoEx = dynamic_cast<ComponentExtendedInterface*>(m_getSpecificInfo);

    m_setSpecificInfo = m_classFactories.participantSetSpecificInfoFactory->createParticipantSetSpecificInfoObject(
        SetSpecificInfoVersionDefault, m_participantServicesInterface);
    m_setSpecificInfoEx = dynamic_cast<ComponentExtendedInterface*>(m_setSpecificInfo);

    m_participantServicesInterface->registerEvent(ParticipantEvent::ParticipantSpecificInfoChanged);
    m_participantServicesInterface->registerEvent(ParticipantEvent::DptfResume);
}

void UnifiedParticipant::destroyParticipant(void)
{
    destroyAllDomains();

    m_participantServicesInterface->unregisterEvent(ParticipantEvent::ParticipantSpecificInfoChanged);
    m_participantServicesInterface->unregisterEvent(ParticipantEvent::DptfResume);

    DELETE_MEMORY_TC(m_getSpecificInfo);
    DELETE_MEMORY_TC(m_setSpecificInfo);
}

void UnifiedParticipant::enableParticipant(void)
{
    m_enabled = true;
}

void UnifiedParticipant::disableParticipant(void)
{
    m_enabled = false;
    clearAllCachedData();
}

void UnifiedParticipant::clearAllCachedData(void)
{
    m_getSpecificInfoEx->clearCachedData();
    m_setSpecificInfoEx->clearCachedData();

    // Clear all cached data on all domains
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->clearAllCachedData();
        }
    }
}

Bool UnifiedParticipant::isParticipantEnabled(void)
{
    return m_enabled;
}

void UnifiedParticipant::createDomain(const Guid& guid, UIntN participantIndex, UIntN domainIndex, Bool enabled,
    DomainType::Type domainType, const std::string& name, const std::string& description,
    DomainFunctionalityVersions domainFunctionalityVersions)
{
    throwIfDomainIndexLocationInvalid(domainIndex);

    // Create domain class
    UnifiedDomain* domain = new UnifiedDomain(guid, participantIndex, domainIndex, enabled, domainType, name, description, domainFunctionalityVersions,
        m_classFactories, m_participantServicesInterface);

    // Add to domain set
    insertDomainAtIndexLocation(domain, domainIndex);

    updateDomainEventRegistrations();

    //
    // When domains come on-line, we need to send any known Config TDP information down to all the domains in the
    // participant so they can make adjustments to their controls.  Event notifications that the control capabilities
    // may have changed are then submitted to the work item queue.
    //
    sendConfigTdpInfoToAllDomainsAndCreateNotification();
}

void UnifiedParticipant::throwIfDomainIndexLocationInvalid(UIntN domainIndex)
{
    if ((domainIndex == m_domains.size()) ||
        ((domainIndex < m_domains.size()) && (m_domains[domainIndex] == nullptr)))
    {
        // do nothing
    }
    else
    {
        throw dptf_exception("Domain cannot be added at domain index specified.");
    }
}

void UnifiedParticipant::insertDomainAtIndexLocation(UnifiedDomain* domain, UIntN domainIndex)
{
    if (domainIndex < m_domains.size())
    {
        if (m_domains[domainIndex] != nullptr)
        {
            throw dptf_exception("Received request to add domain at index that is already used.");
        }
        m_domains[domainIndex] = domain;
    }
    else if (domainIndex == m_domains.size())
    {
        m_domains.push_back(domain);
    }
    else
    {
        // domainIndex > m_domains.size()
        throw dptf_exception("Received invalid domain index.");
    }
}

void UnifiedParticipant::destroyDomain(const Guid& guid)
{
    UIntN i = 0;

    for (i = 0; i < m_domains.size(); i++)
    {
        if ((m_domains[i] != nullptr) && (m_domains[i]->getGuid() == guid))
        {
            break;
        }
    }

    if (i == m_domains.size())
    {
        throw dptf_exception("Domain not found.");
    }

    DELETE_MEMORY_TC(m_domains[i]);

    updateDomainEventRegistrations();
}

void UnifiedParticipant::destroyAllDomains(void)
{
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        DELETE_MEMORY_TC(m_domains[i]);
    }

    updateDomainEventRegistrations();
}

void UnifiedParticipant::updateDomainEventRegistrations(void)
{
    // We register for events at the participant level, not the domain level.  This function
    // looks across all domains to see if any domain needs to register for the specific event.  Domains
    // can't do this themselves as they don't have visibility into the other domains.  This is coded at the
    // participant level since we don't want to have one domain go away and unregister for an event
    // that another domain needs.

    UIntN configTdpTotal = 0;
    UIntN coreControlTotal = 0;
    UIntN displayControlTotal = 0;
    UIntN domainPriorityTotal = 0;
    UIntN performanceControlTotal = 0;
    UIntN powerControlTotal = 0;
    UIntN rfProfileTotal = 0;
    UIntN temperatureThresholdTotal = 0;

    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if ((m_domains[i] != nullptr) && (m_domains[i]->isEnabled() == true))
        {
            // We only register for the event if the domain is enabled and the version of the functionality
            // is > 0.  If it is 0 then the functionality isn't being used.  So, if all domains total 0 for
            // the functionality, then the event isn't requested.
            DomainFunctionalityVersions versions = m_domains[i]->getDomainFunctionalityVersions();
            configTdpTotal += versions.configTdpControlVersion;
            coreControlTotal += versions.coreControlVersion;
            displayControlTotal += versions.displayControlVersion;
            domainPriorityTotal += versions.domainPriorityVersion;
            performanceControlTotal += versions.performanceControlVersion;
            powerControlTotal += versions.powerControlVersion;
            rfProfileTotal += versions.rfProfileControlVersion;
            rfProfileTotal += versions.rfProfileStatusVersion;
            temperatureThresholdTotal += versions.temperatureVersion;
        }
    }

    m_configTdpEventsRegistered = updateDomainEventRegistration(configTdpTotal,
        m_configTdpEventsRegistered, ParticipantEvent::Type::DomainConfigTdpCapabilityChanged);

    m_coreControlEventsRegistered = updateDomainEventRegistration(coreControlTotal,
        m_coreControlEventsRegistered, ParticipantEvent::Type::DomainCoreControlCapabilityChanged);

    m_displayControlEventsRegistered = updateDomainEventRegistration(displayControlTotal,
        m_displayControlEventsRegistered,
        ParticipantEvent::Type::DomainDisplayControlCapabilityChanged,
        ParticipantEvent::Type::DomainDisplayStatusChanged);

    m_domainPriorityEventsRegistered = updateDomainEventRegistration(domainPriorityTotal,
        m_domainPriorityEventsRegistered, ParticipantEvent::Type::DomainPriorityChanged);

    m_performanceControlEventsRegistered = updateDomainEventRegistration(performanceControlTotal,
        m_performanceControlEventsRegistered,
        ParticipantEvent::Type::DomainPerformanceControlCapabilityChanged,
        ParticipantEvent::Type::DomainPerformanceControlsChanged);

    m_powerControlEventsRegistered = updateDomainEventRegistration(powerControlTotal,
        m_powerControlEventsRegistered, ParticipantEvent::Type::DomainPowerControlCapabilityChanged);

    m_rfProfileEventsRegistered = updateDomainEventRegistration(rfProfileTotal,
        m_rfProfileEventsRegistered,
        ParticipantEvent::Type::DomainRadioConnectionStatusChanged,
        ParticipantEvent::Type::DomainRfProfileChanged);

    m_temperatureThresholdEventsRegistered = updateDomainEventRegistration(temperatureThresholdTotal,
        m_temperatureThresholdEventsRegistered, ParticipantEvent::Type::DomainTemperatureThresholdCrossed);
}

Bool UnifiedParticipant::updateDomainEventRegistration(UIntN total, Bool currentlyRegistered,
    ParticipantEvent::Type participantEvent_0,
    ParticipantEvent::Type participantEvent_1)
{
    Bool newRegistration = currentlyRegistered;

    if (total > 0 && currentlyRegistered == false)
    {
        // need to register

        if (participantEvent_0 != ParticipantEvent::Type::Invalid)
        {
            m_participantServicesInterface->registerEvent(participantEvent_0);
        }

        if (participantEvent_1 != ParticipantEvent::Type::Invalid)
        {
            m_participantServicesInterface->registerEvent(participantEvent_1);
        }

        newRegistration = true;
    }
    else if (total == 0 && currentlyRegistered == true)
    {
        // need to unregister

        if (participantEvent_0 != ParticipantEvent::Type::Invalid)
        {
            m_participantServicesInterface->unregisterEvent(participantEvent_0);
        }

        if (participantEvent_1 != ParticipantEvent::Type::Invalid)
        {
            m_participantServicesInterface->unregisterEvent(participantEvent_1);
        }

        newRegistration = false;
    }

    return newRegistration;
}

void UnifiedParticipant::enableDomain(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->enable();
    updateDomainEventRegistrations();
}

void UnifiedParticipant::disableDomain(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->disable();
    updateDomainEventRegistrations();
}

Bool UnifiedParticipant::isDomainEnabled(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->isEnabled();
}

std::string UnifiedParticipant::getName() const
{
    return m_name;
}

XmlNode* UnifiedParticipant::getXml(UIntN domainIndex) const
{
    XmlNode* participantRoot = XmlNode::createWrapperElement("participant");

    participantRoot->addChild(XmlNode::createDataElement("index", StatusFormat::friendlyValue(m_participantIndex)));

    // FIXME : Remove the const_cast.  All of the get functions should be const if possible.
    ParticipantProperties props = const_cast<UnifiedParticipant*>(this)->getParticipantProperties(Constants::Invalid);
    participantRoot->addChild(props.getXml());

    if (domainIndex == 0 || domainIndex == Constants::Invalid)
    {
        // Specific Info XML Status
        try
        {
            participantRoot->addChild(m_getSpecificInfoEx->getXml(Constants::Invalid));
        }
        catch (not_implemented)
        {
        }
        catch (...)
        {
            // Write message log error
            m_participantServicesInterface->writeMessageWarning(ParticipantMessage(FLF, "Unable to get specific info XML status!"));
        }
    }

    XmlNode* domainsRoot = XmlNode::createWrapperElement("domains");
    participantRoot->addChild(domainsRoot);

    // Return all the domains if Constants::Invalid was passed
    // Otherwise return just the queried index
    if (domainIndex == Constants::Invalid)
    {
        for (UIntN i = 0; i < m_domains.size(); i++)
        {
            domainsRoot->addChild(m_domains[i]->getXml());
        }
    }
    else
    {
        domainsRoot->addChild(m_domains[domainIndex]->getXml());
    }

    return participantRoot;
}

XmlNode* UnifiedParticipant::getStatusAsXml(UIntN domainIndex) const
{
    XmlNode* participantRoot = XmlNode::createRoot();

    XmlNode* formatId = XmlNode::createComment("format_id=" + FormatId.toString());

    participantRoot->addChild(formatId);

    participantRoot->addChild(getXml(domainIndex));

    return participantRoot;
}

void UnifiedParticipant::connectedStandbyEntry(void)
{
    // FIXME:  Not sure if the participant needs to do anything here.  If we do, we will probably have different
    // functionality for different participants and will need a class factory to create the versions.
    //throw implement_me();
}

void UnifiedParticipant::connectedStandbyExit(void)
{
    // FIXME:  Not sure if the participant needs to do anything here.  If we do, we will probably have different
    // functionality for different participants and will need a class factory to create the versions.
    //throw implement_me();
}

void UnifiedParticipant::suspend(void)
{
    // FIXME:  Not sure if the participant needs to do anything here.  If we do, we will probably have different
    // functionality for different participants and will need a class factory to create the versions.
    //throw implement_me();
}

void UnifiedParticipant::resume(void)
{
    clearAllCachedData();
}

void UnifiedParticipant::domainConfigTdpCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getConfigTdpControlInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainCoreControlCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getCoreControlInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainDisplayControlCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getDisplayControlInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainDisplayStatusChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getDisplayControlInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPerformanceControlCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getPerformanceControlInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPerformanceControlsChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getPerformanceControlInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPowerControlCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getPowerControlInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPriorityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getDomainPriorityInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getRfProfileStatusInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainRfProfileChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getRfProfileStatusInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainTemperatureThresholdCrossed(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            m_domains[i]->getTemperatureInterfaceExPtr()->clearCachedData();
        }
    }
}

void UnifiedParticipant::participantSpecificInfoChanged(void)
{
    m_getSpecificInfoEx->clearCachedData();
}

ActiveControlStaticCaps UnifiedParticipant::getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getActiveControlInterfacePtr()->getActiveControlStaticCaps(
        participantIndex, domainIndex);
}

ActiveControlStatus UnifiedParticipant::getActiveControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getActiveControlInterfacePtr()->getActiveControlStatus(
        participantIndex, domainIndex);
}

ActiveControlSet UnifiedParticipant::getActiveControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getActiveControlInterfacePtr()->getActiveControlSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setActiveControl(UIntN participantIndex, UIntN domainIndex, UIntN controlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getActiveControlInterfacePtr()->setActiveControl(
        participantIndex, domainIndex, controlIndex);
}

void UnifiedParticipant::setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getActiveControlInterfacePtr()->setActiveControl(
        participantIndex, domainIndex, fanSpeed);
}

ConfigTdpControlDynamicCaps UnifiedParticipant::getConfigTdpControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getConfigTdpControlInterfacePtr()->getConfigTdpControlDynamicCaps(
        participantIndex, domainIndex);
}

ConfigTdpControlStatus UnifiedParticipant::getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getConfigTdpControlInterfacePtr()->getConfigTdpControlStatus(
        participantIndex, domainIndex);
}

ConfigTdpControlSet UnifiedParticipant::getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getConfigTdpControlInterfacePtr()->getConfigTdpControlSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setConfigTdpControl(UIntN participantIndex, UIntN domainIndex, UIntN configTdpControlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getConfigTdpControlInterfacePtr()->setConfigTdpControl(
        participantIndex, domainIndex, configTdpControlIndex);

    //
    // If the Config TDP level changes, we need to send that information down to all the domains in the participant
    // so they can make adjustments to their controls.  Event notifications that the control capabilities may have
    // changed are then submitted to the work item queue.
    //
    sendConfigTdpInfoToAllDomainsAndCreateNotification();
}

CoreControlStaticCaps UnifiedParticipant::getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControlInterfacePtr()->getCoreControlStaticCaps(
        participantIndex, domainIndex);
}

CoreControlDynamicCaps UnifiedParticipant::getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControlInterfacePtr()->getCoreControlDynamicCaps(
        participantIndex, domainIndex);
}

CoreControlLpoPreference UnifiedParticipant::getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControlInterfacePtr()->getCoreControlLpoPreference(
        participantIndex, domainIndex);
}

CoreControlStatus UnifiedParticipant::getCoreControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControlInterfacePtr()->getCoreControlStatus(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setActiveCoreControl(UIntN participantIndex, UIntN domainIndex, const CoreControlStatus& coreControlStatus)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getCoreControlInterfacePtr()->setActiveCoreControl(
        participantIndex, domainIndex, coreControlStatus);
}

DisplayControlDynamicCaps UnifiedParticipant::getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControlInterfacePtr()->getDisplayControlDynamicCaps(
        participantIndex, domainIndex);
}

DisplayControlStatus UnifiedParticipant::getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControlInterfacePtr()->getDisplayControlStatus(
        participantIndex, domainIndex);
}

DisplayControlSet UnifiedParticipant::getDisplayControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControlInterfacePtr()->getDisplayControlSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setDisplayControl(UIntN participantIndex, UIntN domainIndex,
    UIntN displayControlIndex, Bool isOverridable)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getDisplayControlInterfacePtr()->setDisplayControl(
        participantIndex, domainIndex, displayControlIndex, isOverridable);
}

PerformanceControlStaticCaps UnifiedParticipant::getPerformanceControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControlInterfacePtr()->getPerformanceControlStaticCaps(
        participantIndex, domainIndex);
}

PerformanceControlDynamicCaps UnifiedParticipant::getPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControlInterfacePtr()->getPerformanceControlDynamicCaps(
        participantIndex, domainIndex);
}

PerformanceControlStatus UnifiedParticipant::getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControlInterfacePtr()->getPerformanceControlStatus(
        participantIndex, domainIndex);
}

PerformanceControlSet UnifiedParticipant::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControlInterfacePtr()->getPerformanceControlSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPerformanceControlInterfacePtr()->setPerformanceControl(
        participantIndex, domainIndex, performanceControlIndex);
}

void UnifiedParticipant::setPixelClockControl(UIntN participantIndex, UIntN domainIndex,
    const PixelClockDataSet& pixelClockDataSet)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPixelClockControlInterfacePtr()->setPixelClockControl(participantIndex,
        domainIndex, pixelClockDataSet);
}

PixelClockCapabilities UnifiedParticipant::getPixelClockCapabilities(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPixelClockStatusInterfacePtr()->getPixelClockCapabilities(participantIndex,
        domainIndex);
}

PixelClockDataSet UnifiedParticipant::getPixelClockDataSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPixelClockStatusInterfacePtr()->getPixelClockDataSet(participantIndex,
        domainIndex);
}

PowerControlDynamicCapsSet UnifiedParticipant::getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerControlInterfacePtr()->getPowerControlDynamicCapsSet(
        participantIndex, domainIndex);
}

PowerControlStatusSet UnifiedParticipant::getPowerControlStatusSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerControlInterfacePtr()->getPowerControlStatusSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setPowerControl(UIntN participantIndex, UIntN domainIndex, const PowerControlStatusSet& powerControlSet)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPowerControlInterfacePtr()->setPowerControl(
        participantIndex, domainIndex, powerControlSet);
}

PowerStatus UnifiedParticipant::getPowerStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerStatusInterfacePtr()->getPowerStatus(
        participantIndex, domainIndex);
}

DomainPriority UnifiedParticipant::getDomainPriority(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDomainPriorityInterfacePtr()->getDomainPriority(
        participantIndex, domainIndex);
}

RfProfileCapabilities UnifiedParticipant::getRfProfileCapabilities(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getRfProfileControlInterfacePtr()->getRfProfileCapabilities(participantIndex,
        domainIndex);
}

void UnifiedParticipant::setRfProfileCenterFrequency(UIntN participantIndex, UIntN domainIndex,
    const Frequency& centerFrequency)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getRfProfileControlInterfacePtr()->setRfProfileCenterFrequency(participantIndex,
        domainIndex, centerFrequency);
}

RfProfileData UnifiedParticipant::getRfProfileData(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getRfProfileStatusInterfacePtr()->getRfProfileData(participantIndex,
        domainIndex);
}

TemperatureStatus UnifiedParticipant::getTemperatureStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getTemperatureInterfacePtr()->getTemperatureStatus(
        participantIndex, domainIndex);
}

TemperatureThresholds UnifiedParticipant::getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getTemperatureInterfacePtr()->getTemperatureThresholds(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setTemperatureThresholds(UIntN participantIndex, UIntN domainIndex,
    const TemperatureThresholds& temperatureThresholds)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getTemperatureInterfacePtr()->setTemperatureThresholds(
        participantIndex, domainIndex, temperatureThresholds);
}

UtilizationStatus UnifiedParticipant::getUtilizationStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getUtilizationInterfacePtr()->getUtilizationStatus(
        participantIndex, domainIndex);
}

std::map<ParticipantSpecificInfoKey::Type, UIntN> UnifiedParticipant::getParticipantSpecificInfo(UIntN participantIndex,
    const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo)
{
    return m_getSpecificInfo->getParticipantSpecificInfo(participantIndex, requestedInfo);
}

ParticipantProperties UnifiedParticipant::getParticipantProperties(UIntN participantIndex)
{
    return ParticipantProperties(m_guid, m_name, m_description, m_busType,
        m_pciInfo, m_acpiInfo);
}

DomainPropertiesSet UnifiedParticipant::getDomainPropertiesSet(UIntN participantIndex)
{
    std::vector<DomainProperties> domainPropertiesSet;

    for (UIntN i = 0; i < m_domains.size(); i++)
    {
        if (m_domains[i] != nullptr)
        {
            DomainProperties domainProperties(m_domains[i]->getGuid(), i, m_domains[i]->isEnabled(),
                m_domains[i]->getDomainType(), m_domains[i]->getName(), m_domains[i]->getDescription(),
                m_domains[i]->getDomainFunctionalityVersions());
            domainPropertiesSet.push_back(domainProperties);
        }
    }

    return domainPropertiesSet;
}

void UnifiedParticipant::setParticipantDeviceTemperatureIndication(UIntN participantIndex, const Temperature& temperature)
{
    m_setSpecificInfo->setParticipantDeviceTemperatureIndication(participantIndex, temperature);
}

void UnifiedParticipant::setParticipantCoolingPolicy(UIntN participantIndex, const CoolingPreference& coolingPreference)
{
    m_setSpecificInfo->setParticipantCoolingPolicy(participantIndex, coolingPreference);
}

void UnifiedParticipant::throwIfDomainInvalid(UIntN domainIndex)
{
    if ((domainIndex >= m_domains.size()) || (m_domains[domainIndex] == nullptr))
    {
        throw dptf_exception("Domain index is invalid.");
    }
}

void UnifiedParticipant::sendConfigTdpInfoToAllDomainsAndCreateNotification()
{
    try
    {
        // get Config TDP information
        ConfigTdpControlStatus configTdpControlStatus = getFirstConfigTdpControlStatus();
        ConfigTdpControlSet configTdpControlSet = getFirstConfigTdpControlSet();

        // send it to each domain's power and performance control
        for (UIntN domainIndex = 0; domainIndex < m_domains.size(); domainIndex++)
        {
            if (m_domains[domainIndex] != nullptr)
            {
                try
                {
                    m_domains[domainIndex]->getPerformanceControlConfigTdpSyncInterfacePtr()->
                        updateBasedOnConfigTdpInformation(m_participantIndex, domainIndex, configTdpControlSet, 
                        configTdpControlStatus);
                    m_participantServicesInterface->createEventDomainPerformanceControlCapabilityChanged();
                }
                catch (...)
                {
                }
            }
        }
    }
    catch (...)
    {
    }
}

ConfigTdpControlStatus UnifiedParticipant::getFirstConfigTdpControlStatus()
{
    Bool foundStatus = false;
    ConfigTdpControlStatus configTdpControlStatus(Constants::Invalid);
    for (UIntN domainIndex = 0; domainIndex < m_domains.size(); domainIndex++)
    {
        if (m_domains[domainIndex] != nullptr)
        {
            try
            {
                configTdpControlStatus =
                    m_domains[domainIndex]->getConfigTdpControlInterfacePtr()->getConfigTdpControlStatus(
                        m_participantIndex, domainIndex);
                foundStatus = true;
                break;
            }
            catch (...)
            {
            }
        }
    }

    if (foundStatus == true)
    {
        return configTdpControlStatus;
    }
    else
    {
        throw dptf_exception("No domain provided any Config TDP control status.");
    }
}

ConfigTdpControlSet UnifiedParticipant::getFirstConfigTdpControlSet()
{
    Bool foundControlSet = false;
    ConfigTdpControlSet configTdpControlSet(std::vector<ConfigTdpControl>(1, ConfigTdpControl(0, 0, 0, 0)));
    for (UIntN domainIndex = 0; domainIndex < m_domains.size(); domainIndex++)
    {
        if (m_domains[domainIndex] != nullptr)
        {
            try
            {
                configTdpControlSet =
                    m_domains[domainIndex]->getConfigTdpControlInterfacePtr()->getConfigTdpControlSet(
                        m_participantIndex, domainIndex);
                foundControlSet = true;
                break;
            }
            catch (...)
            {
            }
        }
    }

    if (foundControlSet == true)
    {
        return configTdpControlSet;
    }
    else
    {
        throw dptf_exception("No domain provided any Config TDP control set.");
    }
}