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

#include "ScenarioModePublisher.h"
#include "EventPayloadApplicationOptimizationChanged.h"
#include "ManagerLogger.h"
#include "WorkItem.h"
#include "WorkItemQueueManagerInterface.h"
#include "WIPolicyScenarioModeChanged.h"
#include "WIPolicyDttGamingModeChanged.h"
#include "EsifServicesInterface.h"
#include "FrameworkEvent.h"
#include "EventPayloadEnduranceGamingRequest.h"
#include "EventPayloadOnAndOffToggle.h"
#include "EventPayloadOperatingSystemPowerSourceType.h"
#include "EventPayloadExtendedWorkloadPredictionType.h"

using namespace std;

const std::string LogSignature{"[ScenarioMode] "};

ScenarioModePublisher::ScenarioModePublisher(
	DptfManagerInterface* dptfManager,
	const SupportedScenarioModes& supportedScenarios,
	const SupportedGamingModes& supportedGamingModes)
	: m_dptfManager(dptfManager)
	, m_supportedScenarios(supportedScenarios)
	, m_scenarioModeArbitrator(supportedScenarios)
	, m_currentScenarioMode(ScenarioMode::Invalid)
	, m_supportedGamingModes(supportedGamingModes)
	, m_gamingModeArbitrator(supportedGamingModes)
	, m_currentGamingMode(DttGamingMode::Invalid)
{
	logMessageSupportedScenarioModes();
	logMessageSupportedGamingModes();
}

void ScenarioModePublisher::update(FrameworkEvent::Type event, const DptfBuffer& eventPayload)
{
	updateArbitrators(event, eventPayload);
	publishEventsIfModesChanged();
}

void ScenarioModePublisher::updateArbitrators(FrameworkEvent::Type event, const DptfBuffer& eventPayload)
{
	updateGamingModeArbitrator(event, eventPayload);
	updateScenarioModeArbitrator(event, eventPayload);
}

void ScenarioModePublisher::publishEventsIfModesChanged()
{
	publishScenarioModeIfChanged();
	publishGamingModeIfChanged();
}

void ScenarioModePublisher::updateScenarioModeArbitrator(FrameworkEvent::Type event, const DptfBuffer& eventPayload)
{
	if (event == FrameworkEvent::PolicyCollaborationChanged)
	{
		const auto collaborationData = EventPayloadOnAndOffToggle(eventPayload);
		logEvent(event, collaborationData);
		m_scenarioModeArbitrator.updateCollaborationMode(collaborationData.value);
	}

	if (event == FrameworkEvent::DomainExtendedWorkloadPredictionChanged)
	{
		const auto ewpData = EventPayloadExtendedWorkloadPredictionType(eventPayload);
		logEvent(event, ewpData);
		m_scenarioModeArbitrator.updateExtendedWorkloadPrediction(ewpData.status);
	}

	m_scenarioModeArbitrator.updateGamingMode(m_gamingModeArbitrator.arbitrate());

}

void ScenarioModePublisher::updateGamingModeArbitrator(FrameworkEvent::Type event, const DptfBuffer& eventPayload)
{
	if (event == FrameworkEvent::PolicyApplicationOptimizationChanged)
	{
		const auto applicationOptimizationData = EventPayloadApplicationOptimizationChanged(eventPayload);
		logEvent(event, applicationOptimizationData);
		m_gamingModeArbitrator.updateApplicationOptimizationStatus(applicationOptimizationData.isActive);
	}

	if (event == FrameworkEvent::DptfAppBroadcastUnprivileged)
	{
		const auto egData = EventPayloadEnduranceGamingRequest(eventPayload);
		logEvent(event, egData);
		m_gamingModeArbitrator.updateEnduranceGamingStatus(egData.status);
	}

	if (event == FrameworkEvent::PolicyOperatingSystemPowerSourceChanged)
	{
		const auto powerSourceData = EventPayloadOperatingSystemPowerSourceType(eventPayload);
		logEvent(event, powerSourceData);
		m_gamingModeArbitrator.updatePowerSource(powerSourceData.value);
	}
}

void ScenarioModePublisher::publishScenarioModeIfChanged()
{
	const auto newScenarioMode = m_scenarioModeArbitrator.arbitrate();
	if (newScenarioMode != m_currentScenarioMode)
	{
		logMessageScenarioModeChanged(newScenarioMode);
		publishScenarioModeEvent(newScenarioMode);
		m_currentScenarioMode = newScenarioMode;
	}
}

void ScenarioModePublisher::publishGamingModeIfChanged()
{
	const auto newGamingMode = m_gamingModeArbitrator.arbitrate();
	if (m_currentScenarioMode == ScenarioMode::Gaming && newGamingMode != m_currentGamingMode)
	{
		logMessageGamingModeChanged(newGamingMode);
		publishGamingModeEvent(newGamingMode);
		m_currentGamingMode = newGamingMode;
	}
}

