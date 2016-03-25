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

#pragma once

#include "Dptf.h"
#include "ParticipantInterface.h"
#include "esif_sdk_iface_app.h"
#include "Arbitrator.h"
#include "PlatformPowerLimitType.h"

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
    Bool isCreated(void);

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
    void setDisplayControl(UIntN policyIndex, UIntN displayControlIndex);
    void setDisplayControlDynamicCaps(UIntN policyIndex, DisplayControlDynamicCaps newCapabilities);

    // Performance controls
    PerformanceControlStaticCaps getPerformanceControlStaticCaps(void);
    PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(void);
    PerformanceControlStatus getPerformanceControlStatus(void);
    PerformanceControlSet getPerformanceControlSet(void);
    void setPerformanceControl(UIntN policyIndex, UIntN performanceControlIndex);
    void setPerformanceControlDynamicCaps(UIntN policyIndex, PerformanceControlDynamicCaps newCapabilities);

    // Pixel Clock Control
    void setPixelClockControl(UIntN policyIndex, const PixelClockDataSet& pixelClockDataSet);

    // Pixel Clock Status
    PixelClockCapabilities getPixelClockCapabilities(void);
    PixelClockDataSet getPixelClockDataSet(void);

    // Power controls
    PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(void);
    void setPowerControlDynamicCapsSet(UIntN policyIndex, PowerControlDynamicCapsSet capsSet);
    Bool isPowerLimitEnabled(PowerControlType::Type controlType);
    Power getPowerLimit(PowerControlType::Type controlType);
    void setPowerLimit(UIntN policyIndex, PowerControlType::Type controlType,
        const Power& powerLimit);
    TimeSpan getPowerLimitTimeWindow(PowerControlType::Type controlType);
    void setPowerLimitTimeWindow(UIntN policyIndex, PowerControlType::Type controlType,
        const TimeSpan& timeWindow);
    Percentage getPowerLimitDutyCycle(PowerControlType::Type controlType);
    void setPowerLimitDutyCycle(UIntN policyIndex, PowerControlType::Type controlType,
        const Percentage& dutyCycle);

    // Power status
    PowerStatus getPowerStatus(void);

    // Platform Power Controls
    Bool isPlatformPowerLimitEnabled(PlatformPowerLimitType::Type limitType);
    Power getPlatformPowerLimit(PlatformPowerLimitType::Type limitType);
    void setPlatformPowerLimit(PlatformPowerLimitType::Type limitType, const Power& powerLimit);
    TimeSpan getPlatformPowerLimitTimeWindow(PlatformPowerLimitType::Type limitType);
    void setPlatformPowerLimitTimeWindow(PlatformPowerLimitType::Type limitType, const TimeSpan& timeWindow);
    Percentage getPlatformPowerLimitDutyCycle(PlatformPowerLimitType::Type limitType);
    void setPlatformPowerLimitDutyCycle(PlatformPowerLimitType::Type limitType, const Percentage& dutyCycle);

    // Platform Power Status
    Power getMaxBatteryPower(void);
    Power getPlatformRestOfPower(void);
    Power getAdapterPowerRating(void);
    DptfBuffer getBatteryStatus(void);
    DptfBuffer getBatteryInformation(void);
    PlatformPowerSource::Type getPlatformPowerSource(void);
    ChargerType::Type getChargerType(void);
    Power getACPeakPower(void);
    TimeSpan getACPeakTimeWindow(void);
    Power getPlatformBatterySteadyState(void);

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
    DptfBuffer getVirtualSensorCalibrationTable(void);
    DptfBuffer getVirtualSensorPollingTable(void);
    Bool isVirtualTemperature(void);
    void setVirtualTemperature(const Temperature& temperature);

    // utilization
    UtilizationStatus getUtilizationStatus(void);

    // Hardware Duty Cycle
    DptfBuffer getHardwareDutyCycleUtilizationSet(void);
    Bool isEnabledByPlatform(void);
    Bool isSupportedByPlatform(void);
    Bool isEnabledByOperatingSystem(void);
    Bool isSupportedByOperatingSystem(void);
    Bool isHdcOobEnabled(void);
    void setHdcOobEnable(const UInt8& hdcOobEnable);
    void setHardwareDutyCycle(const Percentage& dutyCycle);
    Percentage getHardwareDutyCycle(void);

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
    std::map<PowerControlType::Type, Bool> m_powerLimitEnabled;
    std::map<PowerControlType::Type, Power> m_powerLimit;
    std::map<PowerControlType::Type, TimeSpan> m_powerLimitTimeWindow;
    std::map<PowerControlType::Type, Percentage> m_powerLimitDutyCycle;

    // Power status
    PowerStatus* m_powerStatus;

    // Platform Power Controls
    std::map<PlatformPowerLimitType::Type, Bool> m_platformPowerLimitEnabled;
    std::map<PlatformPowerLimitType::Type, Power> m_platformPowerLimit;
    std::map<PlatformPowerLimitType::Type, TimeSpan> m_platformPowerLimitTimeWindow;
    std::map<PlatformPowerLimitType::Type, Percentage> m_platformPowerLimitDutyCycle;

    // Platform Power Status
    Power* m_maxBatteryPower;
    Power* m_adapterRating;
    Power* m_platformRestOfPower;
    Power* m_acPeakPower;
    TimeSpan* m_acPeakTimeWindow;
    PlatformPowerSource::Type* m_platformPowerSource;
    ChargerType::Type* m_chargerType;
    DptfBuffer m_batteryStatusBuffer;
    DptfBuffer m_batteryInformationBuffer;
    Power* m_batterySteadyState;

    // priority
    DomainPriority* m_domainPriority;

    // RF Profile Control
    RfProfileCapabilities* m_rfProfileCapabilities;

    // RF Profile Status
    RfProfileData* m_rfProfileData;

    // temperature
    TemperatureStatus* m_temperatureStatus;
    TemperatureThresholds* m_temperatureThresholds;
    DptfBuffer m_virtualSensorCalculationTableBuffer;
    DptfBuffer m_virtualSensorPollingTableBuffer;
    Bool* m_isVirtualTemperature;

    // utilization
    UtilizationStatus* m_utilizationStatus;

    // Hardware Duty Cycle
    DptfBuffer m_hardwareDutyCycleUtilizationSet;
    Percentage* m_hardwareDutyCycle;
    Bool* m_isEnabledByPlatform;
    Bool* m_isSupportedByPlatform;
    Bool* m_isEnabledByOperatingSystem;
    Bool* m_isSupportedByOperatingSystem;
    Bool* m_isHdcOobEnabled;

    void clearDomainCachedDataActiveControl();
    void clearDomainCachedDataConfigTdpControl();
    void clearDomainCachedDataCoreControl();
    void clearDomainCachedDataDisplayControl();
    void clearDomainCachedDataPerformanceControl();
    //void clearDomainCachedDataPixelClockControl(); *** Nothing to cache ***
    void clearDomainCachedDataPixelClockStatus();
    void clearDomainCachedDataPowerControl();
    void clearDomainCachedDataPowerStatus();
    void clearDomainCachedDataPlatformPowerControl();
    void clearDomainCachedDataPriority();
    void clearDomainCachedDataRfProfileControl();
    void clearDomainCachedDataRfProfileStatus();
    void clearDomainCachedDataTemperature();
    void clearDomainCachedDataUtilizationStatus();
    void clearDomainCachedDataHardwareDutyCycle();
    void clearDomainCachedDataHdcOobEnable();
    void clearDomainCachedDataPlatformPowerStatus();
};