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
#include "ControlKnobBase.h"
#include "PowerControlFacade.h"
#include <memory>

// control knob for power controls
class dptf_export PowerControlKnob : public ControlKnobBase
{
public:

    PowerControlKnob(
        const PolicyServicesInterfaceContainer& policyServices,
        std::shared_ptr<PowerControlFacade> powerControl,
        UIntN participantIndex,
        UIntN domainIndex);
    ~PowerControlKnob(void);

    virtual void limit(UIntN target) override;
    virtual void unlimit(UIntN target) override;
    virtual Bool canLimit(UIntN target) override;
    virtual Bool canUnlimit(UIntN target) override;
    virtual Bool commitSetting() override;
    virtual void clearRequestForTarget(UIntN target) override;
    virtual void clearAllRequests() override;

    XmlNode* getXml() const;

private:

    std::shared_ptr<PowerControlFacade> m_powerControl;
    std::map<UIntN, Power> m_requests;

    Power calculateNextLowerPowerLimit(
        Power currentPower, Power minimumPowerLimit, Power stepSize, Power currentPowerLimit);
    Power findLowestPowerLimitRequest(const std::map<UIntN, Power>& m_requests);
    Power getTargetRequest(UIntN target);
    UIntN snapToCapabilitiesBounds(Power powerLimit);
    
};