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

#include "PolicyServicesPlatformState.h"

PolicyServicesPlatformState::PolicyServicesPlatformState(DptfManagerInterface* dptfManager, UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

OnOffToggle::Type PolicyServicesPlatformState::getMotion() const
{
	return getDptfManager()->getEventCache()->sensorMotion.get();
}

SensorOrientation::Type PolicyServicesPlatformState::getOrientation() const
{
	return getDptfManager()->getEventCache()->sensorOrientation.get();
}

SensorSpatialOrientation::Type PolicyServicesPlatformState::getSpatialOrientation() const
{
	return getDptfManager()->getEventCache()->sensorSpatialOrientation.get();
}

OsLidState::Type PolicyServicesPlatformState::getLidState() const
{
	return getDptfManager()->getEventCache()->lidState.get();
}

OsPowerSource::Type PolicyServicesPlatformState::getPowerSource() const
{
	return getDptfManager()->getEventCache()->powerSource.get();
}

const std::string& PolicyServicesPlatformState::getForegroundApplicationName() const
{
	return getDptfManager()->getEventCache()->foregroundApplication.get();
}

CoolingMode::Type PolicyServicesPlatformState::getCoolingMode() const
{
	return getDptfManager()->getEventCache()->coolingMode.get();
}

UIntN PolicyServicesPlatformState::getBatteryPercentage() const
{
	return getDptfManager()->getEventCache()->batteryPercentage.get();
}

OsPlatformType::Type PolicyServicesPlatformState::getPlatformType() const
{
	return getDptfManager()->getEventCache()->platformType.get();
}

OsDockMode::Type PolicyServicesPlatformState::getDockMode() const
{
	return getDptfManager()->getEventCache()->dockMode.get();
}

OsPowerSchemePersonality::Type PolicyServicesPlatformState::getPowerSchemePersonality() const
{
	return getDptfManager()->getEventCache()->powerSchemePersonality.get();
}

SystemMode::Type PolicyServicesPlatformState::getSystemMode() const
{
	return getDptfManager()->getEventCache()->systemMode.get();
}

ScenarioMode::Type PolicyServicesPlatformState::getScenarioMode() const
{
	return getDptfManager()->getEventCache()->scenarioMode.get();
}

DttGamingMode::Type PolicyServicesPlatformState::getDttGamingMode() const
{
	return getDptfManager()->getEventCache()->dttGamingMode.get();
}

Bool PolicyServicesPlatformState::getApplicationOptimizationStatus() const
{
	return getDptfManager()->getEventCache()->applicationOptimizationStatus.get();
}

UIntN PolicyServicesPlatformState::getMobileNotification(OsMobileNotificationType::Type notificationType) const
{
	switch (notificationType)
	{
	case OsMobileNotificationType::EmergencyCallMode:
		return getDptfManager()->getEventCache()->emergencyCallModeState.get();

	case OsMobileNotificationType::ScreenState:
		return (UIntN)getDptfManager()->getEventCache()->screenState.get();

	default:
		throw dptf_exception(
			"No cached values for requested OS Mobile Notification Type "
			+ OsMobileNotificationType::ToString(notificationType));
	}
}

OnOffToggle::Type PolicyServicesPlatformState::getMixedRealityMode() const
{
	return getDptfManager()->getEventCache()->mixedRealityMode.get();
}

OnOffToggle::Type PolicyServicesPlatformState::getOsGameMode() const
{
	return getDptfManager()->getEventCache()->osGameMode.get();
}

OsUserPresence::Type PolicyServicesPlatformState::getOsUserPresence() const
{
	return getDptfManager()->getEventCache()->osUserPresence.get();
}

OsSessionState::Type PolicyServicesPlatformState::getSessionState() const
{
	return getDptfManager()->getEventCache()->sessionState.get();
}

OnOffToggle::Type PolicyServicesPlatformState::getScreenState() const
{
	return getDptfManager()->getEventCache()->screenState.get();
}

UIntN PolicyServicesPlatformState::getBatteryCount() const
{
	return getDptfManager()->getEventCache()->batteryCount.get();
}

UIntN PolicyServicesPlatformState::getPowerSlider() const
{
	return getDptfManager()->getEventCache()->powerSlider.get();
}

SensorUserPresence::Type PolicyServicesPlatformState::getPlatformUserPresence() const
{
	return getDptfManager()->getEventCache()->platformUserPresence.get();
}

UserInteraction::Type PolicyServicesPlatformState::getUserInteraction() const
{
	return getDptfManager()->getEventCache()->userInteraction.get();
}

OnOffToggle::Type PolicyServicesPlatformState::getTpgPowerState() const
{
	return getDptfManager()->getEventCache()->tpgPowerState.get();
}

OnOffToggle::Type PolicyServicesPlatformState::getCollaborationMode() const
{
	return getDptfManager()->getEventCache()->collaborationMode.get();
}

SystemInBag::Type PolicyServicesPlatformState::getSystemInBag() const
{
	return getDptfManager()->getEventCache()->systemInBag.get();
}