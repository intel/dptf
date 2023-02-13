/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

const Guid FormatId(0xF0, 0xCB, 0x64, 0x06, 0xE4, 0x2B, 0x46, 0xB5, 0x9C, 0x85, 0x32, 0xD1, 0xA1, 0xB7, 0xCB, 0x68);

UnifiedParticipant::UnifiedParticipant(void)
	: m_guid(Guid())
	, m_participantIndex(Constants::Invalid)
	, m_enabled(false)
	, m_name(Constants::EmptyString)
	, m_description(Constants::EmptyString)
	, m_busType(BusType::None)
	, m_pciInfo(PciInfo())
	, m_acpiInfo(AcpiInfo())
	, m_participantServicesInterface(nullptr)
	, m_getSpecificInfo(nullptr)
	, m_setSpecificInfo(nullptr)
	, m_domains()
	, m_classFactories()
	, m_coreControlEventsRegistered(false)
	, m_displayControlEventsRegistered(false)
	, m_domainPriorityEventsRegistered(false)
	, m_performanceControlEventsRegistered(false)
	, m_powerControlEventsRegistered(false)
	, m_rfProfileEventsRegistered(false)
	, m_temperatureEventsRegistered(false)
	, m_powerStatusEventsRegistered(false)
	, m_batteryStatusEventsRegistered(false)
	, m_loggingEventsRegistered(false)
	, m_energyEventsRegistered(false)
	, m_activeControlEventsRegistered(false)
	, m_socWorkloadClassificationEventsRegistered(false)
	, m_eppSensitivityHintEventsRegistered(false)
	, m_extendedWorkloadPredictionEventsRegistered(false)
{
	// initialize();
}

UnifiedParticipant::UnifiedParticipant(const ControlFactoryList& classFactories)
	: m_guid(Guid())
	, m_participantIndex(Constants::Invalid)
	, m_enabled(false)
	, m_name(Constants::EmptyString)
	, m_description(Constants::EmptyString)
	, m_busType(BusType::None)
	, m_pciInfo(PciInfo())
	, m_acpiInfo(AcpiInfo())
	, m_participantServicesInterface(nullptr)
	, m_getSpecificInfo(nullptr)
	, m_setSpecificInfo(nullptr)
	, m_domains()
	, m_classFactories(classFactories)
	, m_coreControlEventsRegistered(false)
	, m_displayControlEventsRegistered(false)
	, m_domainPriorityEventsRegistered(false)
	, m_performanceControlEventsRegistered(false)
	, m_powerControlEventsRegistered(false)
	, m_rfProfileEventsRegistered(false)
	, m_temperatureEventsRegistered(false)
	, m_powerStatusEventsRegistered(false)
	, m_batteryStatusEventsRegistered(false)
	, m_loggingEventsRegistered(false)
	, m_energyEventsRegistered(false)
	, m_activeControlEventsRegistered(false)
	, m_socWorkloadClassificationEventsRegistered(false)
	, m_eppSensitivityHintEventsRegistered(false)
	, m_extendedWorkloadPredictionEventsRegistered(false)
{
	// initialize();

	// TODO: Check if this is needed
	// In this case, which is provided for validation purposes, any class factories that aren't
	// provided via the constructor will be filled in.  This allows test code to be written
	// that only sends in the 'fake' factories as needed.
}

std::shared_ptr<ParticipantServicesInterface> UnifiedParticipant::getParticipantServices() const
{
	return m_participantServicesInterface;
}

void UnifiedParticipant::initialize(void)
{
	m_guid = Guid();
	m_participantIndex = Constants::Invalid;
	m_enabled = false;
	m_name = Constants::EmptyString;
	m_description = Constants::EmptyString;
	m_busType = BusType::None;
	m_pciInfo = PciInfo();
	m_acpiInfo = AcpiInfo();
	m_participantServicesInterface = nullptr;
	m_getSpecificInfo = nullptr;
	m_setSpecificInfo = nullptr;
	m_coreControlEventsRegistered = false;
	m_displayControlEventsRegistered = false;
	m_domainPriorityEventsRegistered = false;
	m_performanceControlEventsRegistered = false;
	m_powerControlEventsRegistered = false;
	m_rfProfileEventsRegistered = false;
	m_temperatureEventsRegistered = false;
	m_powerStatusEventsRegistered = false;
	m_batteryStatusEventsRegistered = false;
	m_loggingEventsRegistered = false;
	m_energyEventsRegistered = false;
	m_activeControlEventsRegistered = false;
	m_socWorkloadClassificationEventsRegistered = false;
	m_eppSensitivityHintEventsRegistered = false;
	m_extendedWorkloadPredictionEventsRegistered = false;
}

void UnifiedParticipant::initializeSpecificInfoControl(void)
{
	if (!m_getSpecificInfo)
	{
		auto factory = m_classFactories.getFactory(ControlFactoryType::GetSpecificInfo);
		m_getSpecificInfo.reset(dynamic_cast<ParticipantGetSpecificInfoBase*>(
			factory->make(m_participantIndex, 0, GetSpecificInfoVersionDefault, m_participantServicesInterface)));
		m_participantServicesInterface->registerEvent(ParticipantEvent::ParticipantSpecificInfoChanged);
	}
	if (!m_setSpecificInfo)
	{
		auto factory = m_classFactories.getFactory(ControlFactoryType::SetSpecificInfo);
		m_setSpecificInfo.reset(dynamic_cast<ParticipantSetSpecificInfoBase*>(
			factory->make(m_participantIndex, 0, SetSpecificInfoVersionDefault, m_participantServicesInterface)));
	}
}

UnifiedParticipant::~UnifiedParticipant(void)
{
}

void UnifiedParticipant::createParticipant(
	const Guid& guid,
	UIntN participantIndex,
	Bool enabled,
	const std::string& name,
	const std::string& description,
	BusType::Type busType,
	const PciInfo& pciInfo,
	const AcpiInfo& acpiInfo,
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
	m_participantServicesInterface->registerEvent(ParticipantEvent::DptfResume);
}

