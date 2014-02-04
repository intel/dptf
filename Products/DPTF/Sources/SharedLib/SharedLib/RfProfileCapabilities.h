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

#pragma once

#include "Dptf.h"
#include "RfProfileType.h"

class XmlNode;

class RfProfileCapabilities final
{
public:

    RfProfileCapabilities(Frequency defaultCenterFrequency, Percentage leftClipPercent, Percentage rightClipPercent,
        Frequency frequencyAdjustResolution);

    Frequency getDefaultCenterFrequency(void) const;
    Percentage getLeftClipPercent(void) const;
    Percentage getRightClipPercent(void) const;
    Frequency getFrequencyAdjustResolution(void) const;

    Bool operator==(const RfProfileCapabilities& rhs) const;
    Bool operator!=(const RfProfileCapabilities& rhs) const;
    XmlNode* getXml(void);

private:

    Frequency m_defaultCenterFrequency;
    Percentage m_leftClipPercent;
    Percentage m_rightClipPercent;
    Frequency m_frequencyAdjustResolution;

    // Reserved Items not used today
    //RfProfileType::Type m_leftShape;
    //RfProfileType::Type m_rightShape;
    //UInt64 m_leftNotch;
    //UInt64 m_rightNotch;
};