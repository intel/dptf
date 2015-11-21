/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#include "DomainPlatformPowerControlInterface.h"
#include "ControlBase.h"
#include "ParticipantServicesInterface.h"

class DomainPlatformPowerControlBase : public ControlBase, public DomainPlatformPowerControlInterface
{
    friend class PlatformPowerControlState;

public:

    DomainPlatformPowerControlBase(UIntN participantIndex, UIntN domainIndex,
        ParticipantServicesInterface* participantServicesInterface);
    virtual ~DomainPlatformPowerControlBase();

protected:

    Bool checkEnabled(PlatformPowerLimitType::Type limitType);
    void setEnabled(PlatformPowerLimitType::Type limitType, Bool enable);
};