void UnifiedParticipant::destroyParticipant(void)
{
	destroyAllDomains();

	if (m_participantServicesInterface != nullptr)
	{
		if (m_getSpecificInfo != nullptr)
		{
			m_participantServicesInterface->unregisterEvent(ParticipantEvent::ParticipantSpecificInfoChanged);
		}

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
	if (m_getSpecificInfo)
	{
		m_getSpecificInfo->onClearCachedData();
	}

	if (m_setSpecificInfo)
	{
		m_setSpecificInfo->onClearCachedData();
	}

	// Clear all cached data on all domains
	clearCachedData();
}

Bool UnifiedParticipant::isParticipantEnabled(void)
{
	return m_enabled;
}

void UnifiedParticipant::createDomain(
	const Guid& guid,
	UIntN participantIndex,
	UIntN domainIndex,
	Bool enabled,
	DomainType::Type domainType,
	const std::string& name,
	const std::string& description,
	DomainFunctionalityVersions domainFunctionalityVersions)
{
	throwIfDomainIndexLocationInvalid(domainIndex);

	// If domain supports temperature, initialize trip point controls for participant (one time only)
	if (domainFunctionalityVersions.temperatureThresholdVersion)
	{
		initializeSpecificInfoControl();
	}

	// Create domain class
	auto domain = std::make_shared<UnifiedDomain>(
		guid,
		participantIndex,
		domainIndex,
		enabled,
		domainType,
		name,
		description,
		domainFunctionalityVersions,
		m_classFactories,
		m_participantServicesInterface);

	// Add to domain set
	insertDomainAtIndexLocation(domain, domainIndex);

	updateDomainEventRegistrations();
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
	for (auto index = domainIndexes.begin(); index != domainIndexes.end(); ++index)
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
	UInt8 activityStatusTotal = 0;
	UIntN coreControlTotal = 0;
	UIntN displayControlTotal = 0;
	UIntN domainPriorityTotal = 0;
	UIntN performanceControlTotal = 0;
	UIntN powerControlTotal = 0;
	UIntN rfProfileTotal = 0;
	UIntN temperatureControlTotal = 0;
	UIntN platformPowerStatusControlTotal = 0;
	UIntN batteryStatusControlTotal = 0;
	UIntN peakPowerControlTotal = 0;
	UIntN processorControlTotal = 0;
	UIntN socWorkloadClassificationControlTotal = 0;
	UIntN dynamicEppControlTotal = 0;
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
			activityStatusTotal += versions.activityStatusVersion;
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
			batteryStatusControlTotal += versions.batteryStatusVersion;
			peakPowerControlTotal += versions.peakPowerControlVersion;
			processorControlTotal += versions.processorControlVersion;
			socWorkloadClassificationControlTotal += versions.socWorkloadClassificationVersion;
			dynamicEppControlTotal += versions.dynamicEppVersion;
			if ((activeControlTotal != 0) || (activityStatusTotal != 0) || (coreControlTotal != 0)
				|| (displayControlTotal != 0) || (domainPriorityTotal != 0) || (performanceControlTotal != 0)
				|| (powerControlTotal != 0) || (rfProfileTotal != 0) || (temperatureControlTotal != 0)
				|| (platformPowerStatusControlTotal != 0) || (peakPowerControlTotal != 0)
				|| (processorControlTotal != 0) || (batteryStatusControlTotal != 0)
				|| (socWorkloadClassificationControlTotal != 0) || (dynamicEppControlTotal != 0))
			{
				loggingTotal = 1;
			}
		}
	}

	std::set<ParticipantEvent::Type> coreControlEvents;
	coreControlEvents.insert(ParticipantEvent::Type::DomainCoreControlCapabilityChanged);
	m_coreControlEventsRegistered =
		updateDomainEventRegistration(coreControlTotal, m_coreControlEventsRegistered, coreControlEvents);

	std::set<ParticipantEvent::Type> displayControlEvents;
	displayControlEvents.insert(ParticipantEvent::Type::DomainDisplayControlCapabilityChanged);
	displayControlEvents.insert(ParticipantEvent::Type::DomainDisplayStatusChanged);
	m_displayControlEventsRegistered =
		updateDomainEventRegistration(displayControlTotal, m_displayControlEventsRegistered, displayControlEvents);

	std::set<ParticipantEvent::Type> domainPriorityEvents;
	domainPriorityEvents.insert(ParticipantEvent::Type::DomainPriorityChanged);
	m_domainPriorityEventsRegistered =
		updateDomainEventRegistration(domainPriorityTotal, m_domainPriorityEventsRegistered, domainPriorityEvents);

	std::set<ParticipantEvent::Type> performanceControlEvents;
	performanceControlEvents.insert(ParticipantEvent::Type::DomainPerformanceControlCapabilityChanged);
	performanceControlEvents.insert(ParticipantEvent::Type::DomainPerformanceControlsChanged);
	m_performanceControlEventsRegistered = updateDomainEventRegistration(
		performanceControlTotal, m_performanceControlEventsRegistered, performanceControlEvents);

	std::set<ParticipantEvent::Type> powerControlEvents;
	powerControlEvents.insert(ParticipantEvent::Type::DomainPowerControlCapabilityChanged);
	m_powerControlEventsRegistered =
		updateDomainEventRegistration(powerControlTotal, m_powerControlEventsRegistered, powerControlEvents);

	std::set<ParticipantEvent::Type> radioEvents;
	radioEvents.insert(ParticipantEvent::Type::DomainRadioConnectionStatusChanged);
	radioEvents.insert(ParticipantEvent::Type::DomainRfProfileChanged);
	radioEvents.insert(ParticipantEvent::Type::DptfAppBroadcastPrivileged);
	m_rfProfileEventsRegistered =
		updateDomainEventRegistration(rfProfileTotal, m_rfProfileEventsRegistered, radioEvents);

	std::set<ParticipantEvent::Type> temperatureEvents;
	temperatureEvents.insert(ParticipantEvent::Type::DomainTemperatureThresholdCrossed);
	temperatureEvents.insert(ParticipantEvent::Type::DomainVirtualSensorCalibrationTableChanged);
	temperatureEvents.insert(ParticipantEvent::Type::DomainVirtualSensorPollingTableChanged);
	temperatureEvents.insert(ParticipantEvent::Type::DomainVirtualSensorRecalcChanged);
	m_temperatureEventsRegistered =
		updateDomainEventRegistration(temperatureControlTotal, m_temperatureEventsRegistered, temperatureEvents);

	std::set<ParticipantEvent::Type> platformPowerStatusEvents;
	platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainPlatformPowerSourceChanged);
	platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainAdapterPowerRatingChanged);
	platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainPlatformRestOfPowerChanged);
	platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainACNominalVoltageChanged);
	platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainACOperationalCurrentChanged);
	platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainAC1msPercentageOverloadChanged);
	platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainAC2msPercentageOverloadChanged);
	platformPowerStatusEvents.insert(ParticipantEvent::Type::DomainAC10msPercentageOverloadChanged);
	m_powerStatusEventsRegistered = updateDomainEventRegistration(
		platformPowerStatusControlTotal, m_powerStatusEventsRegistered, platformPowerStatusEvents);

	std::set<ParticipantEvent::Type> batteryStatusEvents;
	batteryStatusEvents.insert(ParticipantEvent::Type::DomainBatteryStatusChanged);
	batteryStatusEvents.insert(ParticipantEvent::Type::DomainBatteryInformationChanged);
	batteryStatusEvents.insert(ParticipantEvent::Type::DomainChargerTypeChanged);
	batteryStatusEvents.insert(ParticipantEvent::Type::DomainMaxBatteryPowerChanged);
	batteryStatusEvents.insert(ParticipantEvent::Type::DomainPlatformBatterySteadyStateChanged);
	batteryStatusEvents.insert(ParticipantEvent::Type::DomainBatteryHighFrequencyImpedanceChanged);
	batteryStatusEvents.insert(ParticipantEvent::Type::DomainBatteryNoLoadVoltageChanged);
	batteryStatusEvents.insert(ParticipantEvent::Type::DomainMaxBatteryPeakCurrentChanged);
	m_batteryStatusEventsRegistered =
		updateDomainEventRegistration(batteryStatusControlTotal, m_batteryStatusEventsRegistered, batteryStatusEvents);

	std::set<ParticipantEvent::Type> loggingEvents;
	loggingEvents.insert(ParticipantEvent::Type::DptfParticipantActivityLoggingEnabled);
	loggingEvents.insert(ParticipantEvent::Type::DptfParticipantActivityLoggingDisabled);
	m_loggingEventsRegistered = updateDomainEventRegistration(loggingTotal, m_loggingEventsRegistered, loggingEvents);

	std::set<ParticipantEvent::Type> energyEvents;
	energyEvents.insert(ParticipantEvent::Type::DomainEnergyThresholdCrossed);
	m_energyEventsRegistered =
		updateDomainEventRegistration(activityStatusTotal, m_energyEventsRegistered, energyEvents);

	std::set<ParticipantEvent::Type> activeControlEvents;
	activeControlEvents.insert(ParticipantEvent::Type::DomainFanCapabilityChanged);
	m_activeControlEventsRegistered =
		updateDomainEventRegistration(activeControlTotal, m_activeControlEventsRegistered, activeControlEvents);

	std::set<ParticipantEvent::Type> socWorkloadClassificationEvents;
	socWorkloadClassificationEvents.insert(ParticipantEvent::Type::DomainSocWorkloadClassificationChanged);
	m_socWorkloadClassificationEventsRegistered = updateDomainEventRegistration(
		socWorkloadClassificationControlTotal,
		m_socWorkloadClassificationEventsRegistered,
		socWorkloadClassificationEvents);

	std::set<ParticipantEvent::Type> extendedWorkloadPredictionEvents;
	extendedWorkloadPredictionEvents.insert(ParticipantEvent::Type::DomainExtendedWorkloadPredictionChanged);
	m_extendedWorkloadPredictionEventsRegistered = updateDomainEventRegistration(
		socWorkloadClassificationControlTotal,
		m_extendedWorkloadPredictionEventsRegistered,
		extendedWorkloadPredictionEvents);

	std::set<ParticipantEvent::Type> eppSensitivityHintEvents;
	eppSensitivityHintEvents.insert(ParticipantEvent::Type::DomainEppSensitivityHintChanged);
	m_eppSensitivityHintEventsRegistered = updateDomainEventRegistration(
		dynamicEppControlTotal, m_eppSensitivityHintEventsRegistered, eppSensitivityHintEvents);
}

