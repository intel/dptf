/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
#include "PowerControlStatusSet.h"
#include "XmlNode.h"

PowerControlStatusSet::PowerControlStatusSet(std::vector<PowerControlStatus> powerControlStatus) :
    m_powerControlStatus(powerControlStatus)
{
}

UIntN PowerControlStatusSet::getCount(void) const
{
    return static_cast<UIntN>(m_powerControlStatus.size());
}

const PowerControlStatus& PowerControlStatusSet::operator[](UIntN index) const
{
    return m_powerControlStatus.at(index);
}

Bool PowerControlStatusSet::operator==(const PowerControlStatusSet& rhs) const
{
    return m_powerControlStatus == rhs.m_powerControlStatus;
}

Bool PowerControlStatusSet::operator!=(const PowerControlStatusSet& rhs) const
{
    return m_powerControlStatus != rhs.m_powerControlStatus;
}

XmlNode* PowerControlStatusSet::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("power_control_status_set");

    for (UIntN i = 0; i < m_powerControlStatus.size(); i++)
    {
        root->addChild(m_powerControlStatus[i].getXml());
    }

    return root;
}