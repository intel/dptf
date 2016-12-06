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

#include "UnifiedParticipant.h"
#include "XmlNode.h"
#include "StatusFormat.h"
#include "MapOps.h"

static const Guid FormatId(0xF0, 0xCB, 0x64, 0x06, 0xE4, 0x2B, 0x46, 0xB5, 0x9C, 0x85, 0x32, 0xD1, 0xA1, 0xB7, 0xCB, 0x68);

UnifiedParticipant::UnifiedParticipant(void)
{
    initialize();
}

UnifiedParticipant::UnifiedParticipant(const ControlFactoryList& classFactories) :
    m_classFactories(classFactories)
{
    initialize();

    // TODO: Check if this is needed
    // In this case, which is provided for validation purposes, any class factories that aren't
    // provided via the constructor will be filled in.  This allows test code to be written
    // that only sends in the 'fake' factories as needed.
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
    m_setSpecificInfo = nullptr;
    m_configTdpEventsRegistered = false;
    m_coreControlEventsRegistered = false;
    m_displayControlEventsRegistered = false;
    m_domainPriorityEventsRegistered = false;
    m_performanceControlEventsRegistered = false;
    m_powerControlEventsRegistered = false;
    m_rfProfileEventsRegistered = false;
    m_temperatureEventsRegistered = false;
    m_powerStatusEventsRegistered = false;
    m_loggingEventsRegistered = false;
}

UnifiedParticipant::~UnifiedParticipant(void)
{

}

void UnifiedParticipant::createParticipant(const Guid& guid, UIntN participantIndex, Bool enabled,
    const std::string& name, const std::string& description,
    BusType::Type busType, const PciInfo& pciInfo, const AcpiInfo& acpiInfo,
    std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
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

    m_getSpecificInfo.reset(dynamic_cast<ParticipantGetSpecificInfoBase*>(m_classFactories.getFactory(ControlFactoryType::GetSpecificInfo)->make(
            participantIndex, 0, GetSpecificInfoVersionDefault, m_participantServicesInterface)));

    m_setSpecificInfo.reset(dynamic_cast<ParticipantSetSpecificInfoBase*>(m_classFactories.getFactory(ControlFactoryType::SetSpecificInfo)->make(
            participantIndex, 0, SetSpecificInfoVersionDefault, m_participantServicesInterface)));

    m_participantServicesInterface->registerEvent(ParticipantEvent::ParticipantSpecificInfoChanged);
    m_participantServicesInterface->registerEvent(ParticipantEvent::DptfResume);
}

