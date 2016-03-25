/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

class XmlNode;

class PixelClockData final
{
public:

    PixelClockData(Frequency panelInputFrequencySpecification, Frequency sscEnabledNudgeFrequency,
        Frequency sscDisabledNudgeFrequency);

    Frequency getPanelInputFrequencySpecification(void) const;
    Frequency getSscEnabledNudgeFrequency(void) const;
    Frequency getSscDisabledNudgeFrequency(void) const;

    Bool operator==(const PixelClockData& rhs) const;
    Bool operator!=(const PixelClockData& rhs) const;
    std::shared_ptr<XmlNode> getXml(void) const;

private:

    Frequency m_panelInputFrequencySpecification;
    Frequency m_sscEnabledNudgeFrequency;
    Frequency m_sscDisabledNudgeFrequency;
};