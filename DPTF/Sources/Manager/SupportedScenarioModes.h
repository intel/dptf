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
#include "DptfExport.h"
#include "ScenarioMode.h"
#include "FrameworkEvent.h"
#include <set>

class dptf_export SupportedScenarioModes
{
public:
	SupportedScenarioModes() = default;
	SupportedScenarioModes(const std::set<ScenarioMode::Type>& supportedModes);

	size_t count() const;
	Bool contains(ScenarioMode::Type scenarioMode) const;
	std::string toString() const;
	std::set<FrameworkEvent::Type> getRequiredEvents() const;

private:
	std::set<ScenarioMode::Type> m_supportedModes;
};
