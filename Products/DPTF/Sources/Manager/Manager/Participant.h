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
#include "Domain.h"
#include "ParticipantInterface.h"
#include "ParticipantServices.h"

class XmlNode;

class Participant
{
public:

    Participant(DptfManager* dptfManager);
    ~Participant(void);

    void createParticipant(UIntN participantIndex, const AppParticipantDataPtr participantDataPtr,
        Bool participantEnabled);
    void destroyParticipant(void);

    void enableParticipant(void);
    void disableParticipant(void);
    Bool isParticipantEnabled(void);

    void allocateDomain(UIntN* newDomainIndex);
    void createDomain(UIntN domainIndex, const AppDomainDataPtr domainDataPtr, Bool domainEnabled);
    void destroyAllDomains(void);
    void destroyDomain(UIntN domainIndex);

    Bool isDomainValid(UIntN domainIndex);
    void enableDomain(UIntN domainIndex);
    void disableDomain(UIntN domainIndex);
    Bool isDomainEnabled(UIntN domainIndex);

    UIntN getDomainCount(void) const;
    UIntN getDomainIndex(Domain* domainPtr);

    // This will clear the cached data stored within the participant and associated domains within the framework.
    // It will not ask the actual participant to clear any of its data.
    void clearParticipantCachedData(void);

    void clearArbitrationDataForPolicy(UIntN policyIndex);

    void registerEvent(ParticipantEvent::Type participantEvent);
    void unregisterEvent(ParticipantEvent::Type participantEvent);
    Bool isEventRegistered(ParticipantEvent::Type participantEvent);

    std::string getParticipantName(void) const;
    std::string getDomainName(UIntN domainIndex);
    XmlNode* getXml(UIntN domainIndex) const;
    XmlNode* getStatusAsXml(UIntN domainIndex) const;

    //
    // Event handlers
    //
    void connectedStandbyEntry(void);
    void connectedStandbyExit(void);
    void suspend(void);
    void resume(void);
    void domainConfigTdpCapabilityChanged(void);
    void domainCoreControlCapabilityChanged(void);
    void domainDisplayControlCapabilityChanged(void);
    void domainDisplayStatusChanged(void);
    void domainPerformanceControlCapabilityChanged(void);
    void domainPerformanceControlsChanged(void);
    void domainPowerControlCapabilityChanged(void);
    void domainPriorityChanged(void);
    void domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus);
    void domainRfProfileChanged(void);
    void domainTemperatureThresholdCrossed(void);
    void participantSpecificInfoChanged(void);

    //
    // The following set of functions implement the ParticipantInterface related functionality
    //

    // Active Controls
    ActiveControlStaticCaps getActiveControlStaticCaps(UIntN domainIndex);
    ActiveControlStatus getActiveControlStatus(UIntN domainIndex);
    ActiveControlSet getActiveControlSet(UIntN domainIndex);
    void setActiveControl(UIntN domainIndex, UIntN policyIndex, UIntN controlIndex);
    void setActiveControl(UIntN domainIndex, UIntN policyIndex, const Percentage& fanSpeed);

    // ConfigTdp controls
    ConfigTdpControlDynamicCaps getConfigTdpControlDynamicCaps(UIntN domainIndex);
    ConfigTdpControlStatus getConfigTdpControlStatus(UIntN domainIndex);
    ConfigTdpControlSet getConfigTdpControlSet(UIntN domainIndex);
    void setConfigTdpControl(UIntN domainIndex, UIntN policyIndex, UIntN controlIndex);

    // Core controls
    CoreControlStaticCaps getCoreControlStaticCaps(UIntN domainIndex);
    CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN domainIndex);
    CoreControlLpoPreference getCoreControlLpoPreference(UIntN domainIndex);
    CoreControlStatus getCoreControlStatus(UIntN domainIndex);
    void setActiveCoreControl(UIntN domainIndex, UIntN policyIndex, const CoreControlStatus& coreControlStatus);

    // Display controls
    DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN domainIndex);
    DisplayControlStatus getDisplayControlStatus(UIntN domainIndex);
    DisplayControlSet getDisplayControlSet(UIntN domainIndex);
    void setDisplayControl(UIntN domainIndex, UIntN policyIndex, UIntN displayControlIndex, Bool isOverridable);

    // Performance controls
    PerformanceControlStaticCaps getPerformanceControlStaticCaps(UIntN domainIndex);
    PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN domainIndex);
    PerformanceControlStatus getPerformanceControlStatus(UIntN domainIndex);
    PerformanceControlSet getPerformanceControlSet(UIntN domainIndex);
    void setPerformanceControl(UIntN domainIndex, UIntN policyIndex, UIntN performanceControlIndex);

    // Pixel Clock Control
    void setPixelClockControl(UIntN domainIndex, UIntN policyIndex, const PixelClockDataSet& pixelClockDataSet);

    // Pixel Clock Status
    PixelClockCapabilities getPixelClockCapabilities(UIntN domainIndex);
    PixelClockDataSet getPixelClockDataSet(UIntN domainIndex);

    // Power controls
    PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN domainIndex);
    PowerControlStatusSet getPowerControlStatusSet(UIntN domainIndex);
    void setPowerControl(UIntN domainIndex, UIntN policyIndex, const PowerControlStatusSet& powerControlStatusSet);

    // Power status
    PowerStatus getPowerStatus(UIntN domainIndex);

    // priority
    DomainPriority getDomainPriority(UIntN domainIndex);

    // RF Profile Control
    RfProfileCapabilities getRfProfileCapabilities(UIntN domainIndex);
    void setRfProfileCenterFrequency(UIntN domainIndex, UIntN policyIndex, const Frequency& centerFrequency);

    // RF Profile Status
    RfProfileData getRfProfileData(UIntN domainIndex);

    // temperature
    TemperatureStatus getTemperatureStatus(UIntN domainIndex);
    TemperatureThresholds getTemperatureThresholds(UIntN domainIndex);
    void setTemperatureThresholds(UIntN domainIndex, UIntN policyIndex, const TemperatureThresholds& temperatureThresholds);

    // utilization
    UtilizationStatus getUtilizationStatus(UIntN domainIndex);

    // Get specific info
    std::map<ParticipantSpecificInfoKey::Type, UIntN> getParticipantSpecificInfo(
        const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo);

    // Participant properties
    ParticipantProperties getParticipantProperties(void);
    DomainPropertiesSet getDomainPropertiesSet(void);

    // Set specific info
    void setParticipantDeviceTemperatureIndication(const Temperature& temperature);
    void setParticipantCoolingPolicy(const CoolingPreference& coolingPreference);

private:

    // hide the copy constructor and assignment operator.
    Participant(const Participant& rhs);
    Participant& operator=(const Participant& rhs);

    Bool m_participantCreated;

    DptfManager* m_dptfManager;
    ParticipantInterface* m_theRealParticipant;
    ParticipantServicesInterface* m_participantServices;

    UIntN m_participantIndex;
    Guid m_participantGuid;
    std::string m_participantName;

    // track the events that will be forwarded to the participant
    std::bitset<ParticipantEvent::Max> m_registeredEvents;

    std::vector<Domain*> m_domain;

    void throwIfDomainIndexInvalid(UIntN domainIndex);
};