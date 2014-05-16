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

#include "PowerControlStatus.h"
#include "StatusFormat.h"
#include "XmlNode.h"

PowerControlStatus::PowerControlStatus(PowerControlType::Type powerControlType, Power currentPowerLimit,
    UIntN currentTimeWindow, Percentage currentDutyCycle) : m_powerControlType(powerControlType),
    m_currentPowerLimit(currentPowerLimit), m_currentTimeWindow(currentTimeWindow),
    m_currentDutyCycle(currentDutyCycle)
{
}

PowerControlType::Type PowerControlStatus::getPowerControlType(void) const
{
    return m_powerControlType;
}

Power PowerControlStatus::getCurrentPowerLimit(void) const
{
    return m_currentPowerLimit;
}

UIntN PowerControlStatus::getCurrentTimeWindow(void) const
{
    return m_currentTimeWindow;
}

Percentage PowerControlStatus::getCurrentDutyCycle(void) const
{
    return m_currentDutyCycle;
}

Bool PowerControlStatus::operator==(const PowerControlStatus& rhs) const
{
    return
        (getPowerControlType() == rhs.getPowerControlType()) &&
        (getCurrentPowerLimit() == rhs.getCurrentPowerLimit()) &&
        (getCurrentTimeWindow() == rhs.getCurrentTimeWindow()) &&
        (getCurrentDutyCycle() == rhs.getCurrentDutyCycle());
}

Bool PowerControlStatus::operator!=(const PowerControlStatus& rhs) const
{
    return !(*this == rhs);
}

XmlNode* PowerControlStatus::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("power_control_status");
    root->addChild(XmlNode::createDataElement("control_type", PowerControlType::ToString(m_powerControlType)));
    root->addChild(XmlNode::createDataElement("power_limit", m_currentPowerLimit.toString()));
    root->addChild(XmlNode::createDataElement("time_window", StatusFormat::friendlyValue(m_currentTimeWindow)));
    root->addChild(XmlNode::createDataElement("duty_cycle", m_currentDutyCycle.toString()));
    return root;
}