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
#include "PolicyServicesInterfaceContainer.h"
#include "DomainSetCachedProperty.h"
#include "ParticipantPropertiesCachedProperty.h"
#include "CriticalTripPointsCachedProperty.h"
#include "ActiveTripPointsCachedProperty.h"
#include "PassiveTripPointsCachedProperty.h"
#include "DomainProxy.h"
#include "TimeInterface.h"
#include <memory>
#include <map>

// represents a participant.  contains cached records of participant properties and a list of domains contained within
// the participant.
class dptf_export ParticipantProxy
{
public:

    ParticipantProxy();
    ParticipantProxy(
        UIntN participantIndex,
        const PolicyServicesInterfaceContainer& policyServices,
        std::shared_ptr<TimeInterface> time);
    ~ParticipantProxy();

    // domain access
    DomainProxy& bindDomain(UIntN domainIndex);
    void unbindDomain(UIntN domainIndex);
    DomainProxy& operator[](UIntN domainIndex);
    std::vector<UIntN> getDomainIndexes();

    // properties
    UIntN getIndex() const;
    const DomainPropertiesSet& getDomainPropertiesSet();
    const ParticipantProperties& getParticipantProperties();

    // trip point sets
    CriticalTripPointsCachedProperty& getCriticalTripPointProperty();
    ActiveTripPointsCachedProperty& getActiveTripPointProperty();
    PassiveTripPointsCachedProperty& getPassiveTripPointProperty();

    // temperatures
    Bool supportsTemperatureInterface();
    void setTemperatureThresholds(const Temperature& lowerBound, const Temperature& upperBound);
    TemperatureThresholds getTemperatureThresholds();
    void notifyPlatformOfDeviceTemperature(const Temperature& currentTemperature);

    // thresholds
    void setThresholdCrossed(const Temperature& temperature, UInt64 timestampInMs);
    UInt64 getTimeOfLastThresholdCrossed() const;
    Temperature getTemperatureOfLastThresholdCrossed() const;

    // status
    XmlNode* getXmlForCriticalTripPoints();
    XmlNode* getXmlForActiveTripPoints();
    XmlNode* getXmlForPassiveTripPoints();
    XmlNode* getXmlForPassiveControlKnobs();
    XmlNode* getXmlForTripPointStatistics();
    XmlNode* getXmlForConfigTdpLevel();
private:

    // participant properties
    UIntN m_index;
    ParticipantPropertiesCachedProperty m_participantProperties;
    CriticalTripPointsCachedProperty m_criticalTripPointProperty;
    ActiveTripPointsCachedProperty m_activeTripPointProperty;
    PassiveTripPointsCachedProperty m_passiveTripPointProperty;

    // domain properties
    DomainSetCachedProperty m_domainSetProperty;
    std::map<UIntN, DomainProxy> m_domains;
    void refreshDomainSetIfUninitialized();
    void refreshDomains();
    DomainProxy& getDomain(UIntN domainIndex);

    // Temperatures
    Temperature m_previousLowerBound;
    Temperature m_previousUpperBound;
    Temperature m_lastIndicationTemperatureLowerBound;
    Temperature m_lastThresholdCrossedTemperature;
    UInt64 m_timeOfLastThresholdCrossed;

    // services
    PolicyServicesInterfaceContainer m_policyServices;
    std::shared_ptr<TimeInterface> m_time;
};