Bool UnifiedParticipant::updateDomainEventRegistration(
	UIntN total,
	Bool currentlyRegistered,
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
		PARTICIPANT_LOG_MESSAGE_WARNING({ return "Unable to get participant properties XML status!"; });
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
		catch (not_implemented&)
		{
		}
		catch (...)
		{
			// Write message log error
			PARTICIPANT_LOG_MESSAGE_WARNING({ return "Unable to get specific info XML status!"; });
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
				PARTICIPANT_LOG_MESSAGE_WARNING(
					{ return "Unable to get domain XML status for domain " + std::to_string(domain->first); });
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
			PARTICIPANT_LOG_MESSAGE_WARNING(
				{ return "Unable to get domain XML status for domain " + std::to_string(domainIndex); });
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

std::shared_ptr<XmlNode> UnifiedParticipant::getDiagnosticsAsXml(UIntN domainIndex) const
{
	return getStatusAsXml(domainIndex);
}

std::shared_ptr<XmlNode> UnifiedParticipant::getArbitratorStatusForPolicy(
	UIntN domainIndex,
	UIntN policyIndex,
	ControlFactoryType::Type type) const
{
	auto status = XmlNode::createWrapperElement("domain_status");
	try
	{
		auto matchedDomain = m_domains.find(domainIndex);
		if (matchedDomain != m_domains.end() && matchedDomain->second != nullptr)
		{
			status->addChild(matchedDomain->second->getArbitratorStatusForPolicy(policyIndex, type));
		}
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING(
			{ return "Unable to get arbitrator XML status for domain " + std::to_string(domainIndex); });
	}
	return status;
}

void UnifiedParticipant::clearCachedData()
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->clearAllCachedData();
		}
	}
}

void UnifiedParticipant::clearCachedResults()
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		domain->second->clearAllCachedResults();
	}
}

void UnifiedParticipant::clearTemperatureControlCachedData()
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getTemperatureControl()->clearCachedData();
		}
	}
}

void UnifiedParticipant::clearBatteryStatusControlCachedData()
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getBatteryStatusControl()->clearCachedData();
		}
	}
}

void UnifiedParticipant::connectedStandbyEntry(void)
{
	// FIXME:  Not sure if the participant needs to do anything here.  If we do, we will probably have different
	// functionality for different participants and will need a class factory to create the versions.
	// throw implement_me();
}

void UnifiedParticipant::connectedStandbyExit(void)
{
	// FIXME:  Not sure if the participant needs to do anything here.  If we do, we will probably have different
	// functionality for different participants and will need a class factory to create the versions.
	// throw implement_me();
}

