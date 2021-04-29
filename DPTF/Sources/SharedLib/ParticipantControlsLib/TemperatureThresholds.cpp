/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "TemperatureThresholds.h"
#include "XmlNode.h"
#include "StatusFormat.h"
#include "DptfBufferStream.h"
using namespace StatusFormat;

TemperatureThresholds::TemperatureThresholds()
{
	createInvalid();
}

TemperatureThresholds::TemperatureThresholds(Temperature aux0, Temperature aux1, Temperature hysteresis)
	: m_aux0(aux0)
	, m_aux1(aux1)
	, m_hysteresis(hysteresis)
{
}

TemperatureThresholds TemperatureThresholds::createInvalid()
{
	return TemperatureThresholds(
		Temperature::createInvalid(), Temperature::createInvalid(), Temperature::createInvalid());
}

Temperature TemperatureThresholds::getAux0(void) const
{
	return m_aux0;
}

Temperature TemperatureThresholds::getAux1(void) const
{
	return m_aux1;
}

Temperature TemperatureThresholds::getHysteresis(void) const
{
	return m_hysteresis;
}

std::shared_ptr<XmlNode> TemperatureThresholds::getXml(void)
{
	auto root = XmlNode::createWrapperElement("temperature_thresholds");
	root->addChild(XmlNode::createDataElement("control_name", "Temperature Threshold"));
	root->addChild(XmlNode::createDataElement("aux0", getAux0().toString()));
	root->addChild(XmlNode::createDataElement("aux1", getAux1().toString()));
	root->addChild(XmlNode::createDataElement("hysteresis", m_hysteresis.toString()));

	return root;
}

Bool TemperatureThresholds::operator==(const TemperatureThresholds& thresholds) const
{
	return (
		(m_aux0 == thresholds.m_aux0) && (m_aux1 == thresholds.m_aux1) && (m_hysteresis == thresholds.m_hysteresis));
}

DptfBuffer TemperatureThresholds::toDptfBuffer() const
{
	DptfBuffer buffer;
	buffer.append(m_aux0.toDptfBuffer());
	buffer.append(m_aux1.toDptfBuffer());
	buffer.append(m_hysteresis.toDptfBuffer());
	return buffer;
}

TemperatureThresholds TemperatureThresholds::createFromDptfBuffer(const DptfBuffer& buffer)
{
	if (buffer.size() != (TemperatureThresholds().toDptfBuffer().size()))
	{
		throw dptf_exception("Buffer given to Temperature Thresholds class has invalid length.");
	}

	DptfBuffer bufferCopy = buffer;
	DptfBufferStream stream(bufferCopy);

	TemperatureThresholds newTemperatureThresholds;
	newTemperatureThresholds.m_aux0 = stream.readNextTemperature();
	newTemperatureThresholds.m_aux1 = stream.readNextTemperature();
	newTemperatureThresholds.m_hysteresis = stream.readNextTemperature();

	return newTemperatureThresholds;
}
