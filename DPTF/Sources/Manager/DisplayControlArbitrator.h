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
// Highest display control index wins.  This is the lowest brightness.
//

class DisplayControlArbitrator
{
public:

    DisplayControlArbitrator(DptfManager* dptfManager);
    ~DisplayControlArbitrator(void);

    // arbitrate() returns true if the arbitrated value has changed
    Bool arbitrate(UIntN policyIndex, UIntN displayControlIndex);

    UIntN getArbitratedDisplayControlIndex(void) const;
    void clearPolicyCachedData(UIntN policyIndex);

private:

    // hide the copy constructor and assignment operator.
    DisplayControlArbitrator(const DisplayControlArbitrator& rhs);
    DisplayControlArbitrator& operator=(const DisplayControlArbitrator& rhs);

    DptfManager* m_dptfManager;

    UIntN m_arbitratedDisplayControlIndex;
    std::vector<UIntN> m_requestedDisplayControlIndex;
};