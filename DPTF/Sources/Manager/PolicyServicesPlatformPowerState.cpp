/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#include "PolicyServicesPlatformPowerState.h"
#include "EsifServices.h"
#include "esif_data_misc.h"

// MagicToken is defined in the ESIF HLD for use when calling sleep/hibernate/showdown.
static const UInt32 MagicToken = 0x12345678;

PolicyServicesPlatformPowerState::PolicyServicesPlatformPowerState(DptfManager* dptfManager, UIntN policyIndex) :
    PolicyServices(dptfManager, policyIndex)
{
}

void PolicyServicesPlatformPowerState::sleep(void)
{
    throwIfNotWorkItemThread();
    getEsifServices()->primitiveExecuteSetAsUInt32(SET_SYSTEM_SLEEP, MagicToken);
}

void PolicyServicesPlatformPowerState::hibernate(void)
{
    throwIfNotWorkItemThread();
    getEsifServices()->primitiveExecuteSetAsUInt32(SET_SYSTEM_HIBERNATE, MagicToken);
}

void PolicyServicesPlatformPowerState::shutDown(const Temperature& currentTemperature,
    const Temperature& tripPointTemperature)
{
    throwIfNotWorkItemThread();

    esif_data_complex_shutdown shutdownData;
    shutdownData.temperature = currentTemperature;
    shutdownData.tripPointTemperature = tripPointTemperature;

    getEsifServices()->primitiveExecuteSet(SET_SYSTEM_SHUTDOWN, ESIF_DATA_STRUCTURE,
        &shutdownData, sizeof(esif_data_complex_shutdown), sizeof(esif_data_complex_shutdown));
}