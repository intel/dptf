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

#include "ActiveControlStatus.h"
#include "StatusFormat.h"
#include "XmlNode.h"

ActiveControlStatus::ActiveControlStatus(UIntN currentControlId, UIntN currentSpeed) :
    m_currentControlId(currentControlId), m_currentSpeed(currentSpeed)
{
}

UIntN ActiveControlStatus::getCurrentControlId(void) const
{
    return m_currentControlId;
}

UIntN ActiveControlStatus::getCurrentSpeed(void) const
{
    return m_currentSpeed;
}

Bool ActiveControlStatus::operator==(const ActiveControlStatus rhs) const
{
    return
        ((this->getCurrentControlId() == rhs.getCurrentControlId()) &&
         (this->getCurrentSpeed() == rhs.getCurrentSpeed()));
}

Bool ActiveControlStatus::operator!=(const ActiveControlStatus rhs) const
{
    return !(*this == rhs);
}

XmlNode* ActiveControlStatus::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("active_control_status");

    root->addChild(XmlNode::createDataElement("current_control_id", StatusFormat::friendlyValue(m_currentControlId)));
    root->addChild(XmlNode::createDataElement("current_speed", StatusFormat::friendlyValue(m_currentSpeed)));

    return root;
}