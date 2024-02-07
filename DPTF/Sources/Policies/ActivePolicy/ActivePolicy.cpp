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

#include "ActivePolicy.h"
#include "DomainProxy.h"

using namespace std;

const Guid MyGuid(0x89, 0xC3, 0x95, 0x3A, 0xB8, 0xE4, 0x29, 0x46, 0xA5, 0x26, 0xC5, 0x2C, 0x88, 0x62, 0x6B, 0xAE);
const string MyName("Active Policy");

ActivePolicy::ActivePolicy(void)
	: PolicyBase()
{
}

ActivePolicy::~ActivePolicy(void)
{
}

void ActivePolicy::onCreate(void)
{
	try
	{
		m_art = std::make_shared<ActiveRelationshipTable>(ActiveRelationshipTable::createArtFromDptfBuffer(
			getPolicyServices().platformConfigurationData->getActiveRelationshipTable()));
	}
	catch (std::exception& ex)
	{
		POLICY_LOG_MESSAGE_INFO_EX({ return string("No active relationship table was found. ") + string(ex.what()); });
		m_art.reset(new ActiveRelationshipTable());
	}

	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::ParticipantSpecificInfoChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyActiveRelationshipTableChanged);
	getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainFanCapabilityChanged);
}

void ActivePolicy::onDestroy(void)
{
	turnOffAllFans();
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::ParticipantSpecificInfoChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::PolicyActiveRelationshipTableChanged);
	getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainFanCapabilityChanged);
}

void ActivePolicy::onEnable(void)
{
	refreshArtAndTargetsAndTakeCoolingAction();
}

void ActivePolicy::onDisable(void)
{
	turnOffAllFans();
}

void ActivePolicy::onConnectedStandbyEntry()
{
	turnOffAllFans();
}

void ActivePolicy::onConnectedStandbyExit()
{
	refreshArtAndTargetsAndTakeCoolingAction();
}

Bool ActivePolicy::autoNotifyPlatformOscOnCreateDestroy() const
{
	return true;
}

Bool ActivePolicy::autoNotifyPlatformOscOnConnectedStandbyEntryExit() const
{
	return false;
}

Bool ActivePolicy::autoNotifyPlatformOscOnEnableDisable() const
{
	return true;
}

Bool ActivePolicy::hasActiveControlCapability() const
{
	return true;
}

Guid ActivePolicy::getGuid(void) const
{
	return MyGuid;
}

string ActivePolicy::getName(void) const
{
	return MyName;
}

string ActivePolicy::getStatusAsXml(void) const
{
	auto root = XmlNode::createRoot();
	auto format = XmlNode::createComment("format_id=" + getGuid().toString());
	root->addChild(format);
	auto status = XmlNode::createWrapperElement("active_policy_status");
	status->addChild(getXmlForActiveCoolingControls());
	status->addChild(getXmlForActiveTripPoints());
	status->addChild(m_art->getXml());
	root->addChild(status);
	string statusString = root->toString();
	return statusString;
}

string ActivePolicy::getDiagnosticsAsXml(void) const
{
	return "Active Policy Diagnostics Not Yet Implemented\n";
}

void ActivePolicy::onBindParticipant(UIntN participantIndex)
{
	getParticipantTracker()->remember(participantIndex);
	associateParticipantInArt(getParticipantTracker()->getParticipant(participantIndex));
}

void ActivePolicy::onUnbindParticipant(UIntN participantIndex)
{
	m_art->disassociateParticipant(participantIndex);
	getParticipantTracker()->forget(participantIndex);
}

void ActivePolicy::onBindDomain(UIntN participantIndex, UIntN domainIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		participant->refreshDomainProperties();
		auto domain = std::make_shared<DomainProxy>(domainIndex, participant, getPolicyServices());
		participant->bindDomain(domain);

		if (participantIsTargetDevice(participantIndex))
		{
			updateThresholdsAndCoolTargetParticipant(participant);
		}
		else if (participantIsSourceDevice(participantIndex))
		{
			// take possible cooling action for each target who is associated with this fan
			auto entries = m_art->getEntriesForSource(participantIndex);
			for (auto entry = entries.begin(); entry != entries.end(); entry++)
			{
				auto targetDeviceIndex = (*entry)->getTargetDeviceIndex();
				if (participantIsTargetDevice(targetDeviceIndex)
					&& getParticipantTracker()->getParticipant(targetDeviceIndex)->supportsTemperatureInterface())
				{
					// TODO: This may force a fan speed to be set multiple times.
					// Need to fix it such that all requests are collected and applied once
					updateTargetRequest(getParticipantTracker()->getParticipant(targetDeviceIndex));
				}
			}
		}
		else
		{
			// not a target or source, so do nothing
		}
	}
}

