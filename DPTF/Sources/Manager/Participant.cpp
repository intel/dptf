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

#include "Participant.h"
#include "EsifServicesInterface.h"
#include "EsifDataGuid.h"
#include "EsifDataString.h"
#include "Utility.h"
#include "esif_sdk_iface_app.h"
#include "ManagerMessage.h"
#include "MapOps.h"
#include "Utility.h"

Participant::Participant(DptfManagerInterface* dptfManager) :
    m_participantCreated(false),
    m_dptfManager(dptfManager),
    m_theRealParticipant(nullptr),
    m_participantServices(nullptr),
    m_participantIndex(Constants::Invalid),
    m_participantGuid(Guid()),
    m_participantName("")
{
}

Participant::~Participant(void)
{
    destroyParticipant();
}

void Participant::createParticipant(UIntN participantIndex, const AppParticipantDataPtr participantDataPtr,
    Bool participantEnabled)
{
    if (m_participantCreated == true)
    {
        throw dptf_exception("Participant::createParticipant() already executed.");
    }

    m_theRealParticipant = CreateParticipantInstance();
    if (m_theRealParticipant == nullptr)
    {
        std::stringstream message;
        message << "Failed to create participant instance for participant: " << EsifDataString(&participantDataPtr->fName);
        throw dptf_exception(message.str());
    }

    try
    {
        m_participantServices = std::make_shared<ParticipantServices>(m_dptfManager, participantIndex);
        m_participantIndex = participantIndex;
        m_participantGuid = EsifDataGuid(&participantDataPtr->fDriverType);
        m_participantName = EsifDataString(&participantDataPtr->fName);

        m_theRealParticipant->createParticipant(
            m_participantGuid,
            m_participantIndex,
            participantEnabled,
            m_participantName,
            EsifDataString(&participantDataPtr->fDesc),
            EsifParticipantEnumToBusType(participantDataPtr->fBusEnumerator),
            PciInfo(participantDataPtr->fPciVendor, participantDataPtr->fPciDevice, participantDataPtr->fPciBus,
                participantDataPtr->fPciBusDevice, participantDataPtr->fPciFunction, participantDataPtr->fPciRevision,
                participantDataPtr->fPciClass, participantDataPtr->fPciSubClass, participantDataPtr->fPciProgIf),
            AcpiInfo(EsifDataString(&participantDataPtr->fAcpiDevice), EsifDataString(&participantDataPtr->fAcpiScope),
                EsifDataString(&participantDataPtr->fAcpiUID), participantDataPtr->fAcpiType),
            m_participantServices);
        m_participantCreated = true;
    }
    catch (...)
    {
        m_participantIndex = Constants::Invalid;
        m_participantGuid = Guid();
        m_participantName = "";
        m_participantCreated = false;

        throw;
    }
}

void Participant::destroyParticipant(void)
{
    try
    {
        destroyAllDomains();
    }
    catch (...)
    {
    }

    if (m_theRealParticipant != nullptr)
    {
        try
        {
            m_theRealParticipant->destroyParticipant();
        }
        catch (...)
        {
        }

        try
        {
            DestroyParticipantInstance(m_theRealParticipant);
        }
        catch (...)
        {
        }

        m_theRealParticipant = nullptr;
    }

    m_participantIndex = Constants::Invalid;
    m_participantGuid = Guid();
    m_participantName = "";
}

void Participant::enableParticipant(void)
{
    throwIfRealParticipantIsInvalid();
    m_theRealParticipant->enableParticipant();
}

void Participant::disableParticipant(void)
{
    throwIfRealParticipantIsInvalid();
    m_theRealParticipant->disableParticipant();
}

Bool Participant::isParticipantEnabled(void)
{
    throwIfRealParticipantIsInvalid();
    return m_theRealParticipant->isParticipantEnabled();
}

UIntN Participant::allocateNextDomainIndex()
{
    UIntN firstAvailableIndex = Constants::Invalid;

	auto indexesInUse = MapOps<UIntN, std::shared_ptr<Domain>>::getKeys(m_domains);
    firstAvailableIndex = getFirstAvailableIndex(indexesInUse);

    return firstAvailableIndex;
}

