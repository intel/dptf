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
#include "PolicyServicesInterfaceContainer.h"
#include "DomainSetCachedProperty.h"
#include "ParticipantPropertiesCachedProperty.h"
#include "CriticalTripPointsCachedProperty.h"
#include "ActiveTripPointsCachedProperty.h"
#include "PassiveTripPointsCachedProperty.h"
#include "DomainProxyInterface.h"

// represents a participant.  contains cached records of participant properties and a list of domains contained within
// the participant.
class dptf_export ParticipantProxyInterface
{
public:

    virtual ~ParticipantProxyInterface() {};

    // domain access
    virtual void bindDomain(std::shared_ptr<DomainProxyInterface> domain) = 0;
    virtual void unbindDomain(UIntN domainIndex) = 0;
    virtual std::shared_ptr<DomainProxyInterface> getDomain(UIntN domainIndex) = 0;
    virtual std::vector<UIntN> getDomainIndexes() = 0;
    virtual Bool domainExists(UIntN domainIndex) = 0;

    // properties
    virtual UIntN getIndex() const = 0;
    virtual void refreshDomainProperties() = 0;
    virtual const DomainPropertiesSet& getDomainPropertiesSet() = 0;
    virtual const ParticipantProperties& getParticipantProperties() = 0;

    // capabilities
    virtual Bool supportsTemperatureInterface() = 0;
    virtual Temperature getFirstDomainTemperature() = 0;
    virtual void setTemperatureThresholds(const Temperature& lowerBound, const Temperature& upperBound) = 0;
    virtual TemperatureThresholds getTemperatureThresholds() = 0;
    virtual void refreshHysteresis() = 0;
    virtual void refreshVirtualSensorTables() = 0;
    virtual void notifyPlatformOfDeviceTemperature(const Temperature& currentTemperature) = 0;
    virtual void setThresholdCrossed(const Temperature& temperature, const TimeSpan& timestamp) = 0;
    virtual const TimeSpan& getTimeOfLastThresholdCrossed() const = 0;
    virtual Temperature getTemperatureOfLastThresholdCrossed() const = 0;

    virtual CriticalTripPointsCachedProperty& getCriticalTripPointProperty() = 0;
    virtual ActiveTripPointsCachedProperty& getActiveTripPointProperty() = 0;
    virtual PassiveTripPointsCachedProperty& getPassiveTripPointProperty() = 0;

    virtual std::shared_ptr<XmlNode> getXmlForCriticalTripPoints() = 0;
    virtual std::shared_ptr<XmlNode> getXmlForActiveTripPoints() = 0;
    virtual std::shared_ptr<XmlNode> getXmlForPassiveTripPoints() = 0;
    virtual std::shared_ptr<XmlNode> getXmlForConfigTdpLevel() = 0;
    virtual std::shared_ptr<XmlNode> getXmlForTripPointStatistics() = 0;
};