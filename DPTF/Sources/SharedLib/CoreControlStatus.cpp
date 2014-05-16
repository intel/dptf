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

#include "CoreControlStatus.h"
#include "XmlNode.h"
#include "StatusFormat.h"

CoreControlStatus::CoreControlStatus(UIntN numActiveLogicalProcessors) :
    m_numActiveLogicalProcessors(numActiveLogicalProcessors)
{
}

UIntN CoreControlStatus::getNumActiveLogicalProcessors(void) const
{
    return m_numActiveLogicalProcessors;
}

Bool CoreControlStatus::operator==(const CoreControlStatus& rhs) const
{
    return (m_numActiveLogicalProcessors == rhs.m_numActiveLogicalProcessors);
}

Bool CoreControlStatus::operator!=(const CoreControlStatus& rhs) const
{
    return !(*this == rhs);
}

XmlNode* CoreControlStatus::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("core_control_status");

    root->addChild(XmlNode::createDataElement("active_logical_processors", StatusFormat::friendlyValue(m_numActiveLogicalProcessors)));

    return root;
}