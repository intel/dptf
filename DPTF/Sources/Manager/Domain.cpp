/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
	, m_activeControlStaticCaps(nullptr)
	, m_activeControlStatus(nullptr)
	, m_activeControlSet(nullptr)
	, m_configTdpControlDynamicCaps(nullptr)
	, m_configTdpControlStatus(nullptr)
	, m_configTdpControlSet(nullptr)
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
	, m_powerStatus(nullptr)
	, m_maxBatteryPower(nullptr)
	, m_adapterRating(nullptr)
	, m_platformRestOfPower(nullptr)
	, m_platformPowerSource(nullptr)
	, m_chargerType(nullptr)
	, m_batterySteadyState(nullptr)
	, m_acNominalVoltage(nullptr)
	, m_acOperationalCurrent(nullptr)
	, m_ac1msPercentageOverload(nullptr)
	, m_ac2msPercentageOverload(nullptr)
	, m_ac10msPercentageOverload(nullptr)
	, m_domainPriority(nullptr)
	, m_rfProfileCapabilities(nullptr)
	, m_rfProfileData(nullptr)
	, m_temperatureStatus(nullptr)
	, m_temperatureThresholds(nullptr)
	, m_isVirtualTemperature(nullptr)
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
		m_arbitrator = new Arbitrator(m_dptfManager);

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
	clearDomainCachedDataActiveControl();
	clearDomainCachedDataConfigTdpControl();
	clearDomainCachedDataCoreControl();
	clearDomainCachedDataDisplayControl();
	clearDomainCachedDataPerformanceControl();
	clearDomainCachedDataPowerControl();
	clearDomainCachedDataPowerStatus();
	clearDomainCachedDataPlatformPowerControl();
	clearDomainCachedDataPriority();
	clearDomainCachedDataRfProfileControl();
	clearDomainCachedDataRfProfileStatus();
	clearDomainCachedDataTemperature();
	clearDomainCachedDataUtilizationStatus();
	clearDomainCachedDataPlatformPowerStatus();
}

