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

#include "Domain.h"
#include "EsifDataString.h"
#include "EsifDataGuid.h"

Domain::Domain(DptfManager* dptfManager) :
    m_domainCreated(false), m_dptfManager(dptfManager), m_theRealParticipant(nullptr),
    m_participantIndex(Constants::Invalid), m_domainIndex(Constants::Invalid),
    m_domainGuid(Guid()), m_domainName(""), m_domainType(DomainType::Type::Invalid),
    m_domainFunctionalityVersions(DomainFunctionalityVersions()),
    m_arbitrator(nullptr),
    m_activeControlStaticCaps(nullptr), m_activeControlStatus(nullptr), m_activeControlSet(nullptr),
    m_configTdpControlDynamicCaps(nullptr), m_configTdpControlStatus(nullptr), m_configTdpControlSet(nullptr),
    m_coreControlStaticCaps(nullptr), m_coreControlDynamicCaps(nullptr),
    m_coreControlLpoPreference(nullptr), m_coreControlStatus(nullptr),
    m_displayControlDynamicCaps(nullptr), m_displayControlStatus(nullptr), m_displayControlSet(nullptr),
    m_performanceControlStaticCaps(nullptr), m_performanceControlDynamicCaps(nullptr),
    m_performanceControlStatus(nullptr), m_performanceControlSet(nullptr),
    m_pixelClockCapabilities(nullptr), m_pixelClockDataSet(nullptr),
    m_powerControlDynamicCapsSet(nullptr), m_powerControlStatusSet(nullptr),
    m_powerStatus(nullptr),
    m_domainPriority(nullptr),
    m_rfProfileCapabilities(nullptr),
    m_rfProfileData(nullptr),
    m_temperatureStatus(nullptr), m_temperatureThresholds(nullptr),
    m_utilizationStatus(nullptr)
{
}

Domain::~Domain(void)
{
    destroyDomain();
}

void Domain::createDomain(UIntN participantIndex, UIntN domainIndex, ParticipantInterface* participantInterface,
    const AppDomainDataPtr domainDataPtr, Bool domainEnabled)
{
    if (m_domainCreated == true)
    {
        throw dptf_exception("Domain::createDomain() already executed.");
    }
    m_domainCreated = true;

    m_theRealParticipant = participantInterface;
    m_participantIndex = participantIndex;
    m_domainIndex = domainIndex;
    m_domainGuid = EsifDataGuid(domainDataPtr->fGuid);
    m_domainName = EsifDataString(domainDataPtr->fName);
    m_domainType = EsifDomainTypeToDptfDomainType(domainDataPtr->fType);
    m_domainFunctionalityVersions = DomainFunctionalityVersions(domainDataPtr->fCapabilityBytes);
    m_arbitrator = new Arbitrator(m_dptfManager);

    m_theRealParticipant->createDomain(
        m_domainGuid,
        m_participantIndex,
        m_domainIndex,
        domainEnabled,
        m_domainType,
        m_domainName,
        EsifDataString(domainDataPtr->fDescription),
        m_domainFunctionalityVersions);
}