void UnifiedParticipant::destroyParticipant(void)
{
    destroyAllDomains();

    if (m_participantServicesInterface != nullptr) {
        m_participantServicesInterface->unregisterEvent(ParticipantEvent::ParticipantSpecificInfoChanged);
        m_participantServicesInterface->unregisterEvent(ParticipantEvent::DptfResume);
    }
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
    m_getSpecificInfo->clearCachedData();
    m_setSpecificInfo->clearCachedData();

    // Clear all cached data on all domains
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->clearAllCachedData();
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
    auto domain = std::make_shared<UnifiedDomain>(guid, participantIndex, domainIndex, enabled, domainType, name, description, domainFunctionalityVersions,
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
    auto matchedDomain = m_domains.find(domainIndex);
    if ((matchedDomain != m_domains.end()) && (matchedDomain->second != nullptr))
    {
        throw dptf_exception("Domain cannot be added at domain index specified.");
    }
}

void UnifiedParticipant::insertDomainAtIndexLocation(std::shared_ptr<UnifiedDomain> domain, UIntN domainIndex)
{
    auto matchedDomain = m_domains.find(domainIndex);
    if (matchedDomain != m_domains.end() && matchedDomain->second != nullptr)
    {
        throw dptf_exception("Received request to add domain at index that is already used.");
    }
    else
    {
        m_domains[domainIndex] = domain;
    }
}

void UnifiedParticipant::destroyDomain(const Guid& guid)
{
    auto domainIndexes = MapOps<UIntN, std::shared_ptr<UnifiedDomain>>::getKeys(m_domains);
    for(auto index = domainIndexes.begin(); index != domainIndexes.end(); ++index)
    {
        if ((m_domains[*index] != nullptr) && (m_domains[*index]->getGuid() == guid))
        {
            m_domains.erase(*index);
        }
    }

    updateDomainEventRegistrations();
}

void UnifiedParticipant::destroyAllDomains(void)
{
    m_domains.clear();
    updateDomainEventRegistrations();
}

void UnifiedParticipant::updateDomainEventRegistrations(void)
{
    // We register for events at the participant level, not the domain level.  This function
    // looks across all domains to see if any domain needs to register for the specific event.  Domains
    // can't do this themselves as they don't have visibility into the other domains.  This is coded at the
    // participant level since we don't want to have one domain go away and unregister for an event
    // that another domain needs.

    UInt8 activeControlTotal = 0;
    UIntN configTdpTotal = 0;
    UIntN coreControlTotal = 0;
    UIntN displayControlTotal = 0;
    UIntN domainPriorityTotal = 0;
    UIntN performanceControlTotal = 0;
    UIntN powerControlTotal = 0;
    UIntN rfProfileTotal = 0;
    UIntN temperatureControlTotal = 0;
    UIntN platformPowerStatusControlTotal = 0;
    UIntN loggingTotal = 0;

    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if ((domain->second != nullptr) && (domain->second->isEnabled() == true))
        {
            // We only register for the event if the domain is enabled and the version of the functionality
            // is > 0.  If it is 0 then the functionality isn't being used.  So, if all domains total 0 for
            // the functionality, then the event isn't requested.
            DomainFunctionalityVersions versions = domain->second->getDomainFunctionalityVersions();
            activeControlTotal += versions.activeControlVersion;
            configTdpTotal += versions.configTdpControlVersion;
            coreControlTotal += versions.coreControlVersion;
            displayControlTotal += versions.displayControlVersion;
            domainPriorityTotal += versions.domainPriorityVersion;
            performanceControlTotal += versions.performanceControlVersion;
            powerControlTotal += versions.powerControlVersion;
            rfProfileTotal += versions.rfProfileControlVersion;
            rfProfileTotal += versions.rfProfileStatusVersion;
            temperatureControlTotal += versions.temperatureVersion;
            temperatureControlTotal += versions.temperatureThresholdVersion;
            platformPowerStatusControlTotal += versions.platformPowerStatusVersion;
            if ((activeControlTotal != 0) ||
                (configTdpTotal != 0) ||
                (coreControlTotal != 0) ||
                (displayControlTotal != 0) ||
                (domainPriorityTotal != 0) ||
                (performanceControlTotal != 0) ||
                (powerControlTotal != 0) ||
                (rfProfileTotal != 0) ||
                (temperatureControlTotal != 0) ||
                (platformPowerStatusControlTotal != 0))
            {
                loggingTotal = 1;
            }
        }
    }

    std::set<ParticipantEvent::Type> configTdpEvents;
    configTdpEvents.insert(ParticipantEvent::Type::DomainConfigTdpCapabilityChanged);
    m_configTdpEventsRegistered = updateDomainEventRegistration(configTdpTotal, m_configTdpEventsRegistered,
        configTdpEvents);

    std::set<ParticipantEvent::Type> coreControlEvents;
    coreControlEvents.insert(ParticipantEvent::Type::DomainCoreControlCapabilityChanged);
    m_coreControlEventsRegistered = updateDomainEventRegistration(coreControlTotal, m_coreControlEventsRegistered,
        coreControlEvents);

    std::set<ParticipantEvent::Type> displayControlEvents;
    displayControlEvents.insert(ParticipantEvent::Type::DomainDisplayControlCapabilityChanged);
    displayControlEvents.insert(ParticipantEvent::Type::DomainDisplayStatusChanged);
    m_displayControlEventsRegistered = updateDomainEventRegistration(displayControlTotal,
        m_displayControlEventsRegistered, displayControlEvents);

    std::set<ParticipantEvent::Type> domainPriorityEvents;
    domainPriorityEvents.insert(ParticipantEvent::Type::DomainPriorityChanged);
    m_domainPriorityEventsRegistered = updateDomainEventRegistration(domainPriorityTotal,
        m_domainPriorityEventsRegistered, domainPriorityEvents);

    std::set<ParticipantEvent::Type> performanceControlEvents;
    performanceControlEvents.insert(ParticipantEvent::Type::DomainPerformanceControlCapabilityChanged);
    performanceControlEvents.insert(ParticipantEvent::Type::DomainPerformanceControlsChanged);
    m_performanceControlEventsRegistered = updateDomainEventRegistration(performanceControlTotal,
        m_performanceControlEventsRegistered, performanceControlEvents);

    std::set<ParticipantEvent::Type> powerControlEvents;
    powerControlEvents.insert(ParticipantEvent::Type::DomainPowerControlCapabilityChanged);
    m_powerControlEventsRegistered = updateDomainEventRegistration(powerControlTotal, m_powerControlEventsRegistered,
        powerControlEvents);

    std::set<ParticipantEvent::Type> radioEvents;
    radioEvents.insert(ParticipantEvent::Type::DomainRadioConnectionStatusChanged);
    radioEvents.insert(ParticipantEvent::Type::DomainRfProfileChanged);
    m_rfProfileEventsRegistered = updateDomainEventRegistration(rfProfileTotal, m_rfProfileEventsRegistered,
        radioEvents);

    std::set<ParticipantEvent::Type> temperatureEvents;
    temperatureEvents.insert(ParticipantEvent::Type::DomainTemperatureThresholdCrossed);
    temperatureEvents.insert(ParticipantEvent::Type::DomainVirtualSensorCalibrationTableChanged);
    temperatureEvents.insert(ParticipantEvent::Type::DomainVirtualSensorPollingTableChanged);
    temperatureEvents.insert(ParticipantEvent::Type::DomainVirtualSensorRecalcChanged);
    m_temperatureEventsRegistered = updateDomainEventRegistration(temperatureControlTotal,
        m_temperatureEventsRegistered, temperatureEvents);

    std::set<ParticipantEvent::Type> platformPowerStatusEvents;
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainBatteryStatusChanged);
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainBatteryInformationChanged);
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainPlatformPowerSourceChanged);
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainAdapterPowerRatingChanged);
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainChargerTypeChanged);
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainPlatformRestOfPowerChanged);
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainACPeakPowerChanged);
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainACPeakTimeWindowChanged);
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainMaxBatteryPowerChanged);
    platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainPlatformBatterySteadyStateChanged);
    m_powerStatusEventsRegistered = updateDomainEventRegistration(platformPowerStatusControlTotal,
        m_powerStatusEventsRegistered, platformPowerStatusEvents);

    std::set<ParticipantEvent::Type> loggingEvents;
    loggingEvents.insert(ParticipantEvent::Type::DptfParticipantActivityLoggingEnabled);
    loggingEvents.insert(ParticipantEvent::Type::DptfParticipantActivityLoggingDisabled);
    m_loggingEventsRegistered = updateDomainEventRegistration(loggingTotal,
        m_loggingEventsRegistered, loggingEvents);
}

