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

#include "Dptf.h"
#include "ParticipantInterface.h"
#include "UnifiedDomain.h"
#include "ClassFactories.h"
#include <vector>
#include <string>

class UnifiedParticipant final : public ParticipantInterface
{
public:

    // The default constructor will use the standard class factories
    UnifiedParticipant(void);

    // The second constructor allows the factories to be passed in for validation purposes
    UnifiedParticipant(const ClassFactories& classFactories);

    // The destructor will delete all of the class factory pointers
    ~UnifiedParticipant(void);

    // Participant
    virtual void createParticipant(const Guid& guid, UIntN participantIndex, Bool enabled,
        const std::string& name, const std::string& description, BusType::Type busType,
        const PciInfo& pciInfo, const AcpiInfo& acpiInfo,
        ParticipantServicesInterface* participantServicesInterface) override final;
    virtual void destroyParticipant(void) override final;
    virtual void enableParticipant(void) override final;
    virtual void disableParticipant(void) override final;
    virtual Bool isParticipantEnabled(void) override final;

    // Domain
    virtual void createDomain(const Guid& guid, UIntN participantIndex, UIntN domainIndex, Bool enabled,
        DomainType::Type domainType, const std::string& name, const std::string& description,
        DomainFunctionalityVersions domainFunctionalityVersions) override final;
    virtual void destroyDomain(const Guid& guid) override final;
    virtual void enableDomain(UIntN domainIndex) override final;
    virtual void disableDomain(UIntN domainIndex) override final;
    virtual Bool isDomainEnabled(UIntN domainIndex) override final;

    // Misc
    virtual std::string getName() const override final;
    virtual XmlNode* getXml(UIntN domainIndex) const override final;
    virtual XmlNode* getStatusAsXml(UIntN domainIndex) const override final;