void Domain::destroyDomain(void)
{
    if (m_theRealParticipant != nullptr)
    {
        try
        {
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

std::string Domain::getDomainName(void) const
{
    return m_domainName;
}

void Domain::clearDomainCachedData(void)
{
    clearDomainCachedDataActiveControl();
    clearDomainCachedDataConfigTdpControl();
    clearDomainCachedDataCoreControl();
    clearDomainCachedDataDisplayControl();
    clearDomainCachedDataPerformanceControl();
    //clearDomainCachedDataPixelClockControl(); *** Nothing to cache ***
    clearDomainCachedDataPixelClockStatus();
    clearDomainCachedDataPowerControl();
    clearDomainCachedDataPowerStatus();
    clearDomainCachedDataPriority();
    clearDomainCachedDataRfProfileControl();
    clearDomainCachedDataRfProfileStatus();
    clearDomainCachedDataTemperature();
    clearDomainCachedDataUtilizationStatus();
}

void Domain::clearArbitrationDataForPolicy(UIntN policyIndex)
{
    m_arbitrator->clearPolicyCachedData(policyIndex);
}

//
// The following macro (FILL_CACHE_AND_RETURN) is in place to remove this code many times:
//
//if (m_activeControlStaticCaps == nullptr)
//{
//    ActiveControlStaticCaps var = m_theRealParticipant->getActiveControlStaticCaps(m_participantIndex, m_domainIndex);
//    m_activeControlStaticCaps = new ActiveControlStaticCaps(var);
//}
//
//return *m_activeControlStaticCaps;

#define FILL_CACHE_AND_RETURN(mv, ct, fn) \
    if (mv == nullptr) {ct var = m_theRealParticipant->fn(m_participantIndex, m_domainIndex); mv = new ct(var); } \
    return *mv; \

ActiveControlStaticCaps Domain::getActiveControlStaticCaps(void)
{
    FILL_CACHE_AND_RETURN(m_activeControlStaticCaps, ActiveControlStaticCaps, getActiveControlStaticCaps)
}

ActiveControlStatus Domain::getActiveControlStatus(void)
{
    FILL_CACHE_AND_RETURN(m_activeControlStatus, ActiveControlStatus, getActiveControlStatus)
}

ActiveControlSet Domain::getActiveControlSet(void)
{
    FILL_CACHE_AND_RETURN(m_activeControlSet, ActiveControlSet, getActiveControlSet)
}

void Domain::setActiveControl(UIntN policyIndex, UIntN controlIndex)
{
    ActiveControlArbitrator* activeControlArbitrator = m_arbitrator->getActiveControlArbitrator();

    Bool updated = activeControlArbitrator->arbitrate(policyIndex, controlIndex);

    if (updated == true)
    {
        UIntN arbitratedActiveControlIndex = activeControlArbitrator->getArbitratedActiveControlIndex();
        m_theRealParticipant->setActiveControl(m_participantIndex, m_domainIndex, arbitratedActiveControlIndex);
        clearDomainCachedDataActiveControl();
    }
}

void Domain::setActiveControl(UIntN policyIndex, const Percentage& fanSpeed)
{
    ActiveControlArbitrator* activeControlArbitrator = m_arbitrator->getActiveControlArbitrator();

    Bool updated = activeControlArbitrator->arbitrate(policyIndex, fanSpeed);

    if (updated == true)
    {
        Percentage arbitratedFanSpeedPercentage = activeControlArbitrator->getArbitratedFanSpeedPercentage();
        m_theRealParticipant->setActiveControl(m_participantIndex, m_domainIndex, arbitratedFanSpeedPercentage);
        clearDomainCachedDataActiveControl();
    }
}

ConfigTdpControlDynamicCaps Domain::getConfigTdpControlDynamicCaps(void)
{
    FILL_CACHE_AND_RETURN(m_configTdpControlDynamicCaps, ConfigTdpControlDynamicCaps, getConfigTdpControlDynamicCaps)
}

ConfigTdpControlStatus Domain::getConfigTdpControlStatus(void)
{
    FILL_CACHE_AND_RETURN(m_configTdpControlStatus, ConfigTdpControlStatus, getConfigTdpControlStatus)
}

ConfigTdpControlSet Domain::getConfigTdpControlSet(void)
{
    FILL_CACHE_AND_RETURN(m_configTdpControlSet, ConfigTdpControlSet, getConfigTdpControlSet)
}

void Domain::setConfigTdpControl(UIntN policyIndex, UIntN controlIndex)
{
    ConfigTdpControlArbitrator* configTdpControlArbitrator = m_arbitrator->getConfigTdpControlArbitrator();

    Bool updated = configTdpControlArbitrator->arbitrate(policyIndex, controlIndex);

    if (updated == true)
    {
        UIntN arbitratedConfigTdpControlIndex = configTdpControlArbitrator->getArbitratedConfigTdpControlIndex();
        m_theRealParticipant->setConfigTdpControl(m_participantIndex, m_domainIndex, arbitratedConfigTdpControlIndex);
        clearDomainCachedDataConfigTdpControl();
        clearDomainCachedDataPowerControl();
        clearDomainCachedDataPerformanceControl();
    }
}

CoreControlStaticCaps Domain::getCoreControlStaticCaps(void)
{
    FILL_CACHE_AND_RETURN(m_coreControlStaticCaps, CoreControlStaticCaps, getCoreControlStaticCaps)
}

CoreControlDynamicCaps Domain::getCoreControlDynamicCaps(void)
{
    FILL_CACHE_AND_RETURN(m_coreControlDynamicCaps, CoreControlDynamicCaps, getCoreControlDynamicCaps)
}

CoreControlLpoPreference Domain::getCoreControlLpoPreference(void)
{
    FILL_CACHE_AND_RETURN(m_coreControlLpoPreference, CoreControlLpoPreference, getCoreControlLpoPreference)
}

CoreControlStatus Domain::getCoreControlStatus(void)
{
    FILL_CACHE_AND_RETURN(m_coreControlStatus, CoreControlStatus, getCoreControlStatus)
}

void Domain::setActiveCoreControl(UIntN policyIndex, const CoreControlStatus& coreControlStatus)
{
    CoreControlArbitrator* coreControlArbitrator = m_arbitrator->getCoreControlArbitrator();

    Bool updated = coreControlArbitrator->arbitrate(policyIndex, coreControlStatus);

    if (updated == true)
    {
        CoreControlStatus arbitratedCoreControlStatus = coreControlArbitrator->getArbitratedCoreControlStatus();
        m_theRealParticipant->setActiveCoreControl(m_participantIndex, m_domainIndex, arbitratedCoreControlStatus);
        clearDomainCachedDataCoreControl();
    }
}

DisplayControlDynamicCaps Domain::getDisplayControlDynamicCaps(void)
{
    FILL_CACHE_AND_RETURN(m_displayControlDynamicCaps, DisplayControlDynamicCaps, getDisplayControlDynamicCaps)
}

DisplayControlStatus Domain::getDisplayControlStatus(void)
{
    FILL_CACHE_AND_RETURN(m_displayControlStatus, DisplayControlStatus, getDisplayControlStatus)
}

DisplayControlSet Domain::getDisplayControlSet(void)
{
    FILL_CACHE_AND_RETURN(m_displayControlSet, DisplayControlSet, getDisplayControlSet)
}

void Domain::setDisplayControl(UIntN policyIndex, UIntN displayControlIndex, Bool isOverridable)
{
    DisplayControlArbitrator* displayControlArbitrator = m_arbitrator->getDisplayControlArbitrator();

    Bool updated = displayControlArbitrator->arbitrate(policyIndex, displayControlIndex);

    if (updated == true)
    {
        UIntN arbitratedDisplayControlIndex = displayControlArbitrator->getArbitratedDisplayControlIndex();
        m_theRealParticipant->setDisplayControl(m_participantIndex, m_domainIndex, arbitratedDisplayControlIndex,
            true);  // hardcoded to true to match the DPTF 7.0 algorithm
        clearDomainCachedDataDisplayControl();
    }
}

PerformanceControlStaticCaps Domain::getPerformanceControlStaticCaps(void)
{
    FILL_CACHE_AND_RETURN(m_performanceControlStaticCaps, PerformanceControlStaticCaps, getPerformanceControlStaticCaps)
}

PerformanceControlDynamicCaps Domain::getPerformanceControlDynamicCaps(void)
{
    FILL_CACHE_AND_RETURN(m_performanceControlDynamicCaps, PerformanceControlDynamicCaps, getPerformanceControlDynamicCaps)
}

PerformanceControlStatus Domain::getPerformanceControlStatus(void)
{
    FILL_CACHE_AND_RETURN(m_performanceControlStatus, PerformanceControlStatus, getPerformanceControlStatus)
}

PerformanceControlSet Domain::getPerformanceControlSet(void)
{
    FILL_CACHE_AND_RETURN(m_performanceControlSet, PerformanceControlSet, getPerformanceControlSet)
}

void Domain::setPerformanceControl(UIntN policyIndex, UIntN performanceControlIndex)
{
    PerformanceControlArbitrator* performanceControlArbitrator = m_arbitrator->getPerformanceControlArbitrator();

    Bool updated = performanceControlArbitrator->arbitrate(policyIndex, performanceControlIndex);

    if (updated == true)
    {
        UIntN arbitratedPerformanceControlIndex = performanceControlArbitrator->getArbitratedPerformanceControlIndex();
        m_theRealParticipant->setPerformanceControl(m_participantIndex, m_domainIndex, arbitratedPerformanceControlIndex);
        clearDomainCachedDataPerformanceControl();
    }
}

void Domain::setPixelClockControl(UIntN policyIndex, const PixelClockDataSet& pixelClockDataSet)
{
    // No arbitration.  Last caller wins.

    m_theRealParticipant->setPixelClockControl(m_participantIndex, m_domainIndex, pixelClockDataSet);
    //clearDomainCachedDataPixelClockControl(); *** Nothing to cache ***
    clearDomainCachedDataPixelClockStatus();
}

PixelClockCapabilities Domain::getPixelClockCapabilities(void)
{
    FILL_CACHE_AND_RETURN(m_pixelClockCapabilities, PixelClockCapabilities, getPixelClockCapabilities);
}

PixelClockDataSet Domain::getPixelClockDataSet(void)
{
    FILL_CACHE_AND_RETURN(m_pixelClockDataSet, PixelClockDataSet, getPixelClockDataSet);
}

PowerControlDynamicCapsSet Domain::getPowerControlDynamicCapsSet(void)
{
    FILL_CACHE_AND_RETURN(m_powerControlDynamicCapsSet, PowerControlDynamicCapsSet, getPowerControlDynamicCapsSet)
}

PowerControlStatusSet Domain::getPowerControlStatusSet(void)
{
    FILL_CACHE_AND_RETURN(m_powerControlStatusSet, PowerControlStatusSet, getPowerControlStatusSet)
}

void Domain::setPowerControl(UIntN policyIndex, const PowerControlStatusSet& powerControlStatusSet)
{
    PowerControlArbitrator* powerControlArbitrator = m_arbitrator->getPowerControlArbitrator();

    Bool updated = powerControlArbitrator->arbitrate(policyIndex, powerControlStatusSet);

    if (updated == true)
    {
        PowerControlStatusSet arbitratedPowerControlStatusSet = powerControlArbitrator->getArbitratedPowerControlStatusSet();
        m_theRealParticipant->setPowerControl(m_participantIndex, m_domainIndex, arbitratedPowerControlStatusSet);
        clearDomainCachedDataPowerControl();
    }
}

PowerStatus Domain::getPowerStatus(void)
{
    FILL_CACHE_AND_RETURN(m_powerStatus, PowerStatus, getPowerStatus)
}

DomainPriority Domain::getDomainPriority(void)
{
    FILL_CACHE_AND_RETURN(m_domainPriority, DomainPriority, getDomainPriority)
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

RfProfileData Domain::getRfProfileData(void)
{
    FILL_CACHE_AND_RETURN(m_rfProfileData, RfProfileData, getRfProfileData);
}

TemperatureStatus Domain::getTemperatureStatus(void)
{
    FILL_CACHE_AND_RETURN(m_temperatureStatus, TemperatureStatus, getTemperatureStatus)
}

TemperatureThresholds Domain::getTemperatureThresholds(void)
{
    FILL_CACHE_AND_RETURN(m_temperatureThresholds, TemperatureThresholds, getTemperatureThresholds)
}

void Domain::setTemperatureThresholds(UIntN policyIndex, const TemperatureThresholds& temperatureThresholds)
{
    // The temperature has to be locked for the duration of the WIDomainTemperatureThresholdCrossed event.

    m_arbitrator->getTemperatureThresholdArbitrator()->arbitrate(policyIndex,
        temperatureThresholds, getTemperatureStatus().getCurrentTemperature());
    m_theRealParticipant->setTemperatureThresholds(m_participantIndex, m_domainIndex,
        m_arbitrator->getTemperatureThresholdArbitrator()->getArbitratedTemperatureThresholds());

    // DO NOT invalidate the temperature status (m_temperatureStatus)
    // Only invalidate the temperature thresholds.
    DELETE_MEMORY_TC(m_temperatureThresholds);
}

UtilizationStatus Domain::getUtilizationStatus(void)
{
    FILL_CACHE_AND_RETURN(m_utilizationStatus, UtilizationStatus, getUtilizationStatus)
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

// *** Nothing to cache ***
//void Domain::clearDomainCachedDataPixelClockControl()
//{
//    DELETE_MEMORY_TC();
//}

void Domain::clearDomainCachedDataPixelClockStatus()
{
    DELETE_MEMORY_TC(m_pixelClockCapabilities);
    DELETE_MEMORY_TC(m_pixelClockDataSet);
}

void Domain::clearDomainCachedDataPowerControl()
{
    DELETE_MEMORY_TC(m_powerControlDynamicCapsSet);
    DELETE_MEMORY_TC(m_powerControlStatusSet);
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
}

void Domain::clearDomainCachedDataUtilizationStatus()
{
    DELETE_MEMORY_TC(m_utilizationStatus);
}