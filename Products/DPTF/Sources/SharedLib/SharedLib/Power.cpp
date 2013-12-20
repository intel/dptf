/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
#include "Power.h"
#include "DptfExceptions.h"
#include "StatusFormat.h"
#include "XmlNode.h"

Power::Power(void) : m_power(invalidPower)
{
}

Power::Power(UIntN power) : m_power(power)
{
    if (power > maxValidPower && power != invalidPower)
    {
        throw dptf_exception("Power value " + std::to_string(power) + " out of valid range.");
    }
}

UIntN Power::getPower() const
{
    return m_power;
}

Bool Power::isPowerValid() const
{
    return ((m_power <= maxValidPower) && (m_power != invalidPower));
}

Bool Power::operator>(const Power& rhs) const
{
    return (getPower() > rhs.getPower());
}

Bool Power::operator<(const Power& rhs) const
{
    return (getPower() < rhs.getPower());
}

Bool Power::operator==(const Power& rhs) const
{
    return (getPower() == rhs.getPower());
}

Power Power::operator-(const Power& rhs) const
{
    return Power(getPower() - rhs.getPower());
}

Power Power::operator+(const Power& rhs) const
{
    return Power(getPower() + rhs.getPower());
}

XmlNode* Power::getXml(void)
{
    return getXml("");
}

XmlNode* Power::getXml(std::string tag)
{
    return XmlNode::createDataElement(tag, StatusFormat::friendlyValue(m_power));
}

std::string Power::toString() const
{
    return std::to_string(m_power) + "mW";
}
