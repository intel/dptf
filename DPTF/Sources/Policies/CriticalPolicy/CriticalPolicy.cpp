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

#include "CriticalPolicy.h"
#include "Constants.h"
#include "XmlNode.h"
#include "DomainProxy.h"

using namespace std;

const Guid MyGuid(0xE7, 0x8A, 0xC6, 0x97, 0xFA, 0x15, 0x9C, 0x49, 0xB8, 0xC9, 0x5D, 0xA8, 0x1D, 0x60, 0x6E, 0x0A);
const string MyName("Critical Policy");

CriticalPolicy::CriticalPolicy(void)
	: PolicyBase()
	, m_sleepRequested(false)
	, m_hibernateRequested(false)
	, m_inEmergencyCallMode(false)
{
}

CriticalPolicy::~CriticalPolicy(void)
{
}

void CriticalPolicy::onCreate(void)
{
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyOperatingSystemMobileNotification);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::ParticipantSpecificInfoChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DptfResume);
}

void CriticalPolicy::onDestroy(void)
{
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::PolicyOperatingSystemMobileNotification);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::ParticipantSpecificInfoChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DptfResume);
}

void CriticalPolicy::onEnable(void)
{
}

void CriticalPolicy::onDisable(void)
{
}

void CriticalPolicy::onResume(void)
{
	m_sleepRequested = false;
	m_hibernateRequested = false;
}

void CriticalPolicy::onConnectedStandbyEntry(void)
{
}

void CriticalPolicy::onConnectedStandbyExit(void)
{
}

Bool CriticalPolicy::autoNotifyPlatformOscOnCreateDestroy() const
{
	return true;
}

Bool CriticalPolicy::autoNotifyPlatformOscOnConnectedStandbyEntryExit() const
{
	return false;
}

Bool CriticalPolicy::autoNotifyPlatformOscOnEnableDisable() const
{
	return true;
}

Guid CriticalPolicy::getGuid(void) const
{
	return MyGuid;
}

string CriticalPolicy::getName(void) const
{
	return MyName;
}

string CriticalPolicy::getStatusAsXml(void) const
{
	auto format = XmlNode::createComment("format_id=" + getGuid().toString());
	auto status = XmlNode::createWrapperElement("critical_policy_status");
	status->addChild(m_stats.getXml());
	status->addChild(getXmlForCriticalTripPoints());
	auto root = XmlNode::createRoot();
	root->addChild(format);
	root->addChild(status);
	string statusString = root->toString();
	return statusString;
}

void CriticalPolicy::onBindParticipant(UIntN participantIndex)
{
	getParticipantTracker()->remember(participantIndex);
}

void CriticalPolicy::onUnbindParticipant(UIntN participantIndex)
{
	getParticipantTracker()->forget(participantIndex);
}

void CriticalPolicy::onBindDomain(UIntN participantIndex, UIntN domainIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		participant->refreshDomainProperties();
		auto domain = std::make_shared<DomainProxy>(domainIndex, participant, getPolicyServices());
		participant->bindDomain(domain);

		if (participantHasDesiredProperties(participant))
		{
			takePowerActionBasedOnThermalState(participant);
		}
	}
}

void CriticalPolicy::onUnbindDomain(UIntN participantIndex, UIntN domainIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		participant->refreshDomainProperties();
		participant->unbindDomain(domainIndex);
	}
}

void CriticalPolicy::onOperatingSystemEmergencyCallModeChanged(OnOffToggle::Type emergencyCallMode)
{
	if (emergencyCallMode == OnOffToggle::On)
	{
		m_inEmergencyCallMode = true;
	}
	else
	{
		if (m_inEmergencyCallMode)
		{
			m_inEmergencyCallMode = false;
			reEvaluateAllParticipants();
		}
	}
}

void CriticalPolicy::onParticipantSpecificInfoChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		auto oldTrips = participant->getCriticalTripPointProperty().getTripPoints();

		participant->getCriticalTripPointProperty().refresh();
		participant->refreshHysteresis();

		auto newTrips = participant->getCriticalTripPointProperty().getTripPoints();
		if (oldTrips != newTrips && participantHasDesiredProperties(participant))
		{
			takePowerActionBasedOnThermalState(participant);
		}
	}
}

void CriticalPolicy::onDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex)
		&& participantHasDesiredProperties(getParticipantTracker()->getParticipant(participantIndex)))
	{
		takePowerActionBasedOnThermalState(getParticipantTracker()->getParticipant(participantIndex));
	}
}

void CriticalPolicy::takePowerActionBasedOnThermalState(ParticipantProxyInterface* participant)
{
	auto currentTemperature = participant->getFirstDomainTemperature();
	getPolicyServices().messageLogging->writeMessageDebug(
		PolicyMessage(FLF, "Considering actions based on temperature of " + currentTemperature.toString() + "."));
	auto tripPoints = participant->getCriticalTripPointProperty().getTripPoints();
	setParticipantTemperatureThresholdNotification(currentTemperature, tripPoints.getSortedByValue(), participant);
	auto tripPointCrossed = findTripPointCrossed(tripPoints.getSortedByValue(), currentTemperature);
	Temperature crossedTripPointTemperature = Temperature::createInvalid();
	if (tripPointCrossed != ParticipantSpecificInfoKey::None)
	{
		crossedTripPointTemperature = tripPoints.getTemperature(tripPointCrossed);
	}
	takePowerAction(currentTemperature, tripPointCrossed, crossedTripPointTemperature);
}

