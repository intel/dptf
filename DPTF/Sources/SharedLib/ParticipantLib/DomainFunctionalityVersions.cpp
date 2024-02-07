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

#include "DomainFunctionalityVersions.h"
#include "esif_sdk_capability_type.h"

DomainFunctionalityVersions::DomainFunctionalityVersions(void)
	: activeControlVersion(0)
	, coreControlVersion(0)
	, displayControlVersion(0)
	, domainPriorityVersion(0)
	, energyControlVersion(0)
	, performanceControlVersion(0)
	, powerControlVersion(0)
	, powerStatusVersion(0)
	, temperatureVersion(0)
	, temperatureThresholdVersion(0)
	, utilizationVersion(0)
	, rfProfileControlVersion(0)
	, rfProfileStatusVersion(0)
	, systemPowerControlVersion(0)
	, platformPowerStatusVersion(0)
	, activityStatusVersion(0)
	, peakPowerControlVersion(0)
	, processorControlVersion(0)
	, batteryStatusVersion(0)
	, socWorkloadClassificationVersion(0)
	, managerControlVersion(0)
	, dynamicEppVersion(0)
	, biasControlVersion(0)
{
}

DomainFunctionalityVersions::DomainFunctionalityVersions(UInt8 capabilityBytes[])
{
	activeControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_ACTIVE_CONTROL];
	coreControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_CORE_CONTROL];
	displayControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_DISPLAY_CONTROL];
	domainPriorityVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY];
	energyControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_ENERGY_CONTROL];
	performanceControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PERF_CONTROL];
	powerControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_POWER_CONTROL];
	powerStatusVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_POWER_STATUS];
	temperatureVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_TEMP_STATUS];
	temperatureThresholdVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD];
	utilizationVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_UTIL_STATUS];
	rfProfileControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL];
	rfProfileStatusVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS];
	systemPowerControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PSYS_CONTROL];
	platformPowerStatusVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PLAT_POWER_STATUS];
	activityStatusVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_ACTIVITY_STATUS];
	peakPowerControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL];
	processorControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL];
	batteryStatusVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_BATTERY_STATUS];
	socWorkloadClassificationVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_WORKLOAD_CLASSIFICATION];
	managerControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_MANAGER];
	dynamicEppVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_DYNAMIC_EPP];
	biasControlVersion = capabilityBytes[ESIF_CAPABILITY_TYPE_BIAS_CONTROL];
}

Bool DomainFunctionalityVersions::operator==(const DomainFunctionalityVersions& domainFunctionality)
{
	return (
		(activeControlVersion == domainFunctionality.activeControlVersion)
		&& (coreControlVersion == domainFunctionality.coreControlVersion)
		&& (displayControlVersion == domainFunctionality.displayControlVersion)
		&& (domainPriorityVersion == domainFunctionality.domainPriorityVersion)
		&& (energyControlVersion == domainFunctionality.energyControlVersion)
		&& (performanceControlVersion == domainFunctionality.performanceControlVersion)
		&& (powerControlVersion == domainFunctionality.powerControlVersion)
		&& (powerStatusVersion == domainFunctionality.powerStatusVersion)
		&& (temperatureVersion == domainFunctionality.temperatureVersion)
		&& (temperatureThresholdVersion == domainFunctionality.temperatureThresholdVersion)
		&& (utilizationVersion == domainFunctionality.utilizationVersion)
		&& (rfProfileControlVersion == domainFunctionality.rfProfileControlVersion)
		&& (rfProfileStatusVersion == domainFunctionality.rfProfileStatusVersion)
		&& (systemPowerControlVersion == domainFunctionality.systemPowerControlVersion)
		&& (platformPowerStatusVersion == domainFunctionality.platformPowerStatusVersion)
		&& (activityStatusVersion == domainFunctionality.activityStatusVersion)
		&& (peakPowerControlVersion == domainFunctionality.peakPowerControlVersion)
		&& (processorControlVersion == domainFunctionality.processorControlVersion)
		&& (batteryStatusVersion == domainFunctionality.batteryStatusVersion)
		&& (socWorkloadClassificationVersion == domainFunctionality.socWorkloadClassificationVersion)
		&& (managerControlVersion == domainFunctionality.managerControlVersion)
		&& (dynamicEppVersion == domainFunctionality.dynamicEppVersion)
		&& (biasControlVersion == domainFunctionality.biasControlVersion));
}

Bool DomainFunctionalityVersions::operator!=(const DomainFunctionalityVersions& domainFunctionality)
{
	return !(*this == domainFunctionality);
}
