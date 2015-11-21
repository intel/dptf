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

#include "PlatformPowerControlFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

PlatformPowerControlFacade::PlatformPowerControlFacade(
    UIntN participantIndex, UIntN domainIndex, 
    const DomainProperties& domainProperties, 
    const PolicyServicesInterfaceContainer& policyServices)
    : m_policyServices(policyServices), 
    m_domainProperties(domainProperties),
    m_participantIndex(participantIndex), 
    m_domainIndex(domainIndex)
{
}

PlatformPowerControlFacade::~PlatformPowerControlFacade(void)
{
}

Bool PlatformPowerControlFacade::isPl1PowerLimitEnabled(void)
{
    if (m_pl1PowerLimitEnabled.isInvalid())
    {
        m_pl1PowerLimitEnabled.set(m_policyServices.domainPlatformPowerControl->isPlatformPowerLimitEnabled(
            m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL1));
    }
    return m_pl1PowerLimitEnabled.get();
}

Bool PlatformPowerControlFacade::isPl2PowerLimitEnabled(void)
{
    if (m_pl2PowerLimitEnabled.isInvalid())
    {
        m_pl2PowerLimitEnabled.set(m_policyServices.domainPlatformPowerControl->isPlatformPowerLimitEnabled(
            m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL2));
    }
    return m_pl2PowerLimitEnabled.get();
}

Bool PlatformPowerControlFacade::isPl3PowerLimitEnabled(void)
{
    if (m_pl3PowerLimitEnabled.isInvalid())
    {
        m_pl3PowerLimitEnabled.set(m_policyServices.domainPlatformPowerControl->isPlatformPowerLimitEnabled(
            m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL3));
    }
    return m_pl3PowerLimitEnabled.get();
}

Power PlatformPowerControlFacade::getPl1PowerLimit(void)
{
    if (m_pl1PowerLimit.isInvalid())
    {
        m_pl1PowerLimit.set(m_policyServices.domainPlatformPowerControl->getPlatformPowerLimit(
            m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL1));
    }
    return m_pl1PowerLimit.get();
}

Power PlatformPowerControlFacade::getPl2PowerLimit(void)
{
    if (m_pl2PowerLimit.isInvalid())
    {
        m_pl2PowerLimit.set(m_policyServices.domainPlatformPowerControl->getPlatformPowerLimit(
            m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL2));
    }
    return m_pl2PowerLimit.get();
}

Power PlatformPowerControlFacade::getPl3PowerLimit(void)
{
    if (m_pl3PowerLimit.isInvalid())
    {
        m_pl3PowerLimit.set(m_policyServices.domainPlatformPowerControl->getPlatformPowerLimit(
            m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL3));
    }
    return m_pl3PowerLimit.get();
}

TimeSpan PlatformPowerControlFacade::getPl1TimeWindow(void)
{
    if (m_pl1TimeWindow.isInvalid())
    {
        m_pl1TimeWindow.set(m_policyServices.domainPlatformPowerControl->getPlatformPowerLimitTimeWindow(
            m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL1));
    }
    return m_pl1TimeWindow.get();
}

TimeSpan PlatformPowerControlFacade::getPl3TimeWindow(void)
{
    if (m_pl3TimeWindow.isInvalid())
    {
        m_pl3TimeWindow.set(m_policyServices.domainPlatformPowerControl->getPlatformPowerLimitTimeWindow(
            m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL3));
    }
    return m_pl3TimeWindow.get();
}

Percentage PlatformPowerControlFacade::getPl3DutyCycle(void)
{
    if (m_pl3DutyCycle.isInvalid())
    {
        m_pl3DutyCycle.set(m_policyServices.domainPlatformPowerControl->getPlatformPowerLimitDutyCycle(
            m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL3));
    }
    return m_pl3DutyCycle.get();
}

void PlatformPowerControlFacade::setPl1PowerLimit(const Power& powerLimit)
{
    m_policyServices.domainPlatformPowerControl->setPlatformPowerLimit(
        m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL1, powerLimit);
    m_pl1PowerLimit.set(powerLimit);
}

void PlatformPowerControlFacade::setPl2PowerLimit(const Power& powerLimit)
{
    m_policyServices.domainPlatformPowerControl->setPlatformPowerLimit(
        m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL2, powerLimit);
    m_pl2PowerLimit.set(powerLimit);
}

void PlatformPowerControlFacade::setPl3PowerLimit(const Power& powerLimit)
{
    m_policyServices.domainPlatformPowerControl->setPlatformPowerLimit(
        m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL3, powerLimit);
    m_pl3PowerLimit.set(powerLimit);
}

