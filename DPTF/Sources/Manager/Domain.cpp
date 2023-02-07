/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "Domain.h"
#include "EsifDataString.h"
#include "EsifDataGuid.h"
#include "DptfStatusInterface.h"

Domain::Domain(DptfManagerInterface* dptfManager)
	: m_domainCreated(false)
	, m_dptfManager(dptfManager)
	, m_theRealParticipant(nullptr)
	, m_participantIndex(Constants::Invalid)
	, m_domainIndex(Constants::Invalid)
	, m_domainGuid(Guid())
	, m_domainName("")
	, m_domainType(DomainType::Invalid)
	, m_domainFunctionalityVersions(DomainFunctionalityVersions())
	, m_arbitrator(nullptr)
	, m_coreControlStaticCaps(nullptr)
	, m_coreControlDynamicCaps(nullptr)
	, m_coreControlLpoPreference(nullptr)
	, m_coreControlStatus(nullptr)
	, m_displayControlDynamicCaps(nullptr)
	, m_displayControlStatus(nullptr)
	, m_displayControlSet(nullptr)
	, m_performanceControlStaticCaps(nullptr)
	, m_performanceControlDynamicCaps(nullptr)
	, m_performanceControlStatus(nullptr)
	, m_performanceControlSet(nullptr)
	, m_powerControlDynamicCapsSet(nullptr)
	, m_isPowerShareControl(nullptr)
	, m_powerLimitEnabled(std::map<PowerControlType::Type, Bool>())
	, m_powerLimit(std::map<PowerControlType::Type, Power>())
	, m_powerLimitTimeWindow(std::map<PowerControlType::Type, TimeSpan>())
	, m_powerLimitDutyCycle(std::map<PowerControlType::Type, Percentage>())
	, m_powerStatus(nullptr)
	, m_systemPowerLimitEnabled(std::map<PsysPowerLimitType::Type, Bool>())
	, m_systemPowerLimit(std::map<PsysPowerLimitType::Type, Power>())
	, m_systemPowerLimitTimeWindow(std::map<PsysPowerLimitType::Type, TimeSpan>())
	, m_systemPowerLimitDutyCycle(std::map<PsysPowerLimitType::Type, Percentage>())
	, m_adapterRating(nullptr)
	, m_platformRestOfPower(nullptr)
	, m_platformPowerSource(nullptr)
	, m_acNominalVoltage(nullptr)
	, m_acOperationalCurrent(nullptr)
	, m_ac1msPercentageOverload(nullptr)
	, m_ac2msPercentageOverload(nullptr)
	, m_ac10msPercentageOverload(nullptr)
	, m_domainPriority(nullptr)
	, m_rfProfileCapabilities(nullptr)
	, m_rfProfileData(nullptr)
	, m_utilizationStatus(nullptr)
{
}

Domain::~Domain(void)
{
	destroyDomain();
}

void Domain::createDomain(
	UIntN participantIndex,
	UIntN domainIndex,
	ParticipantInterface* participantInterface,
	const AppDomainDataPtr domainDataPtr,
	Bool domainEnabled)
{
	if (m_domainCreated == true)
	{
		throw dptf_exception("Domain::createDomain() already executed.");
	}

	try
	{
		m_theRealParticipant = participantInterface;
		m_participantIndex = participantIndex;
		m_domainIndex = domainIndex;
		m_domainGuid = EsifDataGuid(&domainDataPtr->fGuid);
		m_domainName = EsifDataString(&domainDataPtr->fName);
		m_domainType = EsifDomainTypeToDptfDomainType(domainDataPtr->fType);
		m_domainFunctionalityVersions = DomainFunctionalityVersions(domainDataPtr->fCapabilityBytes);
		m_arbitrator = new Arbitrator();

		m_dptfManager->getDptfStatus()->clearCache();
		m_theRealParticipant->createDomain(
			m_domainGuid,
			m_participantIndex,
			m_domainIndex,
			domainEnabled,
			m_domainType,
			m_domainName,
			EsifDataString(&domainDataPtr->fDescription),
			m_domainFunctionalityVersions);

		m_domainCreated = true;
	}
	catch (...)
	{
		m_domainCreated = false;
	}
}

void Domain::destroyDomain(void)
{
	if (m_theRealParticipant != nullptr)
	{
		try
		{
			m_dptfManager->getDptfStatus()->clearCache();
			m_theRealParticipant->destroyDomain(m_domainGuid);
		}
		catch (...)
		{
		}

		m_theRealParticipant = nullptr;
	}

	clearDomainCachedData();

	DELETE_MEMORY_TC(m_arbitrator);
}

void Domain::enableDomain(void)
{
	m_theRealParticipant->enableDomain(m_domainIndex);
}

void Domain::disableDomain(void)
{
	m_theRealParticipant->disableDomain(m_domainIndex);
}

Bool Domain::isDomainEnabled(void)
{
	return m_theRealParticipant->isDomainEnabled(m_domainIndex);
}

Bool Domain::isCreated(void)
{
	return m_domainCreated;
}

std::string Domain::getDomainName(void) const
{
	return m_domainName;
}

void Domain::clearDomainCachedData(void)
{
	m_dptfManager->getDptfStatus()->clearCache();
	clearDomainCachedDataCoreControl();
	clearDomainCachedDataDisplayControl();
	clearDomainCachedDataPerformanceControl();
	clearDomainCachedDataPowerControl();
	clearDomainCachedDataPowerStatus();
	clearDomainCachedDataSystemPowerControl();
	clearDomainCachedDataPriority();
	clearDomainCachedDataRfProfileControl();
	clearDomainCachedDataRfProfileStatus();
	clearDomainCachedDataUtilizationStatus();
	clearDomainCachedDataPlatformPowerStatus();
	clearDomainCachedRequestData();
}

void Domain::clearDomainCachedRequestData(void)
{
	DptfRequest clearCachedDataRequest(DptfRequestType::ClearCachedData, m_participantIndex, m_domainIndex);
	PolicyRequest policyRequest(Constants::Invalid, clearCachedDataRequest);
	m_dptfManager->getRequestDispatcher()->dispatchForAllControls(policyRequest);
}

void Domain::clearArbitrationDataForPolicy(UIntN policyIndex)
{
	DptfRequest clearControlRequests(
		DptfRequestType::ClearPolicyRequestsForAllControls, m_participantIndex, m_domainIndex);
	PolicyRequest policyRequest(policyIndex, clearControlRequests);
	m_dptfManager->getRequestDispatcher()->dispatchForAllControls(policyRequest);

	m_arbitrator->clearPolicyCachedData(policyIndex);
}

