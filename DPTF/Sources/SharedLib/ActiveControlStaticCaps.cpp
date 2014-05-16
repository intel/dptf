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

#include "ActiveControlStaticCaps.h"
#include "StatusFormat.h"
#include "XmlNode.h"

ActiveControlStaticCaps::ActiveControlStaticCaps(Bool fineGrainedControl, Bool lowSpeedNotification, UIntN stepSize) :
    m_fineGrainedControl(fineGrainedControl), m_lowSpeedNotification(lowSpeedNotification), m_stepSize(stepSize)
{
}

Bool ActiveControlStaticCaps::supportsFineGrainedControl(void) const
{
    return m_fineGrainedControl;
}

Bool ActiveControlStaticCaps::supportsLowSpeedNotification(void) const
{
    return m_lowSpeedNotification;
}

UIntN ActiveControlStaticCaps::getStepSize(void) const
{
    return m_stepSize;
}

Bool ActiveControlStaticCaps::operator==(const ActiveControlStaticCaps& rhs) const
{
    return
        ((this->supportsFineGrainedControl() == rhs.supportsFineGrainedControl()) &&
         (this->supportsLowSpeedNotification() == rhs.supportsLowSpeedNotification()) &&
         (this->getStepSize() == rhs.getStepSize()));
}

Bool ActiveControlStaticCaps::operator!=(const ActiveControlStaticCaps& rhs) const
{
    return !(*this == rhs);
}

XmlNode* ActiveControlStaticCaps::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("active_control_static_caps");

    root->addChild(XmlNode::createDataElement("fine_grained_control", StatusFormat::friendlyValue(m_fineGrainedControl)));
    root->addChild(XmlNode::createDataElement("low_speed_notification", StatusFormat::friendlyValue(m_lowSpeedNotification)));
    root->addChild(XmlNode::createDataElement("step_size", StatusFormat::friendlyValue(m_stepSize)));

    return root;
}