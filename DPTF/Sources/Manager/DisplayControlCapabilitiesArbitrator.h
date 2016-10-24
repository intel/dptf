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

#pragma once

#include "Dptf.h"
#include "DisplayControlDynamicCaps.h"

class dptf_export DisplayControlCapabilitiesArbitrator
{
public:

    DisplayControlCapabilitiesArbitrator();
    ~DisplayControlCapabilitiesArbitrator();

    // arbitrate() returns true if the arbitrated value has changed
    Bool arbitrate(UIntN policyIndex, const DisplayControlDynamicCaps& caps);
    Bool arbitrateLockRequests(UIntN policyIndex, Bool lock);
    DisplayControlDynamicCaps getArbitratedDisplayControlCapabilities() const;
    Bool getArbitratedLock() const;
    void removeRequestsForPolicy(UIntN policyIndex);

private:

    std::map<UIntN, UIntN> m_requestedMaxDisplayIndex;
    std::map<UIntN, UIntN> m_requestedMinDisplayIndex;
    std::map<UIntN, Bool> m_requestedLocks;

    void updatePolicyRequest(const DisplayControlDynamicCaps &caps, UIntN policyIndex);
    void updatePolicyLockRequest(Bool lock, UIntN policyIndex);
    UIntN getLowestMaxDisplayIndex() const;
    UIntN getHighestMinDisplayIndex() const;
};