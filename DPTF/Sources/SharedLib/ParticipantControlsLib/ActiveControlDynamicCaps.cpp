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

#include "ActiveControlDynamicCaps.h"
#include "esif_sdk_fan.h"
#include "XmlNode.h"
#include "StatusFormat.h"

ActiveControlDynamicCaps::ActiveControlDynamicCaps()
{
	m_minFanSpeed = Percentage::createInvalid();
	m_maxFanSpeed = Percentage::createInvalid();
}

ActiveControlDynamicCaps::ActiveControlDynamicCaps(const Percentage& minFanSpeed, const Percentage& maxFanSpeed)
	: m_minFanSpeed(minFanSpeed)
	, m_maxFanSpeed(maxFanSpeed)
{
	if (minFanSpeed.isValid() && maxFanSpeed.isValid())
	{
		Percentage minSpeed = Percentage::fromWholeNumber(0);
		Percentage maxSpeed = Percentage::fromWholeNumber(100);

		if (minFanSpeed < minSpeed || minFanSpeed > maxSpeed)
		{
			throw dptf_exception("minFanSpeed percentage is not valid");
		}

		if (maxFanSpeed < minSpeed || maxFanSpeed > maxSpeed)
		{
			throw dptf_exception("maxFanSpeed percentage is not valid");
		}

		if (minFanSpeed > maxFanSpeed)
		{
			throw dptf_exception("minFanSpeed > maxFanSpeed");
		}
	}
}

ActiveControlDynamicCaps ActiveControlDynamicCaps::createFromFcdc(const DptfBuffer& buffer)
{
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct EsifDataBinaryFcdcPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFcdcPackage*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty buffer.");
	}
	else if (buffer.size() != sizeof(EsifDataBinaryFcdcPackage))
	{
		throw dptf_exception("Expected binary data size mismatch. (FCDC)");
	}

	Percentage minSpeed = Percentage::createInvalid();
	if (currentRow->minspeed.integer.value != Constants::Invalid)
	{
		minSpeed = Percentage::fromWholeNumber(static_cast<UInt32>(currentRow->minspeed.integer.value));
	}

	Percentage maxSpeed = Percentage::createInvalid();
	if (currentRow->maxspeed.integer.value != Constants::Invalid)
	{
		maxSpeed = Percentage::fromWholeNumber(static_cast<UInt32>(currentRow->maxspeed.integer.value));
	}

	return ActiveControlDynamicCaps(minSpeed, maxSpeed);
}

Percentage ActiveControlDynamicCaps::getMinFanSpeed(void) const
{
	return m_minFanSpeed;
}

Percentage ActiveControlDynamicCaps::getMaxFanSpeed(void) const
{
	return m_maxFanSpeed;
}

Bool ActiveControlDynamicCaps::operator==(const ActiveControlDynamicCaps& rhs) const
{
	return (m_minFanSpeed == rhs.m_minFanSpeed && m_maxFanSpeed == rhs.m_maxFanSpeed);
}

Bool ActiveControlDynamicCaps::operator!=(const ActiveControlDynamicCaps& rhs) const
{
	return !(*this == rhs);
}

std::shared_ptr<XmlNode> ActiveControlDynamicCaps::getXml(void) const
{
	auto root = XmlNode::createWrapperElement("active_control_dynamic_caps");
	root->addChild(XmlNode::createDataElement("min_fan_speed", m_minFanSpeed.toStringWithPrecision(0)));
	root->addChild(XmlNode::createDataElement("max_fan_speed", m_maxFanSpeed.toStringWithPrecision(0)));
	return root;
}

DptfBuffer ActiveControlDynamicCaps::toFcdcBinary()
{
	EsifDataBinaryFcdcPackage package;
	package.revision.integer.type = esif_data_type::ESIF_DATA_UINT64;
	package.revision.integer.value = 1;
	package.minspeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
	if (getMinFanSpeed().isValid())
	{
		package.minspeed.integer.value = getMinFanSpeed().toWholeNumber();
	}
	else
	{
		package.minspeed.integer.value = Constants::Invalid;
	}

	package.maxspeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
	if (getMaxFanSpeed().isValid())
	{
		package.maxspeed.integer.value = getMaxFanSpeed().toWholeNumber();
	}
	else
	{
		package.maxspeed.integer.value = Constants::Invalid;
	}

	UInt32 sizeOfPackage = sizeof(EsifDataBinaryFcdcPackage);
	DptfBuffer buffer(sizeOfPackage);
	buffer.put(0, (UInt8*)&package, sizeOfPackage);
	return buffer;
}
