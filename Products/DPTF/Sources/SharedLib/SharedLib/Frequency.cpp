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

#include "Frequency.h"
#include "DptfExceptions.h"

Frequency::Frequency(void)
    : m_frequency(0), m_valid(false)
{
}

Frequency::Frequency(UInt64 frequency)
    : m_frequency(frequency), m_valid(true)
{
}

Frequency Frequency::createInvalid()
{
    return Frequency();
}

Bool Frequency::operator==(const Frequency& rhs) const
{
    // Do not throw an exception if frequency is not valid.

    if (this->isValid() == true && rhs.isValid() == true)
    {
        return (this->m_frequency == rhs.m_frequency);
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

Bool Frequency::operator!=(const Frequency& rhs) const
{
    // Do not throw an exception if frequency is not valid.
    return !(*this == rhs);
}

Bool Frequency::operator>(const Frequency& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_frequency > rhs.m_frequency);
}

Bool Frequency::operator>=(const Frequency& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_frequency >= rhs.m_frequency);
}

Bool Frequency::operator<(const Frequency& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_frequency < rhs.m_frequency);
}

Bool Frequency::operator<=(const Frequency& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_frequency <= rhs.m_frequency);
}

Frequency Frequency::operator+(const Frequency& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return Frequency(this->m_frequency + rhs.m_frequency);
}

Frequency Frequency::operator-(const Frequency& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);

    if (rhs.m_frequency > this->m_frequency)
    {
        throw dptf_exception("Invalid frequency subtraction requested.  rhs > lhs.");
    }

    return Frequency(this->m_frequency - rhs.m_frequency);
}

Frequency Frequency::operator*(const Frequency& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return Frequency(this->m_frequency * rhs.m_frequency);
}

std::ostream& operator<<(std::ostream& os, const Frequency& frequency)
{
    os << frequency.toString();
    return os;
}

Frequency::operator UInt64(void) const
{
    throwIfInvalid(*this);
    return m_frequency;
}

Bool Frequency::isValid() const
{
    return m_valid;
}

std::string Frequency::toString() const
{
    if (isValid())
    {
        return std::to_string(m_frequency);
    }
    else
    {
        return std::string(Constants::InvalidString);
    }
}

void Frequency::throwIfInvalid(const Frequency& frequency) const
{
    if (frequency.isValid() == false)
    {
        throw dptf_exception("Frequency is not valid.");
    }
}