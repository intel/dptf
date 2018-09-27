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

#include "ParticipantProxy.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

ParticipantProxy::ParticipantProxy()
	: m_policyServices()
	, m_index(Constants::Invalid)
	, m_participantProperties(m_policyServices, Constants::Invalid)
	, m_criticalTripPointProperty(m_policyServices, Constants::Invalid)
	, m_activeTripPointProperty(m_policyServices, Constants::Invalid)
	, m_passiveTripPointProperty(m_policyServices, Constants::Invalid)
	, m_domainSetProperty(m_policyServices, Constants::Invalid)
	, m_previousLowerBound(Temperature::createInvalid())
	, m_previousUpperBound(Temperature::createInvalid())
	, m_lastIndicationTemperatureLowerBound(Temperature::createInvalid())
	, m_lastThresholdCrossedTemperature(Temperature::createInvalid())
	, m_timeOfLastThresholdCrossed(TimeSpan::createInvalid())
{
}

ParticipantProxy::ParticipantProxy(
	UIntN participantIndex,
	const PolicyServicesInterfaceContainer& policyServices,
	std::shared_ptr<TimeInterface> time)
	: m_policyServices(policyServices)
	, m_time(time)
	, m_index(participantIndex)
	, m_participantProperties(policyServices, participantIndex)
	, m_criticalTripPointProperty(policyServices, participantIndex)
	, m_activeTripPointProperty(policyServices, participantIndex)
	, m_passiveTripPointProperty(policyServices, participantIndex)
	, m_domainSetProperty(policyServices, participantIndex)
	, m_previousLowerBound(Temperature::createInvalid())
	, m_previousUpperBound(Temperature::createInvalid())
	, m_lastIndicationTemperatureLowerBound(Temperature::createInvalid())
	, m_lastThresholdCrossedTemperature(Temperature::createInvalid())
	, m_timeOfLastThresholdCrossed(TimeSpan::createInvalid())
{
	m_participantProperties.refresh();
}

ParticipantProxy::~ParticipantProxy()
{
}

UIntN ParticipantProxy::getIndex() const
{
	return m_index;
}

void ParticipantProxy::refreshDomainProperties()
{
	m_domainSetProperty.refresh();
}

const DomainPropertiesSet& ParticipantProxy::getDomainPropertiesSet()
{
	return m_domainSetProperty.getDomainPropertiesSet();
}

const ParticipantProperties& ParticipantProxy::getParticipantProperties()
{
	return m_participantProperties.getParticipantProperties();
}

CriticalTripPointsCachedProperty& ParticipantProxy::getCriticalTripPointProperty()
{
	return m_criticalTripPointProperty;
}

ActiveTripPointsCachedProperty& ParticipantProxy::getActiveTripPointProperty()
{
	return m_activeTripPointProperty;
}

PassiveTripPointsCachedProperty& ParticipantProxy::getPassiveTripPointProperty()
{
	return m_passiveTripPointProperty;
}

Bool ParticipantProxy::domainExists(UIntN domainIndex)
{
	return (m_domains.find(domainIndex) != m_domains.end());
}

std::vector<UIntN> ParticipantProxy::getDomainIndexes()
{
	std::vector<UIntN> domainIndexes;
	for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
	{
		domainIndexes.push_back(domain->first);
	}
	return domainIndexes;
}

void ParticipantProxy::bindDomain(std::shared_ptr<DomainProxyInterface> domain)
{
	m_domains[domain->getDomainIndex()] = domain;
}

void ParticipantProxy::unbindDomain(UIntN domainIndex)
{
	m_domains.erase(domainIndex);
}

void ParticipantProxy::setTemperatureThresholds(const Temperature& lowerBound, const Temperature& upperBound)
{
	if (m_domains.find(0) != m_domains.end())
	{
		if (m_domains[0]->getTemperatureControl()->supportsTemperatureThresholds())
		{
			m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(
				FLF, "Setting thresholds to " + lowerBound.toString() + ":" + upperBound.toString() + "."));
			m_domains[0]->getTemperatureControl()->setTemperatureNotificationThresholds(lowerBound, upperBound);
		}
	}

	m_previousLowerBound = lowerBound;
	m_previousUpperBound = upperBound;
}

