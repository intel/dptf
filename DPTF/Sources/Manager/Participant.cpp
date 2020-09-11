/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "ManagerLogger.h"

Participant::Participant(DptfManagerInterface* dptfManager)
	: m_participantCreated(false)
	, m_dptfManager(dptfManager)
	, m_theRealParticipant(nullptr)
	, m_participantServices(nullptr)
	, m_participantIndex(Constants::Invalid)
	, m_participantGuid(Guid())
	, m_participantName(Constants::EmptyString)
	, m_domains()
{
}

Participant::~Participant(void)
{
	destroyParticipant();
}

void Participant::createParticipant(
	UIntN participantIndex,
	const AppParticipantDataPtr participantDataPtr,
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
		message << "Failed to create participant instance for participant: "
				<< EsifDataString(&participantDataPtr->fName);
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
			PciInfo(
				participantDataPtr->fPciVendor,
				participantDataPtr->fPciDevice,
				participantDataPtr->fPciBus,
				participantDataPtr->fPciBusDevice,
				participantDataPtr->fPciFunction,
				participantDataPtr->fPciRevision,
				participantDataPtr->fPciClass,
				participantDataPtr->fPciSubClass,
				participantDataPtr->fPciProgIf),
			AcpiInfo(
				EsifDataString(&participantDataPtr->fAcpiDevice),
				EsifDataString(&participantDataPtr->fAcpiScope),
				EsifDataString(&participantDataPtr->fAcpiUID),
				participantDataPtr->fAcpiType),
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

		m_domains[domainIndex]->createDomain(
			m_participantIndex, domainIndex, m_theRealParticipant, domainDataPtr, domainEnabled);
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
	m_theRealParticipant->clearCachedResults();
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

std::string Participant::getDiagnosticsAsXml() const
{
	throwIfRealParticipantIsInvalid();
	std::shared_ptr<XmlNode> node = XmlNode::createWrapperElement("participant");
	node->addChild(m_theRealParticipant->getDiagnosticsAsXml(Constants::Invalid));
	return node->toString();
}

std::shared_ptr<XmlNode> Participant::getArbitrationXmlForPolicy(UIntN policyIndex, ControlFactoryType::Type type) const
{
	auto participantRoot = XmlNode::createWrapperElement("participant_status");
	participantRoot->addChild(XmlNode::createDataElement("participant_name", m_theRealParticipant->getName()));

	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		throwIfDomainInvalid(domain->first);
		if (domain->second != nullptr)
		{
			participantRoot->addChild(domain->second->getArbitrationXmlForPolicy(policyIndex, type));
		}
	}

	return participantRoot;
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
		m_theRealParticipant->clearTemperatureControlCachedData();
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
		m_theRealParticipant->clearTemperatureControlCachedData();
	}
}

void Participant::domainVirtualSensorPollingTableChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainVirtualSensorPollingTableChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->clearTemperatureControlCachedData();
	}
}

void Participant::domainVirtualSensorRecalcChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainVirtualSensorRecalcChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->clearTemperatureControlCachedData();
	}
}

void Participant::domainBatteryStatusChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainBatteryStatusChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->clearBatteryStatusControlCachedData();
	}
}

void Participant::domainBatteryInformationChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainBatteryInformationChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->clearBatteryStatusControlCachedData();
	}
}

void Participant::domainBatteryHighFrequencyImpedanceChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainBatteryHighFrequencyImpedanceChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->clearBatteryStatusControlCachedData();
	}
}

void Participant::domainBatteryNoLoadVoltageChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainBatteryNoLoadVoltageChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->clearBatteryStatusControlCachedData();
	}
}

void Participant::domainMaxBatteryPeakCurrentChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainMaxBatteryPeakCurrentChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->clearBatteryStatusControlCachedData();
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
		m_theRealParticipant->clearBatteryStatusControlCachedData();
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

void Participant::domainMaxBatteryPowerChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainMaxBatteryPowerChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->clearBatteryStatusControlCachedData();
	}
}

void Participant::domainPlatformBatterySteadyStateChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainPlatformBatterySteadyStateChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->clearBatteryStatusControlCachedData();
	}
}

void Participant::domainACNominalVoltageChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainACNominalVoltageChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->domainACNominalVoltageChanged();
	}
}

void Participant::domainACOperationalCurrentChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainACOperationalCurrentChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->domainACOperationalCurrentChanged();
	}
}

void Participant::domainAC1msPercentageOverloadChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainAC1msPercentageOverloadChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->domainAC1msPercentageOverloadChanged();
	}
}

void Participant::domainAC2msPercentageOverloadChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainAC2msPercentageOverloadChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->domainAC2msPercentageOverloadChanged();
	}
}

