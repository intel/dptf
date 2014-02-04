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

#include "ControlStatus.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

ControlStatus::ControlStatus(const std::string& name, UIntN min, UIntN max, UIntN current)
    : m_name(name), m_min(min), m_max(max), m_current(current)
{
}

XmlNode* ControlStatus::getXml()
{
    XmlNode* control = XmlNode::createWrapperElement("control");
    control->addChild(XmlNode::createDataElement("name", m_name));
    control->addChild(XmlNode::createDataElement("min", friendlyValue(m_min)));
    control->addChild(XmlNode::createDataElement("max", friendlyValue(m_max)));
    control->addChild(XmlNode::createDataElement("current", friendlyValue(m_current)));
    return control;
}