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

#include "PerformanceControlSet.h"
#include "XmlNode.h"

PerformanceControlSet::PerformanceControlSet(const std::vector<PerformanceControl>& performanceControl) :
    m_performanceControl(performanceControl)
{
}

UIntN PerformanceControlSet::getCount(void) const
{
    return static_cast<UIntN>(m_performanceControl.size());
}

const PerformanceControl& PerformanceControlSet::operator[](UIntN index) const
{
    return m_performanceControl.at(index);
}

Bool PerformanceControlSet::operator==(const PerformanceControlSet& rhs) const
{
    return (m_performanceControl == rhs.m_performanceControl);
}

Bool PerformanceControlSet::operator!=(const PerformanceControlSet& rhs) const
{
    return !(*this == rhs);
}

XmlNode* PerformanceControlSet::getXml()
{
    XmlNode* root = XmlNode::createWrapperElement("performance_control_set");

    for (UIntN i = 0; i < m_performanceControl.size(); i++)
    {
        root->addChild(m_performanceControl[i].getXml());
    }

    return root;
}