Bool UnifiedParticipant::updateDomainEventRegistration(UIntN total, Bool currentlyRegistered,
    std::set<ParticipantEvent::Type> participantEvents)
{
    Bool newRegistration = currentlyRegistered;

    if (total > 0 && currentlyRegistered == false)
    {
        // need to register
        for (auto pEvent = participantEvents.begin(); pEvent != participantEvents.end(); pEvent++)
        {
            if (*pEvent != ParticipantEvent::Type::Invalid)
            {
                m_participantServicesInterface->registerEvent(*pEvent);
            }
        }

        newRegistration = true;
    }
    else if (total == 0 && currentlyRegistered == true)
    {
        // need to unregister
        for (auto pEvent = participantEvents.begin(); pEvent != participantEvents.end(); pEvent++)
        {
            if (*pEvent != ParticipantEvent::Type::Invalid)
            {
                m_participantServicesInterface->unregisterEvent(*pEvent);
            }
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

std::shared_ptr<XmlNode> UnifiedParticipant::getXml(UIntN domainIndex) const
{
    auto participantRoot = XmlNode::createWrapperElement("participant");

    participantRoot->addChild(XmlNode::createDataElement("index", StatusFormat::friendlyValue(m_participantIndex)));
    ParticipantProperties props = getParticipantProperties(Constants::Invalid);
    try
    {
        participantRoot->addChild(props.getXml());
    }
    catch (...)
    {
        m_participantServicesInterface->writeMessageWarning(ParticipantMessage(FLF, "Unable to get participant properties XML status!"));
    }

    if (domainIndex == 0 || domainIndex == Constants::Invalid)
    {
        // Specific Info XML Status
        try
        {
            if (m_getSpecificInfo != nullptr)
            {
                participantRoot->addChild(m_getSpecificInfo->getXml(Constants::Invalid));
            }
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

    auto domainsRoot = XmlNode::createWrapperElement("domains");
    participantRoot->addChild(domainsRoot);

    // Return all the domains if Constants::Invalid was passed
    // Otherwise return just the queried index
    if (domainIndex == Constants::Invalid)
    {
        for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
        {
            try
            {
                if (domain->second != nullptr)
                {
                    domainsRoot->addChild(domain->second->getXml());
                }
            }
            catch (...)
            {
                m_participantServicesInterface->writeMessageWarning(ParticipantMessage(FLF, "Unable to get domain XML status for domain " + domain->first));
            }
        }
    }
    else
    {
        try
        {
            auto matchedDomain = m_domains.find(domainIndex);
            if (matchedDomain != m_domains.end() && matchedDomain->second != nullptr)
            {
                domainsRoot->addChild(matchedDomain->second->getXml());
            }
        }
        catch (...)
        {
            m_participantServicesInterface->writeMessageWarning(ParticipantMessage(FLF, "Unable to get domain XML status for domain " + domainIndex));
        }
    }

    return participantRoot;
}

std::shared_ptr<XmlNode> UnifiedParticipant::getStatusAsXml(UIntN domainIndex) const
{
    auto participantRoot = XmlNode::createRoot();

    auto formatId = XmlNode::createComment("format_id=" + FormatId.toString());

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

void UnifiedParticipant::activityLoggingEnabled(UInt32 domainIndex, UInt32 capabilityBitMask)
{
    if (domainIndex == Constants::Invalid)
    {
        for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
        {
            enableActivityLoggingForDomain(domain->first, capabilityBitMask);
        }
    }
    else
    {
        enableActivityLoggingForDomain(domainIndex, capabilityBitMask);
    }
}

void UnifiedParticipant::activityLoggingDisabled(UInt32 domainIndex, UInt32 capabilityBitMask)
{
    if (domainIndex == Constants::Invalid)
    {
        for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
        {
            disableActivityLoggingForDomain(domain->first, capabilityBitMask);
        }
    }
    else
    {
        disableActivityLoggingForDomain(domainIndex, capabilityBitMask);
    }
}

void UnifiedParticipant::enableActivityLoggingForDomain(UInt32 domainIndex, UInt32 capabilityBitMask)
{
    auto matchedDomain = m_domains.find(domainIndex);
    if (matchedDomain != m_domains.end() && matchedDomain->second != nullptr)
    {
        for (UInt32 index = 0; index <= MAX_ESIF_CAPABILITY_TYPE_ENUM_VALUE; index++)
        {
            UInt32 currentCapability = Constants::Invalid;
            try
            {
                currentCapability = (Capability::ToCapabilityId((eEsifCapabilityType)index));

            }
            catch (dptf_exception&)
            {
                // Probably attempted to convert an invalid capability to UInt32. Ignore!
            }

            if ((currentCapability != Constants::Invalid) && ((capabilityBitMask & currentCapability) != 0))
            {
                switch ((eEsifCapabilityType)index)
                {
                case ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL:
                    if (matchedDomain->second->getDomainFunctionalityVersions().activeControlVersion > 0)
                    {
                        matchedDomain->second->getActiveControl()->enableActivityLogging();
                        sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
                    }
                    break;
                case ESIF_CAPABILITY_TYPE_CORE_CONTROL:
                    if (matchedDomain->second->getDomainFunctionalityVersions().coreControlVersion > 0)
                    {
                        matchedDomain->second->getCoreControl()->enableActivityLogging();
                        sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
                    }
                    break;
                case ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL:
                    if (matchedDomain->second->getDomainFunctionalityVersions().displayControlVersion > 0)
                    {
                        matchedDomain->second->getDisplayControl()->enableActivityLogging();
                        sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
                    }
                    break;
                case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
                    if (matchedDomain->second->getDomainFunctionalityVersions().performanceControlVersion > 0)
                    {
                        matchedDomain->second->getPerformanceControl()->enableActivityLogging();
                        sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
                    }
                    break;
                case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
                    if (matchedDomain->second->getDomainFunctionalityVersions().temperatureThresholdVersion > 0)
                    {
                        matchedDomain->second->getTemperatureControl()->enableActivityLogging();
                        sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
                    }
                    break;
                case ESIF_CAPABILITY_TYPE_CTDP_CONTROL:
                    if (matchedDomain->second->getDomainFunctionalityVersions().configTdpControlVersion > 0)
                    {
                        matchedDomain->second->getConfigTdpControl()->enableActivityLogging();
                        sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
                    }
                    break;
                case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
                    if (matchedDomain->second->getDomainFunctionalityVersions().domainPriorityVersion > 0)
                    {
                        matchedDomain->second->getDomainPriorityControl()->enableActivityLogging();
                        sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
                    }
                    break;
                case ESIF_CAPABILITY_TYPE_POWER_CONTROL:
                    if (matchedDomain->second->getDomainFunctionalityVersions().powerControlVersion > 0)
                    {
                        matchedDomain->second->getPowerControl()->enableActivityLogging();
                        sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
                    }
                    break;
                case ESIF_CAPABILITY_TYPE_POWER_STATUS:
                    if (matchedDomain->second->getDomainFunctionalityVersions().powerStatusVersion > 0)
                    {
                        matchedDomain->second->getPowerStatusControl()->enableActivityLogging();
                        sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
                    }
                    break;
                case ESIF_CAPABILITY_TYPE_PIXELCLOCK_CONTROL:
                case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
                default:
                    //To do for other capabilities as part of data logging
                    break;
                }
            }
        }
    }
}

void UnifiedParticipant::disableActivityLoggingForDomain(UInt32 domainIndex, UInt32 capabilityBitMask)
{
    auto matchedDomain = m_domains.find(domainIndex);
    if (matchedDomain != m_domains.end() && matchedDomain->second != nullptr)
    {
        for (UInt32 index = 0; index <= MAX_ESIF_CAPABILITY_TYPE_ENUM_VALUE; index++)
        {
            UInt32 currentCapability = Constants::Invalid;
            try
            {
                currentCapability = (Capability::ToCapabilityId((eEsifCapabilityType)index));

            }
            catch (dptf_exception&)
            {
                // Probably attempted to convert an invalid capability to UInt32. Ignore!
            }

            if ((currentCapability != Constants::Invalid) && ((capabilityBitMask & currentCapability) != 0))
            {
                switch ((eEsifCapabilityType)index)
                {
                case ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL:
                    matchedDomain->second->getActiveControl()->disableActivityLogging();
                    break;
                case ESIF_CAPABILITY_TYPE_CORE_CONTROL:
                    matchedDomain->second->getCoreControl()->disableActivityLogging();
                    break;
                case ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL:
                    matchedDomain->second->getDisplayControl()->disableActivityLogging();
                    break;
                case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
                    matchedDomain->second->getPerformanceControl()->disableActivityLogging();
                    break;
                case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
                    matchedDomain->second->getTemperatureControl()->disableActivityLogging();
                    break;
                case ESIF_CAPABILITY_TYPE_CTDP_CONTROL:
                    matchedDomain->second->getConfigTdpControl()->disableActivityLogging();
                    break;
                case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
                    matchedDomain->second->getDomainPriorityControl()->disableActivityLogging();
                    break;
                case ESIF_CAPABILITY_TYPE_POWER_CONTROL:
                    matchedDomain->second->getPowerControl()->disableActivityLogging();
                    break;
                case ESIF_CAPABILITY_TYPE_PIXELCLOCK_CONTROL:
                case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
                default:
                    //To do for other capabilities as part of data logging
                    break;
                }
            }
        }
    }
}

void UnifiedParticipant::sendActivityLoggingDataIfEnabled(UInt32 domainIndex, eEsifCapabilityType capability)
{
    throwIfDomainInvalid(domainIndex);

    switch (capability)
    {
    case ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL:
        m_domains[domainIndex]->getActiveControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
        break;
    case ESIF_CAPABILITY_TYPE_CORE_CONTROL:
        m_domains[domainIndex]->getCoreControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
        break;
    case ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL:
        m_domains[domainIndex]->getDisplayControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
        break;
    case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
        m_domains[domainIndex]->getPerformanceControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
        break;
    case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
        m_domains[domainIndex]->getTemperatureControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
        break;
    case ESIF_CAPABILITY_TYPE_CTDP_CONTROL:
        m_domains[domainIndex]->getConfigTdpControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
        break;
    case ESIF_CAPABILITY_TYPE_POWER_CONTROL:
        m_domains[domainIndex]->getPowerControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
        break;
    case ESIF_CAPABILITY_TYPE_POWER_STATUS:
        m_domains[domainIndex]->getPowerStatusControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
        break;
    case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
        m_domains[domainIndex]->getDomainPriorityControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
        break;
    case ESIF_CAPABILITY_TYPE_PIXELCLOCK_CONTROL:
    case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
    default:
        break;
    }
}

void UnifiedParticipant::domainConfigTdpCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getConfigTdpControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainCoreControlCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getCoreControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainDisplayControlCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getDisplayControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainDisplayStatusChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getDisplayControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPerformanceControlCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPerformanceControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPerformanceControlsChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPerformanceControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPowerControlCapabilityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPowerControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPriorityChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getDomainPriorityControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getRfProfileStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainRfProfileChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getRfProfileStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainTemperatureThresholdCrossed(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getTemperatureControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::participantSpecificInfoChanged(void)
{
    m_getSpecificInfo->clearCachedData();
}

void UnifiedParticipant::domainVirtualSensorCalibrationTableChanged(void)
{
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getTemperatureControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainVirtualSensorPollingTableChanged(void)
{
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getTemperatureControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainVirtualSensorRecalcChanged(void)
{
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getTemperatureControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainBatteryStatusChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainBatteryInformationChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPlatformPowerSourceChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainAdapterPowerRatingChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainChargerTypeChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPlatformRestOfPowerChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainACPeakPowerChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainACPeakTimeWindowChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainMaxBatteryPowerChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

void UnifiedParticipant::domainPlatformBatterySteadyStateChanged(void)
{
    // Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->getPlatformPowerStatusControl()->clearCachedData();
        }
    }
}

ActiveControlStaticCaps UnifiedParticipant::getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getActiveControl()->getActiveControlStaticCaps(
        participantIndex, domainIndex);
}

ActiveControlStatus UnifiedParticipant::getActiveControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getActiveControl()->getActiveControlStatus(
        participantIndex, domainIndex);
}

ActiveControlSet UnifiedParticipant::getActiveControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getActiveControl()->getActiveControlSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setActiveControl(UIntN participantIndex, UIntN domainIndex, UIntN controlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getActiveControl()->setActiveControl(
        participantIndex, domainIndex, controlIndex);

    // Do not send activity log if we control fans through control index
    // The logging code understands fan speed in percentage only
}

void UnifiedParticipant::setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getActiveControl()->setActiveControl(
        participantIndex, domainIndex, fanSpeed);

    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL);
}

ConfigTdpControlDynamicCaps UnifiedParticipant::getConfigTdpControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getConfigTdpControl()->getConfigTdpControlDynamicCaps(
        participantIndex, domainIndex);
}

ConfigTdpControlStatus UnifiedParticipant::getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getConfigTdpControl()->getConfigTdpControlStatus(
        participantIndex, domainIndex);
}

ConfigTdpControlSet UnifiedParticipant::getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getConfigTdpControl()->getConfigTdpControlSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setConfigTdpControl(UIntN participantIndex, UIntN domainIndex, UIntN configTdpControlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getConfigTdpControl()->setConfigTdpControl(
        participantIndex, domainIndex, configTdpControlIndex);

    //
    // If the Config TDP level changes, we need to send that information down to all the domains in the participant
    // so they can make adjustments to their controls.  Event notifications that the control capabilities may have
    // changed are then submitted to the work item queue.
    //
    sendConfigTdpInfoToAllDomainsAndCreateNotification();

    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_CTDP_CONTROL);
}

CoreControlStaticCaps UnifiedParticipant::getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControl()->getCoreControlStaticCaps(
        participantIndex, domainIndex);
}

