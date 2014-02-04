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

#include "DomainProxy.h"
#include "StatusFormat.h"
using namespace std;

DomainProxy::DomainProxy(
    UIntN participantIndex,
    UIntN domainIndex,
    DomainProperties domainProperties,
    ParticipantProperties participantProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_domainProperties(domainProperties),
    m_participantProperties(participantProperties),
    m_policyServices(policyServices),
    m_temperatureProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_activeCoolingControl(participantIndex, domainIndex, domainProperties, participantProperties, policyServices),
    m_domainPriorityProperty(participantIndex, domainIndex, domainProperties, policyServices)
{
    // create control facades
    m_performanceControl = std::make_shared<PerformanceControlFacade>(
        participantIndex, domainIndex, domainProperties, policyServices);
    m_powerControl = std::make_shared<PowerControlFacade>(
        participantIndex, domainIndex, domainProperties, policyServices);
    m_displayControl = std::make_shared<DisplayControlFacade>(
        participantIndex, domainIndex, domainProperties, policyServices);
    m_coreControl = std::make_shared<CoreControlFacade>(
        participantIndex, domainIndex, domainProperties, policyServices);
    m_configTdpControl = std::make_shared<ConfigTdpControlFacade>(
        participantIndex, domainIndex, domainProperties, policyServices);
    m_radioFrequencyControl = std::make_shared<RadioFrequencyControlFacade>(
        participantIndex, domainIndex, domainProperties, policyServices);
    m_pixelClockControl = std::make_shared<PixelClockControlFacade>(
        participantIndex, domainIndex, domainProperties, policyServices);

    // create control knobs (TODO: move to passive policy)
    m_pstateControlKnob = std::make_shared<PerformanceControlKnob>(
        policyServices, participantIndex, domainIndex,
        m_performanceControl, PerformanceControlType::PerformanceState);
    m_tstateControlKnob = std::make_shared<PerformanceControlKnob>(
        policyServices, participantIndex, domainIndex,
        m_performanceControl, PerformanceControlType::ThrottleState);
    m_powerControlKnob = std::make_shared<PowerControlKnob>(
        policyServices, m_powerControl, participantIndex, domainIndex);
    m_displayControlKnob = std::make_shared<DisplayControlKnob>(
        policyServices, m_displayControl, participantIndex, domainIndex);
    m_coreControlKnob = std::make_shared<CoreControlKnob>(
        policyServices, participantIndex, domainIndex,
        m_coreControl, m_performanceControl);
}

DomainProxy::DomainProxy()
    : m_participantIndex(Constants::Invalid), m_domainIndex(Constants::Invalid), 
    m_domainProperties(Guid(), Constants::Invalid, false, DomainType::Other, "", "", DomainFunctionalityVersions()),
    m_participantProperties(Guid(), "", "", BusType::None, PciInfo(), AcpiInfo()),
    m_temperatureProperty(Constants::Invalid, Constants::Invalid, m_domainProperties,
        PolicyServicesInterfaceContainer()),
    m_domainPriorityProperty(Constants::Invalid, Constants::Invalid, m_domainProperties,
        PolicyServicesInterfaceContainer()),
    m_activeCoolingControl(Constants::Invalid, Constants::Invalid, m_domainProperties, m_participantProperties,
        PolicyServicesInterfaceContainer())
{
}

DomainProxy::~DomainProxy()
{
}

UIntN DomainProxy::getParticipantIndex() const
{
    return m_participantIndex;
}

UIntN DomainProxy::getDomainIndex() const
{
    return m_domainIndex;
}

const DomainProperties& DomainProxy::getDomainProperties() const
{
    return m_domainProperties;
}

TemperatureProperty& DomainProxy::getTemperatureProperty()
{
    return m_temperatureProperty;
}

ActiveCoolingControl& DomainProxy::getActiveCoolingControl()
{
    return m_activeCoolingControl;
}

DomainPriorityCachedProperty& DomainProxy::getDomainPriorityProperty()
{
    return m_domainPriorityProperty;
}

PerformanceControlFacade& DomainProxy::getPerformanceControl()
{
    return *m_performanceControl;
}

PowerControlFacade& DomainProxy::getPowerControl()
{
    return *m_powerControl;
}

DisplayControlFacade& DomainProxy::getDisplayControl()
{
    return *m_displayControl;
}

CoreControlFacade& DomainProxy::getCoreControl()
{
    return *m_coreControl;
}

ConfigTdpControlFacade& DomainProxy::getConfigTdpControl()
{
    return *m_configTdpControl;
}

RadioFrequencyControlFacade& DomainProxy::getRadioFrequencyControl() const
{
    return *m_radioFrequencyControl;
}

PixelClockControlFacade& DomainProxy::getPixelClockControl() const
{
    return *m_pixelClockControl;
}

UtilizationStatus DomainProxy::getUtilizationStatus()
{
    return m_policyServices.domainUtilization->getUtilizationStatus(m_participantIndex, m_domainIndex);
}

void DomainProxy::clearTemperatureThresholds()
{
    try
    {
        if (m_temperatureProperty.supportsProperty())
        {
            m_temperatureProperty.setTemperatureNotificationThresholds(
                Temperature::createInvalid(), Temperature::createInvalid());
        }
    }
    catch (...)
    {
    }
}

void DomainProxy::initializeControls()
{
    try
    {
        m_performanceControl->initializeControlsIfNeeded();
    }
    catch (...)
    {
    }

    try
    {
        m_powerControl->initializeControlsIfNeeded();
    }
    catch (...)
    {
    }

    try
    {
        m_coreControl->initializeControlsIfNeeded();
    }
    catch (...)
    {
    }
}

void DomainProxy::limit(void)
{
    // attempt to limit each control in turn.  if any is not supported or has a problem, it will return "true" and the
    // execution will continue to the next limit attempt.  these lines of code take advantage of the 'short circuit'
    // rule in C++ in that the next limit will not be called if the previous one returns false.
    limitPowerAndShouldContinue() &&
    limitPstatesWithCoresAndShouldContinue() &&
    limitCoresAndShouldContinue() &&
    limitTstatesAndContinue() &&
    limitDisplayAndContinue();
}

void DomainProxy::unlimit(void)
{
    // attempt to unlimit each control in turn.  if any is not supported or has a problem, it will return "true" and the
    // execution will continue to the next unlimit attempt.  these lines of code take advantage of the 'short circuit'
    // rule in C++ in that the next unlimit will not be called if the previous one returns false.
    unlimitDisplayAndContinue() &&
    unlimitTstatesAndContinue() &&
    unlimitCoresWithPstatesAndShouldContinue() &&
    unlimitPstatesAndShouldContinue() &&
    unlimitPowerAndShouldContinue();
}

Bool DomainProxy::limitPowerAndShouldContinue()
{
    try
    {
        if (m_powerControlKnob->canLimit())
        {
            m_powerControlKnob->limit();
            return false;
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::limitPstatesWithCoresAndShouldContinue()
{
    try
    {
        if (m_pstateControlKnob->canLimit())
        {
            m_pstateControlKnob->limit();

            try
            {
                if (m_coreControlKnob->canLimit())
                {
                    m_coreControlKnob->limit();
                }
            }
            catch (...)
            {
                // don't throw out of function if limiting cores fails
            }

            return false;
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::limitCoresAndShouldContinue()
{
    try
    {
        if (m_coreControlKnob->canLimit())
        {
            m_coreControlKnob->limit();
            return false;
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::limitTstatesAndContinue()
{
    try
    {
        if (m_tstateControlKnob->canLimit())
        {
            UtilizationStatus utilization = UtilizationStatus(Percentage::createInvalid());
            try
            {
                utilization = getUtilizationStatus();
            }
            catch (...)
            {
                // assume 100% utilization if it does not report utilization
                utilization = UtilizationStatus(1.0);
            }

            // only limit t-states if domain is not idle
            if (utilization.getCurrentUtilization() > Percentage(0.0))
            {
                m_tstateControlKnob->limit();
                return false;
            }
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::limitDisplayAndContinue()
{
    try
    {
        if (m_displayControlKnob->canLimit())
        {
            m_displayControlKnob->limit();
            return false;
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::unlimitDisplayAndContinue()
{
    try
    {
        if (m_displayControlKnob->canUnlimit())
        {
            m_displayControlKnob->unlimit();
            return false;
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::unlimitTstatesAndContinue()
{
    try
    {
        if (m_tstateControlKnob->canUnlimit())
        {
            m_tstateControlKnob->unlimit();
            return false;
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::unlimitCoresWithPstatesAndShouldContinue()
{
    try
    {
        if (m_coreControlKnob->canUnlimit())
        {
            m_coreControlKnob->unlimit();

            try
            {
                if (m_pstateControlKnob->canUnlimit())
                {
                    m_pstateControlKnob->unlimit();
                }
            }
            catch (...)
            {
                // we want to catch errors here
            }
            return false;
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::unlimitPstatesAndShouldContinue()
{
    try
    {
        if (m_pstateControlKnob->canUnlimit())
        {
            m_pstateControlKnob->unlimit();
            return false;
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::unlimitPowerAndShouldContinue()
{
    try
    {
        if (m_powerControlKnob->canUnlimit())
        {
            m_powerControlKnob->unlimit();
            return false;
        }
    }
    catch (...)
    {
        // we want to continue on if this control fails for whatever reason
    }
    return true;
}

Bool DomainProxy::canLimit(void)
{
    return
        m_powerControlKnob->canLimit() ||
        m_pstateControlKnob->canLimit() ||
        m_coreControlKnob->canLimit() ||
        m_tstateControlKnob->canLimit() ||
        m_displayControlKnob->canLimit();
}

Bool DomainProxy::canUnlimit(void)
{
    return
        m_displayControlKnob->canUnlimit() ||
        m_tstateControlKnob->canUnlimit() ||
        m_coreControlKnob->canUnlimit() ||
        m_pstateControlKnob->canUnlimit() ||
        m_powerControlKnob->canUnlimit();
}

XmlNode* DomainProxy::getXmlForPassiveControlKnobs()
{
    XmlNode* domainStatus = XmlNode::createWrapperElement("domain_passive_control_knobs");
    domainStatus->addChild(m_domainProperties.getXml());
    domainStatus->addChild(m_participantProperties.getXml());
    domainStatus->addChild(m_powerControlKnob->getXml());
    domainStatus->addChild(m_pstateControlKnob->getXml());
    domainStatus->addChild(m_coreControlKnob->getXml());
    domainStatus->addChild(m_tstateControlKnob->getXml());
    domainStatus->addChild(m_displayControlKnob->getXml());
    return domainStatus;
}

XmlNode* DomainProxy::getXmlForConfigTdpLevel()
{
    XmlNode* domainStatus = XmlNode::createWrapperElement("domain_config_tdp_level");
    domainStatus->addChild(XmlNode::createDataElement(
        "domain_index", StatusFormat::friendlyValue(getDomainIndex())));
    domainStatus->addChild(XmlNode::createDataElement(
        "domain_name", getDomainProperties().getName()));
    domainStatus->addChild(getConfigTdpControl().getXml());
    return domainStatus;
}