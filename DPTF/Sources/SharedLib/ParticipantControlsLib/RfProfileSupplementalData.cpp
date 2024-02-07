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

#include "RfProfileSupplementalData.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

RfProfileSupplementalData::RfProfileSupplementalData(
	RadioConnectionStatus::Type radioConnectionStatus)
	: m_radioConnectionStatus(radioConnectionStatus)
{
}
RadioConnectionStatus::Type RfProfileSupplementalData::getRadioConnectionStatus(void) const
{
	return m_radioConnectionStatus;
}

Bool RfProfileSupplementalData::operator==(const RfProfileSupplementalData& rhs) const
{
	return (m_radioConnectionStatus == rhs.m_radioConnectionStatus);
}

Bool RfProfileSupplementalData::operator!=(const RfProfileSupplementalData& rhs) const
{
	return !(*this == rhs);
}

std::shared_ptr<XmlNode> RfProfileSupplementalData::getXml(void) const
{
	auto supplementalData = XmlNode::createWrapperElement("radio_frequency_supplemental_data");
	supplementalData->addChild(XmlNode::createDataElement(
		"radio_connection_status", RadioConnectionStatus::ToString(m_radioConnectionStatus)));
	return supplementalData;
}
