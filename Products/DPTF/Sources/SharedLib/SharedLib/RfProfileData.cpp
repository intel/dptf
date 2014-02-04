/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

RfProfileData::RfProfileData(Frequency centerFrequency, Frequency leftFrequencySpread,
    Frequency rightFrequencySpread, RfProfileSupplementalData supplementalData) :
    m_centerFrequency(centerFrequency),
    m_leftFrequencySpread(leftFrequencySpread),
    m_rightFrequencySpread(rightFrequencySpread),
    m_supplementalData(supplementalData)
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

RfProfileSupplementalData RfProfileData::getSupplementalData(void) const
{
    return m_supplementalData;
}

Bool RfProfileData::operator==(const RfProfileData& rhs) const
{
    return
        ((m_centerFrequency == rhs.m_centerFrequency) &&
         (m_leftFrequencySpread == rhs.m_leftFrequencySpread) &&
         (m_rightFrequencySpread == rhs.m_rightFrequencySpread) &&
         (m_supplementalData == rhs.m_supplementalData));
}

Bool RfProfileData::operator!=(const RfProfileData& rhs) const
{
    return !(*this == rhs);
}

XmlNode* RfProfileData::getXml(void) const
{
    XmlNode* profileData = XmlNode::createWrapperElement("radio_frequency_profile_data");
    profileData->addChild(XmlNode::createDataElement("center_frequency", m_centerFrequency.toString()));
    profileData->addChild(XmlNode::createDataElement("left_frequency_spread", m_leftFrequencySpread.toString()));
    profileData->addChild(XmlNode::createDataElement("right_frequency_spread", m_rightFrequencySpread.toString()));
    profileData->addChild(m_supplementalData.getXml());
    return profileData;
}