std::shared_ptr<XmlNode> Domain::getArbitrationXmlForPolicy(UIntN policyIndex, ControlFactoryType::Type type) const
{
	auto domainRoot = XmlNode::createWrapperElement("arbitrator_domain_status");
	domainRoot->addChild(XmlNode::createDataElement("domain_name", getDomainName()));
	domainRoot->addChild(m_arbitrator->getArbitrationXmlForPolicy(
		policyIndex, type)); // todo: remove this when all arbitrators are moved to the control
	domainRoot->addChild(m_theRealParticipant->getArbitratorStatusForPolicy(m_domainIndex, policyIndex, type));
	return domainRoot;
}

std::shared_ptr<XmlNode> Domain::getDiagnosticsAsXml() const
{
	return m_theRealParticipant->getDiagnosticsAsXml(m_domainIndex);
}

//
// The following macro (FILL_CACHE_AND_RETURN) is in place to remove this code many times:
//
// if (m_activeControlStaticCaps == nullptr)
//{
//    ActiveControlStaticCaps var = m_theRealParticipant->getActiveControlStaticCaps(m_participantIndex, m_domainIndex);
//    m_activeControlStaticCaps = new ActiveControlStaticCaps(var);
//}
//
// return *m_activeControlStaticCaps;

#define FILL_CACHE_AND_RETURN(mv, ct, fn)                                                                              \
	if (mv == nullptr)                                                                                                 \
	{                                                                                                                  \
		ct var = m_theRealParticipant->fn(m_participantIndex, m_domainIndex);                                          \
		mv = new ct(var);                                                                                              \
	}                                                                                                                  \
	return *mv;

Percentage Domain::getUtilizationThreshold()
{
	return m_theRealParticipant->getUtilizationThreshold(m_participantIndex, m_domainIndex);
}

Percentage Domain::getResidencyUtilization()
{
	return m_theRealParticipant->getResidencyUtilization(m_participantIndex, m_domainIndex);
}

UInt64 Domain::getCoreActivityCounter()
{
	return m_theRealParticipant->getCoreActivityCounter(m_participantIndex, m_domainIndex);
}

UInt32 Domain::getCoreActivityCounterWidth()
{
	return m_theRealParticipant->getCoreActivityCounterWidth(m_participantIndex, m_domainIndex);
}

UInt64 Domain::getTimestampCounter()
{
	return m_theRealParticipant->getTimestampCounter(m_participantIndex, m_domainIndex);
}

UInt32 Domain::getTimestampCounterWidth()
{
	return m_theRealParticipant->getTimestampCounterWidth(m_participantIndex, m_domainIndex);
}

CoreActivityInfo Domain::getCoreActivityInfo()
{
	return m_theRealParticipant->getCoreActivityInfo(m_participantIndex, m_domainIndex);
}

void Domain::setPowerShareEffectiveBias(UInt32 powerShareEffectiveBias)
{
	m_theRealParticipant->setPowerShareEffectiveBias(m_participantIndex, m_domainIndex, powerShareEffectiveBias);
}

CoreControlStaticCaps Domain::getCoreControlStaticCaps(void)
{
	FILL_CACHE_AND_RETURN(m_coreControlStaticCaps, CoreControlStaticCaps, getCoreControlStaticCaps);
}

CoreControlDynamicCaps Domain::getCoreControlDynamicCaps(void)
{
	FILL_CACHE_AND_RETURN(m_coreControlDynamicCaps, CoreControlDynamicCaps, getCoreControlDynamicCaps);
}

CoreControlLpoPreference Domain::getCoreControlLpoPreference(void)
{
	FILL_CACHE_AND_RETURN(m_coreControlLpoPreference, CoreControlLpoPreference, getCoreControlLpoPreference);
}

CoreControlStatus Domain::getCoreControlStatus(void)
{
	FILL_CACHE_AND_RETURN(m_coreControlStatus, CoreControlStatus, getCoreControlStatus);
}

void Domain::setActiveCoreControl(UIntN policyIndex, const CoreControlStatus& coreControlStatus)
{
	CoreControlArbitrator* coreControlArbitrator = m_arbitrator->getCoreControlArbitrator();
	Bool shouldSetControlStatus = false;
	CoreControlStatus newControlStatus(Constants::Invalid);

	if (coreControlArbitrator->hasArbitratedCoreCount())
	{
		auto currentControlStatus = coreControlArbitrator->getArbitratedCoreControlStatus();
		newControlStatus = coreControlArbitrator->arbitrate(policyIndex, coreControlStatus);
		if (currentControlStatus != newControlStatus)
		{
			shouldSetControlStatus = true;
		}
	}
	else
	{
		shouldSetControlStatus = true;
		newControlStatus = coreControlArbitrator->arbitrate(policyIndex, coreControlStatus);
	}

	if (shouldSetControlStatus)
	{
		m_theRealParticipant->setActiveCoreControl(m_participantIndex, m_domainIndex, newControlStatus);
		clearDomainCachedDataCoreControl();
	}
	coreControlArbitrator->commitPolicyRequest(policyIndex, coreControlStatus);
}

DisplayControlDynamicCaps Domain::getDisplayControlDynamicCaps(void)
{
	FILL_CACHE_AND_RETURN(m_displayControlDynamicCaps, DisplayControlDynamicCaps, getDisplayControlDynamicCaps);
}

UIntN Domain::getUserPreferredDisplayIndex(void)
{
	return m_theRealParticipant->getUserPreferredDisplayIndex(m_participantIndex, m_domainIndex);
}

UIntN Domain::getUserPreferredSoftBrightnessIndex(void)
{
	return m_theRealParticipant->getUserPreferredSoftBrightnessIndex(m_participantIndex, m_domainIndex);
}

Bool Domain::isUserPreferredIndexModified()
{
	return m_theRealParticipant->isUserPreferredIndexModified(m_participantIndex, m_domainIndex);
}

DisplayControlStatus Domain::getDisplayControlStatus(void)
{
	FILL_CACHE_AND_RETURN(m_displayControlStatus, DisplayControlStatus, getDisplayControlStatus);
}

UIntN Domain::getSoftBrightnessIndex(void)
{
	return m_theRealParticipant->getSoftBrightnessIndex(m_participantIndex, m_domainIndex);
}

DisplayControlSet Domain::getDisplayControlSet(void)
{
	FILL_CACHE_AND_RETURN(m_displayControlSet, DisplayControlSet, getDisplayControlSet);
}

