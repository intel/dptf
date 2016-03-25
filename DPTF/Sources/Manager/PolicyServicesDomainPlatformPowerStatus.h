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
#include "PolicyServices.h"
#include "DomainPlatformPowerStatusInterface.h"

class PolicyServicesDomainPlatformPowerStatus final : public PolicyServices,
    public DomainPlatformPowerStatusInterface
{
public:

    PolicyServicesDomainPlatformPowerStatus(DptfManagerInterface* dptfManager, UIntN policyIndex);

    virtual Power getMaxBatteryPower(UIntN participantIndex, UIntN domainIndex) override;
    virtual Power getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex) override;
    virtual Power getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex) override;
    virtual DptfBuffer getBatteryStatus(UIntN participantIndex, UIntN domainIndex) override;
    virtual DptfBuffer getBatteryInformation(UIntN participantIndex, UIntN domainIndex) override;
    virtual PlatformPowerSource::Type getPlatformPowerSource(UIntN participantIndex, UIntN domainIndex) override;
    virtual ChargerType::Type getChargerType(UIntN participantIndex, UIntN domainIndex) override;
    virtual Power getACPeakPower(UIntN participantIndex, UIntN domainIndex) override;
    virtual TimeSpan getACPeakTimeWindow(UIntN participantIndex, UIntN domainIndex) override;
    virtual Power getPlatformBatterySteadyState(UIntN participantIndex, UIntN domainIndex) override;
};