    // Event handlers
    virtual void connectedStandbyEntry(void) override final;
    virtual void connectedStandbyExit(void) override final;
    virtual void domainConfigTdpCapabilityChanged(void) override final;
    virtual void domainCoreControlCapabilityChanged(void) override final;
    virtual void domainDisplayControlCapabilityChanged(void) override final;
    virtual void domainDisplayStatusChanged(void) override final;
    virtual void domainPerformanceControlCapabilityChanged(void) override final;
    virtual void domainPerformanceControlsChanged(void) override final;
    virtual void domainPowerControlCapabilityChanged(void) override final;
    virtual void domainPriorityChanged(void) override final;
    virtual void domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus) override final;
    virtual void domainRfProfileChanged(void) override final;
    virtual void domainTemperatureThresholdCrossed(void) override final;
    virtual void participantSpecificInfoChanged(void) override final;

    // Active Controls
    virtual ActiveControlStaticCaps getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual ActiveControlStatus getActiveControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual ActiveControlSet getActiveControlSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, UIntN controlIndex) override final;
    virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed) override final;

    // Config TDP Controls
    virtual ConfigTdpControlDynamicCaps getConfigTdpControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual ConfigTdpControlStatus getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual ConfigTdpControlSet getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setConfigTdpControl(UIntN participantIndex, UIntN domainIndex, UIntN controlIndex) override final;

    // Core Controls
    virtual CoreControlStaticCaps getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual CoreControlLpoPreference getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex) override final;
    virtual CoreControlStatus getCoreControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setActiveCoreControl(UIntN participantIndex, UIntN domainIndex, const CoreControlStatus& coreControlStatus) override final;

    // Display Controls
    virtual DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual DisplayControlStatus getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual DisplayControlSet getDisplayControlSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN controlLimit,
        Bool isOverridable) override final;

    // Performance Controls
    virtual PerformanceControlStaticCaps getPerformanceControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PerformanceControlStatus getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PerformanceControlSet getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex) override final;

    // Pixel Clock Control
    virtual void setPixelClockControl(UIntN participantIndex, UIntN domainIndex, const PixelClockDataSet& pixelClockDataSet) override final;

    // Pixel Clock Status
    virtual PixelClockCapabilities getPixelClockCapabilities(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PixelClockDataSet getPixelClockDataSet(UIntN participantIndex, UIntN domainIndex) override final;

    // Power Controls
    virtual PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual PowerControlStatusSet getPowerControlStatusSet(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setPowerControl(UIntN participantIndex, UIntN domainIndex, const PowerControlStatusSet& powerControlSet) override final;

    // Power Status
    virtual PowerStatus getPowerStatus(UIntN participantIndex, UIntN domainIndex) override final;

    // Domain Priority
    virtual DomainPriority getDomainPriority(UIntN participantIndex, UIntN domainIndex) override final;

    // RF Profile Control
    virtual RfProfileCapabilities getRfProfileCapabilities(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setRfProfileCenterFrequency(UIntN participantIndex, UIntN domainIndex, const Frequency& centerFrequency) override final;

    // RF Profile Status
    virtual RfProfileData getRfProfileData(UIntN participantIndex, UIntN domainIndex) override final;

    // Temperature
    virtual TemperatureStatus getTemperatureStatus(UIntN participantIndex, UIntN domainIndex) override final;
    virtual TemperatureThresholds getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex) override final;
    virtual void setTemperatureThresholds(UIntN participantIndex, UIntN domainIndex,
        const TemperatureThresholds& temperatureThresholds) override final;

    // Utilization
    virtual UtilizationStatus getUtilizationStatus(UIntN participantIndex, UIntN domainIndex) override final;

    //  Get Specific Info
    virtual std::map<ParticipantSpecificInfoKey::Type, UIntN> getParticipantSpecificInfo(
        UIntN participantIndex, const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo) override final;

    // ParticipantProperties
    virtual ParticipantProperties getParticipantProperties(UIntN participantIndex) override final;
    virtual DomainPropertiesSet getDomainPropertiesSet(UIntN participantIndex) override final;

    // Set Specific Info
    virtual void setParticipantDeviceTemperatureIndication(UIntN participantIndex, const Temperature& temperature) override final;
    virtual void setParticipantCoolingPolicy(UIntN participantIndex, const CoolingPreference& coolingPreference) override final;

private:

    // hide the copy constructor and = operator
    UnifiedParticipant(const UnifiedParticipant& rhs);
    UnifiedParticipant& operator=(const UnifiedParticipant& rhs);

    static const UIntN GetSpecificInfoVersionDefault = 1;
    static const UIntN SetSpecificInfoVersionDefault = 1;

    Guid m_guid;
    UIntN m_participantIndex;
    Bool m_enabled;
    std::string m_name;
    std::string m_description;
    BusType::Type m_busType;
    PciInfo m_pciInfo;
    AcpiInfo m_acpiInfo;
    ParticipantServicesInterface* m_participantServicesInterface;

    ParticipantGetSpecificInfoInterface* m_getSpecificInfo;
    ComponentExtendedInterface* m_getSpecificInfoEx;
    ParticipantSetSpecificInfoInterface* m_setSpecificInfo;
    ComponentExtendedInterface* m_setSpecificInfoEx;

    std::vector<UnifiedDomain*> m_domains;

    ClassFactories m_classFactories;

    void initialize(void);

    void createAllMissingClassFactories(void);
    void createDomainActiveControlFactoryIfMissing(void);
    void createDomainConfigTdpControlFactoryIfMissing(void);
    void createDomainCoreControlFactoryIfMissing(void);
    void createDomainDisplayControlFactoryIfMissing(void);
    void createDomainPerformanceControlFactoryIfMissing(void);
    void createDomainPixelClockControlFactoryIfMissing(void);
    void createDomainPixelClockStatusFactoryIfMissing(void);
    void createDomainPowerControlFactoryIfMissing(void);
    void createDomainPowerStatusFactoryIfMissing(void);
    void createDomainPriorityFactoryIfMissing(void);
    void createDomainRfProfileControlFactoryIfMissing(void);
    void createDomainRfProfileStatusFactoryIfMissing(void);
    void createDomainTemperatureFactoryIfMissing(void);
    void createDomainUtilizationFactoryIfMissing(void);
    void createParticipantSetSpecificInfoFactoryIfMissing(void);
    void createParticipantGetSpecificInfoFactoryIfMissing(void);

    void clearAllCachedData(void);
    void destroyAllDomains(void);
    void throwIfDomainIndexLocationInvalid(UIntN domainIndex);
    void insertDomainAtIndexLocation(UnifiedDomain* domain, UIntN domainIndex);

    Bool m_configTdpEventsRegistered;
    Bool m_coreControlEventsRegistered;
    Bool m_displayControlEventsRegistered;
    Bool m_domainPriorityEventsRegistered;
    Bool m_performanceControlEventsRegistered;
    Bool m_powerControlEventsRegistered;
    Bool m_rfProfileEventsRegistered;
    Bool m_temperatureThresholdEventsRegistered;

    void updateDomainEventRegistrations(void);
    Bool updateDomainEventRegistration(UIntN total, Bool currentlyRegistered, ParticipantEvent::Type participantEvent_0,
        ParticipantEvent::Type participantEvent_1 = ParticipantEvent::Type::Invalid);

    void throwIfDomainInvalid(UIntN domainIndex);

    void sendConfigTdpInfoToAllDomainsAndCreateNotification(void);
    ConfigTdpControlStatus getFirstConfigTdpControlStatus(void);
    ConfigTdpControlSet getFirstConfigTdpControlSet(void);
};