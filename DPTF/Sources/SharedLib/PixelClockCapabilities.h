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
#include "PixelClockPanelType.h"
#include "PixelClockChannelType.h"
#include "PixelClockSpreadType.h"

class XmlNode;

class PixelClockCapabilities final
{
public:

    PixelClockCapabilities(PixelClockPanelType::Type pixelClockPanelType, UInt32 clockDeviation, Bool upwardDeviation,
        Bool downwardDeviation, PixelClockChannelType::Type pixelClockChannelType, Bool sscEnabled,
        PixelClockSpreadType::Type sscSpreadDirection, Percentage spreadPercentage);

    PixelClockPanelType::Type getPixelClockPanelType(void) const;
    UInt32 getClockDeviation(void) const;
    Bool isUpwardDeviation(void) const;
    Bool isDownwardDeviation(void) const;
    PixelClockChannelType::Type getPixelClockChannelType(void) const;
    Bool isSscEnabled(void) const; // TODO: change to sscAvailable
    PixelClockSpreadType::Type getSscSpreadDirection(void) const;
    Percentage getSpreadPercentage(void) const;

    Bool operator==(const PixelClockCapabilities& rhs) const;
    Bool operator!=(const PixelClockCapabilities& rhs) const;
    XmlNode* getXml(void) const;

private:

    PixelClockPanelType::Type m_pixelClockPanelType;

    // Panel Tolerance
    UInt32 m_clockDeviation;
    Bool m_upwardDeviation;
    Bool m_downwardDeviation;

    PixelClockChannelType::Type m_pixelClockChannelType;

    // SSC Characteristics
    Bool m_sscEnabled;
    PixelClockSpreadType::Type m_sscSpreadDirection;
    Percentage m_spreadPercentage;
};