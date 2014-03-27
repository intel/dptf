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
#include "PerformanceControlStatus.h"
#include "PerformanceControlSetCachedProperty.h"
#include "PerformanceControlStatusCachedProperty.h"
#include "PerformanceControlCapabilitiesCachedProperty.h"

// this facade class provides a simpler interface on top of performance controls as well as combines all of the
// performance control properties and capabilities into a single class.  these properties also have the ability to be 
// cached.
class dptf_export PerformanceControlFacade
{
public:

    PerformanceControlFacade(
        UIntN participantIndex,
        UIntN domainIndex,
        const DomainProperties& domainProperties,
        const PolicyServicesInterfaceContainer& policyServices);
    ~PerformanceControlFacade();

    // controls
    Bool supportsPerformanceControls();
    void initializeControlsIfNeeded();
    void setControl(UIntN performanceControlIndex);

    // properties
    void refreshCapabilities();
    void refreshControls();
    PerformanceControlStatus getStatus() const;
    const PerformanceControlSet& getControls();
    const PerformanceControlDynamicCaps& getDynamicCapabilities();

private:

    // services
    PolicyServicesInterfaceContainer m_policyServices;

    // domain properties
    UIntN m_participantIndex;
    UIntN m_domainIndex;
    DomainProperties m_domainProperties;

    // control properties
    PerformanceControlSetCachedProperty m_performanceControlSetProperty;
    PerformanceControlCapabilitiesCachedProperty m_performanceControlCapabilitiesProperty;
    Bool m_controlsHaveBeenInitialized;
    UIntN m_lastIssuedPerformanceControlIndex;
};