void Domain::setDisplayControl(UIntN policyIndex, UIntN displayControlIndex)
{
	DisplayControlArbitrator* displayControlArbitrator = m_arbitrator->getDisplayControlArbitrator();
	UIntN arbitratedDisplayControlIndex = displayControlArbitrator->arbitrate(policyIndex, displayControlIndex);
	// always set even if arbitrated value has not changed
	m_theRealParticipant->setDisplayControl(m_participantIndex, m_domainIndex, arbitratedDisplayControlIndex);
	clearDomainCachedDataDisplayControl();
	displayControlArbitrator->commitPolicyRequest(policyIndex, displayControlIndex);
}

void Domain::setSoftBrightness(UIntN policyIndex, UIntN displayControlIndex)
{
	m_theRealParticipant->setSoftBrightness(m_participantIndex, m_domainIndex, displayControlIndex);
}

void Domain::updateUserPreferredSoftBrightnessIndex()
{
	m_theRealParticipant->updateUserPreferredSoftBrightnessIndex(m_participantIndex, m_domainIndex);
}

void Domain::restoreUserPreferredSoftBrightness()
{
	m_theRealParticipant->restoreUserPreferredSoftBrightness(m_participantIndex, m_domainIndex);
}

void Domain::setDisplayControlDynamicCaps(UIntN policyIndex, DisplayControlDynamicCaps newCapabilities)
{
	DisplayControlCapabilitiesArbitrator* arbitrator = m_arbitrator->getDisplayControlCapabilitiesArbitrator();
	Bool shouldSetDisplayCapabilities = false;
	DisplayControlDynamicCaps newCaps(Constants::Invalid, Constants::Invalid);

	if (arbitrator->hasArbitratedDisplayCapabilities())
	{
		auto currentCaps = arbitrator->getArbitratedDisplayControlCapabilities();
		newCaps = arbitrator->arbitrate(policyIndex, newCapabilities);
		if (currentCaps != newCaps)
		{
			shouldSetDisplayCapabilities = true;
		}
	}
	else
	{
		shouldSetDisplayCapabilities = true;
		newCaps = arbitrator->arbitrate(policyIndex, newCapabilities);
	}

	if (shouldSetDisplayCapabilities)
	{
		m_theRealParticipant->setDisplayControlDynamicCaps(m_participantIndex, m_domainIndex, newCaps);
		clearDomainCachedDataDisplayControl();
	}
	arbitrator->commitPolicyRequest(policyIndex, newCapabilities);
}

void Domain::setDisplayCapsLock(UIntN policyIndex, Bool lock)
{
	DisplayControlCapabilitiesArbitrator* arbitrator = m_arbitrator->getDisplayControlCapabilitiesArbitrator();
	Bool updated = arbitrator->arbitrateLockRequests(policyIndex, lock);
	if (updated == true)
	{
		m_theRealParticipant->setDisplayCapsLock(m_participantIndex, m_domainIndex, arbitrator->getArbitratedLock());
	}
}

UInt32 Domain::getRaplEnergyCounter()
{
	return m_theRealParticipant->getRaplEnergyCounter(m_participantIndex, m_domainIndex);
}

EnergyCounterInfo Domain::getRaplEnergyCounterInfo()
{
	return m_theRealParticipant->getRaplEnergyCounterInfo(m_participantIndex, m_domainIndex);
}

double Domain::getRaplEnergyUnit()
{
	return m_theRealParticipant->getRaplEnergyUnit(m_participantIndex, m_domainIndex);
}

UInt32 Domain::getRaplEnergyCounterWidth()
{
	return m_theRealParticipant->getRaplEnergyCounterWidth(m_participantIndex, m_domainIndex);
}

Power Domain::getInstantaneousPower()
{
	return m_theRealParticipant->getInstantaneousPower(m_participantIndex, m_domainIndex);
}

UInt32 Domain::getEnergyThreshold()
{
	return m_theRealParticipant->getEnergyThreshold(m_participantIndex, m_domainIndex);
}

void Domain::setEnergyThreshold(UInt32 energyThreshold)
{
	m_theRealParticipant->setEnergyThreshold(m_participantIndex, m_domainIndex, energyThreshold);
}

void Domain::setEnergyThresholdInterruptDisable()
{
	m_theRealParticipant->setEnergyThresholdInterruptDisable(m_participantIndex, m_domainIndex);
}

Power Domain::getACPeakPower(void)
{
	return m_theRealParticipant->getACPeakPower(m_participantIndex, m_domainIndex);
}

void Domain::setACPeakPower(UIntN policyIndex, const Power& acPeakPower)
{
	PeakPowerControlArbitrator* peakPowerControlArbitrator = m_arbitrator->getPeakPowerControlArbitrator();
	Bool shouldSetPeakPower = false;
	Power newACPeakPower;

	if (peakPowerControlArbitrator->hasArbitratedPeakPower(PeakPowerType::PL4ACPower))
	{
		auto currentACPeakPower = peakPowerControlArbitrator->getArbitratedPeakPower(PeakPowerType::PL4ACPower);
		newACPeakPower = peakPowerControlArbitrator->arbitrate(policyIndex, PeakPowerType::PL4ACPower, acPeakPower);
		if (currentACPeakPower != newACPeakPower)
		{
			shouldSetPeakPower = true;
		}
	}
	else
	{
		shouldSetPeakPower = true;
		newACPeakPower = peakPowerControlArbitrator->arbitrate(policyIndex, PeakPowerType::PL4ACPower, acPeakPower);
	}

	if (shouldSetPeakPower)
	{
		m_theRealParticipant->setACPeakPower(m_participantIndex, m_domainIndex, newACPeakPower);
	}
	peakPowerControlArbitrator->commitPolicyRequest(policyIndex, PeakPowerType::PL4ACPower, acPeakPower);
}

Power Domain::getDCPeakPower(void)
{
	return m_theRealParticipant->getDCPeakPower(m_participantIndex, m_domainIndex);
}

void Domain::setDCPeakPower(UIntN policyIndex, const Power& dcPeakPower)
{
	PeakPowerControlArbitrator* peakPowerControlArbitrator = m_arbitrator->getPeakPowerControlArbitrator();
	Bool shouldSetPeakPower = false;
	Power newDCPeakPower;

	if (peakPowerControlArbitrator->hasArbitratedPeakPower(PeakPowerType::PL4DCPower))
	{
		auto currentDCPeakPower = peakPowerControlArbitrator->getArbitratedPeakPower(PeakPowerType::PL4DCPower);
		newDCPeakPower = peakPowerControlArbitrator->arbitrate(policyIndex, PeakPowerType::PL4DCPower, dcPeakPower);
		if (currentDCPeakPower != newDCPeakPower)
		{
			shouldSetPeakPower = true;
		}
	}
	else
	{
		shouldSetPeakPower = true;
		newDCPeakPower = peakPowerControlArbitrator->arbitrate(policyIndex, PeakPowerType::PL4DCPower, dcPeakPower);
	}

	if (shouldSetPeakPower)
	{
		m_theRealParticipant->setDCPeakPower(m_participantIndex, m_domainIndex, newDCPeakPower);
	}
	peakPowerControlArbitrator->commitPolicyRequest(policyIndex, PeakPowerType::PL4DCPower, dcPeakPower);
}

