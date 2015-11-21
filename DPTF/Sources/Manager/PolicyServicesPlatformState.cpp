/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

SensorMotion::Type PolicyServicesPlatformState::getMotion(void) const
{
    return getDptfManager()->getEventCache()->sensorMotion.get();
}

SensorOrientation::Type PolicyServicesPlatformState::getOrientation(void) const
{
    return getDptfManager()->getEventCache()->sensorOrientation.get();
}

SensorSpatialOrientation::Type PolicyServicesPlatformState::getSpatialOrientation(void) const
{
    return getDptfManager()->getEventCache()->sensorSpatialOrientation.get();
}

OsLidState::Type PolicyServicesPlatformState::getLidState(void) const
{
    return getDptfManager()->getEventCache()->lidState.get();
}

OsPowerSource::Type PolicyServicesPlatformState::getPowerSource(void) const
{
    return getDptfManager()->getEventCache()->powerSource.get();
}

const std::string& PolicyServicesPlatformState::getForegroundApplicationName(void) const
{
    return getDptfManager()->getEventCache()->foregroundApplication.get();
}

CoolingMode::Type PolicyServicesPlatformState::getCoolingMode(void) const
{
    return getDptfManager()->getEventCache()->coolingMode.get();
}

UIntN PolicyServicesPlatformState::getBatteryPercentage(void) const
{
    return getDptfManager()->getEventCache()->batteryPercentage.get();
}

OsPlatformType::Type PolicyServicesPlatformState::getPlatformType(void) const
{
    return getDptfManager()->getEventCache()->platformType.get();
}

OsDockMode::Type PolicyServicesPlatformState::getDockMode(void) const
{
    return getDptfManager()->getEventCache()->dockMode.get();
}
