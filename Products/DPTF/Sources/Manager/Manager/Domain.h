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

#pragma once

#include "ParticipantInterface.h"
#include "Guid.h"
#include "esif_uf_app_iface.h"
#include "Arbitrator.h"

class DptfManager;

class Domain
{
public:

    Domain(DptfManager* dptfManager);
    ~Domain(void);

    void createDomain(UIntN participantIndex, UIntN domainIndex, ParticipantInterface* participantInterface,
        const AppDomainDataPtr domainDataPtr, Bool domainEnabled);
    void destroyDomain(void);

    void enableDomain(void);
    void disableDomain(void);
    Bool isDomainEnabled(void);

    std::string getDomainName(void) const;

    // This will clear the cached data stored within this class in the framework.  It will not ask the
    // actual domain to clear its cache.
    void clearDomainCachedData(void);

    void clearArbitrationDataForPolicy(UIntN policyIndex);

    //
    // The following set of functions pass the call through to the actual domain.  They
    // also provide caching so each policy sees the same representation of the platform
    // for the duration of a work item.
    //

    // Active Controls
    ActiveControlStaticCaps getActiveControlStaticCaps(void);
    ActiveControlStatus getActiveControlStatus(void);
    ActiveControlSet getActiveControlSet(void);
    void setActiveControl(UIntN policyIndex, UIntN controlIndex);
    void setActiveControl(UIntN policyIndex, const Percentage& fanSpeed);

    // ConfigTdp controls
    ConfigTdpControlDynamicCaps getConfigTdpControlDynamicCaps(void);
    ConfigTdpControlStatus getConfigTdpControlStatus(void);
    ConfigTdpControlSet getConfigTdpControlSet(void);
    void setConfigTdpControl(UIntN policyIndex, UIntN controlIndex);

    // Core controls
    CoreControlStaticCaps getCoreControlStaticCaps(void);
    CoreControlDynamicCaps getCoreControlDynamicCaps(void);
    CoreControlLpoPreference getCoreControlLpoPreference(void);
    CoreControlStatus getCoreControlStatus(void);
    void setActiveCoreControl(UIntN policyIndex, const CoreControlStatus& coreControlStatus);

    // Display controls
    DisplayControlDynamicCaps getDisplayControlDynamicCaps(void);
    DisplayControlStatus getDisplayControlStatus(void);
    DisplayControlSet getDisplayControlSet(void);
    void setDisplayControl(UIntN policyIndex, UIntN displayControlIndex, Bool isOverridable);

    // Performance controls
    PerformanceControlStaticCaps getPerformanceControlStaticCaps(void);
    PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(void);
    PerformanceControlStatus getPerformanceControlStatus(void);
    PerformanceControlSet getPerformanceControlSet(void);
    void setPerformanceControl(UIntN policyIndex, UIntN performanceControlIndex);

    // Pixel Clock Control
    void setPixelClockControl(UIntN policyIndex, const PixelClockDataSet& pixelClockDataSet);

    // Pixel Clock Status
    PixelClockCapabilities getPixelClockCapabilities(void);
    PixelClockDataSet getPixelClockDataSet(void);

    // Power controls
    PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(void);
    PowerControlStatusSet getPowerControlStatusSet(void);
    void setPowerControl(UIntN policyIndex, const PowerControlStatusSet& powerControlStatusSet);

    // Power status
    PowerStatus getPowerStatus(void);

    // priority
    DomainPriority getDomainPriority(void);

    // RF Profile Control
    RfProfileCapabilities getRfProfileCapabilities(void);
    void setRfProfileCenterFrequency(UIntN policyIndex, const Frequency& centerFrequency);

    // RF Profile Status
    RfProfileData getRfProfileData(void);

    // temperature
    TemperatureStatus getTemperatureStatus(void);
    TemperatureThresholds getTemperatureThresholds(void);
    void setTemperatureThresholds(UIntN policyIndex, const TemperatureThresholds& temperatureThresholds);

    // utilization
    UtilizationStatus getUtilizationStatus(void);

private:

    // hide the copy constructor and assignment operator.
    Domain(const Domain& rhs);
    Domain& operator=(const Domain& rhs);

    Bool m_domainCreated;

    DptfManager* m_dptfManager;
    ParticipantInterface* m_theRealParticipant;

    UIntN m_participantIndex;
    UIntN m_domainIndex;
    Guid m_domainGuid;
    std::string m_domainName;
    DomainType::Type m_domainType;
    DomainFunctionalityVersions m_domainFunctionalityVersions;

    Arbitrator* m_arbitrator;

    //
    // Cached data.
    // FIXME:  consider adding a dirty bit to speed up clearing the cache.
    //

    // Active Controls
    ActiveControlStaticCaps* m_activeControlStaticCaps;
    ActiveControlStatus* m_activeControlStatus;
    ActiveControlSet* m_activeControlSet;

    // ConfigTdp controls
    ConfigTdpControlDynamicCaps* m_configTdpControlDynamicCaps;
    ConfigTdpControlStatus* m_configTdpControlStatus;
    ConfigTdpControlSet* m_configTdpControlSet;

    // Core controls
    CoreControlStaticCaps* m_coreControlStaticCaps;
    CoreControlDynamicCaps* m_coreControlDynamicCaps;
    CoreControlLpoPreference* m_coreControlLpoPreference;
    CoreControlStatus* m_coreControlStatus;

    // Display controls
    DisplayControlDynamicCaps* m_displayControlDynamicCaps;
    DisplayControlStatus* m_displayControlStatus;
    DisplayControlSet* m_displayControlSet;

    // Performance controls
    PerformanceControlStaticCaps* m_performanceControlStaticCaps;
    PerformanceControlDynamicCaps* m_performanceControlDynamicCaps;
    PerformanceControlStatus* m_performanceControlStatus;
    PerformanceControlSet* m_performanceControlSet;

    // Pixel Clock Control
    // *** nothing to cache

    // Pixel Clock Status
    PixelClockCapabilities* m_pixelClockCapabilities;
    PixelClockDataSet* m_pixelClockDataSet;

    // Power controls
    PowerControlDynamicCapsSet* m_powerControlDynamicCapsSet;
    PowerControlStatusSet* m_powerControlStatusSet;

    // Power status
    PowerStatus* m_powerStatus;

    // priority
    DomainPriority* m_domainPriority;

    // RF Profile Control
    RfProfileCapabilities* m_rfProfileCapabilities;

    // RF Profile Status
    RfProfileData* m_rfProfileData;

    // temperature
    TemperatureStatus* m_temperatureStatus;
    TemperatureThresholds* m_temperatureThresholds;

    // utilization
    UtilizationStatus* m_utilizationStatus;

    void clearDomainCachedDataActiveControl();
    void clearDomainCachedDataConfigTdpControl();
    void clearDomainCachedDataCoreControl();
    void clearDomainCachedDataDisplayControl();
    void clearDomainCachedDataPerformanceControl();
    //void clearDomainCachedDataPixelClockControl(); *** Nothing to cache ***
    void clearDomainCachedDataPixelClockStatus();
    void clearDomainCachedDataPowerControl();
    void clearDomainCachedDataPowerStatus();
    void clearDomainCachedDataPriority();
    void clearDomainCachedDataRfProfileControl();
    void clearDomainCachedDataRfProfileStatus();
    void clearDomainCachedDataTemperature();
    void clearDomainCachedDataUtilizationStatus();
};