void Participant::domainAC10msPercentageOverloadChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainAC10msPercentageOverloadChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->domainAC10msPercentageOverloadChanged();
	}
}

void Participant::domainEnergyThresholdCrossed(void)
{
	if (isEventRegistered(ParticipantEvent::DomainEnergyThresholdCrossed))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->domainEnergyThresholdCrossed();
	}
}

void Participant::domainFanCapabilityChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainFanCapabilityChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->domainFanCapabilityChanged();
	}
}

void Participant::domainSocWorkloadClassificationChanged(void)
{
	if (isEventRegistered(ParticipantEvent::DomainSocWorkloadClassificationChanged))
	{
		throwIfRealParticipantIsInvalid();
		m_theRealParticipant->domainSocWorkloadClassificationChanged();
	}
}

//
// The following functions pass through to the domain implementation
//

Percentage Participant::getUtilizationThreshold(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getUtilizationThreshold();
}

Percentage Participant::getResidencyUtilization(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getResidencyUtilization();
}

UInt64 Participant::getCoreActivityCounter(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getCoreActivityCounter();
}

UInt32 Participant::getCoreActivityCounterWidth(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getCoreActivityCounterWidth();
}

UInt64 Participant::getTimestampCounter(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getTimestampCounter();
}

UInt32 Participant::getTimestampCounterWidth(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getTimestampCounterWidth();
}

void Participant::setPowerShareEffectiveBias(UIntN domainIndex, UInt32 powerShareEffectiveBias)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setPowerShareEffectiveBias(powerShareEffectiveBias);
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

void Participant::setDisplayControlDynamicCaps(
	UIntN domainIndex,
	UIntN policyIndex,
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

UInt32 Participant::getRaplEnergyCounter(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRaplEnergyCounter();
}

double Participant::getRaplEnergyUnit(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRaplEnergyUnit();
}

UInt32 Participant::getRaplEnergyCounterWidth(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRaplEnergyCounterWidth();
}

Power Participant::getInstantaneousPower(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getInstantaneousPower();
}

UInt32 Participant::getEnergyThreshold(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getEnergyThreshold();
}

void Participant::setEnergyThreshold(UIntN domainIndex, UInt32 energyThreshold)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setEnergyThreshold(energyThreshold);
}

void Participant::setEnergyThresholdInterruptDisable(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setEnergyThresholdInterruptDisable();
}

Power Participant::getACPeakPower(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getACPeakPower();
}

void Participant::setACPeakPower(UIntN domainIndex, UIntN policyIndex, const Power& acPeakPower)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setACPeakPower(policyIndex, acPeakPower);
}

Power Participant::getDCPeakPower(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getDCPeakPower();
}

void Participant::setDCPeakPower(UIntN domainIndex, UIntN policyIndex, const Power& dcPeakPower)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setDCPeakPower(policyIndex, dcPeakPower);
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

void Participant::setPerformanceControlDynamicCaps(
	UIntN domainIndex,
	UIntN policyIndex,
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

PowerControlDynamicCapsSet Participant::getPowerControlDynamicCapsSet(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControlDynamicCapsSet();
}

void Participant::setPowerControlDynamicCapsSet(
	UIntN domainIndex,
	UIntN policyIndex,
	PowerControlDynamicCapsSet capsSet)
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

Power Participant::getPowerLimitWithoutCache(UIntN domainIndex, PowerControlType::Type controlType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerLimitWithoutCache(controlType);
}

Bool Participant::isSocPowerFloorEnabled(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->isSocPowerFloorEnabled();
}

Bool Participant::isSocPowerFloorSupported(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->isSocPowerFloorSupported();
}

void Participant::setPowerLimit(
	UIntN domainIndex,
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setPowerLimit(policyIndex, controlType, powerLimit);
}

void Participant::setPowerLimitIgnoringCaps(
	UIntN domainIndex,
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setPowerLimitIgnoringCaps(policyIndex, controlType, powerLimit);
}

TimeSpan Participant::getPowerLimitTimeWindow(UIntN domainIndex, PowerControlType::Type controlType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerLimitTimeWindow(controlType);
}

void Participant::setPowerLimitTimeWindow(
	UIntN domainIndex,
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setPowerLimitTimeWindow(policyIndex, controlType, timeWindow);
}

void Participant::setPowerLimitTimeWindowIgnoringCaps(
	UIntN domainIndex,
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setPowerLimitTimeWindowIgnoringCaps(policyIndex, controlType, timeWindow);
}

Percentage Participant::getPowerLimitDutyCycle(UIntN domainIndex, PowerControlType::Type controlType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerLimitDutyCycle(controlType);
}

void Participant::setPowerLimitDutyCycle(
	UIntN domainIndex,
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const Percentage& dutyCycle)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setPowerLimitDutyCycle(policyIndex, controlType, dutyCycle);
}