void Participant::createDomain(UIntN domainIndex, const AppDomainDataPtr domainDataPtr, Bool domainEnabled)
{
    if (domainIndex == Constants::Invalid || domainIndex == Constants::Esif::NoDomain)
    {
        throw dptf_exception("Domain index is invalid.");
    }

    try
    {
        // create an instance of the domain class and save it at the first available index
        std::shared_ptr<Domain> domain = std::make_shared<Domain>(m_dptfManager);
        m_domains[domainIndex] = domain;

        m_domains[domainIndex]->createDomain(m_participantIndex, domainIndex, m_theRealParticipant,
            domainDataPtr, domainEnabled);
    }
    catch (...)
    {
        throw;
    }
}

void Participant::destroyAllDomains(void)
{
    auto domain = m_domains.begin();
    while (domain != m_domains.end())
    {
        destroyDomain(domain->first);
        domain = m_domains.begin();
    }
}

void Participant::destroyDomain(UIntN domainIndex)
{
    if (isDomainValid(domainIndex))
    {
        try
        {
            m_domains[domainIndex]->destroyDomain();
        }
        catch (...)
        {
        }

        m_domains.erase(domainIndex);
    }
}

Bool Participant::isDomainValid(UIntN domainIndex) const
{
    auto match = m_domains.find(domainIndex);
    return (match != m_domains.end());
}

void Participant::enableDomain(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->enableDomain();
}

void Participant::disableDomain(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->disableDomain();
}

Bool Participant::isDomainEnabled(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->isDomainEnabled();
}

UIntN Participant::getDomainCount(void) const
{
    UIntN count = 0;

    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            count++;
        }
    }

    return count;
}

void Participant::clearParticipantCachedData(void)
{
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->clearDomainCachedData();
        }
    }
}

void Participant::clearArbitrationDataForPolicy(UIntN policyIndex)
{
    for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
    {
        if (domain->second != nullptr)
        {
            domain->second->clearArbitrationDataForPolicy(policyIndex);
        }
    }
}

void Participant::registerEvent(ParticipantEvent::Type participantEvent)
{
    if (m_registeredEvents.test(participantEvent) == false)
    {
        FrameworkEvent::Type frameworkEvent = ParticipantEvent::ToFrameworkEvent(participantEvent);
        m_dptfManager->getEsifServices()->registerEvent(frameworkEvent, m_participantIndex);
        m_registeredEvents.set(participantEvent);
    }
}

void Participant::unregisterEvent(ParticipantEvent::Type participantEvent)
{
    if (m_registeredEvents.test(participantEvent) == true)
    {
        FrameworkEvent::Type frameworkEvent = ParticipantEvent::ToFrameworkEvent(participantEvent);
        m_dptfManager->getEsifServices()->unregisterEvent(frameworkEvent, m_participantIndex);
        m_registeredEvents.reset(participantEvent);
    }
}

Bool Participant::isEventRegistered(ParticipantEvent::Type participantEvent)
{
    return m_registeredEvents.test(participantEvent);
}

std::string Participant::getParticipantName(void) const
{
    return m_participantName;
}

std::string Participant::getDomainName(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDomainName();
}

std::shared_ptr<XmlNode> Participant::getXml(UIntN domainIndex) const
{
    throwIfRealParticipantIsInvalid();
    return m_theRealParticipant->getXml(domainIndex);
}

std::shared_ptr<XmlNode> Participant::getStatusAsXml(UIntN domainIndex) const
{
    throwIfRealParticipantIsInvalid();
    return m_theRealParticipant->getStatusAsXml(domainIndex);
}

//
// Event handlers
//

void Participant::connectedStandbyEntry(void)
{
    if (isEventRegistered(ParticipantEvent::DptfConnectedStandbyEntry))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->connectedStandbyEntry();
    }
}

void Participant::connectedStandbyExit(void)
{
    if (isEventRegistered(ParticipantEvent::DptfConnectedStandbyExit))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->connectedStandbyExit();
    }
}

void Participant::suspend(void)
{
    if (isEventRegistered(ParticipantEvent::DptfSuspend))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->suspend();
    }
}

void Participant::resume(void)
{
    if (isEventRegistered(ParticipantEvent::DptfResume))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->resume();
    }
}