CoreControlDynamicCaps UnifiedParticipant::getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControl()->getCoreControlDynamicCaps(
        participantIndex, domainIndex);
}

CoreControlLpoPreference UnifiedParticipant::getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControl()->getCoreControlLpoPreference(
        participantIndex, domainIndex);
}

CoreControlStatus UnifiedParticipant::getCoreControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControl()->getCoreControlStatus(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setActiveCoreControl(UIntN participantIndex, UIntN domainIndex, const CoreControlStatus& coreControlStatus)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getCoreControl()->setActiveCoreControl(
        participantIndex, domainIndex, coreControlStatus);

    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_CORE_CONTROL);
}

DisplayControlDynamicCaps UnifiedParticipant::getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControl()->getDisplayControlDynamicCaps(
        participantIndex, domainIndex);
}

DisplayControlStatus UnifiedParticipant::getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControl()->getDisplayControlStatus(participantIndex, domainIndex);
}

UIntN UnifiedParticipant::getUserPreferredDisplayIndex(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControl()->getUserPreferredDisplayIndex(participantIndex, domainIndex);
}

Bool UnifiedParticipant::isUserPreferredIndexModified(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControl()->isUserPreferredIndexModified(participantIndex, domainIndex);
}

DisplayControlSet UnifiedParticipant::getDisplayControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControl()->getDisplayControlSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setDisplayControl(UIntN participantIndex, UIntN domainIndex,
    UIntN displayControlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getDisplayControl()->setDisplayControl(participantIndex, domainIndex, displayControlIndex);

    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL);
}

