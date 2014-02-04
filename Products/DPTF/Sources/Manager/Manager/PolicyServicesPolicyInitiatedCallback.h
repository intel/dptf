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

#pragma once

#include "PolicyServices.h"
#include "PolicyInitiatedCallbackInterface.h"

class PolicyServicesPolicyInitiatedCallback final : public PolicyServices, public PolicyInitiatedCallbackInterface
{
public:

    PolicyServicesPolicyInitiatedCallback(DptfManager* dptfManager, UIntN policyIndex);

    virtual UInt64 createPolicyInitiatedImmediateCallback(UInt64 policyDefinedEventCode, UInt64 param1,
        void* param2) override final;
    virtual UInt64 createPolicyInitiatedDeferredCallback(UInt64 policyDefinedEventCode, UInt64 param1,
        void* param2, UInt64 timeDeltaInMilliSeconds) override final;
    virtual Bool removePolicyInitiatedCallback(UInt64 callbackHandle) override final;
};