void Participant::activityLoggingEnabled(UInt32 domainIndex, UInt32 capabilityBitMask)
{
    if (isEventRegistered(ParticipantEvent::DptfParticipantActivityLoggingEnabled))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->activityLoggingEnabled(domainIndex, capabilityBitMask);
    }
}

void Participant::activityLoggingDisabled(UInt32 domainIndex, UInt32 capabilityBitMask)
{
    if (isEventRegistered(ParticipantEvent::DptfParticipantActivityLoggingDisabled))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->activityLoggingDisabled(domainIndex, capabilityBitMask);
    }
}

void Participant::domainConfigTdpCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainConfigTdpCapabilityChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainConfigTdpCapabilityChanged();
    }
}

void Participant::domainCoreControlCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainCoreControlCapabilityChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainCoreControlCapabilityChanged();
    }
}

void Participant::domainDisplayControlCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainDisplayControlCapabilityChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainDisplayControlCapabilityChanged();
    }
}

void Participant::domainDisplayStatusChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainDisplayStatusChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainDisplayStatusChanged();
    }
}

void Participant::domainPerformanceControlCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPerformanceControlCapabilityChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainPerformanceControlCapabilityChanged();
    }
}

void Participant::domainPerformanceControlsChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPerformanceControlsChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainPerformanceControlsChanged();
    }
}

void Participant::domainPowerControlCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPowerControlCapabilityChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainPowerControlCapabilityChanged();
    }
}

void Participant::domainPriorityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPriorityChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainPriorityChanged();
    }
}

void Participant::domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus)
{
    if (isEventRegistered(ParticipantEvent::DomainRadioConnectionStatusChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainRadioConnectionStatusChanged(radioConnectionStatus);
    }
}

void Participant::domainRfProfileChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainRfProfileChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainRfProfileChanged();
    }
}

void Participant::domainTemperatureThresholdCrossed(void)
{
    if (isEventRegistered(ParticipantEvent::DomainTemperatureThresholdCrossed))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainTemperatureThresholdCrossed();
    }
}

void Participant::participantSpecificInfoChanged(void)
{
    if (isEventRegistered(ParticipantEvent::ParticipantSpecificInfoChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->participantSpecificInfoChanged();
    }
}

void Participant::domainVirtualSensorCalibrationTableChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainVirtualSensorCalibrationTableChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainVirtualSensorCalibrationTableChanged();
    }
}

void Participant::domainVirtualSensorPollingTableChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainVirtualSensorPollingTableChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainVirtualSensorPollingTableChanged();
    }
}

void Participant::domainVirtualSensorRecalcChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainVirtualSensorRecalcChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainVirtualSensorRecalcChanged();
    }
}

void Participant::domainBatteryStatusChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainBatteryStatusChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainBatteryStatusChanged();
    }
}

void Participant::domainBatteryInformationChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainBatteryInformationChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainBatteryInformationChanged();
    }
}

void Participant::domainPlatformPowerSourceChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPlatformPowerSourceChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainPlatformPowerSourceChanged();
    }
}

void Participant::domainAdapterPowerRatingChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainAdapterPowerRatingChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainAdapterPowerRatingChanged();
    }
}

void Participant::domainChargerTypeChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainChargerTypeChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainChargerTypeChanged();
    }
}

void Participant::domainPlatformRestOfPowerChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPlatformRestOfPowerChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainPlatformRestOfPowerChanged();
    }
}

void Participant::domainACPeakPowerChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainACPeakPowerChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainACPeakPowerChanged();
    }
}

void Participant::domainACPeakTimeWindowChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainACPeakTimeWindowChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainACPeakTimeWindowChanged();
    }
}

void Participant::domainMaxBatteryPowerChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainMaxBatteryPowerChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainMaxBatteryPowerChanged();
    }
}

void Participant::domainPlatformBatterySteadyStateChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPlatformBatterySteadyStateChanged))
    {
        throwIfRealParticipantIsInvalid();
        m_theRealParticipant->domainPlatformBatterySteadyStateChanged();
    }
}

