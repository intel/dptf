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

#include "PixelClockDataSet.h"
#include "XmlNode.h"

PixelClockDataSet::PixelClockDataSet(const std::vector<PixelClockData>& pixelClockData) :
    m_pixelClockData(pixelClockData)
{
}

UIntN PixelClockDataSet::getCount(void) const
{
    return static_cast<UIntN>(m_pixelClockData.size());
}

PixelClockData PixelClockDataSet::operator[](UIntN index) const
{
    return m_pixelClockData.at(index);
}

Bool PixelClockDataSet::operator==(const PixelClockDataSet& rhs) const
{
    return (m_pixelClockData == rhs.m_pixelClockData);
}

Bool PixelClockDataSet::operator!=(const PixelClockDataSet& rhs) const
{
    return !(*this == rhs);
}

std::shared_ptr<XmlNode> PixelClockDataSet::getXml(void) const
{
    auto dataSet = XmlNode::createWrapperElement("pixel_clock_data_set");
    for (UIntN pcIndex = 0; pcIndex < m_pixelClockData.size(); pcIndex++)
    {
        auto pixelClock = XmlNode::createWrapperElement("pixel_clock");
        pixelClock->addChild(XmlNode::createDataElement("pixel_clock_number", StlOverride::to_string(pcIndex)));
        pixelClock->addChild(m_pixelClockData[pcIndex].getXml());
        dataSet->addChild(pixelClock);
    }
    return dataSet;
}