Bool ParticipantProxy::supportsTemperatureInterface()
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
	{
		if (domain->second->getTemperatureControl()->supportsTemperatureControls())
		{
			return true;
		}
	}
	return false;
}

Temperature ParticipantProxy::getFirstDomainTemperature()
{
	auto participantTemperature = Temperature::createInvalid();
	if (m_domains.size() > 0)
	{
		for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
		{
			if (domain->second->getTemperatureControl()->supportsTemperatureControls())
			{
				participantTemperature = domain->second->getTemperatureControl()->getCurrentTemperature();
				break;
			}
		}
	}
	return participantTemperature;
}

TemperatureThresholds ParticipantProxy::getTemperatureThresholds()
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
	{
		try
		{
			return domain->second->getTemperatureControl()->getTemperatureNotificationThresholds();
		}
		catch (...)
		{
			// swallow the error
		}
	}
	throw dptf_exception(
		"Failed to get temperature thresholds for participant " + std::to_string(getIndex()) + ".");
}

void ParticipantProxy::notifyPlatformOfDeviceTemperature(const Temperature& currentTemperature)
{
	try
	{
		if (m_lastIndicationTemperatureLowerBound != m_previousLowerBound)
		{
			m_lastIndicationTemperatureLowerBound = m_previousLowerBound;
			m_policyServices.participantSetSpecificInfo->setParticipantDeviceTemperatureIndication(
				getIndex(), currentTemperature);
		}
	}
	catch (dptf_exception& ex)
	{
		m_policyServices.messageLogging->writeMessageWarning(PolicyMessage(
			FLF,
			string("Set Device Temperature Indication failed with error \"") + string(ex.what()) + string("\"")
				+ string(".")));
	}
	catch (...)
	{
		m_policyServices.messageLogging->writeMessageWarning(
			PolicyMessage(FLF, string("Set Device Temperature Indication failed.")));
	}
}

void ParticipantProxy::setThresholdCrossed(const Temperature& temperature, const TimeSpan& timestamp)
{
	m_lastThresholdCrossedTemperature = temperature;
	m_timeOfLastThresholdCrossed = timestamp;
	m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(
		FLF,
		"Temperature threshold crossed for participant with temperature " + temperature.toString() + ".",
		getIndex()));
}

const TimeSpan& ParticipantProxy::getTimeOfLastThresholdCrossed() const
{
	return m_timeOfLastThresholdCrossed;
}

Temperature ParticipantProxy::getTemperatureOfLastThresholdCrossed() const
{
	return m_lastThresholdCrossedTemperature;
}

std::shared_ptr<DomainProxyInterface> ParticipantProxy::getDomain(UIntN domainIndex)
{
	auto domainProxyInterfacePtr = m_domains.at(domainIndex);
	if (!domainProxyInterfacePtr)
	{
		throw dptf_exception(
			std::string("The domain at the given index is not valid: ") + std::to_string(domainIndex));
	}

	return domainProxyInterfacePtr;
}

std::shared_ptr<XmlNode> ParticipantProxy::getXmlForCriticalTripPoints()
{
	auto participant = XmlNode::createWrapperElement("participant");
	participant->addChild(XmlNode::createDataElement("index", friendlyValue(m_index)));
	participant->addChild(
		XmlNode::createDataElement("name", m_participantProperties.getParticipantProperties().getName()));
	if (m_domains.find(0) != m_domains.end())
	{
		participant->addChild(
			XmlNode::createDataElement("temperature", getTemperatureForStatus(getDomain(0)).toString()));
	}
	else
	{
		participant->addChild(XmlNode::createDataElement("temperature", "Error"));
	}
	participant->addChild(m_criticalTripPointProperty.getXml());
	return participant;
}

std::shared_ptr<XmlNode> ParticipantProxy::getXmlForActiveTripPoints()
{
	auto participant = XmlNode::createWrapperElement("participant");
	participant->addChild(XmlNode::createDataElement("index", friendlyValue(m_index)));
	participant->addChild(
		XmlNode::createDataElement("name", m_participantProperties.getParticipantProperties().getName()));
	if (m_domains.find(0) != m_domains.end())
	{
		participant->addChild(
			XmlNode::createDataElement("temperature", getTemperatureForStatus(getDomain(0)).toString()));
	}
	else
	{
		participant->addChild(XmlNode::createDataElement("temperature", "Error"));
	}
	participant->addChild(getTemperatureThresholdsForStatus().getXml());
	participant->addChild(m_activeTripPointProperty.getXml());
	return participant;
}