void ScenarioModePublisher::publishScenarioModeEvent(const ScenarioMode::Type scenarioMode)
{
	const shared_ptr<WorkItem> wi = make_shared<WIPolicyScenarioModeChanged>(m_dptfManager, scenarioMode);
	m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(wi);
}

void ScenarioModePublisher::publishGamingModeEvent(const DttGamingMode::Type gamingMode)
{
	const shared_ptr<WorkItem> wi = make_shared<WIPolicyDttGamingModeChanged>(m_dptfManager, gamingMode);
	m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(wi);
}

void ScenarioModePublisher::logMessageScenarioModeChanged(ScenarioMode::Type newScenarioMode) const
{
	MANAGER_LOG_MESSAGE_INFO({
		return LogSignature
			   + "Scenario Mode changed from "s
			   + ScenarioMode::toString(m_currentScenarioMode) + " to "s
			   + ScenarioMode::toString(newScenarioMode) + ". "s
			   + m_scenarioModeArbitrator.toString();
	});
}

void ScenarioModePublisher::logMessageGamingModeChanged(DttGamingMode::Type newGamingMode) const
{
	MANAGER_LOG_MESSAGE_INFO({
		return LogSignature
			   + "Gaming Mode changed from "s
			   + DttGamingMode::toString(m_currentGamingMode) + " to "s
			   + DttGamingMode::toString(newGamingMode) + ". "s
			   + m_gamingModeArbitrator.toString();
	});
}

void ScenarioModePublisher::logMessageSupportedScenarioModes() const
{
	MANAGER_LOG_MESSAGE_INFO({ 
		return LogSignature + "Supported Scenario Modes are {"s + m_supportedScenarios.toString() + "}"s;
	});
}

void ScenarioModePublisher::logMessageSupportedGamingModes() const
{
	MANAGER_LOG_MESSAGE_INFO({
		return LogSignature + "Supported Gaming Modes are {"s + m_supportedGamingModes.toString() + "}"s;
	});
}

void ScenarioModePublisher::logEvent(FrameworkEvent::Type event, const EventPayloadOnAndOffToggle& data) const
{
	if (event == FrameworkEvent::PolicyCollaborationChanged)
	{
		MANAGER_LOG_MESSAGE_INFO({
			return LogSignature
				   + "Event {PolicyCollaborationChanged: "s
				   + OnOffToggle::toString(data.value) + "}"s;
		});
	}
}

void ScenarioModePublisher::logEvent(FrameworkEvent::Type event, const EventPayloadExtendedWorkloadPredictionType& data) const
{
	if (event == FrameworkEvent::DomainExtendedWorkloadPredictionChanged)
	{
		MANAGER_LOG_MESSAGE_INFO({ 
			return LogSignature
				   + "Event {DomainExtendedWorkloadPredictionChanged: "s
				   + ExtendedWorkloadPrediction::toString(data.status) + "}"s; });
	}
}

void ScenarioModePublisher::logEvent(FrameworkEvent::Type event, const EventPayloadApplicationOptimizationChanged& data) const
{
	if (event == FrameworkEvent::PolicyApplicationOptimizationChanged)
	{
		MANAGER_LOG_MESSAGE_INFO({
			return LogSignature
				   + "Event {PolicyApplicationOptimizationChanged: "s
				   + (data.isActive ? "Active"s : "Inactive"s) + "}"s;
		});
	}
}

void ScenarioModePublisher::logEvent(FrameworkEvent::Type event, const EventPayloadEnduranceGamingRequest& data) const
{
	if (event == FrameworkEvent::DptfAppBroadcastUnprivileged)
	{
		MANAGER_LOG_MESSAGE_INFO({
			return LogSignature
				   + "Event {EnduranceGamingRequest: "s
				   + EnduranceGamingStatus::toString(static_cast<EnduranceGamingStatus::Type>(data.status)) + "}"s;
		});
	}
}

void ScenarioModePublisher::logEvent(FrameworkEvent::Type event, const EventPayloadOperatingSystemPowerSourceType& data) const
{
	if (event == FrameworkEvent::PolicyOperatingSystemPowerSourceChanged)
	{
		MANAGER_LOG_MESSAGE_INFO({
			return LogSignature
				   + "Event {PolicyOperatingSystemPowerSourceChanged: "s
				   + OsPowerSource::toString(data.value) + "}"s;
		});
	}
}

EsifServicesInterface* ScenarioModePublisher::getEsifServices() const
{
	return m_dptfManager->getEsifServices();
}