void Domain::clearArbitrationDataForPolicy(UIntN policyIndex)
{
	m_arbitrator->clearPolicyCachedData(policyIndex);
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

ActiveControlStaticCaps Domain::getActiveControlStaticCaps(void)
{
	FILL_CACHE_AND_RETURN(m_activeControlStaticCaps, ActiveControlStaticCaps, getActiveControlStaticCaps);
}

ActiveControlStatus Domain::getActiveControlStatus(void)
{
	FILL_CACHE_AND_RETURN(m_activeControlStatus, ActiveControlStatus, getActiveControlStatus);
}

ActiveControlSet Domain::getActiveControlSet(void)
{
	FILL_CACHE_AND_RETURN(m_activeControlSet, ActiveControlSet, getActiveControlSet);
}

void Domain::setActiveControl(UIntN policyIndex, UIntN controlIndex)
{
	ActiveControlArbitrator* activeControlArbitrator = m_arbitrator->getActiveControlArbitrator();
	Bool shouldSetActiveControlIndex = false;
	UIntN newControlIndex;

	if (activeControlArbitrator->hasArbitratedActiveControlIndex())
	{
		auto currentControlIndex = activeControlArbitrator->getArbitratedActiveControlIndex();
		newControlIndex = activeControlArbitrator->arbitrate(policyIndex, controlIndex);
		if (currentControlIndex != newControlIndex)
		{
			shouldSetActiveControlIndex = true;
		}
	}
	else
	{
		shouldSetActiveControlIndex = true;
		newControlIndex = activeControlArbitrator->arbitrate(policyIndex, controlIndex);
	}

	if (shouldSetActiveControlIndex)
	{
		m_theRealParticipant->setActiveControl(m_participantIndex, m_domainIndex, newControlIndex);
		clearDomainCachedDataActiveControl();
	}
	activeControlArbitrator->commitPolicyRequest(policyIndex, controlIndex);
}

void Domain::setActiveControl(UIntN policyIndex, const Percentage& fanSpeed)
{
	ActiveControlArbitrator* activeControlArbitrator = m_arbitrator->getActiveControlArbitrator();
	Bool shouldSetActiveControlFanSpeed = false;
	Percentage newFanSpeed;

	if (activeControlArbitrator->hasArbitratedFanSpeedPercentage())
	{
		auto currentFanSpeed = activeControlArbitrator->getArbitratedFanSpeedPercentage();
		newFanSpeed = activeControlArbitrator->arbitrate(policyIndex, fanSpeed);
		if (currentFanSpeed != newFanSpeed)
		{
			shouldSetActiveControlFanSpeed = true;
		}
	}
	else
	{
		shouldSetActiveControlFanSpeed = true;
		newFanSpeed = activeControlArbitrator->arbitrate(policyIndex, fanSpeed);
	}

	if (shouldSetActiveControlFanSpeed)
	{
		m_theRealParticipant->setActiveControl(m_participantIndex, m_domainIndex, newFanSpeed);
		clearDomainCachedDataActiveControl();
	}
	activeControlArbitrator->commitPolicyRequest(policyIndex, fanSpeed);
}

UInt32 Domain::getEnergyThreshold()
{
	return m_theRealParticipant->getEnergyThreshold(m_participantIndex, m_domainIndex);
}

void Domain::setEnergyThreshold(UInt32 energyThreshold)
{
	m_theRealParticipant->setEnergyThreshold(m_participantIndex, m_domainIndex, energyThreshold);
}

Temperature Domain::getPowerShareTemperatureThreshold()
{
	return m_theRealParticipant->getPowerShareTemperatureThreshold(m_participantIndex, m_domainIndex);
}

Percentage Domain::getUtilizationThreshold()
{
	return m_theRealParticipant->getUtilizationThreshold(m_participantIndex, m_domainIndex);
}

Percentage Domain::getResidencyUtilization()
{
	return m_theRealParticipant->getResidencyUtilization(m_participantIndex, m_domainIndex);
}

void Domain::setEnergyThresholdInterruptFlag(UInt32 energyThresholdInterruptFlag)
{
	m_theRealParticipant->setEnergyThresholdInterruptFlag(
		m_participantIndex, m_domainIndex, energyThresholdInterruptFlag);
}

ConfigTdpControlDynamicCaps Domain::getConfigTdpControlDynamicCaps(void)
{
	FILL_CACHE_AND_RETURN(m_configTdpControlDynamicCaps, ConfigTdpControlDynamicCaps, getConfigTdpControlDynamicCaps);
}

ConfigTdpControlStatus Domain::getConfigTdpControlStatus(void)
{
	FILL_CACHE_AND_RETURN(m_configTdpControlStatus, ConfigTdpControlStatus, getConfigTdpControlStatus);
}

ConfigTdpControlSet Domain::getConfigTdpControlSet(void)
{
	FILL_CACHE_AND_RETURN(m_configTdpControlSet, ConfigTdpControlSet, getConfigTdpControlSet);
}

void Domain::setConfigTdpControl(UIntN policyIndex, UIntN controlIndex)
{
	ConfigTdpControlArbitrator* configTdpControlArbitrator = m_arbitrator->getConfigTdpControlArbitrator();
	Bool shouldSetConfigTdpControl = false;
	UIntN newControlIndex;

	if (configTdpControlArbitrator->hasArbitratedConfigTdpControlIndex())
	{
		auto currentControlIndex = configTdpControlArbitrator->getArbitratedConfigTdpControlIndex();
		newControlIndex = configTdpControlArbitrator->arbitrate(policyIndex, controlIndex);
		if (currentControlIndex != newControlIndex)
		{
			shouldSetConfigTdpControl = true;
		}
	}
	else
	{
		shouldSetConfigTdpControl = true;
		newControlIndex = configTdpControlArbitrator->arbitrate(policyIndex, controlIndex);
	}

	if (shouldSetConfigTdpControl)
	{
		m_theRealParticipant->setConfigTdpControl(m_participantIndex, m_domainIndex, newControlIndex);
		clearDomainCachedDataConfigTdpControl();
		clearDomainCachedDataPowerControl();
		clearDomainCachedDataPerformanceControl();
	}
	configTdpControlArbitrator->commitPolicyRequest(policyIndex, controlIndex);
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

Bool Domain::isUserPreferredIndexModified()
{
	return m_theRealParticipant->isUserPreferredIndexModified(m_participantIndex, m_domainIndex);
}

DisplayControlStatus Domain::getDisplayControlStatus(void)
{
	FILL_CACHE_AND_RETURN(m_displayControlStatus, DisplayControlStatus, getDisplayControlStatus);
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

Power Domain::getACPeakPower(void)
{
	return m_theRealParticipant->getACPeakPower(m_participantIndex, m_domainIndex);
}

void Domain::setACPeakPower(UIntN policyIndex, const Power& acPeakPower)
{
	PeakPowerControlArbitrator* peakPowerControlArbitrator = m_arbitrator->getPeakPowerControlArbitrator();
	Bool shouldSetPeakPower = false;
	Power newACPeakPower;

	if (peakPowerControlArbitrator->hasArbitratedPeakPower(PeakPowerType::ACPeakPower))
	{
		auto currentACPeakPower = peakPowerControlArbitrator->getArbitratedPeakPower(PeakPowerType::ACPeakPower);
		newACPeakPower = peakPowerControlArbitrator->arbitrate(policyIndex, PeakPowerType::ACPeakPower, acPeakPower);
		if (currentACPeakPower != newACPeakPower)
		{
			shouldSetPeakPower = true;
		}
	}
	else
	{
		shouldSetPeakPower = true;
		newACPeakPower = peakPowerControlArbitrator->arbitrate(policyIndex, PeakPowerType::ACPeakPower, acPeakPower);
	}

	if (shouldSetPeakPower)
	{
		m_theRealParticipant->setACPeakPower(m_participantIndex, m_domainIndex, newACPeakPower);
	}
	peakPowerControlArbitrator->commitPolicyRequest(policyIndex, PeakPowerType::ACPeakPower, acPeakPower);
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

	if (peakPowerControlArbitrator->hasArbitratedPeakPower(PeakPowerType::DCPeakPower))
	{
		auto currentDCPeakPower = peakPowerControlArbitrator->getArbitratedPeakPower(PeakPowerType::DCPeakPower);
		newDCPeakPower = peakPowerControlArbitrator->arbitrate(policyIndex, PeakPowerType::DCPeakPower, dcPeakPower);
		if (currentDCPeakPower != newDCPeakPower)
		{
			shouldSetPeakPower = true;
		}
	}
	else
	{
		shouldSetPeakPower = true;
		newDCPeakPower = peakPowerControlArbitrator->arbitrate(policyIndex, PeakPowerType::DCPeakPower, dcPeakPower);
	}

	if (shouldSetPeakPower)
	{
		m_theRealParticipant->setDCPeakPower(m_participantIndex, m_domainIndex, newDCPeakPower);
	}
	peakPowerControlArbitrator->commitPolicyRequest(policyIndex, PeakPowerType::DCPeakPower, dcPeakPower);
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
	PerformanceControlDynamicCaps newCaps(Constants::Invalid, Constants::Invalid);

	if (arbitrator->hasArbitratedPerformanceControlCapabilities())
	{
		auto currentCaps = arbitrator->getArbitratedPerformanceControlCapabilities();
		newCaps = arbitrator->arbitrate(policyIndex, newCapabilities);
		if (currentCaps != newCaps)
		{
			shouldSetPerformanceCapabilities = true;
		}
	}
	else
	{
		shouldSetPerformanceCapabilities = true;
		newCaps = arbitrator->arbitrate(policyIndex, newCapabilities);
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
	PowerControlDynamicCapsSet newCaps;

	if (arbitrator->hasArbitratedPowerControlCapabilities())
	{
		auto currentCaps = arbitrator->getArbitratedPowerControlCapabilities();
		newCaps = arbitrator->arbitrate(policyIndex, capsSet);
		if (currentCaps != newCaps)
		{
			shouldSetPowerControlCapabilities = true;
		}
	}
	else
	{
		shouldSetPowerControlCapabilities = true;
		newCaps = arbitrator->arbitrate(policyIndex, capsSet);
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

void Domain::setPowerCapsLock(UIntN policyIndex, Bool lock)
{
	PowerControlCapabilitiesArbitrator* arbitrator = m_arbitrator->getPowerControlCapabilitiesArbitrator();
	Bool updated = arbitrator->arbitrateLockRequests(policyIndex, lock);
	if (updated == true)
	{
		m_theRealParticipant->setPowerCapsLock(m_participantIndex, m_domainIndex, arbitrator->getArbitratedLock());
	}
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

TimeSpan Domain::getTau()
{
	return m_theRealParticipant->getTau(m_participantIndex, m_domainIndex);
}

TimeSpan Domain::getFastPollTime()
{
	return m_theRealParticipant->getFastPollTime(m_participantIndex, m_domainIndex);
}

TimeSpan Domain::getSlowPollTime()
{
	return m_theRealParticipant->getSlowPollTime(m_participantIndex, m_domainIndex);
}

UInt32 Domain::getWeightedSlowPollAvgConstant()
{
	return m_theRealParticipant->getWeightedSlowPollAvgConstant(m_participantIndex, m_domainIndex);
}

UInt32 Domain::getRaplEnergyCounter()
{
	return m_theRealParticipant->getRaplEnergyCounter(m_participantIndex, m_domainIndex);
}

double Domain::getRaplEnergyUnit()
{
	return m_theRealParticipant->getRaplEnergyUnit(m_participantIndex, m_domainIndex);
}

UInt32 Domain::getRaplEnergyCounterWidth()
{
	return m_theRealParticipant->getRaplEnergyCounterWidth(m_participantIndex, m_domainIndex);
}

Power Domain::getSlowPollPowerThreshold()
{
	return m_theRealParticipant->getSlowPollPowerThreshold(m_participantIndex, m_domainIndex);
}

Power Domain::getInstantaneousPower()
{
	return m_theRealParticipant->getInstantaneousPower(m_participantIndex, m_domainIndex);
}

PowerStatus Domain::getPowerStatus(void)
{
	FILL_CACHE_AND_RETURN(m_powerStatus, PowerStatus, getPowerStatus);
}

Power Domain::getAveragePower(const PowerControlDynamicCaps& capabilities)
{
	return m_theRealParticipant->getAveragePower(m_participantIndex, m_domainIndex, capabilities);
}

void Domain::setCalculatedAveragePower(Power powerValue)
{
	m_theRealParticipant->setCalculatedAveragePower(m_participantIndex, m_domainIndex, powerValue);
}

Bool Domain::isPlatformPowerLimitEnabled(PlatformPowerLimitType::Type limitType)
{
	auto enabled = m_platformPowerLimitEnabled.find(limitType);
	if (enabled == m_platformPowerLimitEnabled.end())
	{
		m_platformPowerLimitEnabled[limitType] =
			m_theRealParticipant->isPlatformPowerLimitEnabled(m_participantIndex, m_domainIndex, limitType);
	}
	return m_platformPowerLimitEnabled.at(limitType);
}

Power Domain::getPlatformPowerLimit(PlatformPowerLimitType::Type limitType)
{
	auto limit = m_platformPowerLimit.find(limitType);
	if (limit == m_platformPowerLimit.end())
	{
		m_platformPowerLimit[limitType] =
			m_theRealParticipant->getPlatformPowerLimit(m_participantIndex, m_domainIndex, limitType);
	}
	return m_platformPowerLimit.at(limitType);
}

void Domain::setPlatformPowerLimit(UIntN policyIndex, PlatformPowerLimitType::Type limitType, const Power& powerLimit)
{
	PlatformPowerControlArbitrator* platformPowerControlArbitrator = m_arbitrator->getPlatformPowerControlArbitrator();
	Bool shouldSetPowerLimit = false;
	Power newPlatformPowerLimit;

	if (platformPowerControlArbitrator->hasArbitratedPlatformPowerLimit(limitType))
	{
		auto currentPlatformPowerLimit = platformPowerControlArbitrator->getArbitratedPlatformPowerLimit(limitType);
		newPlatformPowerLimit = platformPowerControlArbitrator->arbitrate(policyIndex, limitType, powerLimit);
		if (currentPlatformPowerLimit != newPlatformPowerLimit)
		{
			shouldSetPowerLimit = true;
		}
	}
	else
	{
		shouldSetPowerLimit = true;
		newPlatformPowerLimit = platformPowerControlArbitrator->arbitrate(policyIndex, limitType, powerLimit);
	}

	if (shouldSetPowerLimit)
	{
		m_theRealParticipant->setPlatformPowerLimit(
			m_participantIndex, m_domainIndex, limitType, newPlatformPowerLimit);
		clearDomainCachedDataPlatformPowerControl();
	}
	platformPowerControlArbitrator->commitPolicyRequest(policyIndex, limitType, powerLimit);
}

TimeSpan Domain::getPlatformPowerLimitTimeWindow(PlatformPowerLimitType::Type limitType)
{
	auto timeWindow = m_platformPowerLimitTimeWindow.find(limitType);
	if (timeWindow == m_platformPowerLimitTimeWindow.end())
	{
		m_platformPowerLimitTimeWindow[limitType] =
			m_theRealParticipant->getPlatformPowerLimitTimeWindow(m_participantIndex, m_domainIndex, limitType);
	}
	return m_platformPowerLimitTimeWindow.at(limitType);
}

void Domain::setPlatformPowerLimitTimeWindow(
	UIntN policyIndex,
	PlatformPowerLimitType::Type limitType,
	const TimeSpan& timeWindow)
{
	PlatformPowerControlArbitrator* platformPowerControlArbitrator = m_arbitrator->getPlatformPowerControlArbitrator();
	Bool shouldSetTimeWindow = false;
	TimeSpan newTimeWindow;

	if (platformPowerControlArbitrator->hasArbitratedTimeWindow(limitType))
	{
		auto currentTimeWindow = platformPowerControlArbitrator->getArbitratedTimeWindow(limitType);
		newTimeWindow = platformPowerControlArbitrator->arbitrate(policyIndex, limitType, timeWindow);
		if (currentTimeWindow != newTimeWindow)
		{
			shouldSetTimeWindow = true;
		}
	}
	else
	{
		shouldSetTimeWindow = true;
		newTimeWindow = platformPowerControlArbitrator->arbitrate(policyIndex, limitType, timeWindow);
	}

	if (shouldSetTimeWindow)
	{
		m_theRealParticipant->setPlatformPowerLimitTimeWindow(
			m_participantIndex, m_domainIndex, limitType, newTimeWindow);
		clearDomainCachedDataPlatformPowerControl();
	}
	platformPowerControlArbitrator->commitPolicyRequest(policyIndex, limitType, timeWindow);
}

Percentage Domain::getPlatformPowerLimitDutyCycle(PlatformPowerLimitType::Type limitType)
{
	auto dutyCycle = m_platformPowerLimitDutyCycle.find(limitType);
	if (dutyCycle == m_platformPowerLimitDutyCycle.end())
	{
		m_platformPowerLimitDutyCycle[limitType] =
			m_theRealParticipant->getPlatformPowerLimitDutyCycle(m_participantIndex, m_domainIndex, limitType);
	}
	return m_platformPowerLimitDutyCycle.at(limitType);
}

void Domain::setPlatformPowerLimitDutyCycle(
	UIntN policyIndex,
	PlatformPowerLimitType::Type limitType,
	const Percentage& dutyCycle)
{
	PlatformPowerControlArbitrator* platformPowerControlArbitrator = m_arbitrator->getPlatformPowerControlArbitrator();
	Bool shouldSetDutyCycle = false;
	Percentage newDutyCycle;

	if (platformPowerControlArbitrator->hasArbitratedDutyCycle(limitType))
	{
		auto currentDutyCycle = platformPowerControlArbitrator->getArbitratedDutyCycle(limitType);
		newDutyCycle = platformPowerControlArbitrator->arbitrate(policyIndex, limitType, dutyCycle);
		if (currentDutyCycle != newDutyCycle)
		{
			shouldSetDutyCycle = true;
		}
	}
	else
	{
		shouldSetDutyCycle = true;
		newDutyCycle = platformPowerControlArbitrator->arbitrate(policyIndex, limitType, dutyCycle);
	}

	if (shouldSetDutyCycle)
	{
		m_theRealParticipant->setPlatformPowerLimitDutyCycle(
			m_participantIndex, m_domainIndex, limitType, newDutyCycle);
		clearDomainCachedDataPlatformPowerControl();
	}
	platformPowerControlArbitrator->commitPolicyRequest(policyIndex, limitType, dutyCycle);
}

Power Domain::getMaxBatteryPower(void)
{
	FILL_CACHE_AND_RETURN(m_maxBatteryPower, Power, getMaxBatteryPower);
}

Power Domain::getPlatformRestOfPower(void)
{
	FILL_CACHE_AND_RETURN(m_platformRestOfPower, Power, getPlatformRestOfPower);
}

Power Domain::getAdapterPowerRating(void)
{
	FILL_CACHE_AND_RETURN(m_adapterRating, Power, getAdapterPowerRating);
}

DptfBuffer Domain::getBatteryStatus(void)
{
	if (m_batteryStatusBuffer.size() == 0)
	{
		m_batteryStatusBuffer = m_theRealParticipant->getBatteryStatus(m_participantIndex, m_domainIndex);
	}
	return m_batteryStatusBuffer;
}

DptfBuffer Domain::getBatteryInformation(void)
{
	if (m_batteryInformationBuffer.size() == 0)
	{
		m_batteryInformationBuffer = m_theRealParticipant->getBatteryInformation(m_participantIndex, m_domainIndex);
	}
	return m_batteryInformationBuffer;
}

PlatformPowerSource::Type Domain::getPlatformPowerSource(void)
{
	FILL_CACHE_AND_RETURN(m_platformPowerSource, PlatformPowerSource::Type, getPlatformPowerSource);
}

ChargerType::Type Domain::getChargerType(void)
{
	FILL_CACHE_AND_RETURN(m_chargerType, ChargerType::Type, getChargerType);
}

Power Domain::getPlatformBatterySteadyState(void)
{
	FILL_CACHE_AND_RETURN(m_batterySteadyState, Power, getPlatformBatterySteadyState);
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

RfProfileDataSet Domain::getRfProfileDataSet(void)
{
	FILL_CACHE_AND_RETURN(m_rfProfileData, RfProfileDataSet, getRfProfileDataSet);
}

Temperature Domain::getTccOffsetTemperature()
{
	return m_theRealParticipant->getTccOffsetTemperature(m_participantIndex, m_domainIndex);
}

void Domain::setTccOffsetTemperature(UIntN policyIndex, const Temperature& tccOffset)
{
	m_theRealParticipant->setTccOffsetTemperature(m_participantIndex, m_domainIndex, tccOffset);
}

Temperature Domain::getMaxTccOffsetTemperature()
{
	return m_theRealParticipant->getMaxTccOffsetTemperature(m_participantIndex, m_domainIndex);
}

Temperature Domain::getMinTccOffsetTemperature()
{
	return m_theRealParticipant->getMinTccOffsetTemperature(m_participantIndex, m_domainIndex);
}

TemperatureStatus Domain::getTemperatureStatus(void)
{
	FILL_CACHE_AND_RETURN(m_temperatureStatus, TemperatureStatus, getTemperatureStatus);
}

TemperatureThresholds Domain::getTemperatureThresholds(void)
{
	FILL_CACHE_AND_RETURN(m_temperatureThresholds, TemperatureThresholds, getTemperatureThresholds);
}

void Domain::setTemperatureThresholds(UIntN policyIndex, const TemperatureThresholds& temperatureThresholds)
{
	// The temperature has to be locked for the duration of the WIDomainTemperatureThresholdCrossed event.

	m_arbitrator->getTemperatureThresholdArbitrator()->arbitrate(
		policyIndex, temperatureThresholds, getTemperatureStatus().getCurrentTemperature());
	m_theRealParticipant->setTemperatureThresholds(
		m_participantIndex,
		m_domainIndex,
		m_arbitrator->getTemperatureThresholdArbitrator()->getArbitratedTemperatureThresholds());

	// DO NOT invalidate the temperature status (m_temperatureStatus)
	// Only invalidate the temperature thresholds.
	DELETE_MEMORY_TC(m_temperatureThresholds);
}

UtilizationStatus Domain::getUtilizationStatus(void)
{
	FILL_CACHE_AND_RETURN(m_utilizationStatus, UtilizationStatus, getUtilizationStatus);
}

DptfBuffer Domain::getVirtualSensorCalibrationTable(void)
{
	if (m_virtualSensorCalculationTableBuffer.size() == 0)
	{
		m_virtualSensorCalculationTableBuffer =
			m_theRealParticipant->getCalibrationTable(m_participantIndex, m_domainIndex);
	}
	return m_virtualSensorCalculationTableBuffer;
}

DptfBuffer Domain::getVirtualSensorPollingTable(void)
{
	if (m_virtualSensorPollingTableBuffer.size() == 0)
	{
		m_virtualSensorPollingTableBuffer = m_theRealParticipant->getPollingTable(m_participantIndex, m_domainIndex);
	}
	return m_virtualSensorPollingTableBuffer;
}

Bool Domain::isVirtualTemperature(void)
{
	FILL_CACHE_AND_RETURN(m_isVirtualTemperature, Bool, isVirtualTemperature);
}

void Domain::setVirtualTemperature(const Temperature& temperature)
{
	// No arbitration.
	m_theRealParticipant->setVirtualTemperature(m_participantIndex, m_domainIndex, temperature);
}

void Domain::clearDomainCachedDataActiveControl()
{
	DELETE_MEMORY_TC(m_activeControlStaticCaps);
	DELETE_MEMORY_TC(m_activeControlStatus);
	DELETE_MEMORY_TC(m_activeControlSet);
}

void Domain::clearDomainCachedDataConfigTdpControl()
{
	DELETE_MEMORY_TC(m_configTdpControlDynamicCaps);
	DELETE_MEMORY_TC(m_configTdpControlStatus);
	DELETE_MEMORY_TC(m_configTdpControlSet);
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

void Domain::clearDomainCachedDataTemperature()
{
	DELETE_MEMORY_TC(m_temperatureStatus);
	DELETE_MEMORY_TC(m_temperatureThresholds);
	DELETE_MEMORY_TC(m_isVirtualTemperature);
	m_virtualSensorCalculationTableBuffer.allocate(0);
	m_virtualSensorPollingTableBuffer.allocate(0);
}

void Domain::clearDomainCachedDataUtilizationStatus()
{
	DELETE_MEMORY_TC(m_utilizationStatus);
}

void Domain::clearDomainCachedDataPlatformPowerStatus()
{
	DELETE_MEMORY_TC(m_maxBatteryPower);
	DELETE_MEMORY_TC(m_adapterRating);
	DELETE_MEMORY_TC(m_platformRestOfPower);
	DELETE_MEMORY_TC(m_platformPowerSource);
	DELETE_MEMORY_TC(m_chargerType);
	DELETE_MEMORY_TC(m_batterySteadyState);
	DELETE_MEMORY_TC(m_acNominalVoltage);
	DELETE_MEMORY_TC(m_acOperationalCurrent);
	DELETE_MEMORY_TC(m_ac1msPercentageOverload);
	DELETE_MEMORY_TC(m_ac2msPercentageOverload);
	DELETE_MEMORY_TC(m_ac10msPercentageOverload);
	m_batteryStatusBuffer.allocate(0);
	m_batteryInformationBuffer.allocate(0);
}

void Domain::clearDomainCachedDataPlatformPowerControl()
{
	m_platformPowerLimitEnabled.clear();
	m_platformPowerLimit.clear();
	m_platformPowerLimitTimeWindow.clear();
	m_platformPowerLimitDutyCycle.clear();
}