PerformanceControlStaticCaps Domain::getPerformanceControlStaticCaps(void)
{
	FILL_CACHE_AND_RETURN(
		m_performanceControlStaticCaps, PerformanceControlStaticCaps, getPerformanceControlStaticCaps);
}

PerformanceControlDynamicCaps Domain::getPerformanceControlDynamicCaps(void)
{
	FILL_CACHE_AND_RETURN(
		m_performanceControlDynamicCaps, PerformanceControlDynamicCaps, getPerformanceControlDynamicCaps);
}

PerformanceControlStatus Domain::getPerformanceControlStatus(void)
{
	FILL_CACHE_AND_RETURN(m_performanceControlStatus, PerformanceControlStatus, getPerformanceControlStatus);
}

PerformanceControlSet Domain::getPerformanceControlSet(void)
{
	FILL_CACHE_AND_RETURN(m_performanceControlSet, PerformanceControlSet, getPerformanceControlSet);
}

void Domain::setPerformanceControl(UIntN policyIndex, UIntN performanceControlIndex)
{
	PerformanceControlArbitrator* performanceControlArbitrator = m_arbitrator->getPerformanceControlArbitrator();
	Bool shouldSetPerformanceControlIndex = false;
	UIntN newIndex;

	if (performanceControlArbitrator->hasArbitratedPerformanceControlIndex())
	{
		auto currentIndex = performanceControlArbitrator->getArbitratedPerformanceControlIndex();
		newIndex = performanceControlArbitrator->arbitrate(policyIndex, performanceControlIndex);
		if (currentIndex != newIndex)
		{
			shouldSetPerformanceControlIndex = true;
		}
	}
	else
	{
		shouldSetPerformanceControlIndex = true;
		newIndex = performanceControlArbitrator->arbitrate(policyIndex, performanceControlIndex);
	}

	if (shouldSetPerformanceControlIndex)
	{
		m_theRealParticipant->setPerformanceControl(m_participantIndex, m_domainIndex, newIndex);
		clearDomainCachedDataPerformanceControl();
	}
	performanceControlArbitrator->commitPolicyRequest(policyIndex, performanceControlIndex);
}

void Domain::setPerformanceControlDynamicCaps(UIntN policyIndex, PerformanceControlDynamicCaps newCapabilities)
{
	PerformanceControlCapabilitiesArbitrator* arbitrator = m_arbitrator->getPerformanceControlCapabilitiesArbitrator();
	Bool shouldSetPerformanceCapabilities = false;
	auto currentCaps = getPerformanceControlDynamicCaps();
	PerformanceControlDynamicCaps newCaps(Constants::Invalid, Constants::Invalid);

	if (arbitrator->hasArbitratedPerformanceControlCapabilities())
	{
		auto oldCaps = arbitrator->getArbitratedPerformanceControlCapabilities(currentCaps);
		newCaps = arbitrator->arbitrate(policyIndex, newCapabilities, currentCaps);
		if (oldCaps != newCaps)
		{
			shouldSetPerformanceCapabilities = true;
		}
	}
	else
	{
		shouldSetPerformanceCapabilities = true;
		newCaps = arbitrator->arbitrate(policyIndex, newCapabilities, currentCaps);
	}

	if (shouldSetPerformanceCapabilities)
	{
		m_theRealParticipant->setPerformanceControlDynamicCaps(m_participantIndex, m_domainIndex, newCaps);
		clearDomainCachedDataPerformanceControl();
	}
	arbitrator->commitPolicyRequest(policyIndex, newCapabilities);
}

void Domain::setPerformanceCapsLock(UIntN policyIndex, Bool lock)
{
	PerformanceControlCapabilitiesArbitrator* arbitrator = m_arbitrator->getPerformanceControlCapabilitiesArbitrator();
	Bool updated = arbitrator->arbitrateLockRequests(policyIndex, lock);
	if (updated == true)
	{
		m_theRealParticipant->setPerformanceCapsLock(
			m_participantIndex, m_domainIndex, arbitrator->getArbitratedLock());
	}
}

PowerControlDynamicCapsSet Domain::getPowerControlDynamicCapsSet(void)
{
	FILL_CACHE_AND_RETURN(m_powerControlDynamicCapsSet, PowerControlDynamicCapsSet, getPowerControlDynamicCapsSet);
}

void Domain::setPowerControlDynamicCapsSet(UIntN policyIndex, PowerControlDynamicCapsSet capsSet)
{
	PowerControlCapabilitiesArbitrator* arbitrator = m_arbitrator->getPowerControlCapabilitiesArbitrator();
	Bool shouldSetPowerControlCapabilities = false;
	auto currentCaps = getPowerControlDynamicCapsSet();
	PowerControlDynamicCapsSet newCaps;

	if (arbitrator->hasArbitratedPowerControlCapabilities())
	{
		auto oldCaps = arbitrator->getArbitratedPowerControlCapabilities(currentCaps);
		newCaps = arbitrator->arbitrate(policyIndex, capsSet, currentCaps);
		if (oldCaps != newCaps || currentCaps != newCaps)
		{
			shouldSetPowerControlCapabilities = true;
		}
	}
	else
	{
		shouldSetPowerControlCapabilities = true;
		newCaps = arbitrator->arbitrate(policyIndex, capsSet, currentCaps);
	}

	if (shouldSetPowerControlCapabilities)
	{
		m_theRealParticipant->setPowerControlDynamicCapsSet(m_participantIndex, m_domainIndex, newCaps);
		clearDomainCachedDataPowerControl();
	}
	arbitrator->commitPolicyRequest(policyIndex, capsSet);
}

Bool Domain::isPowerLimitEnabled(PowerControlType::Type controlType)
{
	auto enabled = m_powerLimitEnabled.find(controlType);
	if (enabled == m_powerLimitEnabled.end())
	{
		m_powerLimitEnabled[controlType] =
			m_theRealParticipant->isPowerLimitEnabled(m_participantIndex, m_domainIndex, controlType);
	}
	return m_powerLimitEnabled.at(controlType);
}

