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

#include "ConfigTdpControlStatus.h"
#include "XmlNode.h"
#include "StatusFormat.h"

ConfigTdpControlStatus::ConfigTdpControlStatus(UIntN currentControlIndex) :
    m_currentControlIndex(currentControlIndex)
{
}

UIntN ConfigTdpControlStatus::getCurrentControlIndex(void) const
{
    return m_currentControlIndex;
}

XmlNode* ConfigTdpControlStatus::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("config_tdp_control_status");

    root->addChild(XmlNode::createDataElement("control_index", StatusFormat::friendlyValue(m_currentControlIndex)));

    return root;
}