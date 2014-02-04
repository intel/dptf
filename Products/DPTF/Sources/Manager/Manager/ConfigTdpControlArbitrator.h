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
// Highest config tdp control index wins.  This is the lowest power (tdp).
//

class ConfigTdpControlArbitrator
{
public:

    ConfigTdpControlArbitrator(DptfManager* dptfManager);
    ~ConfigTdpControlArbitrator(void);

    // arbitrate() returns true if the arbitrated value has changed
    Bool arbitrate(UIntN policyIndex, UIntN configTdpControlIndex);

    UIntN getArbitratedConfigTdpControlIndex(void) const;
    void clearPolicyCachedData(UIntN policyIndex);

private:

    // hide the copy constructor and assignment operator.
    ConfigTdpControlArbitrator(const ConfigTdpControlArbitrator& rhs);
    ConfigTdpControlArbitrator& operator=(const ConfigTdpControlArbitrator& rhs);

    DptfManager* m_dptfManager;

    UIntN m_arbitratedConfigTdpControlIndex;
    std::vector<UIntN> m_requestedConfigTdpControlIndex;
};