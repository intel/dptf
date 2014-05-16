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

#include "PixelClockCapabilities.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

PixelClockCapabilities::PixelClockCapabilities(PixelClockPanelType::Type pixelClockPanelType,
    UInt32 clockDeviation, Bool upwardDeviation, Bool downwardDeviation,
    PixelClockChannelType::Type pixelClockChannelType, Bool sscEnabled,
    PixelClockSpreadType::Type sscSpreadDirection, Percentage spreadPercentage) :
    m_pixelClockPanelType(pixelClockPanelType),
    m_clockDeviation(clockDeviation), m_upwardDeviation(upwardDeviation), m_downwardDeviation(downwardDeviation),
    m_pixelClockChannelType(pixelClockChannelType), m_sscEnabled(sscEnabled),
    m_sscSpreadDirection(sscSpreadDirection), m_spreadPercentage(spreadPercentage)
{
}

PixelClockPanelType::Type PixelClockCapabilities::getPixelClockPanelType(void) const
{
    return m_pixelClockPanelType;
}

UInt32 PixelClockCapabilities::getClockDeviation(void) const
{
    return m_clockDeviation;
}

Bool PixelClockCapabilities::isUpwardDeviation(void) const
{
    return m_upwardDeviation;
}

Bool PixelClockCapabilities::isDownwardDeviation(void) const
{
    return m_downwardDeviation;
}

PixelClockChannelType::Type PixelClockCapabilities::getPixelClockChannelType(void) const
{
    return m_pixelClockChannelType;
}

Bool PixelClockCapabilities::isSscEnabled(void) const
{
    return m_sscEnabled;
}

PixelClockSpreadType::Type PixelClockCapabilities::getSscSpreadDirection(void) const
{
    return m_sscSpreadDirection;
}

Percentage PixelClockCapabilities::getSpreadPercentage(void) const
{
    return m_spreadPercentage;
}

Bool PixelClockCapabilities::operator==(const PixelClockCapabilities& rhs) const
{
    return
        ((m_pixelClockPanelType == rhs.m_pixelClockPanelType) &&
         (m_clockDeviation == rhs.m_clockDeviation) &&
         (m_upwardDeviation == rhs.m_upwardDeviation) &&
         (m_downwardDeviation == rhs.m_downwardDeviation) &&
         (m_pixelClockChannelType == rhs.m_pixelClockChannelType) &&
         (m_sscEnabled == rhs.m_sscEnabled) &&
         (m_sscSpreadDirection == rhs.m_sscSpreadDirection) &&
         (m_spreadPercentage == rhs.m_spreadPercentage));
}

Bool PixelClockCapabilities::operator!=(const PixelClockCapabilities& rhs) const
{
    return !(*this == rhs);
}

XmlNode* PixelClockCapabilities::getXml(void) const
{
    XmlNode* capabilities = XmlNode::createWrapperElement("pixel_clock_capabilities");
    capabilities->addChild(XmlNode::createDataElement("panel_type", 
        PixelClockPanelType::ToString(m_pixelClockPanelType)));
    capabilities->addChild(XmlNode::createDataElement("deviation", friendlyValue(m_clockDeviation)));
    capabilities->addChild(XmlNode::createDataElement("upward_deviation", friendlyValue(m_upwardDeviation)));
    capabilities->addChild(XmlNode::createDataElement("downward_deviation", friendlyValue(m_downwardDeviation)));
    capabilities->addChild(XmlNode::createDataElement("channel_type", 
        PixelClockChannelType::ToString(m_pixelClockChannelType)));
    capabilities->addChild(XmlNode::createDataElement("ssc_enabled", friendlyValue(m_sscEnabled)));
    capabilities->addChild(XmlNode::createDataElement("spread_type", 
        PixelClockSpreadType::ToString(m_sscSpreadDirection)));
    capabilities->addChild(XmlNode::createDataElement("spread_percentage", m_spreadPercentage.toString()));
    return capabilities;
}