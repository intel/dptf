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

#include "PixelClockData.h"
#include "XmlNode.h"

PixelClockData::PixelClockData(Frequency panelInputFrequencySpecification, Frequency sscEnabledNudgeFrequency,
    Frequency sscDisabledNudgeFrequency) :
    m_panelInputFrequencySpecification(panelInputFrequencySpecification),
    m_sscEnabledNudgeFrequency(sscEnabledNudgeFrequency),
    m_sscDisabledNudgeFrequency(sscDisabledNudgeFrequency)
{
}

Frequency PixelClockData::getPanelInputFrequencySpecification(void) const
{
    return m_panelInputFrequencySpecification;
}

Frequency PixelClockData::getSscEnabledNudgeFrequency(void) const
{
    return m_sscEnabledNudgeFrequency;
}

Frequency PixelClockData::getSscDisabledNudgeFrequency(void) const
{
    return m_sscDisabledNudgeFrequency;
}

Bool PixelClockData::operator==(const PixelClockData& rhs) const
{
    return
        ((m_panelInputFrequencySpecification == rhs.m_panelInputFrequencySpecification) &&
         (m_sscEnabledNudgeFrequency == rhs.m_sscEnabledNudgeFrequency) &&
         (m_sscDisabledNudgeFrequency == rhs.m_sscDisabledNudgeFrequency));
}

Bool PixelClockData::operator!=(const PixelClockData& rhs) const
{
    return !(*this == rhs);
}

XmlNode* PixelClockData::getXml(void) const
{
    XmlNode* data = XmlNode::createWrapperElement("pixel_clock_data");
    data->addChild(XmlNode::createDataElement("panel_input_frequency", m_panelInputFrequencySpecification.toString()));
    data->addChild(XmlNode::createDataElement("ssc_enabled_nudge_frequency", m_sscEnabledNudgeFrequency.toString()));
    data->addChild(XmlNode::createDataElement("ssc_disabled_nudge_frequency", m_sscDisabledNudgeFrequency.toString()));
    return data;
}