void ActivePolicy::onUnbindDomain(UIntN participantIndex, UIntN domainIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		if (participantIsTargetDevice(participantIndex))
		{
			auto entries = m_art->getEntriesForTarget(participantIndex);
			for (auto entry = entries.begin(); entry != entries.end(); entry++)
			{
				if (participantIsSourceDevice((*entry)->getSourceDeviceIndex()))
				{
					try
					{
						requestFanTurnedOff(*entry);
					}
					catch (...)
					{
						// no action for failure.  make best attempt to turn off the fan.
					}
				}
			}
		}

		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		participant->refreshDomainProperties();
		participant->unbindDomain(domainIndex);
	}
}

void ActivePolicy::onParticipantSpecificInfoChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		auto participant = getParticipantTracker()->getParticipant(participantIndex);
		auto oldTrips = participant->getActiveTripPointProperty().getTripPoints();
		auto oldHysteresis = participant->getTemperatureThresholds().getHysteresis();

		participant->getActiveTripPointProperty().refresh();
		participant->refreshHysteresis();

		auto newTrips = participant->getActiveTripPointProperty().getTripPoints();
		auto newHysteresis = participant->getTemperatureThresholds().getHysteresis();
		if (participantIsTargetDevice(participantIndex) && (oldTrips != newTrips || oldHysteresis != newHysteresis))
		{
			updateThresholdsAndCoolTargetParticipant(participant);
		}
	}
}

void ActivePolicy::onDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
	if (participantIsTargetDevice(participantIndex))
	{
		updateThresholdsAndCoolTargetParticipant(getParticipantTracker()->getParticipant(participantIndex));
	}
}

void ActivePolicy::onDomainFanCapabilityChanged(UIntN participantIndex)
{
	if (getParticipantTracker()->remembers(participantIndex))
	{
		ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
		auto domains = participant->getDomainIndexes();
		for (auto domainIndex = domains.begin(); domainIndex != domains.end(); ++domainIndex)
		{
			auto domain = participant->getDomain(*domainIndex);
			if (domain->getActiveCoolingControl()->supportsActiveCoolingControls())
			{
				domain->getActiveCoolingControl()->refreshCapabilities();
				if (participantIsSourceDevice(participantIndex))
				{
					auto entries = m_art->getEntriesForSource(participantIndex);
					for (auto entry = entries.begin(); entry != entries.end(); entry++)
					{
						auto targetParticipantIndex = (*entry)->getTargetDeviceIndex();
						if (getParticipantTracker()->remembers(targetParticipantIndex)
							&& getParticipantTracker()
								   ->getParticipant(targetParticipantIndex)
								   ->supportsTemperatureInterface())
						{
							auto currentTemperature = getParticipantTracker()
														  ->getParticipant(targetParticipantIndex)
														  ->getFirstDomainTemperature();
							// TODO: This may force a fan speed to be set multiple times.
							// Need to fix it such that all requests are collected and applied once
							requestFanSpeedChange(*entry, currentTemperature);
						}
					}
				}
			}
		}
	}
}

