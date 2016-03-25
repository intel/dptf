/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "DomainFunctionalityVersions.h"
#include "esif_sdk_capability_type.h"

DomainFunctionalityVersions::DomainFunctionalityVersions(void) :
    activeControlVersion(0),
    configTdpControlVersion(0),
    coreControlVersion(0),
    displayControlVersion(0),
    domainPriorityVersion(0),
    performanceControlVersion(0),
    powerControlVersion(0),
    powerStatusVersion(0),
    temperatureVersion(0),
    temperatureThresholdVersion(0),
    utilizationVersion(0),
    pixelClockControlVersion(0),
    pixelClockStatusVersion(0),
    rfProfileControlVersion(0),
    rfProfileStatusVersion(0),
    hardwareDutyCycleVersion(0),
    platformPowerControlVersion(0),
    platformPowerStatusVersion(0)
{
}

DomainFunctionalityVersions::DomainFunctionalityVersions(UInt8 capabilityBytes[])
{
    activeControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL];
    configTdpControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_CTDP_CONTROL];
    coreControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_CORE_CONTROL];
    displayControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL];
    domainPriorityVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY];
    performanceControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PERF_CONTROL];
    powerControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_POWER_CONTROL];
    powerStatusVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_POWER_STATUS];
    temperatureVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_TEMP_STATUS];
    temperatureThresholdVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD];
    utilizationVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_UTIL_STATUS];
    pixelClockControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PIXELCLOCK_CONTROL];
    pixelClockStatusVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PIXELCLOCK_STATUS];
    rfProfileControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL];
    rfProfileStatusVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS];
    hardwareDutyCycleVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_HDC_CONTROL];
    platformPowerControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PSYS_CONTROL];
    platformPowerStatusVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS];
}