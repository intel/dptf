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

#include "ScenarioModeConfigurationParser.h"
#include "StringParser.h"

using namespace std;

const string SupportedScenarioModesPath{"/Configuration/SupportedScenarios/"s};
const string SupportedGamingModesPath{"/Configuration/SupportedScenarios/Gaming/"s};
const DttConfigurationQuery DttConfigurationSupportedScenariosQuery{SupportedScenarioModesPath + ".*"s};
const DttConfigurationQuery DttConfigurationSupportedGamingModeQuery{SupportedGamingModesPath + ".*"s};

pair<SupportedScenarioModes, SupportedGamingModes> ScenarioModeConfigurationParser::parse(
	const DttConfigurationSegment& configuration)
{
	const auto supportedScenarios = readSupportedScenarios(configuration);
	const auto supportedGamingModes = readSupportedGamingModes(configuration);
	return {supportedScenarios, supportedGamingModes};
}

SupportedScenarioModes ScenarioModeConfigurationParser::readSupportedScenarios(
	const DttConfigurationSegment& configuration)
{
	set<ScenarioMode::Type> supportedScenarios;
	const auto keys = configuration.getKeysThatMatch(DttConfigurationSupportedScenariosQuery);
	for (const auto& key : keys)
	{
		const auto scenarioModeString = StringParser::removeString(key, SupportedScenarioModesPath);
		const auto scenarioModeStrings = StringParser::split(scenarioModeString, '/');
		const auto scenarioMode = ScenarioMode::fromString(scenarioModeStrings.front());
		if (scenarioMode != ScenarioMode::Invalid)
		{
			supportedScenarios.emplace(scenarioMode);
		}
	}
	return supportedScenarios;
}

SupportedGamingModes ScenarioModeConfigurationParser::readSupportedGamingModes(
	const DttConfigurationSegment& configuration)
{
	set<DttGamingMode::Type> supportedGamingModes;
	const auto keys = configuration.getKeysThatMatch(DttConfigurationSupportedGamingModeQuery);
	for (const auto& key : keys)
	{
		const auto gamingModeString = configuration.getValueAsString(key);
		const auto gamingMode = DttGamingMode::fromString(gamingModeString);
		if (gamingMode != DttGamingMode::Invalid)
		{
			supportedGamingModes.emplace(gamingMode);
		}
	}
	return supportedGamingModes;
}