void UnifiedParticipant::suspend(void)
{
	// FIXME:  Not sure if the participant needs to do anything here.  If we do, we will probably have different
	// functionality for different participants and will need a class factory to create the versions.
	// throw implement_me();
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
		auto domainFunctionality = matchedDomain->second->getDomainFunctionalityVersions();
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

			if (currentCapability != Constants::Invalid)
			{
				switch ((eEsifCapabilityType)index)
				{
				case ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL:
					if (domainFunctionality.activeControlVersion > 0)
					{
						matchedDomain->second->getActiveControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_CORE_CONTROL:
					if (domainFunctionality.coreControlVersion > 0)
					{
						matchedDomain->second->getCoreControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL:
					if (domainFunctionality.displayControlVersion > 0)
					{
						matchedDomain->second->getDisplayControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_ENERGY_CONTROL:
					if (domainFunctionality.energyControlVersion > 0)
					{
						matchedDomain->second->getEnergyControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
					if (domainFunctionality.performanceControlVersion > 0)
					{
						matchedDomain->second->getPerformanceControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
					if (domainFunctionality.temperatureThresholdVersion > 0)
					{
						matchedDomain->second->getTemperatureControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
					if (domainFunctionality.domainPriorityVersion > 0)
					{
						matchedDomain->second->getDomainPriorityControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_POWER_CONTROL:
					if (domainFunctionality.powerControlVersion > 0)
					{
						matchedDomain->second->getPowerControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_POWER_STATUS:
					if (domainFunctionality.powerStatusVersion > 0)
					{
						matchedDomain->second->getPowerStatusControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_PSYS_CONTROL:
					if (domainFunctionality.systemPowerControlVersion > 0)
					{
						matchedDomain->second->getSystemPowerControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL:
					if (domainFunctionality.peakPowerControlVersion > 0)
					{
						matchedDomain->second->getPeakPowerControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL:
					if (domainFunctionality.processorControlVersion > 0)
					{
						matchedDomain->second->getProcessorControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
					if (domainFunctionality.rfProfileControlVersion > 0)
					{
						matchedDomain->second->getRfProfileControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS:
					if (domainFunctionality.rfProfileStatusVersion > 0)
					{
						matchedDomain->second->getRfProfileStatusControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION:
					if (domainFunctionality.socWorkloadClassificationVersion > 0)
					{
						matchedDomain->second->getSocWorkloadClassificationControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_DYNAMIC_EPP:
					if (domainFunctionality.dynamicEppVersion > 0)
					{
						matchedDomain->second->getDynamicEppControl()->enableActivityLogging();
						sendActivityLoggingDataIfEnabled(domainIndex, (eEsifCapabilityType)index);
					}
					break;
				case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
				case ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS:
				case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
				case ESIF_CAPABILITY_TYPE_BATTERY_STATUS:
				case ESIF_CAPABILITY_TYPE_MANAGER:
					// ESIF handles participant logging for these capabilities
					break;
				default:
					// To do for other capabilities as part of data logging
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
				case ESIF_CAPABILITY_TYPE_ENERGY_CONTROL:
					matchedDomain->second->getEnergyControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
					matchedDomain->second->getPerformanceControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
					matchedDomain->second->getTemperatureControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
					matchedDomain->second->getDomainPriorityControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_POWER_CONTROL:
					matchedDomain->second->getPowerControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_PSYS_CONTROL:
					matchedDomain->second->getSystemPowerControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL:
					matchedDomain->second->getPeakPowerControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL:
					matchedDomain->second->getProcessorControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION:
					matchedDomain->second->getSocWorkloadClassificationControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_DYNAMIC_EPP:
					matchedDomain->second->getDynamicEppControl()->disableActivityLogging();
					break;
				case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
				case ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS:
				case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
				case ESIF_CAPABILITY_TYPE_BATTERY_STATUS:
				case ESIF_CAPABILITY_TYPE_MANAGER:
					// ESIF handles participant logging for these capabilities
					break;
				case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
				case ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS:
				default:
					// To do for other capabilities as part of data logging
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
	case ESIF_CAPABILITY_TYPE_ENERGY_CONTROL:
		m_domains[domainIndex]->getEnergyControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_PERF_CONTROL:
		m_domains[domainIndex]->getPerformanceControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD:
		m_domains[domainIndex]->getTemperatureControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_POWER_CONTROL:
		m_domains[domainIndex]->getPowerControl()->sendActivityLoggingDataIfEnabled(m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_POWER_STATUS:
		m_domains[domainIndex]->getPowerStatusControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY:
		m_domains[domainIndex]->getDomainPriorityControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_PSYS_CONTROL:
		m_domains[domainIndex]->getSystemPowerControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL:
		m_domains[domainIndex]->getPeakPowerControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL:
		m_domains[domainIndex]->getProcessorControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL:
		m_domains[domainIndex]->getRfProfileControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS:
		m_domains[domainIndex]->getRfProfileStatusControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION:
		m_domains[domainIndex]->getSocWorkloadClassificationControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_DYNAMIC_EPP:
		m_domains[domainIndex]->getDynamicEppControl()->sendActivityLoggingDataIfEnabled(
			m_participantIndex, domainIndex);
		break;
	case ESIF_CAPABILITY_TYPE_TEMP_STATUS:
	case ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS:
	case ESIF_CAPABILITY_TYPE_UTIL_STATUS:
	case ESIF_CAPABILITY_TYPE_BATTERY_STATUS:
	case ESIF_CAPABILITY_TYPE_MANAGER:
		// ESIF handles participant logging for these capabilities
		break;
	default:
		break;
	}
}

void UnifiedParticipant::domainSocWorkloadClassificationChanged(UInt32 socWorkloadClassification)
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getSocWorkloadClassificationControl()->clearCachedData();

			try
			{
				domain->second->getSocWorkloadClassificationControl()->updateSocWorkloadClassification(
					socWorkloadClassification);
			}
			catch (const std::exception& ex)
			{
				PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
					return "Unable to update Soc Workload Classification for domain " + std::to_string(domain->first)
						   + ". Exception: " + std::string(ex.what());
				});
			}
		}
	}
}

void UnifiedParticipant::domainEppSensitivityHintChanged(UInt32 eppSensitivityHint)
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getDynamicEppControl()->clearCachedData();

			try
			{
				domain->second->getDynamicEppControl()->updateEppSensitivityHint(eppSensitivityHint);
			}
			catch (const std::exception& ex)
			{
				PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
					return "Unable to update Epp Sensitivity Hint for domain " + std::to_string(domain->first)
						   + ". Exception: " + std::string(ex.what());
				});
			}
		}
	}
}

void UnifiedParticipant::domainExtendedWorkloadPredictionChanged(UInt32 extendedWorkloadPrediction)
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			// Since SOCWL shares the same caching mechanism, clearing the cache here will impact SOCWL
			// Revisit caching once this moves to its own control
			// domain->second->getSocWorkloadClassificationControl()->clearCachedData();

			try
			{
				domain->second->getSocWorkloadClassificationControl()->updateExtendedWorkloadPrediction(extendedWorkloadPrediction);
			}
			catch (const std::exception& ex)
			{
				PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
					return "Unable to update Extended Workload Prediction for domain " + std::to_string(domain->first)
						   + ". Exception: " + std::string(ex.what());
				});
			}
		}
	}
}

void UnifiedParticipant::domainFanOperatingModeChanged()
{
	// Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getActiveControl()->onClearCachedData();
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
			domain->second->getCoreControl()->onClearCachedData();
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
			domain->second->getDisplayControl()->onClearCachedData();
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
			domain->second->getDisplayControl()->onClearCachedData();
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
			domain->second->getPerformanceControl()->onClearCachedData();
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
			domain->second->getPerformanceControl()->onClearCachedData();
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
			domain->second->getPowerControl()->onClearCachedData();
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
			domain->second->getDomainPriorityControl()->onClearCachedData();
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
			domain->second->getRfProfileStatusControl()->onClearCachedData();
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
			domain->second->getRfProfileStatusControl()->onClearCachedData();
		}
	}
}

