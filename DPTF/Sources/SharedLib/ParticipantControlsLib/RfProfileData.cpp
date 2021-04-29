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

#include "RfProfileData.h"
#include "StatusFormat.h"

RfProfileData::RfProfileData(
	Frequency centerFrequency,
	Frequency leftFrequencySpread,
	Frequency rightFrequencySpread,
	Frequency guardband,
	RfProfileSupplementalData supplementalData)
	: m_centerFrequency(centerFrequency)
	, m_leftFrequencySpread(leftFrequencySpread)
	, m_rightFrequencySpread(rightFrequencySpread)
	, m_guardband(guardband)
	, m_supplementalData(supplementalData)
{
}

Frequency RfProfileData::getCenterFrequency(void) const
{
	return m_centerFrequency;
}

Frequency RfProfileData::getLeftFrequencySpread(void) const
{
	return m_leftFrequencySpread;
}

Frequency RfProfileData::getRightFrequencySpread(void) const
{
	return m_rightFrequencySpread;
}

Frequency RfProfileData::getLeftFrequency(void) const
{
	return (m_centerFrequency - m_leftFrequencySpread);
}

Frequency RfProfileData::getRightFrequency(void) const
{
	return (m_centerFrequency + m_rightFrequencySpread);
}

Frequency RfProfileData::getBandFrequencySpread(void) const
{
	return (getRightFrequency() - getLeftFrequency());
}

Frequency RfProfileData::getGuardband(void) const
{
	return m_guardband;
}

Frequency RfProfileData::getLeftFrequencySpreadWithGuardband(void) const
{
	return (m_leftFrequencySpread + m_guardband);
}

Frequency RfProfileData::getRightFrequencySpreadWithGuardband(void) const
{
	return (m_rightFrequencySpread + m_guardband);
}

Frequency RfProfileData::getLeftFrequencyWithGuardband(void) const
{
	return (m_centerFrequency - getLeftFrequencySpreadWithGuardband());
}

Frequency RfProfileData::getRightFrequencyWithGuardband(void) const
{
	return (m_centerFrequency + getRightFrequencySpreadWithGuardband());
}

RfProfileSupplementalData RfProfileData::getSupplementalData(void) const
{
	return m_supplementalData;
}

Bool RfProfileData::operator==(const RfProfileData& rhs) const
{
	return (
		(m_centerFrequency == rhs.m_centerFrequency) && (m_leftFrequencySpread == rhs.m_leftFrequencySpread)
		&& (m_rightFrequencySpread == rhs.m_rightFrequencySpread)
		&& (m_supplementalData == rhs.m_supplementalData));
}

Bool RfProfileData::operator!=(const RfProfileData& rhs) const
{
	return !(*this == rhs);
}

std::shared_ptr<XmlNode> RfProfileData::getXml(void) const
{
	auto profileData = XmlNode::createWrapperElement("radio_frequency_profile_data");
	profileData->addChild(XmlNode::createDataElement("center_frequency", m_centerFrequency.toString()));
	profileData->addChild(
		XmlNode::createDataElement("left_frequency_spread", getLeftFrequencySpreadWithGuardband().toString()));
	profileData->addChild(
		XmlNode::createDataElement("right_frequency_spread", getRightFrequencySpreadWithGuardband().toString()));
	profileData->addChild(m_supplementalData.getXml());
	return profileData;
}