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
#include "HardwareDutyCycleControlFacadeInterface.h"

class dptf_export HardwareDutyCycleControlFacade : public HardwareDutyCycleControlFacadeInterface
{
public:

    HardwareDutyCycleControlFacade(
        UIntN participantIndex,
        UIntN domainIndex,
        const DomainProperties& domainProperties,
        const ParticipantProperties& participantProperties,
        const PolicyServicesInterfaceContainer& policyServices);
    virtual ~HardwareDutyCycleControlFacade(void);

    virtual DptfBuffer getHardwareDutyCycleUtilizationSet() const override;
    virtual Bool isEnabledByPlatform() const override;
    virtual Bool isSupportedByPlatform() const override;
    virtual Bool isEnabledByOperatingSystem() const override;
    virtual Bool isSupportedByOperatingSystem() const override;
    virtual Bool isHdcOobEnabled() const override;
    virtual void setHdcOobEnable(const UInt8& hdcOobEnable) override;
    virtual void setHardwareDutyCycle(const Percentage& dutyCycle) override;
    virtual Percentage getHardwareDutyCycle() const override;
    virtual Bool supportsHdcControls() const;


private:

    PolicyServicesInterfaceContainer m_policyServices;
    DomainProperties m_domainProperties;
    ParticipantProperties m_participantProperties;
    UIntN m_participantIndex;
    UIntN m_domainIndex;
};