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

#include "ActiveControlSet.h"
#include "XmlNode.h"

ActiveControlSet::ActiveControlSet(const std::vector<ActiveControl>& activeControl) :
    m_activeControl(activeControl)
{
}

UIntN ActiveControlSet::getCount(void) const
{
    return static_cast<UIntN>(m_activeControl.size());
}

const ActiveControl& ActiveControlSet::operator[](UIntN index) const
{
    return m_activeControl.at(index);
}

Bool ActiveControlSet::operator==(const ActiveControlSet& rhs) const
{
    return (m_activeControl == rhs.m_activeControl);
}

Bool ActiveControlSet::operator!=(const ActiveControlSet& rhs) const
{
    return !(*this == rhs);
}

XmlNode* ActiveControlSet::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("active_control_set");

    for (UIntN i = 0; i < m_activeControl.size(); i++)
    {
        root->addChild(m_activeControl[i].getXml());
    }

    return root;
}