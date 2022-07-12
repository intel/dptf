/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "PolicyCallbackScheduler.h"

using namespace std;

const Guid MyGuid(0xE7, 0x8A, 0xC6, 0x97, 0xFA, 0x15, 0x9C, 0x49, 0xB8, 0xC9, 0x5D, 0xA8, 0x1D, 0x60, 0x6E, 0x0A);
const string MyName("Critical Policy");
const TimeSpan WatchdogTime = TimeSpan::createFromSeconds(15);

CriticalPolicy::CriticalPolicy(void)
	: PolicyBase()
	, m_sleepRequested(false)
	, m_hibernateRequested(false)
	, m_inEmergencyCallMode(false)
	, m_isTimerStarted(false)
{
}

CriticalPolicy::~CriticalPolicy(void)
{
}

void CriticalPolicy::onCreate(void)
{
	m_scheduler.reset(new PolicyCallbackScheduler(getPolicyServices(), getTime()));

	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyOperatingSystemMobileNotification);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::ParticipantSpecificInfoChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DptfResume);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyInitiatedCallback);
}

void CriticalPolicy::onDestroy(void)
{
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::PolicyOperatingSystemMobileNotification);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::ParticipantSpecificInfoChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DptfResume);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::PolicyInitiatedCallback);

	stopTimer();
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
	stopTimer();
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

Bool CriticalPolicy::hasCriticalShutdownCapability() const
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
	auto root = XmlNode::createRoot();
	auto format = XmlNode::createComment("format_id=" + getGuid().toString());
	root->addChild(format);
	auto status = XmlNode::createWrapperElement("critical_policy_status");
	status->addChild(getXmlForCriticalTripPoints());
	root->addChild(status);
	string statusString = root->toString();
	return statusString;
}

string CriticalPolicy::getDiagnosticsAsXml(void) const
{
	auto status = XmlNode::createWrapperElement("critical_policy_diagnostics");
	status->addChild(getXmlForCriticalTripPoints());
	auto root = XmlNode::createRoot();
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
	POLICY_LOG_MESSAGE_DEBUG(
		{ return "Considering actions based on temperature of " + currentTemperature.toString() + "."; });
	auto tripPoints = participant->getCriticalTripPointProperty().getTripPoints();
	setParticipantTemperatureThresholdNotification(currentTemperature, tripPoints.getSortedByValue(), participant);
	auto tripPointCrossed = findTripPointCrossed(tripPoints.getSortedByValue(), currentTemperature);
	Temperature crossedTripPointTemperature = Temperature::createInvalid();
	if (tripPointCrossed != ParticipantSpecificInfoKey::None)
	{
		crossedTripPointTemperature = tripPoints.getTemperature(tripPointCrossed);
	}
	takePowerAction(
		currentTemperature,
		tripPointCrossed,
		crossedTripPointTemperature,
		participant->getParticipantProperties().getName());
}

void CriticalPolicy::takePowerAction(
	const Temperature& currentTemperature,
	ParticipantSpecificInfoKey::Type crossedTripPoint,
	const Temperature& crossedTripPointTemperature,
	const std::string& participantName)
{
	if (m_inEmergencyCallMode)
	{
		POLICY_LOG_MESSAGE_DEBUG({
			return "Participant crossed the " + ParticipantSpecificInfoKey::ToString(crossedTripPoint)
				   + " trip point but no action is being taken since system is in emergency call mode.";
		});
		return;
	}

	switch (crossedTripPoint)
	{
	case ParticipantSpecificInfoKey::Warm:
		if (!m_sleepRequested)
		{
			POLICY_LOG_MESSAGE_DEBUG({ return "Instructing system to sleep."; });
			m_sleepRequested = true;
			getPolicyServices().platformPowerState->sleep();
			startTimer(WatchdogTime);
		}
		else
		{
			POLICY_LOG_MESSAGE_DEBUG({ return "Sleep has already been requested. Nothing to do."; });
		}
		break;
	case ParticipantSpecificInfoKey::Hot:
		if (!m_hibernateRequested)
		{
			POLICY_LOG_MESSAGE_DEBUG({
				return "Instructing system to hibernate. Current temperature is " + currentTemperature.toString()
					   + ". Trip point temperature is " + crossedTripPointTemperature.toString() + ".";
			});
			m_hibernateRequested = true;
			getPolicyServices().platformPowerState->hibernate(
				currentTemperature, crossedTripPointTemperature, participantName);
			startTimer(WatchdogTime);
		}
		else
		{
			POLICY_LOG_MESSAGE_DEBUG({ return "Hibernate has already been requested. Nothing to do."; });
		}
		break;
	case ParticipantSpecificInfoKey::Critical:
		POLICY_LOG_MESSAGE_DEBUG({
			return "Instructing system to shut down. Current temperature is " + currentTemperature.toString()
				   + ". Trip point temperature is " + crossedTripPointTemperature.toString() + ".";
		});
		getPolicyServices().platformPowerState->shutDown(
			currentTemperature, crossedTripPointTemperature, participantName);

		startTimer(WatchdogTime);
		break;
	case ParticipantSpecificInfoKey::None:
		POLICY_LOG_MESSAGE_DEBUG({ return "No power action needed."; });
		break;
	default:
		throw dptf_exception("An invalid trip point has been selected.");
	}
}

void CriticalPolicy::onPolicyInitiatedCallback(UInt64 eventCode, UInt64 param1, void* param2)
{
	if (eventCode == EventCode::Timer)
	{
		m_sleepRequested = false;
		m_hibernateRequested = false;
		m_isTimerStarted = false;
		reEvaluateAllParticipants();
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

Bool CriticalPolicy::participantHasDesiredProperties(ParticipantProxyInterface* newParticipant) const
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
		try
		{
			ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*participantIndex);
			if (participantHasDesiredProperties(participant))
			{
				try
				{
					allStatus->addChild(participant->getXmlForCriticalTripPoints());
				}
				catch (dptf_exception&)
				{
					// TODO: want to pass in participant index instead
					POLICY_LOG_MESSAGE_ERROR({
						std::stringstream message;
						message << "Failed to get critical trip point status for participant. ParticipantIndex:"
								<< *participantIndex;
						return message.str();
					});
				}
			}
		}
		catch (const std::exception& ex)
		{
			// TODO: want to pass in participant index instead
			POLICY_LOG_MESSAGE_INFO_EX({
				std::stringstream message;
				message << ex.what() << ". ParticipantIndex = " << *participantIndex;
				return message.str();
			});
		}
		catch (...)
		{
			// TODO: want to pass in participant index instead
			POLICY_LOG_MESSAGE_INFO({
				std::stringstream message;
				message << "Failed to retrieve participant for critical policy status. ParticipantIndex = "
						<< *participantIndex;
				return message.str();
			});
		}
	}
	return allStatus;
}

/* Timer functions */
void CriticalPolicy::startTimer(const TimeSpan& timeValue)
{
	if (m_isTimerStarted)
	{
		stopTimer();
	}

	m_scheduler->setTimerForObject(static_cast<void*>(this), timeValue);
	m_isTimerStarted = true;
}

void CriticalPolicy::stopTimer()
{
	if (m_scheduler != nullptr)
	{
		m_scheduler->cancelTimerForObject(static_cast<void*>(this));
		m_isTimerStarted = false;
	}
}