void UnifiedParticipant::domainEnergyThresholdCrossed(void)
{
	// Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getActivityStatusControl()->onClearCachedData();
		}
	}
}

void UnifiedParticipant::domainFanCapabilityChanged(void)
{
	// Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getActiveControl()->clearCachedData();
		}
	}
}

void UnifiedParticipant::participantSpecificInfoChanged(void)
{
	if (m_getSpecificInfo)
	{
		m_getSpecificInfo->onClearCachedData();
	}
}

void UnifiedParticipant::domainPlatformPowerSourceChanged(void)
{
	// Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getPlatformPowerStatusControl()->onClearCachedData();
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
			domain->second->getPlatformPowerStatusControl()->onClearCachedData();
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
			domain->second->getPlatformPowerStatusControl()->onClearCachedData();
		}
	}
}

void UnifiedParticipant::domainACNominalVoltageChanged(void)
{
	// Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getPlatformPowerStatusControl()->onClearCachedData();
		}
	}
}

void UnifiedParticipant::domainACOperationalCurrentChanged(void)
{
	// Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getPlatformPowerStatusControl()->onClearCachedData();
		}
	}
}

void UnifiedParticipant::domainAC1msPercentageOverloadChanged(void)
{
	// Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getPlatformPowerStatusControl()->onClearCachedData();
		}
	}
}

void UnifiedParticipant::domainAC2msPercentageOverloadChanged(void)
{
	// Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getPlatformPowerStatusControl()->onClearCachedData();
		}
	}
}

void UnifiedParticipant::domainAC10msPercentageOverloadChanged(void)
{
	// Clear the cached data.  The data will be reloaded to the cache when requested by the policies.
	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			domain->second->getPlatformPowerStatusControl()->onClearCachedData();
		}
	}
}

Percentage UnifiedParticipant::getUtilizationThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getActivityStatusControl()->getUtilizationThreshold(participantIndex, domainIndex);
}

Percentage UnifiedParticipant::getResidencyUtilization(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getActivityStatusControl()->getResidencyUtilization(participantIndex, domainIndex);
}

UInt64 UnifiedParticipant::getCoreActivityCounter(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getActivityStatusControl()->getCoreActivityCounter(participantIndex, domainIndex);
}

UInt32 UnifiedParticipant::getCoreActivityCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getActivityStatusControl()->getCoreActivityCounterWidth(
		participantIndex, domainIndex);
}

UInt64 UnifiedParticipant::getTimestampCounter(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getActivityStatusControl()->getTimestampCounter(participantIndex, domainIndex);
}

UInt32 UnifiedParticipant::getTimestampCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getActivityStatusControl()->getTimestampCounterWidth(participantIndex, domainIndex);
}

CoreActivityInfo UnifiedParticipant::getCoreActivityInfo(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getActivityStatusControl()->getCoreActivityInfo(participantIndex, domainIndex);
}

CoreControlStaticCaps UnifiedParticipant::getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getCoreControl()->getCoreControlStaticCaps(participantIndex, domainIndex);
}

CoreControlDynamicCaps UnifiedParticipant::getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getCoreControl()->getCoreControlDynamicCaps(participantIndex, domainIndex);
}

CoreControlLpoPreference UnifiedParticipant::getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getCoreControl()->getCoreControlLpoPreference(participantIndex, domainIndex);
}

CoreControlStatus UnifiedParticipant::getCoreControlStatus(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getCoreControl()->getCoreControlStatus(participantIndex, domainIndex);
}

void UnifiedParticipant::setActiveCoreControl(
	UIntN participantIndex,
	UIntN domainIndex,
	const CoreControlStatus& coreControlStatus)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getCoreControl()->setActiveCoreControl(participantIndex, domainIndex, coreControlStatus);

	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_CORE_CONTROL);
}

DisplayControlDynamicCaps UnifiedParticipant::getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getDisplayControl()->getDisplayControlDynamicCaps(participantIndex, domainIndex);
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

UIntN UnifiedParticipant::getUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getDisplayControl()->getUserPreferredSoftBrightnessIndex(
		participantIndex, domainIndex);
}

Bool UnifiedParticipant::isUserPreferredIndexModified(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getDisplayControl()->isUserPreferredIndexModified(participantIndex, domainIndex);
}

UIntN UnifiedParticipant::getSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getDisplayControl()->getSoftBrightnessIndex(participantIndex, domainIndex);
}

DisplayControlSet UnifiedParticipant::getDisplayControlSet(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getDisplayControl()->getDisplayControlSet(participantIndex, domainIndex);
}

void UnifiedParticipant::setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getDisplayControl()->setDisplayControl(participantIndex, domainIndex, displayControlIndex);

	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL);
}

void UnifiedParticipant::setSoftBrightness(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getDisplayControl()->setSoftBrightness(participantIndex, domainIndex, displayControlIndex);

	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL);
}

void UnifiedParticipant::updateUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getDisplayControl()->updateUserPreferredSoftBrightnessIndex(participantIndex, domainIndex);
}