//
// The following functions pass through to the domain implementation
//

ActiveControlStaticCaps Participant::getActiveControlStaticCaps(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getActiveControlStaticCaps();
}

ActiveControlStatus Participant::getActiveControlStatus(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getActiveControlStatus();
}

ActiveControlSet Participant::getActiveControlSet(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getActiveControlSet();
}

void Participant::setActiveControl(UIntN domainIndex, UIntN policyIndex, UIntN controlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setActiveControl(policyIndex, controlIndex);
}

void Participant::setActiveControl(UIntN domainIndex, UIntN policyIndex, const Percentage& fanSpeed)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setActiveControl(policyIndex, fanSpeed);
}

ConfigTdpControlDynamicCaps Participant::getConfigTdpControlDynamicCaps(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getConfigTdpControlDynamicCaps();
}

ConfigTdpControlStatus Participant::getConfigTdpControlStatus(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getConfigTdpControlStatus();
}

ConfigTdpControlSet Participant::getConfigTdpControlSet(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getConfigTdpControlSet();
}

void Participant::setConfigTdpControl(UIntN domainIndex, UIntN policyIndex, UIntN controlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setConfigTdpControl(policyIndex, controlIndex);
}

CoreControlStaticCaps Participant::getCoreControlStaticCaps(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControlStaticCaps();
}

CoreControlDynamicCaps Participant::getCoreControlDynamicCaps(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControlDynamicCaps();
}

CoreControlLpoPreference Participant::getCoreControlLpoPreference(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControlLpoPreference();
}

CoreControlStatus Participant::getCoreControlStatus(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getCoreControlStatus();
}

void Participant::setActiveCoreControl(UIntN domainIndex, UIntN policyIndex, const CoreControlStatus& coreControlStatus)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setActiveCoreControl(policyIndex, coreControlStatus);
}

DisplayControlDynamicCaps Participant::getDisplayControlDynamicCaps(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControlDynamicCaps();
}

DisplayControlStatus Participant::getDisplayControlStatus(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControlStatus();
}

UIntN Participant::getUserPreferredDisplayIndex(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getUserPreferredDisplayIndex();
}

Bool Participant::isUserPreferredIndexModified(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->isUserPreferredIndexModified();
}

DisplayControlSet Participant::getDisplayControlSet(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDisplayControlSet();
}

void Participant::setDisplayControl(UIntN domainIndex, UIntN policyIndex, UIntN displayControlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setDisplayControl(policyIndex, displayControlIndex);
}

void Participant::setDisplayControlDynamicCaps(UIntN domainIndex, UIntN policyIndex, 
    DisplayControlDynamicCaps newCapabilities)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setDisplayControlDynamicCaps(policyIndex, newCapabilities);
}

void Participant::setDisplayCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setDisplayCapsLock(policyIndex, lock);
}

PerformanceControlStaticCaps Participant::getPerformanceControlStaticCaps(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControlStaticCaps();
}

PerformanceControlDynamicCaps Participant::getPerformanceControlDynamicCaps(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControlDynamicCaps();
}

PerformanceControlStatus Participant::getPerformanceControlStatus(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControlStatus();
}

PerformanceControlSet Participant::getPerformanceControlSet(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPerformanceControlSet();
}

void Participant::setPerformanceControl(UIntN domainIndex, UIntN policyIndex, UIntN performanceControlIndex)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPerformanceControl(policyIndex, performanceControlIndex);
}

void Participant::setPerformanceControlDynamicCaps(UIntN domainIndex, UIntN policyIndex, 
    PerformanceControlDynamicCaps newCapabilities)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPerformanceControlDynamicCaps(policyIndex, newCapabilities);
}

void Participant::setPerformanceCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPerformanceCapsLock(policyIndex, lock);
}

void Participant::setPixelClockControl(UIntN domainIndex, UIntN policyIndex, const PixelClockDataSet& pixelClockDataSet)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPixelClockControl(policyIndex, pixelClockDataSet);
}

PixelClockCapabilities Participant::getPixelClockCapabilities(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPixelClockCapabilities();
}

