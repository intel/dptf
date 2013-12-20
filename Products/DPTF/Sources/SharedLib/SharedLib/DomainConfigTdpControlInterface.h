/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
#include "ConfigTdpControlDynamicCaps.h"
#include "ConfigTdpControlStatus.h"
#include "ConfigTdpControl.h"
#include "ConfigTdpControlSet.h"

class DomainConfigTdpControlInterface
{
public:

    virtual ~DomainConfigTdpControlInterface()
    {
    };

    virtual ConfigTdpControlDynamicCaps getConfigTdpControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) = 0;
    virtual ConfigTdpControlStatus getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex) = 0;
    virtual ConfigTdpControlSet getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex) = 0;
    virtual void setConfigTdpControl(UIntN participantIndex, UIntN domainIndex, UIntN controlIndex) = 0;
};