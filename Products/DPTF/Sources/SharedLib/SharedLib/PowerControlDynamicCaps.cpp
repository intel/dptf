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

#include "PowerControlDynamicCaps.h"
#include "StatusFormat.h"
#include "XmlNode.h"

PowerControlDynamicCaps::PowerControlDynamicCaps(PowerControlType::Type powerControlType, Power minPowerLimit,
    Power maxPowerLimit, Power powerStepSize, UIntN minTimeWindow, UIntN maxTimeWindow, Percentage minDutyCycle,
    Percentage maxDutyCycle) : m_powerControlType(powerControlType), m_minPowerLimit(minPowerLimit),
    m_maxPowerLimit(maxPowerLimit), m_powerStepSize(powerStepSize), m_minTimeWindow(minTimeWindow),
    m_maxTimeWindow(maxTimeWindow), m_minDutyCycle(minDutyCycle), m_maxDutyCycle(maxDutyCycle)
{
}

PowerControlType::Type PowerControlDynamicCaps::getPowerControlType(void) const
{
    return m_powerControlType;
}

Power PowerControlDynamicCaps::getMinPowerLimit(void) const
{
    return m_minPowerLimit;
}

Power PowerControlDynamicCaps::getMaxPowerLimit(void) const
{
    return m_maxPowerLimit;
}

Power PowerControlDynamicCaps::getPowerStepSize(void) const
{
    return m_powerStepSize;
}

UIntN PowerControlDynamicCaps::getMinTimeWindow(void) const
{
    return m_minTimeWindow;
}

UIntN PowerControlDynamicCaps::getMaxTimeWindow(void) const
{
    return m_maxTimeWindow;
}

Percentage PowerControlDynamicCaps::getMinDutyCycle(void) const
{
    return m_minDutyCycle;
}

Percentage PowerControlDynamicCaps::getMaxDutyCycle(void) const
{
    return m_maxDutyCycle;
}

Bool PowerControlDynamicCaps::operator==(const PowerControlDynamicCaps& rhs) const
{
    return
        (m_powerControlType == rhs.m_powerControlType &&
         m_minPowerLimit == rhs.m_minPowerLimit &&
         m_maxPowerLimit == rhs.m_maxPowerLimit &&
         m_powerStepSize == rhs.m_powerStepSize &&
         m_minTimeWindow == rhs.m_minTimeWindow &&
         m_maxTimeWindow == rhs.m_maxTimeWindow &&
         m_minDutyCycle == rhs.m_minDutyCycle &&
         m_maxDutyCycle == rhs.m_maxDutyCycle);
}

Bool PowerControlDynamicCaps::operator!=(const PowerControlDynamicCaps& rhs) const
{
    return !(*this == rhs);
}

XmlNode* PowerControlDynamicCaps::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("power_control_dynamic_caps");
    root->addChild(XmlNode::createDataElement("control_type", PowerControlType::ToString(m_powerControlType)));
    root->addChild(XmlNode::createDataElement("max_power_limit", m_maxPowerLimit.toString()));
    root->addChild(XmlNode::createDataElement("min_power_limit", m_minPowerLimit.toString()));
    root->addChild(XmlNode::createDataElement("power_step_size", m_powerStepSize.toString()));
    root->addChild(XmlNode::createDataElement("max_time_window", StatusFormat::friendlyValue(m_maxTimeWindow)));
    root->addChild(XmlNode::createDataElement("min_time_window", StatusFormat::friendlyValue(m_minTimeWindow)));
    root->addChild(XmlNode::createDataElement("max_duty_cycle", m_maxDutyCycle.toString()));
    root->addChild(XmlNode::createDataElement("min_duty_cycle", m_minDutyCycle.toString()));
    return root;
}