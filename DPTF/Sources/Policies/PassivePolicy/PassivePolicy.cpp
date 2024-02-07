/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "PassivePolicy.h"
#include <algorithm>
#include "PassiveControlStatus.h"
#include "TargetLimitAction.h"
#include "TargetUnlimitAction.h"
#include "TargetNoAction.h"
#include "TargetCheckLaterAction.h"
#include "PassiveDomainProxy.h"

using namespace std;

const Guid MyGuid(0xD6, 0x41, 0xA4, 0x42, 0x6A, 0xAE, 0x2B, 0x46, 0xA8, 0x4B, 0x4A, 0x8C, 0xE7, 0x90, 0x27, 0xD3);
const string MyName("Passive Policy");

PassivePolicy::PassivePolicy(void)
	: PolicyBase()
	, m_utilizationBiasThreshold(Percentage(0.0))
{
}

PassivePolicy::~PassivePolicy(void)
{
}

void PassivePolicy::onCreate(void)
{
	try
	{
		m_trt = std::make_shared<ThermalRelationshipTable>(ThermalRelationshipTable::createTrtFromDptfBuffer(
			getPolicyServices().platformConfigurationData->getThermalRelationshipTable()));
	}
	catch (std::exception& ex)
	{
		POLICY_LOG_MESSAGE_INFO_EX({ return "No thermal relationship table was found. " + string(ex.what()); });
		m_trt.reset(new ThermalRelationshipTable());
	}

	try
	{
		m_utilizationBiasThreshold =
			Percentage::fromWholeNumber(getPolicyServices().platformConfigurationData->readConfigurationUInt32(
				"PreferenceBiasUtilizationThreshold"));
	}
	catch (...)
	{
		m_utilizationBiasThreshold = Percentage(0.0);
	}

	m_callbackScheduler.reset(new CallbackScheduler(getPolicyServices(), m_trt, getTime()));

	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::ParticipantSpecificInfoChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainPowerControlCapabilityChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainPerformanceControlCapabilityChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainPerformanceControlsChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainCoreControlCapabilityChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainPriorityChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainDisplayControlCapabilityChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyThermalRelationshipTableChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyInitiatedCallback);
}

void PassivePolicy::onDestroy(void)
{
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::ParticipantSpecificInfoChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainPowerControlCapabilityChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(
		PolicyEvent::DomainPerformanceControlCapabilityChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainPerformanceControlsChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainCoreControlCapabilityChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainPriorityChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainDisplayControlCapabilityChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::PolicyThermalRelationshipTableChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::PolicyInitiatedCallback);
	if (m_callbackScheduler != nullptr)
	{
		m_callbackScheduler->cancelAllCallbackRequests();
	}
}

void PassivePolicy::onEnable(void)
{
	reloadTrtIfDifferent();
}

void PassivePolicy::onDisable(void)
{
	// no defined action for this event
}

void PassivePolicy::onConnectedStandbyEntry(void)
{
	// no defined action for this event
}

void PassivePolicy::onConnectedStandbyExit(void)
{
	// no defined action for this event
}

Bool PassivePolicy::autoNotifyPlatformOscOnCreateDestroy() const
{
	return true;
}

Bool PassivePolicy::autoNotifyPlatformOscOnConnectedStandbyEntryExit() const
{
	return false;
}

Bool PassivePolicy::autoNotifyPlatformOscOnEnableDisable() const
{
	return true;
}

Bool PassivePolicy::hasPassiveControlCapability() const
{
	return true;
}

Guid PassivePolicy::getGuid(void) const
{
	return MyGuid;
}

string PassivePolicy::getName(void) const
{
	return MyName;
}

string PassivePolicy::getStatusAsXml(void) const
{
	auto root = XmlNode::createRoot();
	auto format = XmlNode::createComment("format_id=" + getGuid().toString());
	root->addChild(format);
	auto status = XmlNode::createWrapperElement("passive_policy_status");
	status->addChild(getXmlForPassiveTripPoints());
	status->addChild(m_trt->getXml());
	PassiveControlStatus controlStatus(m_trt, getParticipantTracker());
	status->addChild(controlStatus.getXml());
	status->addChild(getXmlForTripPointStatistics(m_trt->getAllTargetIndexes()));
	status->addChild(m_callbackScheduler->getXml());
	status->addChild(XmlNode::createDataElement(
		"utilization_threshold", m_utilizationBiasThreshold.getCurrentUtilization().toString()));
	root->addChild(status);
	string statusString = root->toString();
	return statusString;
}

