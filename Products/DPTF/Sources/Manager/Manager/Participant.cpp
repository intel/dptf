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

#include "Participant.h"
#include "DptfManager.h"
#include "EsifServices.h"
#include "EsifDataGuid.h"
#include "EsifDataString.h"
#include "Utility.h"
#include "esif_uf_app_iface.h"
#include "ManagerMessage.h"

Participant::Participant(DptfManager* dptfManager) :
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
    m_participantCreated = true;

    m_theRealParticipant = CreateParticipantInstance();
    if (m_theRealParticipant == nullptr)
    {
        std::stringstream message;
        message << "Failed to create participant instance for participant: " << std::string(EsifDataString(participantDataPtr->fName));
        throw dptf_exception(message.str());
    }

    try
    {
        m_participantServices = new ParticipantServices(m_dptfManager, participantIndex);
        m_participantIndex = participantIndex;
        m_participantGuid = EsifDataGuid(participantDataPtr->fDriverType);
        m_participantName = EsifDataString(participantDataPtr->fName);

        m_theRealParticipant->createParticipant(
            m_participantGuid,
            m_participantIndex,
            participantEnabled,
            m_participantName,
            EsifDataString(participantDataPtr->fDesc),
            EsifParticipantEnumToBusType(participantDataPtr->fBusEnumerator),
            PciInfo(participantDataPtr->fPciVendor, participantDataPtr->fPciDevice, participantDataPtr->fPciBus,
                participantDataPtr->fPciBusDevice, participantDataPtr->fPciFunction, participantDataPtr->fPciRevision,
                participantDataPtr->fPciClass, participantDataPtr->fPciSubClass, participantDataPtr->fPciProgIf),
            AcpiInfo(EsifDataString(participantDataPtr->fAcpiDevice), EsifDataString(participantDataPtr->fAcpiScope),
                participantDataPtr->fAcpiUID, participantDataPtr->fAcpiType),
            m_participantServices);
    }
    catch (...)
    {
        DELETE_MEMORY_TC(m_participantServices);

        m_participantIndex = Constants::Invalid;
        m_participantGuid = Guid();
        m_participantName = "";

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

    DELETE_MEMORY_TC(m_participantServices);

    m_participantIndex = Constants::Invalid;
    m_participantGuid = Guid();
    m_participantName = "";
}

void Participant::enableParticipant(void)
{
    m_theRealParticipant->enableParticipant();
}

void Participant::disableParticipant(void)
{
    m_theRealParticipant->disableParticipant();
}

Bool Participant::isParticipantEnabled(void)
{
    return m_theRealParticipant->isParticipantEnabled();
}

void Participant::allocateDomain(UIntN* newDomainIndex)
{
    UIntN firstAvailableIndex = Constants::Invalid;
    Domain* domain = nullptr;

    try
    {
        // create an instance of the domain class and save it at the first available index
        domain = new Domain(m_dptfManager);
        firstAvailableIndex = getFirstNonNullIndex(m_domain);
        m_domain[firstAvailableIndex] = domain;
    }
    catch (...)
    {
        if (firstAvailableIndex != Constants::Invalid)
        {
            m_domain[firstAvailableIndex] = nullptr;
        }
        delete domain;
        throw;
    }

    *newDomainIndex = firstAvailableIndex;
}

void Participant::createDomain(UIntN domainIndex, const AppDomainDataPtr domainDataPtr, Bool domainEnabled)
{
    if (domainIndex >= m_domain.size() || m_domain[domainIndex] == nullptr)
    {
        throw dptf_exception("Domain index is invalid.");
    }

    m_domain[domainIndex]->createDomain(m_participantIndex, domainIndex, m_theRealParticipant,
        domainDataPtr, domainEnabled);
}

void Participant::destroyAllDomains(void)
{
    for (UIntN i = 0; i < m_domain.size(); i++)
    {
        destroyDomain(i);
    }
}

void Participant::destroyDomain(UIntN domainIndex)
{
    if ((domainIndex < m_domain.size()) &&
        (m_domain[domainIndex] != nullptr))
    {
        try
        {
            m_domain[domainIndex]->destroyDomain();
        }
        catch (...)
        {
        }

        DELETE_MEMORY_TC(m_domain[domainIndex]);
    }
}

Bool Participant::isDomainValid(UIntN domainIndex)
{
    return ((domainIndex < m_domain.size()) && (m_domain[domainIndex] != nullptr));
}

void Participant::enableDomain(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->enableDomain();
}

void Participant::disableDomain(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->disableDomain();
}

Bool Participant::isDomainEnabled(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->isDomainEnabled();
}

UIntN Participant::getDomainCount(void) const
{
    UIntN count = 0;

    for (UIntN i = 0; i < m_domain.size(); i++)
    {
        if (m_domain[i] != nullptr)
        {
            count++;
        }
    }

    return count;
}

UIntN Participant::getDomainIndex(Domain* domainPtr)
{
    for (UIntN i = 0; i < m_domain.size(); i++)
    {
        if (m_domain[i] == domainPtr)
        {
            return i;
        }
    }

    throw dptf_exception("Domain pointer is invalid.");
}

