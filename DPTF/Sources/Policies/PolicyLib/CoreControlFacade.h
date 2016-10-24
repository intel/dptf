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

#include "CoreControlFacadeInterface.h"
#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"

// this facade class provides a simpler interface on top of core controls as well as combines all of the core control 
// properties and capabilities into a single class.  these properties also have the ability to be cached.
class dptf_export CoreControlFacade : public CoreControlFacadeInterface
{
public:

    CoreControlFacade(
        UIntN participantIndex,
        UIntN domainIndex,
        const DomainProperties& domainProperties,
        const PolicyServicesInterfaceContainer& policyServices);
    ~CoreControlFacade();

    // controls
    virtual Bool supportsCoreControls() override;
    virtual void initializeControlsIfNeeded() override;
    virtual void setControlsToMax() override;
    virtual void setActiveCoreControl(CoreControlStatus coreControl) override;

    // properties
    virtual CoreControlStatus getStatus() override;
    virtual CoreControlDynamicCaps getDynamicCapabilities() override;
    virtual CoreControlStaticCaps getStaticCapabilities() override;
    virtual CoreControlLpoPreference getPreferences() override;
    virtual void refreshCapabilities() override;
    virtual void refreshPreferences() override;
    virtual void setValueWithinCapabilities() override;

private:

    // services
    PolicyServicesInterfaceContainer m_policyServices;

    // domain properties
    UIntN m_participantIndex;
    UIntN m_domainIndex;
    DomainProperties m_domainProperties;

    // core control properties
    CoreControlCapabilitiesCachedProperty m_capabilities;
    CoreControlPreferencesCachedProperty m_preferences;
    Bool m_controlsHaveBeenInitialized;
    CoreControlStatus m_lastSetCoreControlStatus;

    void throwIfControlNotSupported();
};