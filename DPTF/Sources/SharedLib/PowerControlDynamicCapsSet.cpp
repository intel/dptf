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

#include "PowerControlDynamicCapsSet.h"
#include "XmlNode.h"

PowerControlDynamicCapsSet::PowerControlDynamicCapsSet(const std::vector<PowerControlDynamicCaps>& powerControlDynamicCaps) :
    m_powerControlDynamicCaps(powerControlDynamicCaps)
{
}

UIntN PowerControlDynamicCapsSet::getCount(void) const
{
    return static_cast<UIntN>(m_powerControlDynamicCaps.size());
}

const PowerControlDynamicCaps& PowerControlDynamicCapsSet::operator[](UIntN index) const
{
    return m_powerControlDynamicCaps.at(index);
}

Bool PowerControlDynamicCapsSet::operator==(const PowerControlDynamicCapsSet& rhs) const
{
    return (m_powerControlDynamicCaps == rhs.m_powerControlDynamicCaps);
}

Bool PowerControlDynamicCapsSet::operator!=(const PowerControlDynamicCapsSet& rhs) const
{
    return !(*this == rhs);
}

XmlNode* PowerControlDynamicCapsSet::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("power_control_dynamic_caps_set");

    for (UIntN i = 0; i < m_powerControlDynamicCaps.size(); i++)
    {
        root->addChild(m_powerControlDynamicCaps[i].getXml());
    }

    return root;
}