void Participant::clearParticipantCachedData(void)
{
    for (UIntN i = 0; i < m_domain.size(); i++)
    {
        if (m_domain[i] != nullptr)
        {
            m_domain[i]->clearDomainCachedData();
        }
    }
}

void Participant::clearArbitrationDataForPolicy(UIntN policyIndex)
{
    for (UIntN i = 0; i < m_domain.size(); i++)
    {
        if (m_domain[i] != nullptr)
        {
            m_domain[i]->clearArbitrationDataForPolicy(policyIndex);
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
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getDomainName();
}

XmlNode* Participant::getXml(UIntN domainIndex) const
{
    return m_theRealParticipant->getXml(domainIndex);
}

XmlNode* Participant::getStatusAsXml(UIntN domainIndex) const
{
    return m_theRealParticipant->getStatusAsXml(domainIndex);
}

//
// Event handlers
//

void Participant::connectedStandbyEntry(void)
{
    if (isEventRegistered(ParticipantEvent::DptfConnectedStandbyEntry))
    {
        m_theRealParticipant->connectedStandbyEntry();
    }
}

void Participant::connectedStandbyExit(void)
{
    if (isEventRegistered(ParticipantEvent::DptfConnectedStandbyExit))
    {
        m_theRealParticipant->connectedStandbyExit();
    }
}

void Participant::domainConfigTdpCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainConfigTdpCapabilityChanged))
    {
        m_theRealParticipant->domainConfigTdpCapabilityChanged();
    }
}

void Participant::domainCoreControlCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainCoreControlCapabilityChanged))
    {
        m_theRealParticipant->domainCoreControlCapabilityChanged();
    }
}

void Participant::domainDisplayControlCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainDisplayControlCapabilityChanged))
    {
        m_theRealParticipant->domainDisplayControlCapabilityChanged();
    }
}

void Participant::domainDisplayStatusChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainDisplayStatusChanged))
    {
        m_theRealParticipant->domainDisplayStatusChanged();
    }
}

void Participant::domainPerformanceControlCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPerformanceControlCapabilityChanged))
    {
        m_theRealParticipant->domainPerformanceControlCapabilityChanged();
    }
}

void Participant::domainPerformanceControlsChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPerformanceControlsChanged))
    {
        m_theRealParticipant->domainPerformanceControlsChanged();
    }
}

void Participant::domainPowerControlCapabilityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPowerControlCapabilityChanged))
    {
        m_theRealParticipant->domainPowerControlCapabilityChanged();
    }
}

void Participant::domainPriorityChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainPriorityChanged))
    {
        m_theRealParticipant->domainPriorityChanged();
    }
}

void Participant::domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus)
{
    if (isEventRegistered(ParticipantEvent::DomainRadioConnectionStatusChanged))
    {
        m_theRealParticipant->domainRadioConnectionStatusChanged(radioConnectionStatus);
    }
}

void Participant::domainRfProfileChanged(void)
{
    if (isEventRegistered(ParticipantEvent::DomainRfProfileChanged))
    {
        m_theRealParticipant->domainRfProfileChanged();
    }
}

void Participant::domainTemperatureThresholdCrossed(void)
{
    if (isEventRegistered(ParticipantEvent::DomainTemperatureThresholdCrossed))
    {
        m_theRealParticipant->domainTemperatureThresholdCrossed();
    }
}

void Participant::participantSpecificInfoChanged(void)
{
    if (isEventRegistered(ParticipantEvent::ParticipantSpecificInfoChanged))
    {
        m_theRealParticipant->participantSpecificInfoChanged();
    }
}

//
// The following functions pass through to the domain implementation
//

ActiveControlStaticCaps Participant::getActiveControlStaticCaps(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getActiveControlStaticCaps();
}

ActiveControlStatus Participant::getActiveControlStatus(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getActiveControlStatus();
}

ActiveControlSet Participant::getActiveControlSet(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getActiveControlSet();
}

void Participant::setActiveControl(UIntN domainIndex, UIntN policyIndex, UIntN controlIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setActiveControl(policyIndex, controlIndex);
}

void Participant::setActiveControl(UIntN domainIndex, UIntN policyIndex, const Percentage& fanSpeed)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setActiveControl(policyIndex, fanSpeed);
}

ConfigTdpControlDynamicCaps Participant::getConfigTdpControlDynamicCaps(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getConfigTdpControlDynamicCaps();
}

ConfigTdpControlStatus Participant::getConfigTdpControlStatus(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getConfigTdpControlStatus();
}

ConfigTdpControlSet Participant::getConfigTdpControlSet(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getConfigTdpControlSet();
}

void Participant::setConfigTdpControl(UIntN domainIndex, UIntN policyIndex, UIntN controlIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setConfigTdpControl(policyIndex, controlIndex);
}

CoreControlStaticCaps Participant::getCoreControlStaticCaps(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getCoreControlStaticCaps();
}

CoreControlDynamicCaps Participant::getCoreControlDynamicCaps(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getCoreControlDynamicCaps();
}

