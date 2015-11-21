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

#include "PolicyServicesPlatformPowerState.h"
#include "EsifServices.h"
#include "esif_sdk_data_misc.h"
#include "esif_ccb_thread.h"

void* ThreadSleep(void* self);
void* ThreadHibernate(void* self);

// MagicToken is defined in the ESIF HLD for use when calling sleep/hibernate/showdown.
static const UInt32 MagicToken = 0x12345678;

PolicyServicesPlatformPowerState::PolicyServicesPlatformPowerState(DptfManagerInterface* dptfManager, 
    UIntN policyIndex) :
    PolicyServices(dptfManager, policyIndex)
{

}

void PolicyServicesPlatformPowerState::sleep(void)
{
    throwIfNotWorkItemThread();

    esif_thread_t threadId;
    esif_ccb_thread_create(&threadId, ThreadSleep, this);
}

void PolicyServicesPlatformPowerState::hibernate(void)
{
    throwIfNotWorkItemThread();

    esif_thread_t threadId;
    esif_ccb_thread_create(&threadId, ThreadHibernate, this);
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

void* ThreadSleep(void* self)
{
    if (self != nullptr)
    {
        try
        {
            PolicyServicesPlatformPowerState* me = (PolicyServicesPlatformPowerState*)self;
            me->getEsifServices()->primitiveExecuteSetAsUInt32(SET_SYSTEM_SLEEP, MagicToken);
        }
        catch (...)
        {
        	// do nothing on error
        }
    }
    return nullptr;
}

void* ThreadHibernate(void* self)
{
    if (self != nullptr)
    {
        try
        {
            PolicyServicesPlatformPowerState* me = (PolicyServicesPlatformPowerState*)self;
            me->getEsifServices()->primitiveExecuteSetAsUInt32(SET_SYSTEM_HIBERNATE, MagicToken);
        }
        catch (...)
        {
            // do nothing on error
        }
    }
    return nullptr;
}