void ActivePolicy::onActiveRelationshipTableChanged(void)
{
	vector<UIntN> indexes = m_art->getAllTargets();
	for (auto participantIndex = indexes.begin(); participantIndex != indexes.end(); participantIndex++)
	{
		try
		{
			if (getParticipantTracker()->remembers(*participantIndex))
			{
				auto target = getParticipantTracker()->getParticipant(*participantIndex);
				target->setTemperatureThresholds(Temperature::createInvalid(), Temperature::createInvalid());
			}
		}
		catch (std::exception& ex)
		{
			POLICY_LOG_MESSAGE_DEBUG_EX(
				{ return "Failed to reset temperature thresholds for participant: " + std::string(ex.what()); });
		}

		auto entries = m_art->getEntriesForTarget(*participantIndex);
		for (auto entry = entries.begin(); entry != entries.end(); entry++)
		{
			try
			{
				requestFanTurnedOff(*entry);
			}
			catch (...)
			{
				// no action for failure.  make best attempt to turn off the fan.
			}
		}
	}
	m_art.reset(new ActiveRelationshipTable(ActiveRelationshipTable::createArtFromDptfBuffer(
		getPolicyServices().platformConfigurationData->getActiveRelationshipTable())));
	associateAllParticipantsInArt();

	auto targetIndexes = m_art->getAllTargets();
	for (auto target = targetIndexes.begin(); target != targetIndexes.end(); target++)
	{
		// TODO: This may force a fan speed to be set multiple times.
		// Need to fix it such that all requests are collected and applied once
		if (getParticipantTracker()->remembers(*target))
		{
			updateThresholdsAndCoolTargetParticipant(getParticipantTracker()->getParticipant(*target));
		}
	}
}

Temperature ActivePolicy::getCurrentTemperature(ParticipantProxyInterface* participant)
{
	auto currentTemperature = participant->getFirstDomainTemperature();

	POLICY_LOG_MESSAGE_DEBUG({
		std::stringstream message;
		message << "Considering actions based on temperature of " << currentTemperature.toString();
		message << " for participant " << std::to_string(participant->getIndex());
		return message.str();
	});

	return currentTemperature;
}

void ActivePolicy::updateTargetRequest(ParticipantProxyInterface* participant)
{
	if (participant->getActiveTripPointProperty().supportsProperty())
	{
		if (participant->getDomainPropertiesSet().getDomainCount() > 0)
		{
			auto currentTemperature = getCurrentTemperature(participant);
			requestFanSpeedChangesForTarget(participant, currentTemperature);
		}
	}
}

void ActivePolicy::updateThresholdsAndCoolTargetParticipant(ParticipantProxyInterface* participant)
{
	if (participant->getActiveTripPointProperty().supportsProperty())
	{
		if (participant->getDomainPropertiesSet().getDomainCount() > 0)
		{
			auto currentTemperature = getCurrentTemperature(participant);
			setTripPointNotificationForTarget(participant, currentTemperature);
			requestFanSpeedChangesForTarget(participant, currentTemperature);
		}
	}
}

void ActivePolicy::requestFanSpeedChangesForTarget(
	ParticipantProxyInterface* target,
	const Temperature& currentTemperature)
{
	auto artEntriesForTarget = m_art->getEntriesForTarget(target->getIndex());
	for (auto entry = artEntriesForTarget.begin(); entry != artEntriesForTarget.end(); entry++)
	{
		if (participantIsSourceDevice((*entry)->getSourceDeviceIndex()))
		{
			requestFanSpeedChange(*entry, currentTemperature);
		}
	}
}

void ActivePolicy::requestFanSpeedChange(
	std::shared_ptr<ActiveRelationshipTableEntry> entry,
	const Temperature& currentTemperature)
{
	auto targetIndex = entry->getTargetDeviceIndex();
	auto sourceIndex = entry->getSourceDeviceIndex();
	if (getParticipantTracker()->remembers(targetIndex) && getParticipantTracker()->remembers(sourceIndex))
	{
		auto tripPoints =
			getParticipantTracker()->getParticipant(targetIndex)->getActiveTripPointProperty().getTripPoints();
		auto sourceParticipant = getParticipantTracker()->getParticipant(sourceIndex);
		auto domainIndexes = sourceParticipant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			auto sourceDomain = sourceParticipant->getDomain(*domainIndex);
			std::shared_ptr<ActiveCoolingControlFacadeInterface> coolingControl =
				sourceDomain->getActiveCoolingControl();
			if (coolingControl->supportsFineGrainControl())
			{
				Percentage fanSpeed = selectFanSpeed(entry, tripPoints, currentTemperature);
				POLICY_LOG_MESSAGE_DEBUG({ return "Requesting fan speed of " + fanSpeed.toString() + "."; });
				coolingControl->requestFanSpeedPercentage(targetIndex, fanSpeed);
			}
		}
	}
}