void UnifiedParticipant::setDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex, DisplayControlDynamicCaps newCapabilities)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getDisplayControl()->setDisplayControlDynamicCaps(participantIndex, domainIndex, newCapabilities);

    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL);
}

void UnifiedParticipant::setDisplayCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getDisplayControl()->setDisplayCapsLock(participantIndex, domainIndex, lock);
}

PerformanceControlStaticCaps UnifiedParticipant::getPerformanceControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControl()->getPerformanceControlStaticCaps(
        participantIndex, domainIndex);
}

PerformanceControlDynamicCaps UnifiedParticipant::getPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControl()->getPerformanceControlDynamicCaps(
        participantIndex, domainIndex);
}

PerformanceControlStatus UnifiedParticipant::getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControl()->getPerformanceControlStatus(
        participantIndex, domainIndex);
}

PerformanceControlSet UnifiedParticipant::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControl()->getPerformanceControlSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPerformanceControl()->setPerformanceControl(
        participantIndex, domainIndex, performanceControlIndex);

    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PERF_CONTROL);
}

void UnifiedParticipant::setPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex,
    PerformanceControlDynamicCaps newCapabilities)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPerformanceControl()->setPerformanceControlDynamicCaps(
        participantIndex, domainIndex, newCapabilities);

    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PERF_CONTROL);
}

