/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
#include "CachedValue.h"
#include "SensorOrientation.h"
#include "CoolingMode.h"
#include "OnOffToggle.h"
#include "SensorSpatialOrientation.h"
#include "OsPowerSource.h"
#include "OsLidState.h"
#include "OsPlatformType.h"
#include "OsDockMode.h"
#include "OsPowerSchemePersonality.h"

class EventCache
{
public:
	EventCache();
	~EventCache();

	CachedValue<SensorOrientation::Type> sensorOrientation;
	CachedValue<CoolingMode::Type> coolingMode;
	CachedValue<OnOffToggle::Type> sensorMotion;
	CachedValue<std::string> foregroundApplication;
	CachedValue<SensorSpatialOrientation::Type> sensorSpatialOrientation;
	CachedValue<OsPowerSource::Type> powerSource;
	CachedValue<OsLidState::Type> lidState;
	CachedValue<UIntN> batteryPercentage;
	CachedValue<OsPlatformType::Type> platformType;
	CachedValue<OsDockMode::Type> dockMode;
	CachedValue<OsPowerSchemePersonality::Type> powerSchemePersonality;
	CachedValue<UIntN> emergencyCallModeState;
	CachedValue<UIntN> screenState;
};
