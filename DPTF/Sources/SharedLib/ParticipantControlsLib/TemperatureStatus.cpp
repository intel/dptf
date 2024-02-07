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

#include "TemperatureStatus.h"
#include "XmlNode.h"
#include "DptfBufferStream.h"

TemperatureStatus::TemperatureStatus()
	: m_currentTemperature(Temperature::createInvalid())
{
}

TemperatureStatus::TemperatureStatus(Temperature currentTemperature)
	: m_currentTemperature(currentTemperature){};

Temperature TemperatureStatus::getCurrentTemperature(void) const
{
	return m_currentTemperature;
}

std::shared_ptr<XmlNode> TemperatureStatus::getXml(void)
{
	std::shared_ptr<XmlNode> root = XmlNode::createWrapperElement("temperature_status");
	root->addChild(XmlNode::createDataElement("temperature_status", getCurrentTemperature().toString()));

	return root;
}

DptfBuffer TemperatureStatus::toDptfBuffer() const
{
	DptfBuffer buffer;
	buffer.append(m_currentTemperature.toDptfBuffer());
	return buffer;
}

TemperatureStatus TemperatureStatus::createFromDptfBuffer(const DptfBuffer& buffer)
{
	if (buffer.size() != (TemperatureStatus().toDptfBuffer().size()))
	{
		throw dptf_exception("Buffer given to Temperature Status class has invalid length.");
	}

	DptfBuffer bufferCopy = buffer;
	DptfBufferStream stream(bufferCopy);

	return TemperatureStatus(stream.readNextTemperature());
}
