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

#include "Dptf.h"
#include "PolicyServices.h"
#include "PlatformStateInterface.h"

class PolicyServicesPlatformState final : public PolicyServices, public PlatformStateInterface
{
public:
	PolicyServicesPlatformState(DptfManagerInterface* dptfManager, UIntN policyIndex);
	virtual OnOffToggle::Type getMotion(void) const override;
	virtual SensorOrientation::Type getOrientation(void) const override;
	virtual SensorSpatialOrientation::Type getSpatialOrientation(void) const override;
	virtual OsLidState::Type getLidState(void) const override;
	virtual OsPowerSource::Type getPowerSource(void) const override;
	virtual const std::string& getForegroundApplicationName(void) const override;
	virtual CoolingMode::Type getCoolingMode(void) const override;
	virtual UIntN getBatteryPercentage(void) const override;
	virtual OsPlatformType::Type getPlatformType(void) const override;
	virtual OsDockMode::Type getDockMode(void) const override;
	virtual OsPowerSchemePersonality::Type getPowerSchemePersonality(void) const override;
	virtual UIntN getMobileNotification(OsMobileNotificationType::Type notificationType) const override;
	virtual OnOffToggle::Type getMixedRealityMode(void) const override;
	virtual OsUserPresence::Type getOsUserPresence(void) const override;
	virtual OsSessionState::Type getSessionState(void) const override;
	virtual OnOffToggle::Type getScreenState(void) const override;
	virtual UIntN getBatteryCount(void) const override;
	virtual UIntN getPowerSlider(void) const override;
	virtual SensorUserPresence::Type getPlatformUserPresence(void) const override;
	virtual OnOffToggle::Type getOsGameMode(void) const override;
	virtual UserInteraction::Type getUserInteraction(void) const override;
	virtual SystemMode::Type getSystemMode(void) const override;
	virtual OnOffToggle::Type getTpgPowerState(void) const override;
	virtual OnOffToggle::Type getCollaborationMode(void) const override;
	virtual SystemInBag::Type getSystemInBag(void) const override;
	virtual ScenarioMode::Type getScenarioMode(void) const override;
	virtual DttGamingMode::Type getDttGamingMode(void) const override;
	virtual Bool getApplicationOptimizationStatus(void) const override;
};
