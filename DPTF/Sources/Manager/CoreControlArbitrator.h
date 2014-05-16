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
#include "CoreControlStatus.h"

class DptfManager;

//
// Arbitration Rule:
//
// Lowest number of active logical processors wins
//

class CoreControlArbitrator
{
public:

    CoreControlArbitrator(DptfManager* dptfManager);
    ~CoreControlArbitrator(void);

    // arbitrate() returns true if the arbitrated value has changed
    Bool arbitrate(UIntN policyIndex, const CoreControlStatus& coreControlStatus);

    CoreControlStatus getArbitratedCoreControlStatus(void) const;
    void clearPolicyCachedData(UIntN policyIndex);

private:

    // hide the copy constructor and assignment operator.
    CoreControlArbitrator(const CoreControlArbitrator& rhs);
    CoreControlArbitrator& operator=(const CoreControlArbitrator& rhs);

    DptfManager* m_dptfManager;

    CoreControlStatus m_arbitratedCoreControlStatus;
    std::vector<CoreControlStatus*> m_requestedCoreControlStatus;
};