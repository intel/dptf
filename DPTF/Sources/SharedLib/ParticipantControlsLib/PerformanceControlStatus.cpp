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

#include "PerformanceControlStatus.h"
#include "StatusFormat.h"
#include "XmlNode.h"

PerformanceControlStatus::PerformanceControlStatus(UIntN currentControlSetIndex)
	: m_currentControlSetIndex(currentControlSetIndex)
{
}

UIntN PerformanceControlStatus::getCurrentControlSetIndex(void) const
{
	return m_currentControlSetIndex;
}

std::shared_ptr<XmlNode> PerformanceControlStatus::getXml(void)
{
	auto root = XmlNode::createWrapperElement("performance_control_status");

	root->addChild(XmlNode::createDataElement("current_index", StatusFormat::friendlyValue(m_currentControlSetIndex)));

	return root;
}
