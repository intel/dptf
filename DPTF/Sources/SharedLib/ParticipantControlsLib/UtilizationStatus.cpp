/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "UtilizationStatus.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

UtilizationStatus::UtilizationStatus(Percentage currentUtilization)
	: m_currentUtilization(currentUtilization)
{
}

Percentage UtilizationStatus::getCurrentUtilization(void) const
{
	return m_currentUtilization;
}

std::shared_ptr<XmlNode> UtilizationStatus::getXml(std::string tag)
{
	std::shared_ptr<XmlNode> root = XmlNode::createWrapperElement("util_status");
	root->addChild(XmlNode::createDataElement(tag, m_currentUtilization.toString()));
	root->addChild(XmlNode::createDataElement("control_name", "Utilization Status"));
	return root;
}