void Participant::setSocPowerFloorState(UIntN domainIndex, UIntN policyIndex, Bool socPowerFloorState)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setSocPowerFloorState(policyIndex, socPowerFloorState);
}

void Participant::setPowerCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setPowerCapsLock(policyIndex, lock);
}

Bool Participant::isPowerShareControl(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->isPowerShareControl();
}

double Participant::getPidKpTerm(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPidKpTerm();
}

double Participant::getPidKiTerm(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPidKiTerm();
}

TimeSpan Participant::getAlpha(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getAlpha();
}

TimeSpan Participant::getFastPollTime(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getFastPollTime();
}

TimeSpan Participant::getSlowPollTime(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSlowPollTime();
}

TimeSpan Participant::getWeightedSlowPollAvgConstant(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getWeightedSlowPollAvgConstant();
}

Power Participant::getSlowPollPowerThreshold(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSlowPollPowerThreshold();
}

void Participant::removePowerLimitPolicyRequest(
	UIntN domainIndex,
	UIntN policyIndex,
	PowerControlType::Type controlType)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->removePowerLimitPolicyRequest(policyIndex, controlType);
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

void Participant::setCalculatedAveragePower(UIntN domainIndex, Power powerValue)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setCalculatedAveragePower(powerValue);
}

Bool Participant::isSystemPowerLimitEnabled(UIntN domainIndex, PsysPowerLimitType::Type limitType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->isSystemPowerLimitEnabled(limitType);
}

Power Participant::getSystemPowerLimit(UIntN domainIndex, PsysPowerLimitType::Type limitType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSystemPowerLimit(limitType);
}

void Participant::setSystemPowerLimit(
	UIntN domainIndex,
	UIntN policyIndex,
	PsysPowerLimitType::Type limitType,
	const Power& powerLimit)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setSystemPowerLimit(policyIndex, limitType, powerLimit);
}

TimeSpan Participant::getSystemPowerLimitTimeWindow(UIntN domainIndex, PsysPowerLimitType::Type limitType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSystemPowerLimitTimeWindow(limitType);
}

void Participant::setSystemPowerLimitTimeWindow(
	UIntN domainIndex,
	UIntN policyIndex,
	PsysPowerLimitType::Type limitType,
	const TimeSpan& timeWindow)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setSystemPowerLimitTimeWindow(policyIndex, limitType, timeWindow);
}

Percentage Participant::getSystemPowerLimitDutyCycle(UIntN domainIndex, PsysPowerLimitType::Type limitType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSystemPowerLimitDutyCycle(limitType);
}

void Participant::setSystemPowerLimitDutyCycle(
	UIntN domainIndex,
	UIntN policyIndex,
	PsysPowerLimitType::Type limitType,
	const Percentage& dutyCycle)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setSystemPowerLimitDutyCycle(policyIndex, limitType, dutyCycle);
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

PlatformPowerSource::Type Participant::getPlatformPowerSource(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPlatformPowerSource();
}

UInt32 Participant::getACNominalVoltage(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getACNominalVoltage();
}

UInt32 Participant::getACOperationalCurrent(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getACOperationalCurrent();
}

Percentage Participant::getAC1msPercentageOverload(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getAC1msPercentageOverload();
}

Percentage Participant::getAC2msPercentageOverload(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getAC2msPercentageOverload();
}

Percentage Participant::getAC10msPercentageOverload(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getAC10msPercentageOverload();
}

void Participant::notifyForProchotDeassertion(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->notifyForProchotDeassertion();
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

Percentage Participant::getSscBaselineSpreadValue(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSscBaselineSpreadValue();
}

Percentage Participant::getSscBaselineThreshold(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSscBaselineThreshold();
}

Percentage Participant::getSscBaselineGuardBand(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSscBaselineGuardBand();
}

RfProfileDataSet Participant::getRfProfileDataSet(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRfProfileDataSet();
}

UtilizationStatus Participant::getUtilizationStatus(UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getUtilizationStatus();
}

void Participant::setPowerSharePolicyPower(UIntN domainIndex, const Power& powerSharePolicyPower)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->setPowerSharePolicyPower(powerSharePolicyPower);
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
	if ((match == m_domains.end()) || (match->second == nullptr) || (match->second->isCreated() == false))
	{
		ManagerMessage message = ManagerMessage(m_dptfManager, FLF, "Domain index is invalid for this participant.");
		message.addMessage("Domain Index", domainIndex);
		message.setParticipantIndex(m_participantIndex);
		MANAGER_LOG_MESSAGE_WARNING({ return message; });
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

EsifServicesInterface* Participant::getEsifServices() const
{
	return m_dptfManager->getEsifServices();
}