PixelClockDataSet Participant::getPixelClockDataSet(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPixelClockDataSet();
}

PowerControlDynamicCapsSet Participant::getPowerControlDynamicCapsSet(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerControlDynamicCapsSet();
}

void Participant::setPowerControlDynamicCapsSet(UIntN domainIndex, UIntN policyIndex, PowerControlDynamicCapsSet capsSet)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPowerControlDynamicCapsSet(policyIndex, capsSet);
}

Bool Participant::isPowerLimitEnabled(UIntN domainIndex, PowerControlType::Type controlType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->isPowerLimitEnabled(controlType);
}

Power Participant::getPowerLimit(UIntN domainIndex, PowerControlType::Type controlType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerLimit(controlType);
}

void Participant::setPowerLimit(UIntN domainIndex, UIntN policyIndex, PowerControlType::Type controlType,
    const Power& powerLimit)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPowerLimit(policyIndex, controlType, powerLimit);
}

void Participant::setPowerLimitIgnoringCaps(UIntN domainIndex, UIntN policyIndex,
    PowerControlType::Type controlType, const Power& powerLimit)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPowerLimitIgnoringCaps(policyIndex, controlType, powerLimit);
}

TimeSpan Participant::getPowerLimitTimeWindow(UIntN domainIndex, PowerControlType::Type controlType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerLimitTimeWindow(controlType);
}

void Participant::setPowerLimitTimeWindow(UIntN domainIndex, UIntN policyIndex, PowerControlType::Type controlType,
    const TimeSpan& timeWindow)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPowerLimitTimeWindow(policyIndex, controlType, timeWindow);
}

void Participant::setPowerLimitTimeWindowIgnoringCaps(UIntN domainIndex, UIntN policyIndex,
    PowerControlType::Type controlType, const TimeSpan& timeWindow)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPowerLimitTimeWindowIgnoringCaps(policyIndex, controlType, timeWindow);
}

Percentage Participant::getPowerLimitDutyCycle(UIntN domainIndex, PowerControlType::Type controlType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerLimitDutyCycle(controlType);
}

void Participant::setPowerLimitDutyCycle(UIntN domainIndex, UIntN policyIndex, PowerControlType::Type controlType,
    const Percentage& dutyCycle)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPowerLimitDutyCycle(policyIndex, controlType, dutyCycle);
}

void Participant::setPowerCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPowerCapsLock(policyIndex, lock);
}

PowerStatus Participant::getPowerStatus(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPowerStatus();
}

Power Participant::getAveragePower(UIntN domainIndex, const PowerControlDynamicCaps& capabilities)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getAveragePower(capabilities);
}

Bool Participant::isPlatformPowerLimitEnabled(UIntN domainIndex, PlatformPowerLimitType::Type limitType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->isPlatformPowerLimitEnabled(limitType);
}

Power Participant::getPlatformPowerLimit(UIntN domainIndex, PlatformPowerLimitType::Type limitType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerLimit(limitType);
}

void Participant::setPlatformPowerLimit(UIntN domainIndex, PlatformPowerLimitType::Type limitType, const Power& powerLimit)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPlatformPowerLimit(limitType, powerLimit);
}

TimeSpan Participant::getPlatformPowerLimitTimeWindow(UIntN domainIndex, PlatformPowerLimitType::Type limitType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerLimitTimeWindow(limitType);
}

void Participant::setPlatformPowerLimitTimeWindow(UIntN domainIndex, PlatformPowerLimitType::Type limitType, const TimeSpan& timeWindow)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPlatformPowerLimitTimeWindow(limitType, timeWindow);
}

Percentage Participant::getPlatformPowerLimitDutyCycle(UIntN domainIndex, PlatformPowerLimitType::Type limitType)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerLimitDutyCycle(limitType);
}

void Participant::setPlatformPowerLimitDutyCycle(UIntN domainIndex, PlatformPowerLimitType::Type limitType, const Percentage& dutyCycle)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setPlatformPowerLimitDutyCycle(limitType, dutyCycle);
}

Power Participant::getMaxBatteryPower(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getMaxBatteryPower();
}

Power Participant::getPlatformRestOfPower(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformRestOfPower();
}

Power Participant::getAdapterPowerRating(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getAdapterPowerRating();
}

