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

#pragma once

#include "Dptf.h"
#include "PolicyServicesInterfaceContainer.h"
#include "DomainSetCachedProperty.h"
#include "ParticipantPropertiesCachedProperty.h"
#include "CriticalTripPointsCachedProperty.h"
#include "ActiveTripPointsCachedProperty.h"
#include "PassiveTripPointsCachedProperty.h"
#include "DomainProxyInterface.h"
#include "TimeInterface.h"
#include "ParticipantProxyInterface.h"

// represents a participant.  contains cached records of participant properties and a list of domains contained within
// the participant.
class dptf_export ParticipantProxy : public ParticipantProxyInterface
{
public:
	ParticipantProxy();
	ParticipantProxy(
		UIntN participantIndex,
		const PolicyServicesInterfaceContainer& policyServices,
		const std::shared_ptr<TimeInterface>& time);
	~ParticipantProxy() override = default;

	// domain access
	virtual void bindDomain(std::shared_ptr<DomainProxyInterface> domain) override;
	virtual void unbindDomain(UIntN domainIndex) override;
	virtual std::shared_ptr<DomainProxyInterface> getDomain(UIntN domainIndex) override;
	virtual std::vector<UIntN> getDomainIndexes() override;
	virtual Bool domainExists(UIntN domainIndex) override;

	// properties
	virtual UIntN getIndex() const override;
	virtual void refreshDomainProperties() override;
	virtual const DomainPropertiesSet& getDomainPropertiesSet() override;
	virtual const ParticipantProperties& getParticipantProperties() override;

	// trip point sets
	virtual CriticalTripPointsCachedProperty& getCriticalTripPointProperty() override;
	virtual ActiveTripPointsCachedProperty& getActiveTripPointProperty() override;
	virtual PassiveTripPointsCachedProperty& getPassiveTripPointProperty() override;
	virtual std::shared_ptr<XmlNode> getXmlForCriticalTripPoints() override;
	virtual std::shared_ptr<XmlNode> getXmlForActiveTripPoints() override;
	virtual std::shared_ptr<XmlNode> getXmlForPassiveTripPoints() override;

	// temperatures
	virtual Bool supportsTemperatureInterface() override;
	virtual Temperature getFirstDomainTemperature() override;
	virtual void setTemperatureThresholds(const Temperature& lowerBound, const Temperature& upperBound) override;
	virtual TemperatureThresholds getTemperatureThresholds() override;
	virtual void notifyPlatformOfDeviceTemperature(const Temperature& currentTemperature) override;
	virtual std::shared_ptr<XmlNode> getXmlForTripPointStatistics() override;
	virtual void refreshHysteresis() override;

	// thresholds
	virtual void setThresholdCrossed(const Temperature& temperature, const TimeSpan& timestamp) override;
	virtual const TimeSpan& getTimeOfLastThresholdCrossed() const override;
	virtual Temperature getTemperatureOfLastThresholdCrossed() const override;

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;
	std::shared_ptr<TimeInterface> m_time;

	// participant properties
	UIntN m_index;
	ParticipantPropertiesCachedProperty m_participantProperties;
	CriticalTripPointsCachedProperty m_criticalTripPointProperty;
	ActiveTripPointsCachedProperty m_activeTripPointProperty;
	PassiveTripPointsCachedProperty m_passiveTripPointProperty;

	// domain properties
	DomainSetCachedProperty m_domainSetProperty;
	std::map<UIntN, std::shared_ptr<DomainProxyInterface>> m_domains;

	// Temperatures
	static Temperature getTemperatureForStatus(const std::shared_ptr<DomainProxyInterface>& domainProxy);
	TemperatureThresholds getTemperatureThresholdsForStatus();

	Temperature m_previousLowerBound;
	Temperature m_previousUpperBound;
	Temperature m_lastIndicationTemperatureLowerBound;
	Temperature m_lastThresholdCrossedTemperature;
	TimeSpan m_timeOfLastThresholdCrossed;
	const PolicyServicesInterfaceContainer& getPolicyServices() const;
};
