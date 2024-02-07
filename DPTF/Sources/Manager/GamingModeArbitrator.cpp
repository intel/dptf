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

#include "GamingModeArbitrator.h"

using namespace std;

GamingModeArbitrator::GamingModeArbitrator(const SupportedGamingModes& supportedModes)
	: m_supportedModes(supportedModes)
	, m_powerSource(OsPowerSource::Invalid)
	, m_isApplicationOptimizationActive(false)
	, m_isEnduranceGamingActive(false)
{
}

DttGamingMode::Type GamingModeArbitrator::arbitrate() const
{
	if (m_supportedModes.contains(DttGamingMode::MaxPerformance) && 
		m_isApplicationOptimizationActive && 
		m_powerSource == OsPowerSource::AC)
	{
		return DttGamingMode::MaxPerformance;
	}

	if (m_supportedModes.contains(DttGamingMode::EnduranceGaming) && 
		m_isEnduranceGamingActive && 
		m_powerSource == OsPowerSource::DC)
	{
		return DttGamingMode::EnduranceGaming;
	}

	return DttGamingMode::Invalid;
}

void GamingModeArbitrator::updatePowerSource(OsPowerSource::Type powerSource)
{
	m_powerSource = powerSource;
}

void GamingModeArbitrator::updateApplicationOptimizationStatus(Bool isActive)
{
	m_isApplicationOptimizationActive = isActive;
}

void GamingModeArbitrator::updateEnduranceGamingStatus(Bool isActive)
{
	m_isEnduranceGamingActive = isActive;
}

std::string GamingModeArbitrator::toString() const
{
	const auto gamingMode = arbitrate();
	stringstream stream;
	stream << "DttGamingMode: "s << DttGamingMode::toString(gamingMode) << " {"s
		   << "PowerSource: "s << OsPowerSource::toString(m_powerSource) << ", "s
		   << "ApplicationOptimization: "s << (m_isApplicationOptimizationActive ? "Active"s : "Inactive"s) << ", "s
		   << "EnduranceGaming: "s << (m_isEnduranceGamingActive ? "Active"s : "Inactive"s) << "}"s;
	return stream.str();
}