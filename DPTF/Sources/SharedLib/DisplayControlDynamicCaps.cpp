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

#include "DisplayControlDynamicCaps.h"
#include "XmlNode.h"
#include "StatusFormat.h"

DisplayControlDynamicCaps::DisplayControlDynamicCaps(UIntN currentUpperLimit, UIntN currentLowerLimit) :
    m_currentUpperLimit(currentUpperLimit), m_currentLowerLimit(currentLowerLimit)
{
}

UIntN DisplayControlDynamicCaps::getCurrentUpperLimit(void) const
{
    return m_currentUpperLimit;
}

UIntN DisplayControlDynamicCaps::getCurrentLowerLimit(void) const
{
    return m_currentLowerLimit;
}

Bool DisplayControlDynamicCaps::operator==(const DisplayControlDynamicCaps& rhs) const
{
    return (m_currentUpperLimit == rhs.m_currentUpperLimit &&
            m_currentLowerLimit == rhs.m_currentLowerLimit);
}

Bool DisplayControlDynamicCaps::operator!=(const DisplayControlDynamicCaps& rhs) const
{
    return !(*this == rhs);
}

XmlNode* DisplayControlDynamicCaps::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("display_control_dynamic_caps");

    root->addChild(XmlNode::createDataElement("upper_limit_index", StatusFormat::friendlyValue(m_currentUpperLimit)));
    root->addChild(XmlNode::createDataElement("lower_limit_index", StatusFormat::friendlyValue(m_currentLowerLimit)));

    return root;
}