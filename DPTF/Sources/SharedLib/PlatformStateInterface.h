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
#include "OnOffToggle.h"
#include "SensorOrientation.h"
#include "SensorSpatialOrientation.h"
#include "OsLidState.h"
#include "OsPowerSource.h"
#include "CoolingMode.h"
#include "OsPlatformType.h"
#include "OsDockMode.h"
#include "OsPowerSchemePersonality.h"
#include "OsMobileNotificationType.h"
#include "OsUserPresence.h"
#include "SensorUserPresence.h"
#include "OsSessionState.h"
#include "UserInteraction.h"
#include "SystemMode.h"
#include "SystemInBag.h"
#include "ScenarioMode.h"
#include "DttGamingMode.h"

class PlatformStateInterface
{
public:
	virtual ~PlatformStateInterface() {};

	virtual OnOffToggle::Type getMotion(void) const = 0;
	virtual SensorOrientation::Type getOrientation(void) const = 0;
	virtual SensorSpatialOrientation::Type getSpatialOrientation(void) const = 0;
	virtual OsLidState::Type getLidState(void) const = 0;
	virtual OsPowerSource::Type getPowerSource(void) const = 0;
	virtual const std::string& getForegroundApplicationName(void) const = 0;
	virtual CoolingMode::Type getCoolingMode(void) const = 0;
	virtual UIntN getBatteryPercentage(void) const = 0;
	virtual OsPlatformType::Type getPlatformType(void) const = 0;
	virtual OsDockMode::Type getDockMode(void) const = 0;
	virtual OsPowerSchemePersonality::Type getPowerSchemePersonality(void) const = 0;
	virtual UIntN getMobileNotification(OsMobileNotificationType::Type notificationType) const = 0;
	virtual OnOffToggle::Type getMixedRealityMode(void) const = 0;
	virtual OsUserPresence::Type getOsUserPresence(void) const = 0;
	virtual OsSessionState::Type getSessionState(void) const = 0;
	virtual OnOffToggle::Type getScreenState(void) const = 0;
	virtual UIntN getBatteryCount(void) const = 0;
	virtual UIntN getPowerSlider(void) const = 0;
	virtual SensorUserPresence::Type getPlatformUserPresence(void) const = 0;
	virtual OnOffToggle::Type getOsGameMode(void) const = 0;
	virtual UserInteraction::Type getUserInteraction(void) const = 0;
	virtual SystemMode::Type getSystemMode(void) const = 0;
	virtual OnOffToggle::Type getTpgPowerState(void) const = 0;
	virtual OnOffToggle::Type getCollaborationMode(void) const = 0;
	virtual SystemInBag::Type getSystemInBag(void) const = 0;
	virtual ScenarioMode::Type getScenarioMode(void) const = 0;
	virtual DttGamingMode::Type getDttGamingMode(void) const = 0;
	virtual Bool getApplicationOptimizationStatus(void) const = 0;
};
