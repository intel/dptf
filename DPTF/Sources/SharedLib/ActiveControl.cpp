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

#include "ActiveControl.h"
#include "StatusFormat.h"
#include "XmlNode.h"

ActiveControl::ActiveControl(UIntN controlId, UIntN tripPoint, UIntN speed, UIntN noiseLevel, UIntN power) :
    m_controlId(controlId), m_tripPoint(tripPoint), m_speed(speed), m_noiseLevel(noiseLevel), m_power(power)
{
}

UIntN ActiveControl::getControlId(void) const
{
    return m_controlId;
}

UIntN ActiveControl::getTripPoint(void) const
{
    return m_tripPoint;
}

UIntN ActiveControl::getSpeed(void) const
{
    return m_speed;
}

UIntN ActiveControl::getNoiseLevel(void) const
{
    return m_noiseLevel;
}

UIntN ActiveControl::getPower(void) const
{
    return m_power;
}

Bool ActiveControl::operator==(const ActiveControl& rhs) const
{
    return
        ((this->getControlId() == rhs.getControlId()) &&
         (this->getTripPoint() == rhs.getTripPoint()) &&
         (this->getSpeed() == rhs.getSpeed()) &&
         (this->getNoiseLevel() == rhs.getNoiseLevel()) &&
         (this->getPower() == rhs.getPower()));
}

Bool ActiveControl::operator!=(const ActiveControl& rhs) const
{
    return !(*this == rhs);
}

XmlNode* ActiveControl::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("active_control");

    root->addChild(XmlNode::createDataElement("control_id", StatusFormat::friendlyValue(m_controlId)));
    root->addChild(XmlNode::createDataElement("trip_point", StatusFormat::friendlyValue(m_tripPoint)));
    root->addChild(XmlNode::createDataElement("speed", StatusFormat::friendlyValue(m_speed)));
    root->addChild(XmlNode::createDataElement("noise_level", StatusFormat::friendlyValue(m_noiseLevel)));
    root->addChild(XmlNode::createDataElement("power", StatusFormat::friendlyValue(m_power)));

    return root;
}