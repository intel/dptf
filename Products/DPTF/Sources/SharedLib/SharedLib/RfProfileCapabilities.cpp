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

#include "RfProfileCapabilities.h"

RfProfileCapabilities::RfProfileCapabilities(Frequency defaultCenterFrequency, Percentage leftClipPercent,
    Percentage rightClipPercent, Frequency frequencyAdjustResolution) :
    m_defaultCenterFrequency(defaultCenterFrequency),
    m_leftClipPercent(leftClipPercent),
    m_rightClipPercent(rightClipPercent),
    m_frequencyAdjustResolution(frequencyAdjustResolution)
{
}

Frequency RfProfileCapabilities::getDefaultCenterFrequency(void) const
{
    return m_defaultCenterFrequency;
}

Percentage RfProfileCapabilities::getLeftClipPercent(void) const
{
    return m_leftClipPercent;
}

Percentage RfProfileCapabilities::getRightClipPercent(void) const
{
    return m_rightClipPercent;
}

Frequency RfProfileCapabilities::getFrequencyAdjustResolution(void) const
{
    return m_frequencyAdjustResolution;
}

Bool RfProfileCapabilities::operator==(const RfProfileCapabilities& rhs) const
{
    return
        ((m_defaultCenterFrequency == rhs.m_defaultCenterFrequency) &&
         (m_leftClipPercent == rhs.m_leftClipPercent) &&
         (m_rightClipPercent == rhs.m_rightClipPercent) &&
         (m_frequencyAdjustResolution == rhs.m_frequencyAdjustResolution));
}

Bool RfProfileCapabilities::operator!=(const RfProfileCapabilities& rhs) const
{
    return !(*this == rhs);
}

XmlNode* RfProfileCapabilities::getXml(void)
{
    // FIXME
    throw implement_me();
}