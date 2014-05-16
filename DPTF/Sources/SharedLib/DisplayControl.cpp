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

#include "DisplayControl.h"
#include "XmlNode.h"
#include "StatusFormat.h"

DisplayControl::DisplayControl(Percentage brightness) : m_brightness(brightness)
{
}

Percentage DisplayControl::getBrightness(void) const
{
    return m_brightness;
}

Bool DisplayControl::operator==(const DisplayControl& rhs) const
{
    return (this->getBrightness() == rhs.getBrightness());
}

Bool DisplayControl::operator!=(const DisplayControl& rhs) const
{
    return !(*this == rhs);
}

Bool DisplayControl::operator<(const DisplayControl& rhs) const
{
    return (m_brightness < rhs.m_brightness);
}

XmlNode* DisplayControl::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("display_control");
    root->addChild(XmlNode::createDataElement("brightness", m_brightness.toString()));
    return root;
}