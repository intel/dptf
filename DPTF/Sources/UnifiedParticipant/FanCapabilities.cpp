/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "FanCapabilities.h"

FanCapabilities::FanCapabilities()
	: m_fanCapabilities(Constants::Invalid)
{
}

FanCapabilities::FanCapabilities(UInt32 fanCapabilities)
{
	m_fanCapabilities = fanCapabilities;
}

UInt32 FanCapabilities::getFanCapabilities() const
{
	return m_fanCapabilities;
}

void FanCapabilities::setFanCapabilities(UInt32 fanCapabilities)
{
	m_fanCapabilities = fanCapabilities;
}

std::shared_ptr<XmlNode> FanCapabilities::getXml() const
{
	auto capabilityList = XmlNode::createWrapperElement("fan_capability_list");

	for (UInt32 capabilityIndex = 0; capabilityIndex < FanCapabilityNames.size(); capabilityIndex++)
	{
		capabilityList->addChild(createCapabilityXmlNode(FanCapabilityNames[capabilityIndex], capabilityIndex));
	}

	return capabilityList;
}

std::shared_ptr<XmlNode> FanCapabilities::createCapabilityXmlNode(std::string capabilityName, UInt32 capabilityIndex)
	const
{
	auto capabilityNode = XmlNode::createWrapperElement("fan_capability");
	capabilityNode->addChild(XmlNode::createDataElement("fan_capability_name", capabilityName));

	UInt32 mask = (1 << capabilityIndex);
	UInt32 capabilityIndicator = (m_fanCapabilities & mask);

	if (capabilityIndicator > 0)
	{
		capabilityNode->addChild(XmlNode::createDataElement("fan_capability_value", "Supported"));
	}
	else
	{
		capabilityNode->addChild(XmlNode::createDataElement("fan_capability_value", "Not Supported"));
	}

	return capabilityNode;
}

FanCapabilities::~FanCapabilities()
{
}