string PassivePolicy::getDiagnosticsAsXml(void) const
{
	return "Passive Policy Diagnostics Not Yet Implemented\n";
}

void PassivePolicy::onBindParticipant(UIntN participantIndex)
{
	getParticipantTracker()->remember(participantIndex);
	associateParticipantInTrt(getParticipantTracker()->getParticipant(participantIndex), m_trt);
	m_callbackScheduler->setTrt(m_trt);
}

void PassivePolicy::onUnbindParticipant(UIntN participantIndex)
{
	removeAllRequestsForTarget(participantIndex);
	m_trt->disassociateParticipant(participantIndex);
	m_callbackScheduler->removeParticipantFromSchedule(participantIndex);
	m_callbackScheduler->setTrt(m_trt);
	m_targetMonitor.stopMonitoring(participantIndex);
	getParticipantTracker()->forget(participantIndex);
}

void PassivePolicy::onBindDomain(UIntN participantIndex, UIntN domainIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		participant->refreshDomainProperties();
		auto domain = std::make_shared<PassiveDomainProxy>(domainIndex, participant, getPolicyServices());
		participant->bindDomain(domain);

		domain->setTstateUtilizationThreshold(m_utilizationBiasThreshold);

		if (participantIsSourceDevice(participantIndex))
		{
			domain->initializeControls();
		}

		takePossibleThermalActionForTarget(participantIndex);
	}
}

void PassivePolicy::onUnbindDomain(UIntN participantIndex, UIntN domainIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		participant->refreshDomainProperties();
		participant->unbindDomain(domainIndex);
	}
}

void PassivePolicy::onParticipantSpecificInfoChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		auto oldTrips = participant->getPassiveTripPointProperty().getTripPoints();
		auto oldHysteresis = participant->getTemperatureThresholds().getHysteresis();

		participant->getPassiveTripPointProperty().refresh();
		participant->refreshHysteresis();

		auto newTrips = participant->getPassiveTripPointProperty().getTripPoints();
		auto newHysteresis = participant->getTemperatureThresholds().getHysteresis();

		if (participantIsTargetDevice(participantIndex) && (oldTrips != newTrips || oldHysteresis != newHysteresis)
			&& (participant->getDomainPropertiesSet().getDomainCount() > 0))
		{
			takePossibleThermalActionForTarget(participantIndex);
		}
	}
}

void PassivePolicy::onDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
	try
	{
		if (participantIsTargetDevice(participantIndex))
		{
			auto participant = getParticipantTracker()->getParticipant(participantIndex);
			if (participant->getDomainPropertiesSet().getDomainCount() > 0)
			{
				auto currentTemperature = participant->getFirstDomainTemperature();
				participant->setThresholdCrossed(currentTemperature, getTime()->getCurrentTime());
				takePossibleThermalActionForTarget(participantIndex, currentTemperature);
			}
		}
	}
	catch (std::exception& ex)
	{
		// TODO: might want to pass in participant index instead
		POLICY_LOG_MESSAGE_WARNING_EX({
			std::stringstream message;
			message << "Failed to take thermal action for target on temperature threshold crossed event: "
					<< string(ex.what()) << ". ParticipantIndex = " << participantIndex;
			return message.str();
		});
	}
}

void PassivePolicy::onDomainPowerControlCapabilityChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		auto domainIndexes = participant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			try
			{
				auto domain = dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
				if ((domain.get() != nullptr) && (domain->getPowerControl()->supportsPowerControls()))
				{
					domain->getPowerControl()->refreshCapabilities();
					domain->adjustPowerRequests();
					domain->setArbitratedPowerLimit();
				}
			}
			catch (std::exception& ex)
			{
				POLICY_LOG_MESSAGE_ERROR_EX({ return ex.what(); });
			}
		}
	}
}

void PassivePolicy::onDomainPerformanceControlCapabilityChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		auto domainIndexes = participant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			try
			{
				auto domain = dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
				if ((domain.get() != nullptr) && (domain->getPerformanceControl()->supportsPerformanceControls()))
				{
					domain->getPerformanceControl()->refreshCapabilities();
					domain->adjustPerformanceRequests();
					domain->setArbitratedPerformanceLimit();
				}
			}
			catch (std::exception& ex)
			{
				POLICY_LOG_MESSAGE_ERROR_EX({ return ex.what(); });
			}
		}
	}
}