Power Domain::getPowerLimit(PowerControlType::Type controlType)
{
	auto limit = m_powerLimit.find(controlType);
	if (limit == m_powerLimit.end())
	{
		m_powerLimit[controlType] = m_theRealParticipant->getPowerLimit(m_participantIndex, m_domainIndex, controlType);
	}
	return m_powerLimit.at(controlType);
}

Power Domain::getPowerLimitWithoutCache(PowerControlType::Type controlType)
{
	return m_theRealParticipant->getPowerLimitWithoutCache(m_participantIndex, m_domainIndex, controlType);
}

Bool Domain::isSocPowerFloorEnabled()
{
	return m_theRealParticipant->isSocPowerFloorEnabled(m_participantIndex, m_domainIndex);
}

Bool Domain::isSocPowerFloorSupported()
{
	return m_theRealParticipant->isSocPowerFloorSupported(m_participantIndex, m_domainIndex);
}

void Domain::setPowerLimit(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit)
{
	PowerControlArbitrator* powerControlArbitrator = m_arbitrator->getPowerControlArbitrator();
	Bool shouldSetPowerLimit = false;
	Power newPowerLimit;

	if (powerControlArbitrator->hasArbitratedPowerLimit(controlType))
	{
		auto currentPowerLimit = powerControlArbitrator->getArbitratedPowerLimit(controlType);
		newPowerLimit = powerControlArbitrator->arbitrate(policyIndex, controlType, powerLimit);
		if (currentPowerLimit != newPowerLimit)
		{
			shouldSetPowerLimit = true;
		}
	}
	else
	{
		shouldSetPowerLimit = true;
		newPowerLimit = powerControlArbitrator->arbitrate(policyIndex, controlType, powerLimit);
	}

	if (shouldSetPowerLimit)
	{
		m_theRealParticipant->setPowerLimit(m_participantIndex, m_domainIndex, controlType, newPowerLimit);
		clearDomainCachedDataPowerControl();
	}
	powerControlArbitrator->commitPolicyRequest(policyIndex, controlType, powerLimit);
}

void Domain::setPowerLimitWithoutUpdatingEnabled(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit)
{
	PowerControlArbitrator* powerControlArbitrator = m_arbitrator->getPowerControlArbitrator();
	Bool shouldSetPowerLimit = false;
	Power newPowerLimit;

	if (powerControlArbitrator->hasArbitratedPowerLimit(controlType))
	{
		auto currentPowerLimit = powerControlArbitrator->getArbitratedPowerLimit(controlType);
		newPowerLimit = powerControlArbitrator->arbitrate(policyIndex, controlType, powerLimit);
		if (currentPowerLimit != newPowerLimit)
		{
			shouldSetPowerLimit = true;
		}
	}
	else
	{
		shouldSetPowerLimit = true;
		newPowerLimit = powerControlArbitrator->arbitrate(policyIndex, controlType, powerLimit);
	}

	if (shouldSetPowerLimit)
	{
		m_theRealParticipant->setPowerLimitWithoutUpdatingEnabled(
			m_participantIndex, m_domainIndex, controlType, newPowerLimit);
		clearDomainCachedDataPowerControl();
	}
	powerControlArbitrator->commitPolicyRequest(policyIndex, controlType, powerLimit);
}

void Domain::setPowerLimitIgnoringCaps(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit)
{
	m_theRealParticipant->setPowerLimitIgnoringCaps(m_participantIndex, m_domainIndex, controlType, powerLimit);
}

TimeSpan Domain::getPowerLimitTimeWindow(PowerControlType::Type controlType)
{
	auto limit = m_powerLimitTimeWindow.find(controlType);
	if (limit == m_powerLimitTimeWindow.end())
	{
		m_powerLimitTimeWindow[controlType] =
			m_theRealParticipant->getPowerLimitTimeWindow(m_participantIndex, m_domainIndex, controlType);
	}
	return m_powerLimitTimeWindow.at(controlType);
}

void Domain::setPowerLimitTimeWindow(UIntN policyIndex, PowerControlType::Type controlType, const TimeSpan& timeWindow)
{
	PowerControlArbitrator* powerControlArbitrator = m_arbitrator->getPowerControlArbitrator();
	Bool shouldSetTimeWindow = false;
	TimeSpan newTimeWindow;

	if (powerControlArbitrator->hasArbitratedTimeWindow(controlType))
	{
		auto currentTimeWindow = powerControlArbitrator->getArbitratedTimeWindow(controlType);
		newTimeWindow = powerControlArbitrator->arbitrate(policyIndex, controlType, timeWindow);
		if (currentTimeWindow != newTimeWindow)
		{
			shouldSetTimeWindow = true;
		}
	}
	else
	{
		shouldSetTimeWindow = true;
		newTimeWindow = powerControlArbitrator->arbitrate(policyIndex, controlType, timeWindow);
	}

	if (shouldSetTimeWindow)
	{
		m_theRealParticipant->setPowerLimitTimeWindow(m_participantIndex, m_domainIndex, controlType, newTimeWindow);
		clearDomainCachedDataPowerControl();
	}
	powerControlArbitrator->commitPolicyRequest(policyIndex, controlType, timeWindow);
}

void Domain::setPowerLimitTimeWindowWithoutUpdatingEnabled(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	PowerControlArbitrator* powerControlArbitrator = m_arbitrator->getPowerControlArbitrator();
	Bool shouldSetTimeWindow = false;
	TimeSpan newTimeWindow;

	if (powerControlArbitrator->hasArbitratedTimeWindow(controlType))
	{
		auto currentTimeWindow = powerControlArbitrator->getArbitratedTimeWindow(controlType);
		newTimeWindow = powerControlArbitrator->arbitrate(policyIndex, controlType, timeWindow);
		if (currentTimeWindow != newTimeWindow)
		{
			shouldSetTimeWindow = true;
		}
	}
	else
	{
		shouldSetTimeWindow = true;
		newTimeWindow = powerControlArbitrator->arbitrate(policyIndex, controlType, timeWindow);
	}

	if (shouldSetTimeWindow)
	{
		m_theRealParticipant->setPowerLimitTimeWindowWithoutUpdatingEnabled(m_participantIndex, m_domainIndex, controlType, newTimeWindow);
		clearDomainCachedDataPowerControl();
	}
	powerControlArbitrator->commitPolicyRequest(policyIndex, controlType, timeWindow);
}

void Domain::setPowerLimitTimeWindowIgnoringCaps(
	UIntN policyIndex,
	PowerControlType::Type controlType,
	const TimeSpan& timeWindow)
{
	m_theRealParticipant->setPowerLimitTimeWindowIgnoringCaps(
		m_participantIndex, m_domainIndex, controlType, timeWindow);
}

