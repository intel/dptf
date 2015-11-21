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

#include "PowerControlFacadeInterface.h"
#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"

// this facade class provides a simpler interface on top of power controls as well as combines all of the power
// control properties and capabilities into a single class.  these properties also have the ability to be cached.
class dptf_export PowerControlFacade : public PowerControlFacadeInterface
{
public:

    PowerControlFacade(
        UIntN participantIndex,
        UIntN domainIndex,
        const DomainProperties& domainProperties,
        const PolicyServicesInterfaceContainer& policyServices);
    virtual ~PowerControlFacade();

    // commands
    virtual void initializeControlsIfNeeded() override;
    virtual void setControlsToMax() override;
    virtual void setCapability(const PowerControlDynamicCaps& capability) override;
    virtual void refreshCapabilities() override;
    virtual void setPowerLimitPL1(const Power& powerLimit) override;
    virtual void setPowerLimitPL2(const Power& powerLimit) override;
    virtual void setPowerLimitPL3(const Power& powerLimit) override;
    virtual void setPowerLimitPL4(const Power& powerLimit) override;
    virtual void setPowerLimitTimeWindowPL1(const TimeSpan& timeWindow) override;
    virtual void setPowerLimitTimeWindowPL3(const TimeSpan& timeWindow) override;
    virtual void setPowerLimitDutyCyclePL3(const Percentage& dutyCycle) override;
    virtual void setValuesWithinCapabilities() override;

    // queries
    virtual Bool supportsPowerControls() const override;
    virtual Bool supportsPowerStatus() const override;
    virtual Bool isPl1PowerLimitEnabled(void) override;
    virtual Bool isPl2PowerLimitEnabled(void) override;
    virtual Bool isPl3PowerLimitEnabled(void) override;
    virtual Bool isPl4PowerLimitEnabled(void) override;
    virtual PowerStatus getCurrentPower() override;
    virtual const PowerControlDynamicCapsSet& getCapabilities() override;
    virtual Power getPowerLimitPL1() override;
    virtual Power getPowerLimitPL2() override;
    virtual Power getPowerLimitPL3() override;
    virtual Power getPowerLimitPL4() override;
    virtual TimeSpan getPowerLimitTimeWindowPL1() override;
    virtual TimeSpan getPowerLimitTimeWindowPL3() override;
    virtual Percentage getPowerLimitDutyCyclePL3() override;
    virtual Power getLivePowerLimitPL1() override;
    virtual Power getLivePowerLimitPL2() override;
    virtual Power getLivePowerLimitPL3() override;
    virtual Power getLivePowerLimitPL4() override;
    virtual TimeSpan getLivePowerLimitTimeWindowPL1() override;
    virtual TimeSpan getLivePowerLimitTimeWindowPL3() override;
    virtual Percentage getLivePowerLimitDutyCyclePL3() override;

private:

    PolicyServicesInterfaceContainer m_policyServices;
    UIntN m_participantIndex;
    UIntN m_domainIndex;
    DomainProperties m_domainProperties;

    PowerStatusProperty m_powerStatusProperty;
    PowerControlCapabilitiesCachedProperty m_powerControlCapabilitiesProperty;
    Bool m_controlsHaveBeenInitialized;
    
    std::map<PowerControlType::Type, Power> m_lastSetPowerLimit;
    std::map<PowerControlType::Type, TimeSpan> m_lastSetTimeWindow;
    std::map<PowerControlType::Type, Percentage> m_lastSetDutyCycle;

    void setTimeWindowsWithinCapabilities();
    void setPowerLimitsWithinCapabilities();
    void throwIfControlNotSupported() const;
};