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

#include "ActivePolicy.h"

// This file is nearly identical between each policy.  We could use a macro in its place.  However, for easier
// debugging, the code is being left as-is.  At a later date it would be fine to convert to a macro.

extern "C"
{
    dptf_export PolicyInterface* CreatePolicyInstance(void)
    {
        PolicyInterface* policyInterface = nullptr;

        try
        {
            policyInterface = new ActivePolicy();
        }
        catch (...)
        {
            policyInterface = nullptr;
        }

        return policyInterface;
    }

    dptf_export void DestroyPolicyInstance(PolicyInterface* policyInterface)
    {
        DELETE_MEMORY_TC(policyInterface);
    }
}