void PassivePolicy::onDomainPerformanceControlsChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		auto domainIndexes = participant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			try
			{
				auto domain = participant->getDomain(*domainIndex);
				if (domain->getPerformanceControl()->supportsPerformanceControls())
				{
					domain->getPerformanceControl()->refreshControls();
				}
			}
			catch (std::exception& ex)
			{
				POLICY_LOG_MESSAGE_ERROR_EX({ return ex.what(); });
			}
		}
	}
}

void PassivePolicy::onDomainCoreControlCapabilityChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		auto domainIndexes = participant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			try
			{
				auto domain = dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
				if ((domain.get() != nullptr) && (domain->getCoreControl()->supportsCoreControls()))
				{
					domain->getCoreControl()->refreshCapabilities();
					domain->adjustCoreRequests();
					domain->setArbitratedCoreLimit();
				}
			}
			catch (std::exception& ex)
			{
				POLICY_LOG_MESSAGE_ERROR_EX({ return ex.what(); });
			}
		}
	}
}

void PassivePolicy::onDomainDisplayControlCapabilityChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		auto domainIndexes = participant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			try
			{
				auto domain = dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
				if ((domain.get() != nullptr) && (domain->getDisplayControl()->supportsDisplayControls()))
				{
					domain->getDisplayControl()->refreshCapabilities();
					domain->clearAllDisplayControlRequests();
				}
			}
			catch (std::exception& ex)
			{
				POLICY_LOG_MESSAGE_ERROR_EX({ return ex.what(); });
			}
		}
	}
}

void PassivePolicy::onDomainPriorityChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		auto domainIndexes = participant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			try
			{
				auto domain = participant->getDomain(*domainIndex);
				domain->getDomainPriorityProperty().refresh();
			}
			catch (std::exception& ex)
			{
				POLICY_LOG_MESSAGE_ERROR_EX({ return ex.what(); });
			}
		}
	}
}

void PassivePolicy::onThermalRelationshipTableChanged(void)
{
	reloadTrtIfDifferent();
}

void PassivePolicy::onPolicyInitiatedCallback(UInt64 eventCode, UInt64 param1, void* param2)
{
	UIntN targetIndex = (UIntN)param1;
	if (participantIsTargetDevice(targetIndex))
	{
		m_callbackScheduler->acknowledgeCallback(targetIndex);
		takeThermalActionForTarget(targetIndex);
	}
}

void PassivePolicy::onOverrideTimeObject(const std::shared_ptr<TimeInterface>& timeObject)
{
	m_callbackScheduler->setTimeObject(timeObject);
}

void PassivePolicy::takeThermalActionForTarget(UIntN target)
{
	TargetActionBase* action = determineAction(target);
	action->execute();
	delete action;
}

TargetActionBase* PassivePolicy::determineAction(UIntN target)
{
	auto participant = getParticipantTracker()->getParticipant(target);
	if (participant->getDomainPropertiesSet().getDomainCount() > 0)
	{
		auto currentTemperature = participant->getFirstDomainTemperature();
		auto passiveTripPoints = participant->getPassiveTripPointProperty().getTripPoints();
		auto psv = passiveTripPoints.getTemperature(ParticipantSpecificInfoKey::PSV);
		if (currentTemperature > psv)
		{
			return new TargetLimitAction(
				getPolicyServices(),
				getTime(),
				getParticipantTracker(),
				m_trt,
				m_callbackScheduler,
				m_targetMonitor,
				target);
		}
		else if ((currentTemperature < psv) && (m_targetMonitor.isMonitoring(target)))
		{
			return new TargetUnlimitAction(
				getPolicyServices(),
				getTime(),
				getParticipantTracker(),
				m_trt,
				m_callbackScheduler,
				m_targetMonitor,
				target);
		}
		else if (currentTemperature == psv)
		{
			return new TargetCheckLaterAction(
				getPolicyServices(),
				getTime(),
				getParticipantTracker(),
				m_trt,
				m_callbackScheduler,
				m_targetMonitor,
				target);
		}
	}

	return new TargetNoAction(
		getPolicyServices(), getTime(), getParticipantTracker(), m_trt, m_callbackScheduler, m_targetMonitor, target);
}

