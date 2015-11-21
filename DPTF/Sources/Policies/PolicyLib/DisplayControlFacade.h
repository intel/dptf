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

#include "DisplayControlFacadeInterface.h"

class dptf_export DisplayControlFacade : public DisplayControlFacadeInterface
{
public:

    DisplayControlFacade(
        UIntN participantIndex,
        UIntN domainIndex,
        const DomainProperties& domainProperties,
        const PolicyServicesInterfaceContainer& policyServices);
    ~DisplayControlFacade();

    // controls
    virtual Bool supportsDisplayControls() override;
    virtual void setControl(UIntN displayControlIndex) override;

    // properties
    virtual void refreshCapabilities() override;
    virtual void invalidateControlSet() override;
    virtual const DisplayControlStatus& getStatus() override;
    virtual const DisplayControlSet& getControls() override;
    virtual const DisplayControlDynamicCaps& getCapabilities() override;
    virtual void setValueWithinCapabilities() override;

private:

    // services
    PolicyServicesInterfaceContainer m_policyServices;

    // domain properties
    UIntN m_participantIndex;
    UIntN m_domainIndex;
    DomainProperties m_domainProperties;
    
    // display control properties
    DisplayControlSetCachedProperty m_displayControlSetProperty;
    DisplayControlStatusProperty m_displayControlStatusProperty;
    DisplayControlCapabilitiesCachedProperty m_displayControlCapabilitiesProperty;
};