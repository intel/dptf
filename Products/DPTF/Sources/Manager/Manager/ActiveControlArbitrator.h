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

#include "Dptf.h"


class DptfManager;

//
// Arbitration Rule:
//
// 1) if fine grain fan control is supported we only arbitrate based on percentage.  otherwise we only arbitrate
//    based on control index.
// 2) for percentage the highest fan speed wins
// 3) for control index the lowest index (which is highest fan speed) wins
//

class ActiveControlArbitrator
{
public:

    ActiveControlArbitrator(DptfManager* dptfManager);
    ~ActiveControlArbitrator(void);

    // arbitrate() returns true if the arbitrated value has changed
    Bool arbitrate(UIntN policyIndex, const Percentage& fanSpeed);
    Bool arbitrate(UIntN policyIndex, UIntN activeControlIndex);

    Percentage getArbitratedFanSpeedPercentage(void) const;
    UIntN getArbitratedActiveControlIndex(void) const;

    void clearPolicyCachedData(UIntN policyIndex);

private:

    // hide the copy constructor and assignment operator.
    ActiveControlArbitrator(const ActiveControlArbitrator& rhs);
    ActiveControlArbitrator& operator=(const ActiveControlArbitrator& rhs);

    DptfManager* m_dptfManager;

    Percentage m_arbitratedFanSpeedPercentage;
    std::vector<Percentage> m_requestedfanSpeedPercentage;

    UIntN m_arbitratedActiveControlIndex;
    std::vector<UIntN> m_requestedActiveControlIndex;
};