void PassivePolicy::setParticipantTemperatureThresholdNotification(
	ParticipantProxyInterface* participant,
	Temperature currentTemperature)
{
	auto passiveTripPoints = participant->getPassiveTripPointProperty().getTripPoints();
	Temperature psv = passiveTripPoints.getTemperature(ParticipantSpecificInfoKey::PSV);
	Temperature lowerBoundTemperature(Temperature::createInvalid());
	Temperature upperBoundTemperature = psv;
	if (currentTemperature >= psv)
	{
		if (passiveTripPoints.hasKey(ParticipantSpecificInfoKey::NTT)
			&& (passiveTripPoints.getTemperature(ParticipantSpecificInfoKey::NTT) != Temperature(Constants::MaxUInt32)))
		{
			// lower bound is psv + some multiple of ntt
			// upper bound is lower bound + ntt
			auto ntt = passiveTripPoints.getTemperature(ParticipantSpecificInfoKey::NTT);
			lowerBoundTemperature = psv;
			while ((lowerBoundTemperature + ntt) <= currentTemperature)
			{
				lowerBoundTemperature = lowerBoundTemperature + ntt;
			}
			upperBoundTemperature = lowerBoundTemperature + ntt;
		}
		else
		{
			lowerBoundTemperature = psv;
			upperBoundTemperature = Temperature::createInvalid();
		}
	}
	participant->setTemperatureThresholds(lowerBoundTemperature, upperBoundTemperature);
}

void PassivePolicy::notifyPlatformOfDeviceTemperature(
	ParticipantProxyInterface* participant,
	Temperature currentTemperature)
{
	auto passiveTripPoints = participant->getPassiveTripPointProperty().getTripPoints();
	if (passiveTripPoints.hasKey(ParticipantSpecificInfoKey::NTT)
		&& (passiveTripPoints.getTemperature(ParticipantSpecificInfoKey::NTT) != Temperature(Constants::MaxUInt32)))
	{
		auto ntt = passiveTripPoints.getTemperature(ParticipantSpecificInfoKey::NTT);
		auto psv = passiveTripPoints.getTemperature(ParticipantSpecificInfoKey::PSV);
		if (currentTemperature >= (psv + ntt))
		{
			participant->notifyPlatformOfDeviceTemperature(currentTemperature);
		}
	}
}

void PassivePolicy::associateParticipantInTrt(
	ParticipantProxyInterface* participant,
	std::shared_ptr<ThermalRelationshipTable> trt)
{
	auto participantProperties = participant->getParticipantProperties();
	trt->associateParticipant(
		participantProperties.getAcpiInfo().getAcpiScope(), participant->getIndex(), participantProperties.getName());
}

void PassivePolicy::reloadTrtIfDifferent()
{
	std::shared_ptr<ThermalRelationshipTable> newTrt;
	try
	{
		newTrt = std::make_shared<ThermalRelationshipTable>(ThermalRelationshipTable::createTrtFromDptfBuffer(
			getPolicyServices().platformConfigurationData->getThermalRelationshipTable()));
	}
	catch (...)
	{
		newTrt.reset(new ThermalRelationshipTable());
	}

	associateAllParticipantsInTrt(newTrt);
	if (*m_trt != *newTrt)
	{
		vector<UIntN> indexes = getParticipantTracker()->getAllTrackedIndexes();
		for (auto participantIndex = indexes.begin(); participantIndex != indexes.end(); participantIndex++)
		{
			if (m_trt->isParticipantTargetDevice(*participantIndex))
			{
				auto target = getParticipantTracker()->getParticipant(*participantIndex);
				target->setTemperatureThresholds(Temperature::createInvalid(), Temperature::createInvalid());
			}
		}

		clearAllSourceControls(); // resets the sources from current TRT
		m_trt = newTrt;
		associateAllParticipantsInTrt(m_trt);
		m_targetMonitor.stopMonitoringAll();
		m_callbackScheduler.reset(new CallbackScheduler(getPolicyServices(), m_trt, getTime()));
		clearAllSourceControls(); // resets the sources from new TRT
		takePossibleThermalActionForAllTargets();
	}
}

void PassivePolicy::removeAllRequestsForTarget(UIntN target)
{
	if (participantIsTargetDevice(target))
	{
		vector<UIntN> participantIndexes = getParticipantTracker()->getAllTrackedIndexes();
		for (auto participantIndex = participantIndexes.begin(); participantIndex != participantIndexes.end();
			 participantIndex++)
		{
			auto participant = getParticipantTracker()->getParticipant(*participantIndex);
			vector<UIntN> domainIndexes = participant->getDomainIndexes();
			for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
			{
				auto domain = dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
				if (domain.get() != nullptr)
				{
					domain->clearAllRequestsForTarget(target);
				}
			}
		}
	}
}

