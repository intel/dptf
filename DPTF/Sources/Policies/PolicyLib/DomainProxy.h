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

#include "Dptf.h"

#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"
#include "ActiveCoolingControl.h"
#include "DomainPriorityCachedProperty.h"

#include "TemperatureControlFacade.h"
#include "PerformanceControlFacade.h"
#include "PowerControlFacade.h"
#include "DisplayControlFacade.h"
#include "CoreControlFacade.h"
#include "ConfigTdpControlFacade.h"
#include "RadioFrequencyControlFacade.h"
#include "PixelClockControlFacade.h"
#include "HardwareDutyCycleControlFacadeInterface.h"

#include "PowerControlKnob.h"
#include "DisplayControlKnob.h"
#include "CoreControlKnob.h"
#include "PerformanceControlKnob.h"
#include "DomainProxyInterface.h"
#include "PlatformPowerControlFacade.h"

// represents a domain inside a participant.  holds cached records of all properties and potential controls for the
// domain.
class dptf_export DomainProxy : public DomainProxyInterface
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
    UIntN getParticipantIndex() const override;
    UIntN getDomainIndex() const override;
    const DomainProperties& getDomainProperties() const override;
    const ParticipantProperties& getParticipantProperties() const override;
    DomainPriorityCachedProperty& getDomainPriorityProperty() override;
    UtilizationStatus getUtilizationStatus() override;

    // domain actions
    void clearTemperatureThresholds() override;

    // control facades
    virtual void initializeControls() override;
    virtual void setControlsToMax() override;
    virtual std::shared_ptr<TemperatureControlFacadeInterface> getTemperatureControl() override;
    virtual ActiveCoolingControl& getActiveCoolingControl() override;
    virtual std::shared_ptr<PerformanceControlFacadeInterface> getPerformanceControl() override;
    virtual std::shared_ptr<PowerControlFacadeInterface> getPowerControl() override;
    virtual std::shared_ptr<PlatformPowerControlFacadeInterface> getPlatformPowerControl() override;
    virtual std::shared_ptr<DisplayControlFacadeInterface> getDisplayControl() override;
    virtual std::shared_ptr<CoreControlFacadeInterface> getCoreControl() override;
    virtual ConfigTdpControlFacade& getConfigTdpControl() override;
    virtual RadioFrequencyControlFacade& getRadioFrequencyControl() const override;
    virtual PixelClockControlFacade& getPixelClockControl() const override;
    virtual std::shared_ptr<HardwareDutyCycleControlFacadeInterface> getHardwareDutyCycleControl() const override;

    // passive controls (TODO: move to passive policy)
    virtual void requestLimit(UIntN target) override;
    virtual void requestUnlimit(UIntN target) override;
    virtual Bool canLimit(UIntN target) override;
    virtual Bool canUnlimit(UIntN target) override;
    virtual Bool commitLimits() override;
    virtual void setArbitratedPowerLimit() override;
    virtual void setArbitratedPerformanceLimit() override;
    virtual void setArbitratedCoreLimit() override;
    virtual void adjustPowerRequests() override;
    virtual void adjustPerformanceRequests() override;
    virtual void adjustCoreRequests() override;
    virtual void setTstateUtilizationThreshold(UtilizationStatus tstateUtilizationThreshold) override;
    virtual void clearAllRequestsForTarget(UIntN target) override;
    virtual void clearAllPerformanceControlRequests() override;
    virtual void clearAllPowerControlRequests() override;
    virtual void clearAllCoreControlRequests() override;
    virtual void clearAllDisplayControlRequests() override;
    virtual void clearAllControlKnobRequests() override;

    // status
    XmlNode* getXmlForPassiveControlKnobs();
    XmlNode* getXmlForConfigTdpLevel();
    virtual XmlNode* getXml() const override;
    
private:

    // domain properties
    UIntN m_participantIndex;
    UIntN m_domainIndex;
    DomainProperties m_domainProperties;
    ParticipantProperties m_participantProperties;
    DomainPriorityCachedProperty m_domainPriorityProperty;
    ActiveCoolingControl m_activeCoolingControl;

    // control facades
    std::shared_ptr<TemperatureControlFacadeInterface> m_temperatureControl;
    std::shared_ptr<PerformanceControlFacade> m_performanceControl;
    std::shared_ptr<PowerControlFacade> m_powerControl;
    std::shared_ptr<PlatformPowerControlFacade> m_platformPowerControl;
    std::shared_ptr<DisplayControlFacadeInterface> m_displayControl;
    std::shared_ptr<CoreControlFacadeInterface> m_coreControl;
    std::shared_ptr<ConfigTdpControlFacade> m_configTdpControl;
    std::shared_ptr<RadioFrequencyControlFacade> m_radioFrequencyControl;
    std::shared_ptr<PixelClockControlFacade> m_pixelClockControl;
    std::shared_ptr<HardwareDutyCycleControlFacadeInterface> m_hardwareDutyCycleControl;

    // services
    PolicyServicesInterfaceContainer m_policyServices;

    // control knobs (TODO: move to passive policy)
    std::shared_ptr<PerformanceControlKnob> m_pstateControlKnob;
    std::shared_ptr<PerformanceControlKnob> m_tstateControlKnob;
    std::shared_ptr<PowerControlKnob> m_powerControlKnob;
    std::shared_ptr<DisplayControlKnob> m_displayControlKnob;
    std::shared_ptr<CoreControlKnob> m_coreControlKnob;
    std::shared_ptr<std::map<UIntN, UIntN>> m_perfControlRequests;

    // limiting/unlimiting helper functions (TODO: move to passive policy)
    Bool requestLimitPowerAndShouldContinue(UIntN target);
    Bool requestLimitPstatesWithCoresAndShouldContinue(UIntN target);
    Bool requestLimitCoresAndShouldContinue(UIntN target);
    Bool requestLimitTstatesAndContinue(UIntN target);
    Bool requestLimitDisplayAndContinue(UIntN target);
    Bool requestUnlimitDisplayAndContinue(UIntN target);
    Bool requestUnlimitTstatesAndContinue(UIntN target);
    Bool requestUnlimitCoresWithPstatesAndShouldContinue(UIntN target);
    Bool requestUnlimitPstatesAndShouldContinue(UIntN target);
    Bool requestUnlimitPowerAndShouldContinue(UIntN target);
};