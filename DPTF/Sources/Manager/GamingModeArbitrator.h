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
#include "DttGamingMode.h"
#include "SupportedGamingModes.h"
#include "OsPowerSource.h"

class dptf_export GamingModeArbitrator
{
public:
	GamingModeArbitrator(const SupportedGamingModes& supportedModes);

	DttGamingMode::Type arbitrate() const;
	void updatePowerSource(OsPowerSource::Type powerSource);
	void updateApplicationOptimizationStatus(Bool isActive);
	void updateEnduranceGamingStatus(Bool isActive);
	std::string toString() const;

private:
	SupportedGamingModes m_supportedModes;
	OsPowerSource::Type m_powerSource;
	Bool m_isApplicationOptimizationActive;
	Bool m_isEnduranceGamingActive;
};