Bool PassivePolicy::participantIsSourceDevice(UIntN participantIndex) const
{
	return getParticipantTracker()->remembers(participantIndex) && m_trt->isParticipantSourceDevice(participantIndex);
}

Bool PassivePolicy::participantIsTargetDevice(UIntN participantIndex) const
{
	return getParticipantTracker()->remembers(participantIndex) && m_trt->isParticipantTargetDevice(participantIndex);
}

std::shared_ptr<XmlNode> PassivePolicy::getXmlForPassiveTripPoints() const
{
	auto allStatus = XmlNode::createWrapperElement("passive_trip_point_status");
	vector<UIntN> participantIndexes = getParticipantTracker()->getAllTrackedIndexes();
	for (auto participantIndex = participantIndexes.begin(); participantIndex != participantIndexes.end();
		 participantIndex++)
	{
		auto participant = getParticipantTracker()->getParticipant(*participantIndex);
		if (participantIsTargetDevice(*participantIndex)
			&& participant->getPassiveTripPointProperty().supportsProperty())
		{
			allStatus->addChild(participant->getXmlForPassiveTripPoints());
		}
	}
	return allStatus;
}

void PassivePolicy::clearAllSourceControls()
{
	vector<UIntN> allIndicies = getParticipantTracker()->getAllTrackedIndexes();
	for (auto index = allIndicies.begin(); index != allIndicies.end(); ++index)
	{
		auto participant = getParticipantTracker()->getParticipant(*index);
		if (participantIsSourceDevice(participant->getIndex()))
		{
			auto domainIndexes = participant->getDomainIndexes();
			for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
			{
				auto domain = dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
				if (domain.get() != nullptr)
				{
					domain->clearAllControlKnobRequests();
					domain->setControlsToMax();
				}
			}
		}
	}
}

void PassivePolicy::associateAllParticipantsInTrt(std::shared_ptr<ThermalRelationshipTable> trt)
{
	vector<UIntN> allIndicies = getParticipantTracker()->getAllTrackedIndexes();
	for (auto index = allIndicies.begin(); index != allIndicies.end(); ++index)
	{
		auto participant = getParticipantTracker()->getParticipant(*index);
		associateParticipantInTrt(participant, trt);
	}
}

void PassivePolicy::takePossibleThermalActionForAllTargets()
{
	vector<UIntN> allIndicies = getParticipantTracker()->getAllTrackedIndexes();
	for (auto index = allIndicies.begin(); index != allIndicies.end(); ++index)
	{
		takePossibleThermalActionForTarget(*index);
	}
}

void PassivePolicy::takePossibleThermalActionForTarget(UIntN participantIndex)
{
	try
	{
		if (participantIsTargetDevice(participantIndex)
			&& getParticipantTracker()->getParticipant(participantIndex)->supportsTemperatureInterface())
		{
			auto currentTemperature =
				getParticipantTracker()->getParticipant(participantIndex)->getFirstDomainTemperature();
			takePossibleThermalActionForTarget(participantIndex, currentTemperature);
		}
	}
	catch (std::exception& ex)
	{
		// TODO: want to pass in participant index and domain index instead
		POLICY_LOG_MESSAGE_WARNING_EX({
			std::stringstream message;
			message << "Failed to take thermal action for target: " << string(ex.what())
					<< ". ParticipantIndex = " << participantIndex;
			return message.str();
		});
	}
}

void PassivePolicy::takePossibleThermalActionForTarget(UIntN participantIndex, const Temperature& temperature)
{
	try
	{
		if (participantIsTargetDevice(participantIndex)
			&& getParticipantTracker()->getParticipant(participantIndex)->supportsTemperatureInterface())
		{
			auto participant = getParticipantTracker()->getParticipant(participantIndex);
			setParticipantTemperatureThresholdNotification(participant, temperature);
			notifyPlatformOfDeviceTemperature(participant, temperature);
			takeThermalActionForTarget(participantIndex);
		}
	}
	catch (std::exception& ex)
	{
		// TODO: want to pass in participant index and domain index instead
		POLICY_LOG_MESSAGE_WARNING_EX({
			std::stringstream message;
			message << "Failed to take thermal action for target: " << string(ex.what())
					<< ". ParticipantIndex = " << participantIndex;
			return message.str();
		});
	}
}
