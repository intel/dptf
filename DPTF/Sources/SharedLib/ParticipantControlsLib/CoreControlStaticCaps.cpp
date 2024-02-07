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

#include "CoreControlStaticCaps.h"
#include "XmlNode.h"
#include "StatusFormat.h"

CoreControlStaticCaps::CoreControlStaticCaps(UIntN totalLogicalProcessors)
	: m_totalLogicalProcessors(totalLogicalProcessors)
{
}

UIntN CoreControlStaticCaps::getTotalLogicalProcessors(void) const
{
	return m_totalLogicalProcessors;
}

std::shared_ptr<XmlNode> CoreControlStaticCaps::getXml(void)
{
	auto root = XmlNode::createWrapperElement("core_control_static_caps");

	root->addChild(
		XmlNode::createDataElement("total_logical_processors", StatusFormat::friendlyValue(m_totalLogicalProcessors)));

	return root;
}
