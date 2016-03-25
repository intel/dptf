/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "DisplayControlStatus.h"
#include "XmlNode.h"
#include "StatusFormat.h"

DisplayControlStatus::DisplayControlStatus(UIntN brightnessLimitIndex) :
    m_brightnessLimitIndex(brightnessLimitIndex)
{
}

UIntN DisplayControlStatus::getBrightnessLimitIndex(void) const
{
    return m_brightnessLimitIndex;
}

std::shared_ptr<XmlNode> DisplayControlStatus::getXml(void)
{
    auto root = XmlNode::createWrapperElement("display_control_status");

    root->addChild(XmlNode::createDataElement("brightness_limit_index", StatusFormat::friendlyValue(m_brightnessLimitIndex)));

    return root;
}