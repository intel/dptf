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

#pragma once
#include "DptfManagerInterface.h"
#include "EventObserverInterface.h"
#include "GamingModeArbitrator.h"
#include "ScenarioModeArbitrator.h"
#include "SupportedScenarioModes.h"
#include "SupportedGamingModes.h"
#include "ScenarioMode.h"
#include "DttGamingMode.h"
#include "EventPayloadApplicationOptimizationChanged.h"
#include "EventPayloadEnduranceGamingRequest.h"
#include "EventPayloadExtendedWorkloadPredictionType.h"
#include "EventPayloadOnAndOffToggle.h"
#include "EventPayloadOperatingSystemPowerSourceType.h"

class dptf_export ScenarioModePublisher : public EventObserverInterface
{
public:
	ScenarioModePublisher(
		DptfManagerInterface* dptfManager,
		const SupportedScenarioModes& supportedScenarios,
		const SupportedGamingModes& supportedGamingModes);

	void update(FrameworkEvent::Type event, const DptfBuffer& eventPayload) override;

private:
	DptfManagerInterface* m_dptfManager;

	SupportedScenarioModes m_supportedScenarios;
	ScenarioModeArbitrator m_scenarioModeArbitrator;
	ScenarioMode::Type m_currentScenarioMode;

	SupportedGamingModes m_supportedGamingModes;
	GamingModeArbitrator m_gamingModeArbitrator;
	DttGamingMode::Type m_currentGamingMode;

	void updateArbitrators(FrameworkEvent::Type event, const DptfBuffer& eventPayload);
	void publishEventsIfModesChanged();
	void publishScenarioModeIfChanged();
	void publishGamingModeIfChanged();
	void updateScenarioModeArbitrator(FrameworkEvent::Type event, const DptfBuffer& eventPayload);
	void updateGamingModeArbitrator(FrameworkEvent::Type event, const DptfBuffer& eventPayload);
	void publishScenarioModeEvent(ScenarioMode::Type scenarioMode);
	void publishGamingModeEvent(DttGamingMode::Type gamingMode);
	void logMessageScenarioModeChanged(ScenarioMode::Type newScenarioMode) const;
	void logMessageGamingModeChanged(DttGamingMode::Type newGamingMode) const;
	void logMessageSupportedScenarioModes() const;
	void logMessageSupportedGamingModes() const;
	void logEvent(FrameworkEvent::Type event, const EventPayloadOnAndOffToggle& data) const;
	void logEvent(FrameworkEvent::Type event, const EventPayloadExtendedWorkloadPredictionType& data) const;
	void logEvent(FrameworkEvent::Type event, const EventPayloadApplicationOptimizationChanged& data) const;
	void logEvent(FrameworkEvent::Type event, const EventPayloadEnduranceGamingRequest& data) const;
	void logEvent(FrameworkEvent::Type event, const EventPayloadOperatingSystemPowerSourceType& data) const;
	EsifServicesInterface* getEsifServices() const;
};