void UnifiedParticipant::setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPerformanceControl()->setPerformanceCapsLock(
        participantIndex, domainIndex, lock);
}

void UnifiedParticipant::setPixelClockControl(UIntN participantIndex, UIntN domainIndex,
    const PixelClockDataSet& pixelClockDataSet)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPixelClockControl()->setPixelClockControl(participantIndex,
        domainIndex, pixelClockDataSet);
}

PixelClockCapabilities UnifiedParticipant::getPixelClockCapabilities(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPixelClockStatusControl()->getPixelClockCapabilities(participantIndex,
        domainIndex);
}

PixelClockDataSet UnifiedParticipant::getPixelClockDataSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPixelClockStatusControl()->getPixelClockDataSet(participantIndex,
        domainIndex);
}

PowerControlDynamicCapsSet UnifiedParticipant::getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerControl()->getPowerControlDynamicCapsSet(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex,
    PowerControlDynamicCapsSet capsSet)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPowerControl()->setPowerControlDynamicCapsSet(
        participantIndex, domainIndex, capsSet);
}

PowerStatus UnifiedParticipant::getPowerStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerStatusControl()->getPowerStatus(
        participantIndex, domainIndex);
}

Power UnifiedParticipant::getAveragePower(UIntN participantIndex, UIntN domainIndex, const PowerControlDynamicCaps& capabilities)
{
    throwIfDomainInvalid(domainIndex);
    auto averagePower = m_domains[domainIndex]->getPowerStatusControl()->getAveragePower(
        participantIndex, domainIndex, capabilities);
    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_STATUS);

    return averagePower;
}