void PlatformPowerControlFacade::setPl1TimeWindow(const TimeSpan& timeWindow)
{
    m_policyServices.domainPlatformPowerControl->setPlatformPowerLimitTimeWindow(
        m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL1, timeWindow);
    m_pl1TimeWindow.set(timeWindow);
}

void PlatformPowerControlFacade::setPl3TimeWindow(const TimeSpan& timeWindow)
{
    m_policyServices.domainPlatformPowerControl->setPlatformPowerLimitTimeWindow(
        m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL3, timeWindow);
    m_pl1TimeWindow.set(timeWindow);
}

void PlatformPowerControlFacade::setPl3DutyCycle(const Percentage& dutyCycle)
{
    m_policyServices.domainPlatformPowerControl->setPlatformPowerLimitDutyCycle(
        m_participantIndex, m_domainIndex, PlatformPowerLimitType::PSysPL3, dutyCycle);
    m_pl3DutyCycle.set(dutyCycle);
}

XmlNode* PlatformPowerControlFacade::getXml() const
{
    XmlNode* control = XmlNode::createWrapperElement("platform_power_control");
    control->addChild(createPl1XmlData());
    control->addChild(createPl2XmlData());
    control->addChild(createPl3XmlData());
    return control;
}

XmlNode* PlatformPowerControlFacade::createPl1XmlData() const
{
    XmlNode* pl1 = XmlNode::createWrapperElement("power_limit_1");
    if (m_pl1PowerLimitEnabled.isValid())
    {
        pl1->addChild(XmlNode::createDataElement("enabled", friendlyValue(m_pl1PowerLimitEnabled.get())));
    }
    else
    {
        pl1->addChild(XmlNode::createDataElement("enabled", "Invalid"));
    }

    if (m_pl1PowerLimit.isValid())
    {
        pl1->addChild(XmlNode::createDataElement("power_limit", m_pl1PowerLimit.get().toString()));
    }
    else
    {
        pl1->addChild(XmlNode::createDataElement("power_limit", "Invalid"));
    }
    
    if (m_pl1TimeWindow.isValid())
    {
        pl1->addChild(XmlNode::createDataElement("time_window", m_pl1TimeWindow.get().toStringMicroseconds()));
    }
    else
    {
        pl1->addChild(XmlNode::createDataElement("time_window", "Invalid"));
    }
    pl1->addChild(XmlNode::createDataElement("duty_cycle", "NA"));
    return pl1;
}

XmlNode* PlatformPowerControlFacade::createPl2XmlData() const
{
    XmlNode* pl2 = XmlNode::createWrapperElement("power_limit_2");
    if (m_pl2PowerLimitEnabled.isValid())
    {
        pl2->addChild(XmlNode::createDataElement("enabled", friendlyValue(m_pl2PowerLimitEnabled.get())));
    }
    else
    {
        pl2->addChild(XmlNode::createDataElement("enabled", "Invalid"));
    }

    if (m_pl2PowerLimit.isValid())
    {
        pl2->addChild(XmlNode::createDataElement("power_limit", m_pl2PowerLimit.get().toString()));
    }
    else
    {
        pl2->addChild(XmlNode::createDataElement("power_limit", "Invalid"));
    }

    pl2->addChild(XmlNode::createDataElement("time_window", "NA"));
    pl2->addChild(XmlNode::createDataElement("duty_cycle", "NA"));
    return pl2;
}

XmlNode* PlatformPowerControlFacade::createPl3XmlData() const
{
    XmlNode* pl3 = XmlNode::createWrapperElement("power_limit_3");
    if (m_pl3PowerLimitEnabled.isValid())
    {
        pl3->addChild(XmlNode::createDataElement("enabled", friendlyValue(m_pl3PowerLimitEnabled.get())));
    }
    else
    {
        pl3->addChild(XmlNode::createDataElement("enabled", "Invalid"));
    }

    if (m_pl3PowerLimit.isValid())
    {
        pl3->addChild(XmlNode::createDataElement("power_limit", m_pl3PowerLimit.get().toString()));
    }
    else
    {
        pl3->addChild(XmlNode::createDataElement("power_limit", "Invalid"));
    }

    if (m_pl3TimeWindow.isValid())
    {
        pl3->addChild(XmlNode::createDataElement("time_window", m_pl3TimeWindow.get().toStringMicroseconds()));
    }
    else
    {
        pl3->addChild(XmlNode::createDataElement("time_window", "Invalid"));
    }

    if (m_pl3DutyCycle.isValid())
    {
        pl3->addChild(XmlNode::createDataElement("duty_cycle", m_pl3DutyCycle.get().toString()));
    }
    else
    {
        pl3->addChild(XmlNode::createDataElement("duty_cycle", "Invalid"));
    }
    return pl3;
}