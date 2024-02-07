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

#include "ScenarioModeArbitrator.h"

using namespace std;

ScenarioModeArbitrator::ScenarioModeArbitrator(const SupportedScenarioModes& supportedModes)
	: m_supportedModes(supportedModes)
	, m_ewp(ExtendedWorkloadPrediction::Invalid)
	, m_collaborationMode(OnOffToggle::Off)
	, m_gamingMode(DttGamingMode::Invalid)
{
}

ScenarioMode::Type ScenarioModeArbitrator::arbitrate() const
{
	if (m_supportedModes.contains(ScenarioMode::Gaming) && 
		m_gamingMode != DttGamingMode::Invalid)
	{
		return ScenarioMode::Gaming;
	}

	if (m_supportedModes.contains(ScenarioMode::Collaboration) && 
		m_collaborationMode == OnOffToggle::On)
	{
		return ScenarioMode::Collaboration;
	}

	if (m_supportedModes.contains(ScenarioMode::EwpPerformance) && 
		m_ewp == ExtendedWorkloadPrediction::Performance)
	{
		return ScenarioMode::EwpPerformance;
	}

	if (m_supportedModes.contains(ScenarioMode::EwpCoolAndQuiet) && 
		m_ewp == ExtendedWorkloadPrediction::Default)
	{
		return ScenarioMode::EwpCoolAndQuiet;
	}

	return ScenarioMode::Invalid;
}

void ScenarioModeArbitrator::updateExtendedWorkloadPrediction(ExtendedWorkloadPrediction::Type ewp)
{
	m_ewp = ewp;
}

void ScenarioModeArbitrator::updateCollaborationMode(OnOffToggle::Type collaborationMode)
{
	m_collaborationMode = collaborationMode;
}

void ScenarioModeArbitrator::updateGamingMode(DttGamingMode::Type gamingMode)
{
	m_gamingMode = gamingMode;
}

string ScenarioModeArbitrator::toString() const
{
	const auto scenarioMode = arbitrate();
	stringstream stream;
	stream << "ScenarioMode: "s << ScenarioMode::toString(scenarioMode) << " {"s
		   << "Ewp: "s << ExtendedWorkloadPrediction::toString(m_ewp) << ", "s
		   << "CollaborationMode: "s << OnOffToggle::toString(m_collaborationMode) << ", "s
		   << "DttGamingMode: "s << DttGamingMode::toString(m_gamingMode) << "}"s;
	return stream.str();
}