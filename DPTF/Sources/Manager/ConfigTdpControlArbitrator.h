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

//
// Arbitration Rule:
//
// Highest config tdp control index wins.  This is the lowest power (tdp).
//

class dptf_export ConfigTdpControlArbitrator
{
public:

    ConfigTdpControlArbitrator();
    ~ConfigTdpControlArbitrator(void);

    // arbitrate() returns true if the arbitrated value has changed
    Bool arbitrate(UIntN policyIndex, UIntN configTdpControlIndex);

    UIntN getArbitratedConfigTdpControlIndex(void) const;
    void clearPolicyCachedData(UIntN policyIndex);

private:

    // hide the copy constructor.
    ConfigTdpControlArbitrator(const ConfigTdpControlArbitrator& rhs);

    UIntN m_arbitratedConfigTdpControlIndex;
    std::map<UIntN, UIntN> m_requestedConfigTdpControlIndex;
};