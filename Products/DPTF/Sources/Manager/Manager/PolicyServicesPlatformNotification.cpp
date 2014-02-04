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

#include "PolicyServicesPlatformNotification.h"
#include "Policy.h"
#include "EsifServices.h"
#include "esif_data_misc.h"
#include "esif_ccb_memory.h"
#include "esif_primitive_type.h"
#include "esif_data_type.h"

PolicyServicesPlatformNotification::PolicyServicesPlatformNotification(DptfManager* dptfManager, UIntN policyIndex) :
    PolicyServices(dptfManager, policyIndex)
{
}

void PolicyServicesPlatformNotification::notifyPlatformPolicyTakeControl()
{
    throwIfNotWorkItemThread();
    executeOsc(getPolicy()->getGuid(), OscActionTakeControl);
}

void PolicyServicesPlatformNotification::notifyPlatformPolicyReleaseControl()
{
    throwIfNotWorkItemThread();
    executeOsc(getPolicy()->getGuid(), OscActionReleaseControl);
}

void PolicyServicesPlatformNotification::executeOsc(const Guid& guid, UInt32 oscCapabilities)
{
    Bool successful = false;

    struct esif_data_complex_osc osc;
    esif_ccb_memcpy(osc.guid, guid, Guid::GuidSize);
    osc.revision = 1;
    osc.count = 2;
    osc.status = 0;
    osc.capabilities = oscCapabilities;

    try
    {
        getEsifServices()->primitiveExecuteSet(SET_OPERATING_SYSTEM_CAPABILITIES, ESIF_DATA_STRUCTURE, &osc,
            sizeof(esif_data_complex_osc), sizeof(esif_data_complex_osc));
        successful = true;
    }
    catch (...)
    {
    }

    if (osc.status & 0xE)
    {
        if (osc.status & 0x2)
        {
            throw dptf_exception("Platform supports _OSC but unable to process _OSC request.");
        }

        if (osc.status & 0x4)
        {
            throw dptf_exception("Platform failed _OSC Reason: Unrecognized UUID.");
        }

        if (osc.status & 0x8)
        {
            throw dptf_exception("Platform failed _OSC Reason: Unrecognized revision.");
        }
    }

    if (successful == false)
    {
        throw dptf_exception("Failure during execution of _OSC.");
    }
}