void UnifiedParticipant::restoreUserPreferredSoftBrightness(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getDisplayControl()->restoreUserPreferredSoftBrightness(participantIndex, domainIndex);
}

void UnifiedParticipant::setDisplayControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	DisplayControlDynamicCaps newCapabilities)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getDisplayControl()->setDisplayControlDynamicCaps(
		participantIndex, domainIndex, newCapabilities);

	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL);
}

void UnifiedParticipant::setDisplayCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getDisplayControl()->setDisplayCapsLock(participantIndex, domainIndex, lock);
}

UInt32 UnifiedParticipant::getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getEnergyControl()->getRaplEnergyCounter(participantIndex, domainIndex);
}

double UnifiedParticipant::getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getEnergyControl()->getRaplEnergyUnit(participantIndex, domainIndex);
}

EnergyCounterInfo UnifiedParticipant::getRaplEnergyCounterInfo(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getEnergyControl()->getRaplEnergyCounterInfo(participantIndex, domainIndex);
}

UInt32 UnifiedParticipant::getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getEnergyControl()->getRaplEnergyCounterWidth(participantIndex, domainIndex);
}

Power UnifiedParticipant::getInstantaneousPower(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getEnergyControl()->getInstantaneousPower(participantIndex, domainIndex);
}

UInt32 UnifiedParticipant::getEnergyThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getEnergyControl()->getEnergyThreshold(participantIndex, domainIndex);
}

void UnifiedParticipant::setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getEnergyControl()->setEnergyThreshold(participantIndex, domainIndex, energyThreshold);
}

void UnifiedParticipant::setEnergyThresholdInterruptDisable(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getEnergyControl()->setEnergyThresholdInterruptDisable(participantIndex, domainIndex);
}

Power UnifiedParticipant::getACPeakPower(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPeakPowerControl()->getACPeakPower(participantIndex, domainIndex);
}

void UnifiedParticipant::setACPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& acPeakPower)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPeakPowerControl()->setACPeakPower(participantIndex, domainIndex, acPeakPower);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL);
}

Power UnifiedParticipant::getDCPeakPower(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPeakPowerControl()->getDCPeakPower(participantIndex, domainIndex);
}

void UnifiedParticipant::setDCPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& dcPeakPower)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPeakPowerControl()->setDCPeakPower(participantIndex, domainIndex, dcPeakPower);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL);
}

PerformanceControlStaticCaps UnifiedParticipant::getPerformanceControlStaticCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPerformanceControl()->getPerformanceControlStaticCaps(
		participantIndex, domainIndex);
}

PerformanceControlDynamicCaps UnifiedParticipant::getPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPerformanceControl()->getPerformanceControlDynamicCaps(
		participantIndex, domainIndex);
}

PerformanceControlStatus UnifiedParticipant::getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPerformanceControl()->getPerformanceControlStatus(participantIndex, domainIndex);
}

PerformanceControlSet UnifiedParticipant::getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPerformanceControl()->getPerformanceControlSet(participantIndex, domainIndex);
}

void UnifiedParticipant::setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPerformanceControl()->setPerformanceControl(
		participantIndex, domainIndex, performanceControlIndex);

	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PERF_CONTROL);
}

void UnifiedParticipant::setPerformanceControlDynamicCaps(
	UIntN participantIndex,
	UIntN domainIndex,
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
	m_domains[domainIndex]->getPerformanceControl()->setPerformanceCapsLock(participantIndex, domainIndex, lock);
}

PowerControlDynamicCapsSet UnifiedParticipant::getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getPowerControlDynamicCapsSet(participantIndex, domainIndex);
}

void UnifiedParticipant::setPowerControlDynamicCapsSet(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlDynamicCapsSet capsSet)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->setPowerControlDynamicCapsSet(participantIndex, domainIndex, capsSet);
}

PowerStatus UnifiedParticipant::getPowerStatus(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerStatusControl()->getPowerStatus(participantIndex, domainIndex);
}

Power UnifiedParticipant::getAveragePower(
	UIntN participantIndex,
	UIntN domainIndex,
	const PowerControlDynamicCaps& capabilities)
{
	throwIfDomainInvalid(domainIndex);
	auto averagePower =
		m_domains[domainIndex]->getPowerStatusControl()->getAveragePower(participantIndex, domainIndex, capabilities);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_STATUS);

	return averagePower;
}

Power UnifiedParticipant::getPowerValue(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerStatusControl()->getPowerValue(participantIndex, domainIndex);
}

void UnifiedParticipant::setCalculatedAveragePower(UIntN participantIndex, UIntN domainIndex, Power powerValue)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerStatusControl()->setCalculatedAveragePower(
		participantIndex, domainIndex, powerValue);
}

Bool UnifiedParticipant::isSystemPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSystemPowerControl()->isSystemPowerLimitEnabled(
		participantIndex, domainIndex, limitType);
}

Power UnifiedParticipant::getSystemPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSystemPowerControl()->getSystemPowerLimit(
		participantIndex, domainIndex, limitType);
}

void UnifiedParticipant::setSystemPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const Power& powerLimit)
{
	throwIfDomainInvalid(domainIndex);
	auto snappedPowerLimit = snapPowerToAbovePL1MinValue(participantIndex, domainIndex, powerLimit);
	m_domains[domainIndex]->getSystemPowerControl()->setSystemPowerLimit(
		participantIndex, domainIndex, limitType, snappedPowerLimit);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PSYS_CONTROL);
}

TimeSpan UnifiedParticipant::getSystemPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSystemPowerControl()->getSystemPowerLimitTimeWindow(
		participantIndex, domainIndex, limitType);
}

void UnifiedParticipant::setSystemPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const TimeSpan& timeWindow)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getSystemPowerControl()->setSystemPowerLimitTimeWindow(
		participantIndex, domainIndex, limitType, timeWindow);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PSYS_CONTROL);
}

Percentage UnifiedParticipant::getSystemPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getSystemPowerControl()->getSystemPowerLimitDutyCycle(
		participantIndex, domainIndex, limitType);
}

void UnifiedParticipant::setSystemPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PsysPowerLimitType::Type limitType,
	const Percentage& dutyCycle)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getSystemPowerControl()->setSystemPowerLimitDutyCycle(
		participantIndex, domainIndex, limitType, dutyCycle);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PSYS_CONTROL);
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

PlatformPowerSource::Type UnifiedParticipant::getPlatformPowerSource(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPlatformPowerStatusControl()->getPlatformPowerSource(
		participantIndex, domainIndex);
}