void ActivePolicy::requestFanTurnedOff(std::shared_ptr<ActiveRelationshipTableEntry> entry)
{
	auto sourceIndex = entry->getSourceDeviceIndex();
	if (getParticipantTracker()->remembers(sourceIndex))
	{
		POLICY_LOG_MESSAGE_DEBUG(
			{ return "Requesting fan turned off for participant " + std::to_string(sourceIndex) + "."; });

		auto sourceParticipant = getParticipantTracker()->getParticipant(sourceIndex);
		auto domainIndexes = sourceParticipant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			auto sourceDomain = sourceParticipant->getDomain(*domainIndex);
			std::shared_ptr<ActiveCoolingControlFacadeInterface> coolingControl =
				sourceDomain->getActiveCoolingControl();
			if (coolingControl->supportsFineGrainControl())
			{
				coolingControl->clearFanSpeedRequestForTarget(entry->getTargetDeviceIndex());
				coolingControl->setHighestFanSpeedPercentage();
			}
		}
	}
}

void ActivePolicy::turnOffAllFans()
{
	if (m_art != nullptr)
	{
		POLICY_LOG_MESSAGE_DEBUG({ return "Turning off all fans."; });
		vector<UIntN> sources = m_art->getAllSources();
		for (auto source = sources.begin(); source != sources.end(); source++)
		{
			if (getParticipantTracker()->remembers(*source))
			{
				auto sourceParticipant = getParticipantTracker()->getParticipant(*source);
				auto domainIndexes = sourceParticipant->getDomainIndexes();
				for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
				{
					try
					{
						auto sourceDomain = sourceParticipant->getDomain(*domainIndex);
						if (sourceDomain != nullptr)
						{
							std::shared_ptr<ActiveCoolingControlFacadeInterface> coolingControl =
								sourceDomain->getActiveCoolingControl();
							if (coolingControl != nullptr)
							{
								coolingControl->forceFanOff();
							}
						}
					}
					catch (...)
					{
						// swallow errors when attempting to force fans off
					}
				}
			}
		}
	}
}

void ActivePolicy::refreshArtAndTargetsAndTakeCoolingAction()
{
	reloadArt();
	takeCoolingActionsForAllParticipants();
}

void ActivePolicy::reloadArt()
{
	m_art.reset(new ActiveRelationshipTable(ActiveRelationshipTable::createArtFromDptfBuffer(
		getPolicyServices().platformConfigurationData->getActiveRelationshipTable())));
	associateAllParticipantsInArt();
}

void ActivePolicy::takeCoolingActionsForAllParticipants()
{
	vector<UIntN> targets = m_art->getAllTargets();
	for (auto target = targets.begin(); target != targets.end(); target++)
	{
		if (getParticipantTracker()->remembers(*target))
		{
			auto targetParticipant = getParticipantTracker()->getParticipant(*target);
			targetParticipant->getActiveTripPointProperty().refresh();
			updateThresholdsAndCoolTargetParticipant(targetParticipant);
		}
	}
}

void ActivePolicy::setTripPointNotificationForTarget(
	ParticipantProxyInterface* target,
	const Temperature& currentTemperature)
{
	auto tripPoints = target->getActiveTripPointProperty().getTripPoints();
	Temperature lowerTemperatureThreshold = determineLowerTemperatureThreshold(currentTemperature, tripPoints);
	Temperature upperTemperatureThreshold = determineUpperTemperatureThreshold(currentTemperature, tripPoints);
	target->setTemperatureThresholds(lowerTemperatureThreshold, upperTemperatureThreshold);
	target->notifyPlatformOfDeviceTemperature(currentTemperature);
}

Temperature ActivePolicy::determineLowerTemperatureThreshold(
	const Temperature& currentTemperature,
	SpecificInfo& tripPoints) const
{
	auto trips = tripPoints.getSortedByKey();
	Temperature lowerTemperatureThreshold(Temperature::createInvalid());
	for (UIntN ac = 0; ac < trips.size(); ++ac)
	{
		if (currentTemperature >= trips[ac].second)
		{
			lowerTemperatureThreshold = trips[ac].second;
			break;
		}
	}
	return lowerTemperatureThreshold;
}

