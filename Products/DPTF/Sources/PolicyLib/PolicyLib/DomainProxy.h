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
#include "TemperatureProperty.h"
#include "ActiveCoolingControl.h"
#include "DomainPriorityCachedProperty.h"

#include "PerformanceControlFacade.h"
#include "PowerControlFacade.h"
#include "DisplayControlFacade.h"
#include "CoreControlFacade.h"
#include "ConfigTdpControlFacade.h"
#include "RadioFrequencyControlFacade.h"
#include "PixelClockControlFacade.h"

#include "PowerControlKnob.h"
#include "DisplayControlKnob.h"
#include "CoreControlKnob.h"
#include "PerformanceControlKnob.h"

#include <memory>

// represents a domain inside a participant.  holds cached records of all properties and potential controls for the
// domain.
class dptf_export DomainProxy
{
public:

    DomainProxy();
    DomainProxy(
        UIntN participantIndex,
        UIntN domainIndex,
        DomainProperties domainProperties,
        ParticipantProperties participantProperties,
        const PolicyServicesInterfaceContainer& policyServices);
    ~DomainProxy();

    // domain properties
    UIntN getParticipantIndex() const;
    UIntN getDomainIndex() const;
    const DomainProperties& getDomainProperties() const;
    TemperatureProperty& getTemperatureProperty();
    DomainPriorityCachedProperty& getDomainPriorityProperty();
    UtilizationStatus getUtilizationStatus();

    // domain actions
    void clearTemperatureThresholds();

    // control facades
    void initializeControls();
    ActiveCoolingControl& getActiveCoolingControl();
    PerformanceControlFacade& getPerformanceControl();
    PowerControlFacade& getPowerControl();
    DisplayControlFacade& getDisplayControl();
    CoreControlFacade& getCoreControl();
    ConfigTdpControlFacade& getConfigTdpControl();
    RadioFrequencyControlFacade& getRadioFrequencyControl() const;
    PixelClockControlFacade& getPixelClockControl() const;

    // passive controls (TODO: move to passive policy)
    void limit(void);
    void unlimit(void);
    Bool canLimit(void);
    Bool canUnlimit(void);

    // status
    XmlNode* getXmlForPassiveControlKnobs();
    XmlNode* getXmlForConfigTdpLevel();
    
private:

    // domain properties
    UIntN m_participantIndex;
    UIntN m_domainIndex;
    DomainProperties m_domainProperties;
    ParticipantProperties m_participantProperties;
    TemperatureProperty m_temperatureProperty;
    DomainPriorityCachedProperty m_domainPriorityProperty;
    ActiveCoolingControl m_activeCoolingControl;

    // control facades
    std::shared_ptr<PerformanceControlFacade> m_performanceControl;
    std::shared_ptr<PowerControlFacade> m_powerControl;
    std::shared_ptr<DisplayControlFacade> m_displayControl;
    std::shared_ptr<CoreControlFacade> m_coreControl;
    std::shared_ptr<ConfigTdpControlFacade> m_configTdpControl;
    std::shared_ptr<RadioFrequencyControlFacade> m_radioFrequencyControl;
    std::shared_ptr<PixelClockControlFacade> m_pixelClockControl;

    // services
    PolicyServicesInterfaceContainer m_policyServices;

    // control knobs (TODO: move to passive policy)
    std::shared_ptr<PerformanceControlKnob> m_pstateControlKnob;
    std::shared_ptr<PerformanceControlKnob> m_tstateControlKnob;
    std::shared_ptr<PowerControlKnob> m_powerControlKnob;
    std::shared_ptr<DisplayControlKnob> m_displayControlKnob;
    std::shared_ptr<CoreControlKnob> m_coreControlKnob;

    // limiting/unlimiting helper functions (TODO: move to passive policy)
    Bool limitPowerAndShouldContinue();
    Bool limitPstatesWithCoresAndShouldContinue();
    Bool limitCoresAndShouldContinue();
    Bool limitTstatesAndContinue();
    Bool limitDisplayAndContinue();
    Bool unlimitDisplayAndContinue();
    Bool unlimitTstatesAndContinue();
    Bool unlimitCoresWithPstatesAndShouldContinue();
    Bool unlimitPstatesAndShouldContinue();
    Bool unlimitPowerAndShouldContinue();
};