Percentage Domain::getPowerLimitDutyCycle(PowerControlType::Type controlType)
{
	auto limit = m_powerLimitDutyCycle.find(controlType);
	if (limit == m_powerLimitDutyCycle.end())
	{
		m_powerLimitDutyCycle[controlType] =
			m_theRealParticipant->getPowerLimitDutyCycle(m_participantIndex, m_domainIndex, controlType);
	}
	return m_powerLimitDutyCycle.at(controlType);
}

void Domain::setPowerLimitDutyCycle(UIntN policyIndex, PowerControlType::Type controlType, const Percentage& dutyCycle)
{
	PowerControlArbitrator* powerControlArbitrator = m_arbitrator->getPowerControlArbitrator();
	Bool shouldSetDutyCycle = false;
	Percentage newDutyCycle;

	if (powerControlArbitrator->hasArbitratedDutyCycle(controlType))
	{
		auto currentDutyCycle = powerControlArbitrator->getArbitratedDutyCycle(controlType);
		newDutyCycle = powerControlArbitrator->arbitrate(policyIndex, controlType, dutyCycle);
		if (currentDutyCycle != newDutyCycle)
		{
			shouldSetDutyCycle = true;
		}
	}
	else
	{
		shouldSetDutyCycle = true;
		newDutyCycle = powerControlArbitrator->arbitrate(policyIndex, controlType, dutyCycle);
	}

	if (shouldSetDutyCycle)
	{
		m_theRealParticipant->setPowerLimitDutyCycle(m_participantIndex, m_domainIndex, controlType, newDutyCycle);
		clearDomainCachedDataPowerControl();
	}
	powerControlArbitrator->commitPolicyRequest(policyIndex, controlType, dutyCycle);
}

void Domain::setSocPowerFloorState(UIntN policyIndex, Bool socPowerFloorState)
{
	m_theRealParticipant->setSocPowerFloorState(m_participantIndex, m_domainIndex, socPowerFloorState);
}

void Domain::setPowerCapsLock(UIntN policyIndex, Bool lock)
{
	PowerControlCapabilitiesArbitrator* arbitrator = m_arbitrator->getPowerControlCapabilitiesArbitrator();
	Bool updated = arbitrator->arbitrateLockRequests(policyIndex, lock);
	if (updated == true)
	{
		m_theRealParticipant->setPowerCapsLock(m_participantIndex, m_domainIndex, arbitrator->getArbitratedLock());
	}
}

TimeSpan Domain::getPowerSharePowerLimitTimeWindow()
{
	return m_theRealParticipant->getPowerSharePowerLimitTimeWindow(m_participantIndex, m_domainIndex);
}

Bool Domain::isPowerShareControl()
{
	FILL_CACHE_AND_RETURN(m_isPowerShareControl, Bool, isPowerShareControl);
}

double Domain::getPidKpTerm()
{
	return m_theRealParticipant->getPidKpTerm(m_participantIndex, m_domainIndex);
}

double Domain::getPidKiTerm()
{
	return m_theRealParticipant->getPidKiTerm(m_participantIndex, m_domainIndex);
}

TimeSpan Domain::getAlpha()
{
	return m_theRealParticipant->getAlpha(m_participantIndex, m_domainIndex);
}

TimeSpan Domain::getFastPollTime()
{
	return m_theRealParticipant->getFastPollTime(m_participantIndex, m_domainIndex);
}

TimeSpan Domain::getSlowPollTime()
{
	return m_theRealParticipant->getSlowPollTime(m_participantIndex, m_domainIndex);
}

TimeSpan Domain::getWeightedSlowPollAvgConstant()
{
	return m_theRealParticipant->getWeightedSlowPollAvgConstant(m_participantIndex, m_domainIndex);
}

Power Domain::getSlowPollPowerThreshold()
{
	return m_theRealParticipant->getSlowPollPowerThreshold(m_participantIndex, m_domainIndex);
}

Power Domain::getArbitratedPowerLimit(PowerControlType::Type controlType)
{
	try
	{
		auto arbitrator = m_arbitrator->getPowerControlArbitrator();
		return arbitrator->getArbitratedPowerLimit(controlType);
	}
	catch (const std::exception&)
	{
		// if there are no policy requests then the arbitrator will throw an exception.
		// in this case we need to return the max limit
		auto caps = m_theRealParticipant->getPowerControlDynamicCapsSet(m_participantIndex, m_domainIndex);
		if (caps.hasCapability(controlType))
		{
			auto cap = caps.getCapability(controlType);
			return cap.getMaxPowerLimit();
		}
		else
		{
			return Power::createInvalid();
		}
	}
}

void Domain::removePowerLimitPolicyRequest(UIntN policyIndex, PowerControlType::Type controlType)
{
	auto arbitrator = m_arbitrator->getPowerControlArbitrator();
	auto currLimit = getArbitratedPowerLimit(controlType);
	arbitrator->removePowerLimitRequestForPolicy(policyIndex, controlType);
	auto newLimit = getArbitratedPowerLimit(controlType);
	if (currLimit != newLimit)
	{
		m_theRealParticipant->setPowerLimit(m_participantIndex, m_domainIndex, controlType, newLimit);
		clearDomainCachedDataPowerControl();
	}
}

PowerStatus Domain::getPowerStatus(void)
{
	FILL_CACHE_AND_RETURN(m_powerStatus, PowerStatus, getPowerStatus);
}

Power Domain::getAveragePower(const PowerControlDynamicCaps& capabilities)
{
	return m_theRealParticipant->getAveragePower(m_participantIndex, m_domainIndex, capabilities);
}

Power Domain::getPowerValue(void)
{
	return m_theRealParticipant->getPowerValue(m_participantIndex, m_domainIndex);
}

void Domain::setCalculatedAveragePower(Power powerValue)
{
	m_theRealParticipant->setCalculatedAveragePower(m_participantIndex, m_domainIndex, powerValue);
}

Bool Domain::isSystemPowerLimitEnabled(PsysPowerLimitType::Type limitType)
{
	auto enabled = m_systemPowerLimitEnabled.find(limitType);
	if (enabled == m_systemPowerLimitEnabled.end())
	{
		m_systemPowerLimitEnabled[limitType] =
			m_theRealParticipant->isSystemPowerLimitEnabled(m_participantIndex, m_domainIndex, limitType);
	}
	return m_systemPowerLimitEnabled.at(limitType);
}

