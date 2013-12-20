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
#include "Percentage.h"
#include "DptfExceptions.h"
#include "StatusFormat.h"
#include "XmlNode.h"

Percentage::Percentage(UIntN percentage) : m_percentage(percentage)
{
    if (percentage > 100 && percentage != invalidPercentage)
    {
        throw dptf_exception("Percentage out of valid range");
    }
}

Percentage::Percentage(void) :
    m_percentage(invalidPercentage)
{
}

UIntN Percentage::getPercentage() const
{
    return m_percentage;
}

Bool Percentage::isPercentageValid() const
{
    return ((m_percentage <= 100) && (m_percentage != invalidPercentage));
}

Bool Percentage::operator==(const Percentage& rhs) const
{
    return (this->getPercentage() == rhs.getPercentage());
}

Bool Percentage::operator!=(const Percentage& rhs) const
{
    return (this->getPercentage() != rhs.getPercentage());
}

Bool Percentage::operator>(const Percentage& rhs) const
{
    throwIfInvalidPercentage();

    return (this->getPercentage() > rhs.getPercentage());
}

Bool Percentage::operator>=(const Percentage& rhs) const
{
    throwIfInvalidPercentage();

    return (this->getPercentage() >= rhs.getPercentage());
}

Bool Percentage::operator<(const Percentage& rhs) const
{
    throwIfInvalidPercentage();

    return (this->getPercentage() < rhs.getPercentage());
}

Bool Percentage::operator<=(const Percentage& rhs) const
{
    throwIfInvalidPercentage();

    return (this->getPercentage() <= rhs.getPercentage());
}

void Percentage::throwIfInvalidPercentage(void) const
{
    if (isPercentageValid() == false)
    {
        throw dptf_exception("Percentage is invalid.");
    }
}

XmlNode* Percentage::getXml(std::string tag)
{
    return XmlNode::createDataElement(tag, StatusFormat::friendlyValue(m_percentage));
}

XmlNode* Percentage::getXml(void)
{
    return getXml("");
}

std::string Percentage::toString() const
{
    return std::to_string(m_percentage);
}