DptfBuffer Participant::getBatteryStatus(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getBatteryStatus();
}

DptfBuffer Participant::getBatteryInformation(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getBatteryInformation();
}

PlatformPowerSource::Type Participant::getPlatformPowerSource(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformPowerSource();
}

ChargerType::Type Participant::getChargerType(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getChargerType();
}

Power Participant::getACPeakPower(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getACPeakPower();
}

TimeSpan Participant::getACPeakTimeWindow(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getACPeakTimeWindow();
}

Power Participant::getPlatformBatterySteadyState(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getPlatformBatterySteadyState();
}

DomainPriority Participant::getDomainPriority(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getDomainPriority();
}

RfProfileCapabilities Participant::getRfProfileCapabilities(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getRfProfileCapabilities();
}

void Participant::setRfProfileCenterFrequency(UIntN domainIndex, UIntN policyIndex, const Frequency& centerFrequency)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setRfProfileCenterFrequency(policyIndex, centerFrequency);
}

RfProfileData Participant::getRfProfileData(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getRfProfileData();
}

TemperatureStatus Participant::getTemperatureStatus(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getTemperatureStatus();
}

TemperatureThresholds Participant::getTemperatureThresholds(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getTemperatureThresholds();
}

void Participant::setTemperatureThresholds(UIntN domainIndex, UIntN policyIndex, const TemperatureThresholds& temperatureThresholds)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setTemperatureThresholds(policyIndex, temperatureThresholds);
}

UtilizationStatus Participant::getUtilizationStatus(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getUtilizationStatus();
}

DptfBuffer Participant::getVirtualSensorCalibrationTable(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getVirtualSensorCalibrationTable();
}

DptfBuffer Participant::getVirtualSensorPollingTable(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->getVirtualSensorPollingTable();
}

Bool Participant::isVirtualTemperature(UIntN domainIndex)
{
    throwIfDomainInvalid(domainIndex);
    return m_domains[domainIndex]->isVirtualTemperature();
}

void Participant::setVirtualTemperature(UIntN domainIndex, const Temperature& temperature)
{
    throwIfDomainInvalid(domainIndex);
    m_domains[domainIndex]->setVirtualTemperature(temperature);
}

std::map<ParticipantSpecificInfoKey::Type, Temperature> Participant::getParticipantSpecificInfo(
    const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo)
{
    throwIfRealParticipantIsInvalid();
    return m_theRealParticipant->getParticipantSpecificInfo(m_participantIndex, requestedInfo);
}

ParticipantProperties Participant::getParticipantProperties(void) const
{
    throwIfRealParticipantIsInvalid();
    return m_theRealParticipant->getParticipantProperties(m_participantIndex);
}

DomainPropertiesSet Participant::getDomainPropertiesSet(void) const
{
    throwIfRealParticipantIsInvalid();
    return m_theRealParticipant->getDomainPropertiesSet(m_participantIndex);
}

void Participant::setParticipantDeviceTemperatureIndication(const Temperature& temperature)
{
    throwIfRealParticipantIsInvalid();
    m_theRealParticipant->setParticipantDeviceTemperatureIndication(m_participantIndex, temperature);
}

void Participant::setParticipantSpecificInfo(ParticipantSpecificInfoKey::Type tripPoint, const Temperature& tripValue)
{
    throwIfRealParticipantIsInvalid();
    m_theRealParticipant->setParticipantSpecificInfo(m_participantIndex, tripPoint, tripValue);
}

void Participant::throwIfDomainInvalid(UIntN domainIndex) const
{
    auto match = m_domains.find(domainIndex);
    if((match == m_domains.end()) || (match->second == nullptr) || (match->second->isCreated() == false))
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF, "Domain index is invalid for this participant.");
        message.addMessage("Domain Index", domainIndex);
        message.setParticipantIndex(m_participantIndex);
        m_dptfManager->getEsifServices()->writeMessageWarning(message);

        throw dptf_exception(message);
    }
}

void Participant::throwIfRealParticipantIsInvalid() const
{
    if ((m_theRealParticipant == nullptr) || (m_participantCreated == false))
    {
        throw dptf_exception("Real Participant is not valid.");
    }
}