UInt32 UnifiedParticipant::getACNominalVoltage(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPlatformPowerStatusControl()->getACNominalVoltage(participantIndex, domainIndex);
}

UInt32 UnifiedParticipant::getACOperationalCurrent(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPlatformPowerStatusControl()->getACOperationalCurrent(
		participantIndex, domainIndex);
}

Percentage UnifiedParticipant::getAC1msPercentageOverload(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPlatformPowerStatusControl()->getAC1msPercentageOverload(
		participantIndex, domainIndex);
}

Percentage UnifiedParticipant::getAC2msPercentageOverload(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPlatformPowerStatusControl()->getAC2msPercentageOverload(
		participantIndex, domainIndex);
}

Percentage UnifiedParticipant::getAC10msPercentageOverload(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPlatformPowerStatusControl()->getAC10msPercentageOverload(
		participantIndex, domainIndex);
}

void UnifiedParticipant::notifyForProchotDeassertion(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPlatformPowerStatusControl()->notifyForProchotDeassertion(participantIndex, domainIndex);
}

DomainPriority UnifiedParticipant::getDomainPriority(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getDomainPriorityControl()->getDomainPriority(participantIndex, domainIndex);
}

RfProfileCapabilities UnifiedParticipant::getRfProfileCapabilities(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRfProfileControl()->getRfProfileCapabilities(participantIndex, domainIndex);
}

void UnifiedParticipant::setRfProfileCenterFrequency(
	UIntN participantIndex,
	UIntN domainIndex,
	const Frequency& centerFrequency)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getRfProfileControl()->setRfProfileCenterFrequency(
		participantIndex, domainIndex, centerFrequency);
}

Percentage UnifiedParticipant::getSscBaselineSpreadValue(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRfProfileControl()->getSscBaselineSpreadValue(participantIndex, domainIndex);
}

Percentage UnifiedParticipant::getSscBaselineThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRfProfileControl()->getSscBaselineThreshold(participantIndex, domainIndex);
}

Percentage UnifiedParticipant::getSscBaselineGuardBand(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRfProfileControl()->getSscBaselineGuardBand(participantIndex, domainIndex);
}

RfProfileDataSet UnifiedParticipant::getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRfProfileStatusControl()->getRfProfileDataSet(participantIndex, domainIndex);
}

UInt32 UnifiedParticipant::getWifiCapabilities(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRfProfileStatusControl()->getWifiCapabilities(participantIndex, domainIndex);
}

UInt32 UnifiedParticipant::getRfiDisable(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRfProfileStatusControl()->getRfiDisable(participantIndex, domainIndex);
}

void UnifiedParticipant::setDdrRfiTable(
	UIntN participantIndex,
	UIntN domainIndex,
	DdrfChannelBandPackage::WifiRfiDdr ddrRfiStruct)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getRfProfileStatusControl()->setDdrRfiTable(participantIndex, domainIndex, ddrRfiStruct);
}

void UnifiedParticipant::setProtectRequest(UIntN participantIndex, UIntN domainIndex, UInt64 frequencyRate)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getRfProfileStatusControl()->setProtectRequest(
		participantIndex, domainIndex, frequencyRate);
}

UInt64 UnifiedParticipant::getDvfsPoints(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getRfProfileStatusControl()->getDvfsPoints(participantIndex, domainIndex);
}

UtilizationStatus UnifiedParticipant::getUtilizationStatus(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getUtilizationControl()->getUtilizationStatus(participantIndex, domainIndex);
}

std::map<ParticipantSpecificInfoKey::Type, Temperature> UnifiedParticipant::getParticipantSpecificInfo(
	UIntN participantIndex,
	const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo)
{
	std::map<ParticipantSpecificInfoKey::Type, Temperature> specInfoMap;
	if (m_getSpecificInfo)
	{
		specInfoMap = m_getSpecificInfo->getParticipantSpecificInfo(participantIndex, requestedInfo);
	}
	else
	{
		throw dptf_exception("Specific info have not been initialized yet.");
	}

	return specInfoMap;
}

ParticipantProperties UnifiedParticipant::getParticipantProperties(UIntN participantIndex) const
{
	return ParticipantProperties(m_guid, m_name, m_description, m_busType, m_pciInfo, m_acpiInfo);
}

DomainPropertiesSet UnifiedParticipant::getDomainPropertiesSet(UIntN participantIndex) const
{
	std::vector<DomainProperties> domainPropertiesSet;

	for (auto domain = m_domains.begin(); domain != m_domains.end(); ++domain)
	{
		if (domain->second != nullptr)
		{
			DomainProperties domainProperties(
				domain->second->getGuid(),
				domain->first,
				domain->second->isEnabled(),
				domain->second->getDomainType(),
				domain->second->getName(),
				domain->second->getDescription(),
				domain->second->getDomainFunctionalityVersions());
			domainPropertiesSet.push_back(domainProperties);
		}
		else
		{
			throw dptf_exception("Domain index is invalid.");
		}
	}

	return domainPropertiesSet;
}

void UnifiedParticipant::setParticipantDeviceTemperatureIndication(
	UIntN participantIndex,
	const Temperature& temperature)
{
	if (m_setSpecificInfo)
	{
		m_setSpecificInfo->setParticipantDeviceTemperatureIndication(participantIndex, temperature);
	}
}

void UnifiedParticipant::setParticipantSpecificInfo(
	UIntN participantIndex,
	ParticipantSpecificInfoKey::Type tripPoint,
	const Temperature& tripValue)
{
	if (m_setSpecificInfo)
	{
		m_setSpecificInfo->setParticipantSpecificInfo(participantIndex, tripPoint, tripValue);
	}
}

void UnifiedParticipant::throwIfDomainInvalid(UIntN domainIndex) const
{
	auto matchedDomain = m_domains.find(domainIndex);
	if ((matchedDomain == m_domains.end()) || (matchedDomain->second == nullptr))
	{
		throw dptf_exception("Domain index is invalid.");
	}
}

Power UnifiedParticipant::snapPowerToAbovePL1MinValue(UIntN participantIndex, UIntN domainIndex, Power powerToSet)
{
	try
	{
		if (m_domains[domainIndex]->getDomainFunctionalityVersions().powerControlVersion)
		{
			auto powerCaps =
				m_domains[domainIndex]->getPowerControl()->getPowerControlDynamicCapsSet(participantIndex, domainIndex);
			if (powerCaps.hasCapability(PowerControlType::PL1))
			{
				powerToSet = std::max(powerToSet, powerCaps.getCapability(PowerControlType::PL1).getMinPowerLimit());
			}
		}
	}
	catch (...)
	{
	}
	return powerToSet;
}