std::shared_ptr<XmlNode> ParticipantProxy::getXmlForPassiveTripPoints()
{
	auto participant = XmlNode::createWrapperElement("participant");
	participant->addChild(XmlNode::createDataElement("index", friendlyValue(m_index)));
	participant->addChild(
		XmlNode::createDataElement("name", m_participantProperties.getParticipantProperties().getName()));
	if (m_domains.find(0) != m_domains.end())
	{
		participant->addChild(
			XmlNode::createDataElement("temperature", getTemperatureForStatus(getDomain(0)).toString()));
	}
	else
	{
		participant->addChild(XmlNode::createDataElement("temperature", "Error"));
	}
	participant->addChild(getTemperatureThresholdsForStatus().getXml());
	participant->addChild(m_passiveTripPointProperty.getXml());
	return participant;
}

std::shared_ptr<XmlNode> ParticipantProxy::getXmlForTripPointStatistics()
{
	auto stats = XmlNode::createWrapperElement("participant_trip_point_statistics");
	stats->addChild(XmlNode::createDataElement("participant_index", friendlyValue(m_index)));
	stats->addChild(
		XmlNode::createDataElement("participant_name", m_participantProperties.getParticipantProperties().getName()));
	if (m_domains.find(0) != m_domains.end())
	{
		Bool supportsTripPoints = getDomain(0)->getTemperatureControl()->supportsTemperatureControls();
		stats->addChild(XmlNode::createDataElement("supports_trip_points", friendlyValue(supportsTripPoints)));
	}
	else
	{
		stats->addChild(XmlNode::createDataElement("supports_trip_points", friendlyValue(false)));
	}

	if (m_timeOfLastThresholdCrossed.isInvalid() || m_timeOfLastThresholdCrossed.asMillisecondsInt() == 0)
	{
		stats->addChild(XmlNode::createDataElement("time_since_last_trip", Constants::InvalidString));
	}
	else
	{
		auto timeSinceLastTrip = m_time->getCurrentTime() - m_timeOfLastThresholdCrossed;
		stats->addChild(XmlNode::createDataElement("time_since_last_trip", timeSinceLastTrip.toStringSeconds()));
	}

	stats->addChild(
		XmlNode::createDataElement("temperature_of_last_trip", m_lastThresholdCrossedTemperature.toString()));

	return stats;
}

std::shared_ptr<XmlNode> ParticipantProxy::getXmlForConfigTdpLevel()
{
	auto status = XmlNode::createWrapperElement("participant_config_tdp_level");
	status->addChild(XmlNode::createDataElement("participant_index", StatusFormat::friendlyValue(getIndex())));
	status->addChild(XmlNode::createDataElement("participant_name", getParticipantProperties().getName()));
	for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
	{
		status->addChild(domain->second->getXmlForConfigTdpLevel());
	}
	return status;
}

Temperature ParticipantProxy::getTemperatureForStatus(std::shared_ptr<DomainProxyInterface> domainProxy)
{
	if (domainProxy->getTemperatureControl()->supportsTemperatureControls())
	{
		try
		{
			return domainProxy->getTemperatureControl()->getCurrentTemperature();
		}
		catch (...)
		{
			return Temperature::createInvalid();
		}
	}
	else
	{
		return Temperature::createInvalid();
	}
}

TemperatureThresholds ParticipantProxy::getTemperatureThresholdsForStatus()
{
	try
	{
		return getTemperatureThresholds();
	}
	catch (...)
	{
		return TemperatureThresholds::createInvalid();
	}
}

void ParticipantProxy::refreshHysteresis()
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
	{
		if (domain->second->getTemperatureControl()->supportsTemperatureThresholds())
		{
			domain->second->getTemperatureControl()->refreshHysteresis();
		}
	}
}

void ParticipantProxy::refreshVirtualSensorTables()
{
	for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
	{
		if (domain->second->getTemperatureControl()->supportsTemperatureControls())
		{
			domain->second->getTemperatureControl()->refreshVirtualSensorTables();
		}
	}
}