Power Domain::getSystemPowerLimit(PsysPowerLimitType::Type limitType)
{
	auto limit = m_systemPowerLimit.find(limitType);
	if (limit == m_systemPowerLimit.end())
	{
		m_systemPowerLimit[limitType] =
			m_theRealParticipant->getSystemPowerLimit(m_participantIndex, m_domainIndex, limitType);
	}
	return m_systemPowerLimit.at(limitType);
}

void Domain::setSystemPowerLimit(UIntN policyIndex, PsysPowerLimitType::Type limitType, const Power& powerLimit)
{
	SystemPowerControlArbitrator* systemPowerControlArbitrator = m_arbitrator->getSystemPowerControlArbitrator();
	Bool shouldSetPowerLimit = false;
	Power newSystemPowerLimit;

	if (systemPowerControlArbitrator->hasArbitratedSystemPowerLimit(limitType))
	{
		auto currentSystemPowerLimit = systemPowerControlArbitrator->getArbitratedSystemPowerLimit(limitType);
		newSystemPowerLimit = systemPowerControlArbitrator->arbitrate(policyIndex, limitType, powerLimit);
		if (currentSystemPowerLimit != newSystemPowerLimit)
		{
			shouldSetPowerLimit = true;
		}
	}
	else
	{
		shouldSetPowerLimit = true;
		newSystemPowerLimit = systemPowerControlArbitrator->arbitrate(policyIndex, limitType, powerLimit);
	}

	if (shouldSetPowerLimit)
	{
		m_theRealParticipant->setSystemPowerLimit(m_participantIndex, m_domainIndex, limitType, newSystemPowerLimit);
		clearDomainCachedDataSystemPowerControl();
	}
	systemPowerControlArbitrator->commitPolicyRequest(policyIndex, limitType, powerLimit);
}

TimeSpan Domain::getSystemPowerLimitTimeWindow(PsysPowerLimitType::Type limitType)
{
	auto timeWindow = m_systemPowerLimitTimeWindow.find(limitType);
	if (timeWindow == m_systemPowerLimitTimeWindow.end())
	{
		m_systemPowerLimitTimeWindow[limitType] =
			m_theRealParticipant->getSystemPowerLimitTimeWindow(m_participantIndex, m_domainIndex, limitType);
	}
	return m_systemPowerLimitTimeWindow.at(limitType);
}

void Domain::setSystemPowerLimitTimeWindow(
	UIntN policyIndex,
	PsysPowerLimitType::Type limitType,
	const TimeSpan& timeWindow)
{
	SystemPowerControlArbitrator* SystemPowerControlArbitrator = m_arbitrator->getSystemPowerControlArbitrator();
	Bool shouldSetTimeWindow = false;
	TimeSpan newTimeWindow;

	if (SystemPowerControlArbitrator->hasArbitratedTimeWindow(limitType))
	{
		auto currentTimeWindow = SystemPowerControlArbitrator->getArbitratedTimeWindow(limitType);
		newTimeWindow = SystemPowerControlArbitrator->arbitrate(policyIndex, limitType, timeWindow);
		if (currentTimeWindow != newTimeWindow)
		{
			shouldSetTimeWindow = true;
		}
	}
	else
	{
		shouldSetTimeWindow = true;
		newTimeWindow = SystemPowerControlArbitrator->arbitrate(policyIndex, limitType, timeWindow);
	}

	if (shouldSetTimeWindow)
	{
		m_theRealParticipant->setSystemPowerLimitTimeWindow(
			m_participantIndex, m_domainIndex, limitType, newTimeWindow);
		clearDomainCachedDataSystemPowerControl();
	}
	SystemPowerControlArbitrator->commitPolicyRequest(policyIndex, limitType, timeWindow);
}

Percentage Domain::getSystemPowerLimitDutyCycle(PsysPowerLimitType::Type limitType)
{
	auto dutyCycle = m_systemPowerLimitDutyCycle.find(limitType);
	if (dutyCycle == m_systemPowerLimitDutyCycle.end())
	{
		m_systemPowerLimitDutyCycle[limitType] =
			m_theRealParticipant->getSystemPowerLimitDutyCycle(m_participantIndex, m_domainIndex, limitType);
	}
	return m_systemPowerLimitDutyCycle.at(limitType);
}

void Domain::setSystemPowerLimitDutyCycle(
	UIntN policyIndex,
	PsysPowerLimitType::Type limitType,
	const Percentage& dutyCycle)
{
	SystemPowerControlArbitrator* systemPowerControlArbitrator = m_arbitrator->getSystemPowerControlArbitrator();
	Bool shouldSetDutyCycle = false;
	Percentage newDutyCycle;

	if (systemPowerControlArbitrator->hasArbitratedDutyCycle(limitType))
	{
		auto currentDutyCycle = systemPowerControlArbitrator->getArbitratedDutyCycle(limitType);
		newDutyCycle = systemPowerControlArbitrator->arbitrate(policyIndex, limitType, dutyCycle);
		if (currentDutyCycle != newDutyCycle)
		{
			shouldSetDutyCycle = true;
		}
	}
	else
	{
		shouldSetDutyCycle = true;
		newDutyCycle = systemPowerControlArbitrator->arbitrate(policyIndex, limitType, dutyCycle);
	}

	if (shouldSetDutyCycle)
	{
		m_theRealParticipant->setSystemPowerLimitDutyCycle(m_participantIndex, m_domainIndex, limitType, newDutyCycle);
		clearDomainCachedDataSystemPowerControl();
	}
	systemPowerControlArbitrator->commitPolicyRequest(policyIndex, limitType, dutyCycle);
}

void Domain::setPowerSharePolicyPower(const Power& powerSharePolicyPower)
{
	m_theRealParticipant->setPowerSharePolicyPower(m_participantIndex, m_domainIndex, powerSharePolicyPower);
}

Power Domain::getPlatformRestOfPower(void)
{
	FILL_CACHE_AND_RETURN(m_platformRestOfPower, Power, getPlatformRestOfPower);
}

Power Domain::getAdapterPowerRating(void)
{
	FILL_CACHE_AND_RETURN(m_adapterRating, Power, getAdapterPowerRating);
}

PlatformPowerSource::Type Domain::getPlatformPowerSource(void)
{
	FILL_CACHE_AND_RETURN(m_platformPowerSource, PlatformPowerSource::Type, getPlatformPowerSource);
}

UInt32 Domain::getACNominalVoltage(void)
{
	FILL_CACHE_AND_RETURN(m_acNominalVoltage, UInt32, getACNominalVoltage);
}

