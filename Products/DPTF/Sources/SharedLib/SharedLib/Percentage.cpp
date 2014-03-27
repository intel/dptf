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

#include "Percentage.h"
#include "DptfExceptions.h"
#include "Constants.h"
#include <iomanip>

Percentage::Percentage(void)
    : m_percentage(0.0), m_valid(false)
{
}

Percentage::Percentage(double percentage)
    : m_percentage(percentage), m_valid(true)
{
}

Percentage Percentage::createInvalid()
{
    return Percentage();
}

Percentage Percentage::fromWholeNumber(UIntN wholeNumber)
{
    double value = ((double)wholeNumber / 100.0);
    return Percentage(value);
}

Bool Percentage::operator==(const Percentage& rhs) const
{
    // Do not throw an exception if percentage is not valid.

    if (this->isValid() == true && rhs.isValid() == true)
    {
        return (this->m_percentage == rhs.m_percentage);
    }
    else if (this->isValid() == false && rhs.isValid() == false)
    {
        return true;
    }
    else
    {
        return false;
    }
}

Bool Percentage::operator!=(const Percentage& rhs) const
{
    return !(*this == rhs);
}

Bool Percentage::operator>(const Percentage& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_percentage > rhs.m_percentage);
}

Bool Percentage::operator>=(const Percentage& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_percentage >= rhs.m_percentage);
}

Bool Percentage::operator<(const Percentage& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_percentage < rhs.m_percentage);
}

Bool Percentage::operator<=(const Percentage& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_percentage <= rhs.m_percentage);
}

std::ostream& operator<<(std::ostream& os, const Percentage& percentage)
{
    os << percentage.toString();
    return os;
}

Percentage::operator double(void) const
{
    throwIfInvalid(*this);
    return m_percentage;
}

Bool Percentage::isValid() const
{
    return m_valid;
}

UIntN Percentage::toWholeNumber() const
{
    return (UIntN)(m_percentage * 100);
}

std::string Percentage::toString() const
{
    if (isValid())
    {
        std::stringstream stream;
        stream << std::setprecision(2) << std::fixed << (m_percentage * 100);
        return stream.str();
    }
    else
    {
        return Constants::InvalidString;
    }
}

void Percentage::throwIfInvalid(const Percentage& percentage) const
{
    if (percentage.isValid() == false)
    {
        throw dptf_exception("Percentage is not valid.");
    }
}