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
#include "PowerControlStatusSet.h"

class DptfManager;

//
// Arbitration Rule:
//
// for each (PL1, PL2, Pl3)
//  lowest power level wins
//  lowest time window wins
//
// *It is allowed to mix and match between policies.  The lowest PL1 power level can be from one policy and the
//  lowest time window can be from another policy
//


class PowerControlArbitrator
{
public:

    PowerControlArbitrator(DptfManager* dptfManager);
    ~PowerControlArbitrator(void);

    // arbitrate() returns true if the arbitrated value has changed
    Bool arbitrate(UIntN policyIndex, const PowerControlStatusSet& powerControlStatusSet);

    PowerControlStatusSet getArbitratedPowerControlStatusSet(void) const;
    void clearPolicyCachedData(UIntN policyIndex);

private:

    // hide the copy constructor and assignment operator.
    PowerControlArbitrator(const PowerControlArbitrator& rhs);
    PowerControlArbitrator& operator=(const PowerControlArbitrator& rhs);

    DptfManager* m_dptfManager;

    PowerControlStatusSet* m_arbitratedPowerControlStatusSet;
    std::vector<PowerControlStatusSet*> m_requestedPowerControlStatusSet;

    void savePolicyRequest(UIntN policyIndex, const PowerControlStatusSet& powerControlStatusSet);
    std::vector<PowerControlStatus> createInitialArbitratedPowerControlStatusVector();
    void arbitrate(std::vector<PowerControlStatus> &arbitratedPowerControlStatusVector);
    PowerControlStatusSet getArbitratedPowerControlStatusSet(std::vector<PowerControlStatus>& arbitratedPowerControlStatusVector);
};