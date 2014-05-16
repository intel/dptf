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

#include "PerformanceControlDynamicCaps.h"
#include "StatusFormat.h"
#include "XmlNode.h"

PerformanceControlDynamicCaps::PerformanceControlDynamicCaps(UIntN currentLowerLimitIndex,
    UIntN currentUpperLimitIndex) : m_currentLowerLimitIndex(currentLowerLimitIndex),
    m_currentUpperLimitIndex(currentUpperLimitIndex)
{
    //FIXME: add test and throw invalid_argument if needed.
    //       Which number is higher between the two?
}

UIntN PerformanceControlDynamicCaps::getCurrentLowerLimitIndex(void) const
{
    return m_currentLowerLimitIndex;
}

UIntN PerformanceControlDynamicCaps::getCurrentUpperLimitIndex(void) const
{
    return m_currentUpperLimitIndex;
}

XmlNode* PerformanceControlDynamicCaps::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("performance_control_dynamic_caps");

    root->addChild(XmlNode::createDataElement("upper_limit_index", StatusFormat::friendlyValue(m_currentUpperLimitIndex)));
    root->addChild(XmlNode::createDataElement("lower_limit_index", StatusFormat::friendlyValue(m_currentLowerLimitIndex)));

    return root;
}