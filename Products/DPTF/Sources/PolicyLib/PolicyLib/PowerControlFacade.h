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
#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"
#include "PowerControlSetCachedProperty.h"
#include "PowerStatusProperty.h"
#include "PowerControlCapabilitiesCachedProperty.h"

// this facade class provides a simpler interface on top of power controls as well as combines all of the power
// control properties and capabilities into a single class.  these properties also have the ability to be cached.
class dptf_export PowerControlFacade
{
public:

    PowerControlFacade(
        UIntN participantIndex,
        UIntN domainIndex,
        const DomainProperties& domainProperties,
        const PolicyServicesInterfaceContainer& policyServices);
    ~PowerControlFacade();

    // controls
    Bool supportsPowerControls();
    void initializeControlsIfNeeded();
    void setControl(const PowerControlStatus& powerControlStatus, UIntN controlSetIndex);

    // properties
    void refreshCapabilities();
    void refreshControls();
    PowerStatus getCurrentPower();
    PowerControlStatus getLastIssuedPowerLimit();
    const PowerControlStatusSet& getControls();
    const PowerControlDynamicCapsSet& getCapabilities();
    UIntN getPl1ControlSetIndex();
    UIntN getPlControlSetIndex(PowerControlType::Type plType);

private:

    // services
    PolicyServicesInterfaceContainer m_policyServices;

    // domain properties
    UIntN m_participantIndex;
    UIntN m_domainIndex;
    DomainProperties m_domainProperties;

    // control properties
    PowerControlSetCachedProperty m_powerControlSetProperty;
    PowerStatusProperty m_powerStatusProperty;
    PowerControlCapabilitiesCachedProperty m_powerControlCapabilitiesProperty;
    Bool m_controlsHaveBeenInitialized;
    PowerControlStatus m_lastIssuedPowerControlStatus;
};