Bool UnifiedParticipant::isPlatformPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex,
    PlatformPowerLimitType::Type limitType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerControl()->isPlatformPowerLimitEnabled(
        participantIndex, domainIndex, limitType);
}

Power UnifiedParticipant::getPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex,
    PlatformPowerLimitType::Type limitType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerControl()->getPlatformPowerLimit(
        participantIndex, domainIndex, limitType);
}

void UnifiedParticipant::setPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex,
    PlatformPowerLimitType::Type limitType, const Power& powerLimit)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPlatformPowerControl()->setPlatformPowerLimit(
        participantIndex, domainIndex, limitType, powerLimit);
}

TimeSpan UnifiedParticipant::getPlatformPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex,
    PlatformPowerLimitType::Type limitType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerControl()->getPlatformPowerLimitTimeWindow(
        participantIndex, domainIndex, limitType);
}

void UnifiedParticipant::setPlatformPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex,
    PlatformPowerLimitType::Type limitType, const TimeSpan& timeWindow)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPlatformPowerControl()->setPlatformPowerLimitTimeWindow(
        participantIndex, domainIndex, limitType, timeWindow);
}

Percentage UnifiedParticipant::getPlatformPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex,
    PlatformPowerLimitType::Type limitType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerControl()->getPlatformPowerLimitDutyCycle(
        participantIndex, domainIndex, limitType);
}

void UnifiedParticipant::setPlatformPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex,
    PlatformPowerLimitType::Type limitType, const Percentage& dutyCycle)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPlatformPowerControl()->setPlatformPowerLimitDutyCycle(
        participantIndex, domainIndex, limitType, dutyCycle);
}

Power UnifiedParticipant::getMaxBatteryPower(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getMaxBatteryPower(
        participantIndex, domainIndex);
}

Power UnifiedParticipant::getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getPlatformRestOfPower(
        participantIndex, domainIndex);
}

Power UnifiedParticipant::getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getAdapterPowerRating(
        participantIndex, domainIndex);
}

DptfBuffer UnifiedParticipant::getBatteryStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getBatteryStatus(
        participantIndex, domainIndex);
}

DptfBuffer UnifiedParticipant::getBatteryInformation(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getBatteryInformation(
        participantIndex, domainIndex);
}

PlatformPowerSource::Type UnifiedParticipant::getPlatformPowerSource(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getPlatformPowerSource(
        participantIndex, domainIndex);
}

ChargerType::Type UnifiedParticipant::getChargerType(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getChargerType(
        participantIndex, domainIndex);
}

Power UnifiedParticipant::getACPeakPower(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getACPeakPower(
        participantIndex, domainIndex);
}

TimeSpan UnifiedParticipant::getACPeakTimeWindow(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getACPeakTimeWindow(
        participantIndex, domainIndex);
}

Power UnifiedParticipant::getPlatformBatterySteadyState(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerStatusControl()->getPlatformBatterySteadyState(
        participantIndex, domainIndex);
}

DomainPriority UnifiedParticipant::getDomainPriority(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDomainPriorityControl()->getDomainPriority(
        participantIndex, domainIndex);
}

RfProfileCapabilities UnifiedParticipant::getRfProfileCapabilities(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getRfProfileControl()->getRfProfileCapabilities(participantIndex,
        domainIndex);
}

void UnifiedParticipant::setRfProfileCenterFrequency(UIntN participantIndex, UIntN domainIndex,
    const Frequency& centerFrequency)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getRfProfileControl()->setRfProfileCenterFrequency(participantIndex,
        domainIndex, centerFrequency);
}

RfProfileData UnifiedParticipant::getRfProfileData(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getRfProfileStatusControl()->getRfProfileData(participantIndex,
        domainIndex);
}

TemperatureStatus UnifiedParticipant::getTemperatureStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getTemperatureControl()->getTemperatureStatus(
        participantIndex, domainIndex);
}

TemperatureThresholds UnifiedParticipant::getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getTemperatureControl()->getTemperatureThresholds(
        participantIndex, domainIndex);
}

void UnifiedParticipant::setTemperatureThresholds(UIntN participantIndex, UIntN domainIndex,
    const TemperatureThresholds& temperatureThresholds)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getTemperatureControl()->setTemperatureThresholds(
        participantIndex, domainIndex, temperatureThresholds);

    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD);
}

UtilizationStatus UnifiedParticipant::getUtilizationStatus(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getUtilizationControl()->getUtilizationStatus(
        participantIndex, domainIndex);
}

DptfBuffer UnifiedParticipant::getCalibrationTable(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getTemperatureControl()->getCalibrationTable(participantIndex, domainIndex);
}

DptfBuffer UnifiedParticipant::getPollingTable(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getTemperatureControl()->getPollingTable(participantIndex, domainIndex);
}

