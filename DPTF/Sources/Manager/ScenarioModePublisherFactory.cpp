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

#include "ScenarioModePublisherFactory.h"
#include "ScenarioModeConfigurationParser.h"
#include "ScenarioModePublisher.h"

using namespace std;

shared_ptr<EventObserverInterface> ScenarioModePublisherFactory::make(
	DptfManagerInterface* manager,
	const std::shared_ptr<EventNotifierInterface>& eventNotifier,
	const DttConfigurationSegment& configuration)
{
	const auto configurations = ScenarioModeConfigurationParser::parse(configuration);
	const auto supportedScenarioModes = configurations.first;
	const auto supportedGamingModes = configurations.second;

	set<FrameworkEvent::Type> events;
	events.merge(supportedScenarioModes.getRequiredEvents());
	events.merge(supportedGamingModes.getRequiredEvents());

	const auto scenarioModePublisher = make_shared<ScenarioModePublisher>(
		manager, 
		supportedScenarioModes, 
		supportedGamingModes);
	eventNotifier->registerObserver(scenarioModePublisher, events);
	return scenarioModePublisher;
}