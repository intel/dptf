/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "esif_sdk_fan.h"
#include "StatusFormat.h"
#include "XmlNode.h"
#include "DptfBufferStream.h"

ActiveControlStatus::ActiveControlStatus(UIntN currentControlId, UIntN currentSpeed)
	: m_currentControlId(currentControlId)
	, m_currentSpeed(currentSpeed)
{
}

ActiveControlStatus ActiveControlStatus::createFromFst(const DptfBuffer& buffer)
{
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct EsifDataBinaryFstPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFstPackage*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty FST buffer.");
	}
	else if (buffer.size() != sizeof(EsifDataBinaryFstPackage))
	{
		throw dptf_exception("Expected binary data size mismatch. (FST)");
	}

	return ActiveControlStatus(
		static_cast<UInt32>(currentRow->control.integer.value), static_cast<UInt32>(currentRow->speed.integer.value));
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
	return (
		(this->getCurrentControlId() == rhs.getCurrentControlId())
		&& (this->getCurrentSpeed() == rhs.getCurrentSpeed()));
}

Bool ActiveControlStatus::operator!=(const ActiveControlStatus rhs) const
{
	return !(*this == rhs);
}

std::shared_ptr<XmlNode> ActiveControlStatus::getXml(void)
{
	auto root = XmlNode::createWrapperElement("active_control_status");
	root->addChild(XmlNode::createDataElement("current_control_id", StatusFormat::friendlyValue(m_currentControlId)));
	root->addChild(XmlNode::createDataElement("current_speed", StatusFormat::friendlyValue(m_currentSpeed)));
	return root;
}

DptfBuffer ActiveControlStatus::toFstBinary(void) const
{
	struct EsifDataBinaryFstPackage fst;
	fst.revision.integer.value = 1;
	fst.revision.type = ESIF_DATA_UINT64;
	fst.control.integer.value = m_currentControlId;
	fst.control.type = ESIF_DATA_UINT64;
	fst.speed.integer.value = m_currentSpeed;
	fst.speed.type = ESIF_DATA_UINT64;

	DptfBuffer buffer;
	buffer.append((UInt8*)&fst, sizeof(fst));
	return buffer;
}