Bool UnifiedParticipant::isPowerLimitEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->isPowerLimitEnabled(participantIndex, domainIndex, controlType);
}

Power UnifiedParticipant::getPowerLimit(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getPowerLimit(participantIndex, domainIndex, controlType);
}

Power UnifiedParticipant::getPowerLimitWithoutCache(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getPowerLimitWithoutCache(
		participantIndex, domainIndex, controlType);
}

Bool UnifiedParticipant::isSocPowerFloorEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->isSocPowerFloorEnabled(participantIndex, domainIndex);
}

Bool UnifiedParticipant::isSocPowerFloorSupported(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->isSocPowerFloorSupported(participantIndex, domainIndex);
}

void UnifiedParticipant::setPowerLimit(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throwIfDomainInvalid(domainIndex);
	auto snappedPowerLimit = snapPowerToAbovePL1MinValue(participantIndex, domainIndex, powerLimit);
	m_domains[domainIndex]->getPowerControl()->setPowerLimit(
		participantIndex, domainIndex, controlType, snappedPowerLimit);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::setPowerLimitWithoutUpdatingEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throwIfDomainInvalid(domainIndex);
	auto snappedPowerLimit = snapPowerToAbovePL1MinValue(participantIndex, domainIndex, powerLimit);
	m_domains[domainIndex]->getPowerControl()->setPowerLimitWithoutUpdatingEnabled(
		participantIndex, domainIndex, controlType, snappedPowerLimit);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::setPowerLimitIgnoringCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Power& powerLimit)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->setPowerLimitIgnoringCaps(
		participantIndex, domainIndex, controlType, powerLimit);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

TimeSpan UnifiedParticipant::getPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getPowerLimitTimeWindow(
		participantIndex, domainIndex, controlType);
}

void UnifiedParticipant::setPowerLimitTimeWindow(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->setPowerLimitTimeWindow(
		participantIndex, domainIndex, controlType, timeWindow);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::setPowerLimitTimeWindowWithoutUpdatingEnabled(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->setPowerLimitTimeWindowWithoutUpdatingEnabled(
		participantIndex, domainIndex, controlType, timeWindow);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::setPowerLimitTimeWindowIgnoringCaps(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->setPowerLimitTimeWindowIgnoringCaps(
		participantIndex, domainIndex, controlType, timeWindow);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

Percentage UnifiedParticipant::getPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getPowerLimitDutyCycle(
		participantIndex, domainIndex, controlType);
}

void UnifiedParticipant::setPowerLimitDutyCycle(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType,
	const Percentage& dutyCycle)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->setPowerLimitDutyCycle(
		participantIndex, domainIndex, controlType, dutyCycle);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::setSocPowerFloorState(UIntN participantIndex, UIntN domainIndex, Bool socPowerFloorState)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->setSocPowerFloorState(participantIndex, domainIndex, socPowerFloorState);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::clearPowerLimit(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->clearPowerLimit(participantIndex, domainIndex);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_POWER_CONTROL);
}

void UnifiedParticipant::setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->setPowerCapsLock(participantIndex, domainIndex, lock);
}

TimeSpan UnifiedParticipant::getPowerSharePowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getPowerSharePowerLimitTimeWindow(participantIndex, domainIndex);
}

Bool UnifiedParticipant::isPowerShareControl(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->isPowerShareControl(participantIndex, domainIndex);
}

double UnifiedParticipant::getPidKpTerm(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getPidKpTerm(participantIndex, domainIndex);
}

double UnifiedParticipant::getPidKiTerm(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getPidKiTerm(participantIndex, domainIndex);
}

TimeSpan UnifiedParticipant::getAlpha(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getAlpha(participantIndex, domainIndex);
}

TimeSpan UnifiedParticipant::getFastPollTime(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getFastPollTime(participantIndex, domainIndex);
}

TimeSpan UnifiedParticipant::getSlowPollTime(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getSlowPollTime(participantIndex, domainIndex);
}

TimeSpan UnifiedParticipant::getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getWeightedSlowPollAvgConstant(participantIndex, domainIndex);
}

Power UnifiedParticipant::getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getPowerControl()->getSlowPollPowerThreshold(participantIndex, domainIndex);
}

void UnifiedParticipant::removePowerLimitPolicyRequest(
	UIntN participantIndex,
	UIntN domainIndex,
	PowerControlType::Type controlType)
{
	// Do nothing.  Not an error.
}

void UnifiedParticipant::setPowerSharePolicyPower(
	UIntN participantIndex,
	UIntN domainIndex,
	const Power& powerSharePolicyPower)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPowerControl()->setPowerSharePolicyPower(
		participantIndex, domainIndex, powerSharePolicyPower);
}

void UnifiedParticipant::setPowerShareEffectiveBias(
	UIntN participantIndex,
	UIntN domainIndex,
	UInt32 powerShareEffectiveBias)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getActivityStatusControl()->setPowerShareEffectiveBias(
		participantIndex, domainIndex, powerShareEffectiveBias);
}

UInt32 UnifiedParticipant::getSocDgpuPerformanceHintPoints(UIntN participantIndex, UIntN domainIndex)
{
	throwIfDomainInvalid(domainIndex);
	return m_domains[domainIndex]->getActivityStatusControl()->getSocDgpuPerformanceHintPoints(participantIndex, domainIndex);
}

void UnifiedParticipant::setRfProfileOverride(
	UIntN participantIndex,
	UIntN domainIndex,
	const DptfBuffer& rfProfileBufferData)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getRfProfileStatusControl()->setRfProfileOverride(
		participantIndex, domainIndex, rfProfileBufferData);
}

void UnifiedParticipant::setPerfPreferenceMax(
	UIntN participantIndex,
	UIntN domainIndex,
	Percentage minMaxRatio)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPerformanceControl()->setPerfPreferenceMax(
		participantIndex, domainIndex, minMaxRatio);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PERF_CONTROL);
}

void UnifiedParticipant::setPerfPreferenceMin(UIntN participantIndex, UIntN domainIndex, Percentage minMaxRatio)
{
	throwIfDomainInvalid(domainIndex);
	m_domains[domainIndex]->getPerformanceControl()->setPerfPreferenceMin(
		participantIndex, domainIndex, minMaxRatio);
	sendActivityLoggingDataIfEnabled(domainIndex, ESIF_CAPABILITY_TYPE_PERF_CONTROL);
}
