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
#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"
#include "XmlNode.h"
#include "PlatformPowerStatusFacadeInterface.h"

class dptf_export PlatformPowerStatusFacade : public PlatformPowerStatusFacadeInterface
{
public:

    PlatformPowerStatusFacade(
        UIntN participantIndex,
        UIntN domainIndex,
        const DomainProperties& domainProperties,
        const PolicyServicesInterfaceContainer& policyServices);
    ~PlatformPowerStatusFacade(void);

    virtual Power getMaxBatteryPower(void) override;
    virtual Power getPlatformRestOfPower(void) override;
    virtual Power getAdapterPowerRating(void) override;
    virtual DptfBuffer getBatteryStatus(void) override;
    virtual DptfBuffer getBatteryInformation(void) override;
    virtual PlatformPowerSource::Type getPlatformPowerSource(void) override;
    virtual ChargerType::Type getChargerType(void) override;
    virtual Power getACPeakPower(void) override;
    virtual TimeSpan getACPeakTimeWindow(void) override;
    virtual Power getPlatformBatterySteadyState(void) override;

    std::shared_ptr<XmlNode> getXml() const;

private:

    // services
    PolicyServicesInterfaceContainer m_policyServices;
    
    // control properties
    DomainProperties m_domainProperties;
    UIntN m_participantIndex;
    UIntN m_domainIndex;

    Power m_maxBatteryPower;
    Power m_platformRestOfPower;
    Power m_adapterPowerRating;
    PlatformPowerSource::Type m_platformPowerSource;
    ChargerType::Type m_chargerType;
    Power m_acPeakPower;
    TimeSpan m_acPeakTimeWindow;
    Power m_batterySteadyState;
};