CoreControlLpoPreference Participant::getCoreControlLpoPreference(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getCoreControlLpoPreference();
}

CoreControlStatus Participant::getCoreControlStatus(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getCoreControlStatus();
}

void Participant::setActiveCoreControl(UIntN domainIndex, UIntN policyIndex, const CoreControlStatus& coreControlStatus)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setActiveCoreControl(policyIndex, coreControlStatus);
}

DisplayControlDynamicCaps Participant::getDisplayControlDynamicCaps(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getDisplayControlDynamicCaps();
}

DisplayControlStatus Participant::getDisplayControlStatus(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getDisplayControlStatus();
}

DisplayControlSet Participant::getDisplayControlSet(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getDisplayControlSet();
}

void Participant::setDisplayControl(UIntN domainIndex, UIntN policyIndex, UIntN displayControlIndex, Bool isOverridable)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setDisplayControl(policyIndex, displayControlIndex, isOverridable);
}

PerformanceControlStaticCaps Participant::getPerformanceControlStaticCaps(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getPerformanceControlStaticCaps();
}

PerformanceControlDynamicCaps Participant::getPerformanceControlDynamicCaps(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getPerformanceControlDynamicCaps();
}

PerformanceControlStatus Participant::getPerformanceControlStatus(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getPerformanceControlStatus();
}

PerformanceControlSet Participant::getPerformanceControlSet(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getPerformanceControlSet();
}

void Participant::setPerformanceControl(UIntN domainIndex, UIntN policyIndex, UIntN performanceControlIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setPerformanceControl(policyIndex, performanceControlIndex);
}

void Participant::setPixelClockControl(UIntN domainIndex, UIntN policyIndex, const PixelClockDataSet& pixelClockDataSet)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setPixelClockControl(policyIndex, pixelClockDataSet);
}

PixelClockCapabilities Participant::getPixelClockCapabilities(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getPixelClockCapabilities();
}

PixelClockDataSet Participant::getPixelClockDataSet(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getPixelClockDataSet();
}

PowerControlDynamicCapsSet Participant::getPowerControlDynamicCapsSet(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getPowerControlDynamicCapsSet();
}

PowerControlStatusSet Participant::getPowerControlStatusSet(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getPowerControlStatusSet();
}

void Participant::setPowerControl(UIntN domainIndex, UIntN policyIndex, const PowerControlStatusSet& powerControlStatusSet)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setPowerControl(policyIndex, powerControlStatusSet);
}

PowerStatus Participant::getPowerStatus(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getPowerStatus();
}

DomainPriority Participant::getDomainPriority(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getDomainPriority();
}

RfProfileCapabilities Participant::getRfProfileCapabilities(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getRfProfileCapabilities();
}

void Participant::setRfProfileCenterFrequency(UIntN domainIndex, UIntN policyIndex, const Frequency& centerFrequency)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setRfProfileCenterFrequency(policyIndex, centerFrequency);
}

RfProfileData Participant::getRfProfileData(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getRfProfileData();
}

TemperatureStatus Participant::getTemperatureStatus(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getTemperatureStatus();
}

TemperatureThresholds Participant::getTemperatureThresholds(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getTemperatureThresholds();
}

void Participant::setTemperatureThresholds(UIntN domainIndex, UIntN policyIndex, const TemperatureThresholds& temperatureThresholds)
{
    throwIfDomainIndexInvalid(domainIndex);
    m_domain[domainIndex]->setTemperatureThresholds(policyIndex, temperatureThresholds);
}

UtilizationStatus Participant::getUtilizationStatus(UIntN domainIndex)
{
    throwIfDomainIndexInvalid(domainIndex);
    return m_domain[domainIndex]->getUtilizationStatus();
}

std::map<ParticipantSpecificInfoKey::Type, UIntN> Participant::getParticipantSpecificInfo(
    const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo)
{
    return m_theRealParticipant->getParticipantSpecificInfo(m_participantIndex, requestedInfo);
}

ParticipantProperties Participant::getParticipantProperties(void)
{
    return m_theRealParticipant->getParticipantProperties(m_participantIndex);
}

DomainPropertiesSet Participant::getDomainPropertiesSet(void)
{
    return m_theRealParticipant->getDomainPropertiesSet(m_participantIndex);
}

void Participant::setParticipantDeviceTemperatureIndication(const Temperature& temperature)
{
    m_theRealParticipant->setParticipantDeviceTemperatureIndication(m_participantIndex, temperature);
}

void Participant::setParticipantCoolingPolicy(const CoolingPreference& coolingPreference)
{
    m_theRealParticipant->setParticipantCoolingPolicy(m_participantIndex, coolingPreference);
}

void Participant::throwIfDomainIndexInvalid(UIntN domainIndex)
{
    if ((domainIndex >= m_domain.size()) ||
        (m_domain[domainIndex] == nullptr))
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF, "Domain index is invalid for this participant.");
        message.addMessage("Domain Index", domainIndex);
        message.setParticipantIndex(m_participantIndex);
        m_dptfManager->getEsifServices()->writeMessageWarning(message);

        throw dptf_exception(message);
    }
}