void CriticalPolicy::takePowerAction(
	const Temperature& currentTemperature,
	ParticipantSpecificInfoKey::Type crossedTripPoint,
	const Temperature& crossedTripPointTemperature)
{
	if (m_inEmergencyCallMode)
	{
		std::string debugMessage = "Participant crossed the " + ParticipantSpecificInfoKey::ToString(crossedTripPoint)
								   + " trip point but no action is being taken since system is in emergency call mode.";
		getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, debugMessage));
		return;
	}

	switch (crossedTripPoint)
	{
	case ParticipantSpecificInfoKey::Warm:
		if (!m_sleepRequested)
		{
			getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Instructing system to sleep."));
			m_stats.sleepSignalled();
			m_sleepRequested = true;
			getPolicyServices().platformPowerState->sleep();
		}
		else
		{
			getPolicyServices().messageLogging->writeMessageDebug(
				PolicyMessage(FLF, "Sleep has already been requested. Nothing to do."));
		}
		break;
	case ParticipantSpecificInfoKey::Hot:
		if (!m_hibernateRequested)
		{
			getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
				FLF,
				string("Instructing system to hibernate. ") + string("Current temperature is ")
					+ currentTemperature.toString()
					+ string(". ")
					+ string("Trip point temperature is ")
					+ crossedTripPointTemperature.toString()
					+ string(".")));
			m_stats.hibernateSignalled();
			m_hibernateRequested = true;
			getPolicyServices().platformPowerState->hibernate(currentTemperature, crossedTripPointTemperature);
		}
		else
		{
			getPolicyServices().messageLogging->writeMessageDebug(
				PolicyMessage(FLF, "Hibernate has already been requested. Nothing to do."));
		}
		break;
	case ParticipantSpecificInfoKey::Critical:
		getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
			FLF,
			string("Instructing system to shut down. ") + string("Current temperature is ")
				+ currentTemperature.toString()
				+ string(". ")
				+ string("Trip point temperature is ")
				+ crossedTripPointTemperature.toString()
				+ string(".")));
		m_stats.shutdownSignalled();
		getPolicyServices().platformPowerState->shutDown(currentTemperature, crossedTripPointTemperature);
		break;
	case ParticipantSpecificInfoKey::None:
		getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "No power action needed."));
		break;
	default:
		throw dptf_exception("An invalid trip point has been selected.");
	}
}

void CriticalPolicy::reEvaluateAllParticipants()
{
	auto allParticipants = getParticipantTracker()->getAllTrackedIndexes();
	for (auto pIndex = allParticipants.begin(); pIndex != allParticipants.end(); pIndex++)
	{
		auto participant = getParticipantTracker()->getParticipant(*pIndex);
		if (participantHasDesiredProperties(participant))
		{
			takePowerActionBasedOnThermalState(participant);
		}
	}
}

void CriticalPolicy::setParticipantTemperatureThresholdNotification(
	Temperature currentTemperature,
	std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>> tripPoints,
	ParticipantProxyInterface* participant)
{
	Temperature lowerTemperatureThreshold = determineLowerTemperatureThreshold(currentTemperature, tripPoints);
	Temperature upperTemperatureThreshold = determineUpperTemperatureThreshold(currentTemperature, tripPoints);
	participant->setTemperatureThresholds(lowerTemperatureThreshold, upperTemperatureThreshold);
}

Temperature CriticalPolicy::determineLowerTemperatureThreshold(
	Temperature currentTemperature,
	std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>> tripPoints)
{
	Temperature lowerTemperatureThreshold(Temperature::createInvalid());
	for (auto tp = tripPoints.begin(); tp != tripPoints.end(); ++tp)
	{
		if (currentTemperature >= tp->second)
		{
			lowerTemperatureThreshold = tp->second;
		}
		else
		{
			break;
		}
	}
	return lowerTemperatureThreshold;
}

Temperature CriticalPolicy::determineUpperTemperatureThreshold(
	Temperature currentTemperature,
	std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>> tripPoints)
{
	Temperature upperTemperatureThreshold(Temperature::createInvalid());
	for (auto tp = tripPoints.begin(); tp != tripPoints.end(); ++tp)
	{
		if (currentTemperature < tp->second)
		{
			upperTemperatureThreshold = tp->second;
			break;
		}
	}
	return upperTemperatureThreshold;
}

ParticipantSpecificInfoKey::Type CriticalPolicy::findTripPointCrossed(
	const std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>>& tripPoints,
	const Temperature& currentTemperature)
{
	auto crossedTripPoint = ParticipantSpecificInfoKey::None;
	for (auto tp = tripPoints.begin(); tp != tripPoints.end(); ++tp)
	{
		if (currentTemperature >= tp->second)
		{
			if (tp->first > crossedTripPoint)
			{
				crossedTripPoint = tp->first;
			}
		}
		else
		{
			break;
		}
	}
	return crossedTripPoint;
}

Bool CriticalPolicy::participantHasDesiredProperties(ParticipantProxyInterface* newParticipant)
{
	return (
		newParticipant->supportsTemperatureInterface()
		&& newParticipant->getCriticalTripPointProperty().supportsProperty());
}

std::shared_ptr<XmlNode> CriticalPolicy::getXmlForCriticalTripPoints() const
{
	auto allStatus = XmlNode::createWrapperElement("critical_trip_point_status");
	vector<UIntN> participantTndexes = getParticipantTracker()->getAllTrackedIndexes();
	for (auto participantIndex = participantTndexes.begin(); participantIndex != participantTndexes.end();
		 participantIndex++)
	{
		ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*participantIndex);
		if (participant->getCriticalTripPointProperty().supportsProperty())
		{
			try
			{
				allStatus->addChild(participant->getXmlForCriticalTripPoints());
			}
			catch (dptf_exception)
			{
				getPolicyServices().messageLogging->writeMessageError(
					PolicyMessage(FLF, "Failed to get critical trip point status for participant.", *participantIndex));
			}
		}
	}
	return allStatus;
}