Bool UnifiedParticipant::isVirtualTemperature(UIntN participantIndex, UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getTemperatureControl()->isVirtualTemperature(participantIndex, domainIndex);
}

void UnifiedParticipant::setVirtualTemperature(UIntN participantIndex, UIntN domainIndex,
    const Temperature& temperature)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getTemperatureControl()->setVirtualTemperature(
        participantIndex, domainIndex, temperature);
}

std::map<ParticipantSpecificInfoKey::Type, Temperature> UnifiedParticipant::getParticipantSpecificInfo(UIntN participantIndex, const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo)
{
    return m_getSpecificInfo->getParticipantSpecificInfo(participantIndex, requestedInfo);
}

ParticipantProperties UnifiedParticipant::getParticipantProperties(UIntN participantIndex) const
{
    return ParticipantProperties(m_guid, m_name, m_description, m_busType,
        m_pciInfo, m_acpiInfo);
}

DomainPropertiesSet UnifiedParticipant::getDomainPropertiesSet(UIntN participantIndex) const
{
    std::vector<DomainProperties> domainPropertiesSet;

    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            DomainProperties domainProperties(domain->second->getGuid(), domain->first, domain->second->isEnabled(),
                domain->second->getDomainType(), domain->second->getName(), domain->second->getDescription(),
                domain->second->getDomainFunctionalityVersions());
            domainPropertiesSet.push_back(domainProperties);
        }
    }

    return domainPropertiesSet;
}

void UnifiedParticipant::setParticipantDeviceTemperatureIndication(UIntN participantIndex, const Temperature& temperature)
{
    m_setSpecificInfo->setParticipantDeviceTemperatureIndication(participantIndex, temperature);
}

void UnifiedParticipant::setParticipantSpecificInfo(UIntN participantIndex,
    ParticipantSpecificInfoKey::Type tripPoint, const Temperature& tripValue)
{
    m_setSpecificInfo->setParticipantSpecificInfo(participantIndex, tripPoint, tripValue);
}

void UnifiedParticipant::throwIfDomainInvalid(UIntN domainIndex) const
{
    auto matchedDomain = m_domains.find(domainIndex);
    if ((matchedDomain == m_domains.end()) || (matchedDomain->second == nullptr))
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
        for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
        {
            if (domain->second != nullptr)
            {
                try
                {
                    domain->second->getPerformanceControl()->
                        updateBasedOnConfigTdpInformation(m_participantIndex, domain->first, configTdpControlSet,
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
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            try
            {
                configTdpControlStatus =
                    domain->second->getConfigTdpControl()->getConfigTdpControlStatus(
                        m_participantIndex, domain->first);
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
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            try
            {
                configTdpControlSet = domain->second->getConfigTdpControl()->getConfigTdpControlSet(
                        m_participantIndex, domain->first);
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

Bool UnifiedParticipant::isPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex,
    PowerControlType::Type controlType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerControl()->isPowerLimitEnabled(
        participantIndex, domainIndex, controlType);
}

Power UnifiedParticipant::getPowerLimit(UIntN participantIndex, UIntN domainIndex,
    PowerControlType::Type controlType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerControl()->getPowerLimit(
        participantIndex, domainIndex, controlType);
}

void UnifiedParticipant::setPowerLimit(UIntN participantIndex, UIntN domainIndex,
    PowerControlType::Type controlType, const Power& powerLimit)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPowerControl()->setPowerLimit(
        participantIndex, domainIndex, controlType, powerLimit);
    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::setPowerLimitIgnoringCaps(UIntN participantIndex, UIntN domainIndex,
    PowerControlType::Type controlType, const Power& powerLimit)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPowerControl()->setPowerLimitIgnoringCaps(
        participantIndex, domainIndex, controlType, powerLimit);
    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

TimeSpan UnifiedParticipant::getPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex,
    PowerControlType::Type controlType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerControl()->getPowerLimitTimeWindow(
        participantIndex, domainIndex, controlType);
}

void UnifiedParticipant::setPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex,
    PowerControlType::Type controlType, const TimeSpan& timeWindow)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPowerControl()->setPowerLimitTimeWindow(
        participantIndex, domainIndex, controlType, timeWindow);
    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::setPowerLimitTimeWindowIgnoringCaps(UIntN participantIndex, UIntN domainIndex,
    PowerControlType::Type controlType, const TimeSpan& timeWindow)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPowerControl()->setPowerLimitTimeWindowIgnoringCaps(
        participantIndex, domainIndex, controlType, timeWindow);
    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

Percentage UnifiedParticipant::getPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex,
    PowerControlType::Type controlType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerControl()->getPowerLimitDutyCycle(
        participantIndex, domainIndex, controlType);
}

void UnifiedParticipant::setPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex,
    PowerControlType::Type controlType, const Percentage& dutyCycle)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPowerControl()->setPowerLimitDutyCycle(
        participantIndex, domainIndex, controlType, dutyCycle);
    sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->getPowerControl()->setPowerCapsLock(participantIndex, domainIndex, lock);
}