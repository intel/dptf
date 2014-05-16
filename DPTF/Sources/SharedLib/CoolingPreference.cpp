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

#include "CoolingPreference.h"

CoolingPreference::CoolingPreference(
    CoolingMode::Type coolingMode,
    CoolingModeAcousticLimit::Type coolingModeAcousticLimit,
    CoolingModePowerLimit::Type coolingModePowerLimit) :
    m_coolingMode(coolingMode),
    m_coolingModeAcousticLimit(coolingModeAcousticLimit),
    m_coolingModePowerLimit(coolingModePowerLimit)
{
}

EsifDataBinaryScp CoolingPreference::getEsifCompliantBinary(void) const
{
    EsifDataBinaryScp data;

    data.mode = m_coolingMode;
    data.acousticLimit = m_coolingModeAcousticLimit;
    data.powerLimit = m_coolingModePowerLimit;

    return data;
}

Bool CoolingPreference::operator==(const CoolingPreference& rhs) const
{
    return
        (this->m_coolingMode == rhs.m_coolingMode) &&
        (this->m_coolingModeAcousticLimit == rhs.m_coolingModeAcousticLimit) &&
        (this->m_coolingModePowerLimit == rhs.m_coolingModePowerLimit);
}

Bool CoolingPreference::operator!=(const CoolingPreference& rhs) const
{
    return !(*this == rhs);
}

XmlNode* CoolingPreference::getXml() const
{
    XmlNode* status = XmlNode::createWrapperElement("cooling_preference");
    status->addChild(XmlNode::createDataElement(
        "cooling_mode", CoolingMode::ToString(m_coolingMode)));
    status->addChild(XmlNode::createDataElement(
        "acoustic_limit", CoolingModeAcousticLimit::ToString(m_coolingModeAcousticLimit)));
    status->addChild(XmlNode::createDataElement(
        "power_limit", CoolingModePowerLimit::ToString(m_coolingModePowerLimit)));
    return status;
}