Temperature ActivePolicy::determineUpperTemperatureThreshold(
	const Temperature& currentTemperature,
	SpecificInfo& tripPoints) const
{
	auto trips = tripPoints.getSortedByKey();
	Temperature upperTemperatureThreshold(Temperature::createInvalid());
	for (UIntN ac = 0; ac < trips.size(); ++ac)
	{
		if ((currentTemperature < trips[ac].second) && (trips[ac].second != Temperature(Constants::MaxUInt32)))
		{
			upperTemperatureThreshold = trips[ac].second;
		}
	}
	return upperTemperatureThreshold;
}

Percentage ActivePolicy::selectFanSpeed(
	std::shared_ptr<ActiveRelationshipTableEntry> entry,
	SpecificInfo& tripPoints,
	const Temperature& temperature)
{
	Percentage fanSpeed = Percentage::createInvalid();
	UIntN crossedTripPointIndex = findTripPointCrossed(tripPoints, temperature);
	if (crossedTripPointIndex != Constants::Invalid)
	{
		// find fan speed at index or greater
		fanSpeed = 0.0;
		for (UIntN entryAcIndex = crossedTripPointIndex; entryAcIndex < ActiveCoolingControl::FanOffIndex;
			 ++entryAcIndex)
		{
			if (entry->ac(entryAcIndex) != Constants::Invalid)
			{
				fanSpeed = (double)entry->ac(entryAcIndex) / 100.0;
				break;
			}
		}
	}
	return fanSpeed;
}

UIntN ActivePolicy::findTripPointCrossed(SpecificInfo& tripPoints, const Temperature& temperature)
{
	auto trips = tripPoints.getSortedByKey();
	for (UIntN index = 0; index < trips.size(); index++)
	{
		if (temperature >= trips[index].second)
		{
			UIntN acIndex = trips[index].first - ParticipantSpecificInfoKey::AC0;
			return acIndex;
		}
	}
	return Constants::Invalid;
}

void ActivePolicy::associateAllParticipantsInArt()
{
	vector<UIntN> participantIndicies = getParticipantTracker()->getAllTrackedIndexes();
	for (auto index = participantIndicies.begin(); index != participantIndicies.end(); index++)
	{
		associateParticipantInArt(getParticipantTracker()->getParticipant(*index));
	}
}

void ActivePolicy::associateParticipantInArt(ParticipantProxyInterface* participant)
{
	auto participantProperties = participant->getParticipantProperties();
	m_art->associateParticipant(
		participantProperties.getAcpiInfo().getAcpiScope(), participant->getIndex(), participantProperties.getName());
}

Bool ActivePolicy::participantIsTargetDevice(UIntN participantIndex)
{
	return getParticipantTracker()->remembers(participantIndex) && m_art->isParticipantTargetDevice(participantIndex);
}

Bool ActivePolicy::participantIsSourceDevice(UIntN participantIndex)
{
	return getParticipantTracker()->remembers(participantIndex) && m_art->isParticipantSourceDevice(participantIndex);
}

std::shared_ptr<XmlNode> ActivePolicy::getXmlForActiveTripPoints() const
{
	auto allStatus = XmlNode::createWrapperElement("active_trip_point_status");
	vector<UIntN> indexes = getParticipantTracker()->getAllTrackedIndexes();
	for (auto participantIndex = indexes.begin(); participantIndex != indexes.end(); participantIndex++)
	{
		ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*participantIndex);
		if (m_art->isParticipantTargetDevice(*participantIndex)
			&& participant->getActiveTripPointProperty().supportsProperty())
		{
			allStatus->addChild(participant->getXmlForActiveTripPoints());
		}
	}
	return allStatus;
}

std::shared_ptr<XmlNode> ActivePolicy::getXmlForActiveCoolingControls() const
{
	auto fanStatus = XmlNode::createWrapperElement("fan_status");
	vector<UIntN> participantTndexes = m_art->getAllSources();
	for (auto participantIndex = participantTndexes.begin(); participantIndex != participantTndexes.end();
		 participantIndex++)
	{
		if (getParticipantTracker()->remembers(*participantIndex))
		{
			ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*participantIndex);
			auto domainIndexes = participant->getDomainIndexes();
			for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
			{
				auto domain = participant->getDomain(*domainIndex);
				if (domain->getActiveCoolingControl()->supportsActiveCoolingControls())
				{
					try
					{
						fanStatus->addChild(domain->getActiveCoolingControl()->getXml());
					}
					catch (...)
					{
					}
				}
			}
		}
	}
	return fanStatus;
}