UInt32 Domain::getACOperationalCurrent(void)
{
	FILL_CACHE_AND_RETURN(m_acOperationalCurrent, UInt32, getACOperationalCurrent);
}

Percentage Domain::getAC1msPercentageOverload(void)
{
	FILL_CACHE_AND_RETURN(m_ac1msPercentageOverload, Percentage, getAC1msPercentageOverload);
}

Percentage Domain::getAC2msPercentageOverload(void)
{
	FILL_CACHE_AND_RETURN(m_ac2msPercentageOverload, Percentage, getAC2msPercentageOverload);
}

Percentage Domain::getAC10msPercentageOverload(void)
{
	FILL_CACHE_AND_RETURN(m_ac10msPercentageOverload, Percentage, getAC10msPercentageOverload);
}

void Domain::notifyForProchotDeassertion(void)
{
	m_theRealParticipant->notifyForProchotDeassertion(m_participantIndex, m_domainIndex);
}

DomainPriority Domain::getDomainPriority(void)
{
	FILL_CACHE_AND_RETURN(m_domainPriority, DomainPriority, getDomainPriority);
}

RfProfileCapabilities Domain::getRfProfileCapabilities(void)
{
	FILL_CACHE_AND_RETURN(m_rfProfileCapabilities, RfProfileCapabilities, getRfProfileCapabilities);
}

void Domain::setRfProfileCenterFrequency(UIntN policyIndex, const Frequency& centerFrequency)
{
	// No arbitration.  Last caller wins.

	m_theRealParticipant->setRfProfileCenterFrequency(m_participantIndex, m_domainIndex, centerFrequency);
	clearDomainCachedDataRfProfileControl();
	clearDomainCachedDataRfProfileStatus();
}

Percentage Domain::getSscBaselineSpreadValue()
{
	return m_theRealParticipant->getSscBaselineSpreadValue(m_participantIndex, m_domainIndex);
}

Percentage Domain::getSscBaselineThreshold()
{
	return m_theRealParticipant->getSscBaselineThreshold(m_participantIndex, m_domainIndex);
}

Percentage Domain::getSscBaselineGuardBand()
{
	return m_theRealParticipant->getSscBaselineGuardBand(m_participantIndex, m_domainIndex);
}

RfProfileDataSet Domain::getRfProfileDataSet(void)
{
	FILL_CACHE_AND_RETURN(m_rfProfileData, RfProfileDataSet, getRfProfileDataSet);
}

UInt32 Domain::getWifiCapabilities(void)
{
	return m_theRealParticipant->getWifiCapabilities(m_participantIndex, m_domainIndex);
}

UInt32 Domain::getRfiDisable(void)
{
	return m_theRealParticipant->getRfiDisable(m_participantIndex, m_domainIndex);
}

UInt64 Domain::getDvfsPoints(void)
{
	return m_theRealParticipant->getDvfsPoints(m_participantIndex, m_domainIndex);
}

void Domain::setDdrRfiTable(const DdrfChannelBandPackage::WifiRfiDdr ddrRfiStruct)
{
	m_theRealParticipant->setDdrRfiTable(m_participantIndex, m_domainIndex, ddrRfiStruct);
}

void Domain::setProtectRequest(const UInt64 frequencyRate)
{
	m_theRealParticipant->setProtectRequest(m_participantIndex, m_domainIndex, frequencyRate);
}

UtilizationStatus Domain::getUtilizationStatus(void)
{
	FILL_CACHE_AND_RETURN(m_utilizationStatus, UtilizationStatus, getUtilizationStatus);
}

void Domain::clearDomainCachedDataCoreControl()
{
	DELETE_MEMORY_TC(m_coreControlStaticCaps);
	DELETE_MEMORY_TC(m_coreControlDynamicCaps);
	DELETE_MEMORY_TC(m_coreControlLpoPreference);
	DELETE_MEMORY_TC(m_coreControlStatus);
}

void Domain::clearDomainCachedDataDisplayControl()
{
	DELETE_MEMORY_TC(m_displayControlDynamicCaps);
	DELETE_MEMORY_TC(m_displayControlStatus);
	DELETE_MEMORY_TC(m_displayControlSet);
}

void Domain::clearDomainCachedDataPerformanceControl()
{
	DELETE_MEMORY_TC(m_performanceControlStaticCaps);
	DELETE_MEMORY_TC(m_performanceControlDynamicCaps);
	DELETE_MEMORY_TC(m_performanceControlStatus);
	DELETE_MEMORY_TC(m_performanceControlSet);
}

void Domain::clearDomainCachedDataPowerControl()
{
	DELETE_MEMORY_TC(m_powerControlDynamicCapsSet);
	m_powerLimitEnabled.clear();
	m_powerLimit.clear();
	m_powerLimitTimeWindow.clear();
	m_powerLimitDutyCycle.clear();
	DELETE_MEMORY_TC(m_isPowerShareControl);
}

void Domain::clearDomainCachedDataPowerStatus()
{
	DELETE_MEMORY_TC(m_powerStatus);
}

void Domain::clearDomainCachedDataPriority()
{
	DELETE_MEMORY_TC(m_domainPriority);
}

void Domain::clearDomainCachedDataRfProfileControl()
{
	DELETE_MEMORY_TC(m_rfProfileCapabilities);
}

void Domain::clearDomainCachedDataRfProfileStatus()
{
	DELETE_MEMORY_TC(m_rfProfileData);
}

void Domain::clearDomainCachedDataUtilizationStatus()
{
	DELETE_MEMORY_TC(m_utilizationStatus);
}

void Domain::clearDomainCachedDataPlatformPowerStatus()
{
	DELETE_MEMORY_TC(m_adapterRating);
	DELETE_MEMORY_TC(m_platformRestOfPower);
	DELETE_MEMORY_TC(m_platformPowerSource);
	DELETE_MEMORY_TC(m_acNominalVoltage);
	DELETE_MEMORY_TC(m_acOperationalCurrent);
	DELETE_MEMORY_TC(m_ac1msPercentageOverload);
	DELETE_MEMORY_TC(m_ac2msPercentageOverload);
	DELETE_MEMORY_TC(m_ac10msPercentageOverload);
}

void Domain::clearDomainCachedDataSystemPowerControl()
{
	m_systemPowerLimitEnabled.clear();
	m_systemPowerLimit.clear();
	m_systemPowerLimitTimeWindow.clear();
	m_systemPowerLimitDutyCycle.clear();
}

UInt32 Domain::getSocDgpuPerformanceHintPoints(void)
{
	return m_theRealParticipant->getSocDgpuPerformanceHintPoints(m_participantIndex, m_domainIndex);
}
