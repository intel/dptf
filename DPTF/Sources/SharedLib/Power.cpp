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

#include "Power.h"

static const UInt32 MaxValidPower = 10000000; // 10,000 watts

Power::Power(void)
    : m_power(0), m_valid(false)
{
}

Power::Power(UInt32 power)
    : m_power(power), m_valid(true)
{
    if (power > MaxValidPower)
    {
        m_valid = false;
        throw dptf_exception("Power value " + std::to_string(power) + " out of valid range.");
    }
}

Power Power::createInvalid()
{
    return Power();
}

Bool Power::operator==(const Power& rhs) const
{
    // Do not throw an exception if power is not valid.

    if (this->isValid() == true && rhs.isValid() == true)
    {
        return (this->m_power == rhs.m_power);
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

Bool Power::operator!=(const Power& rhs) const
{
    // Do not throw an exception if power is not valid.
    return !(*this == rhs);
}

Bool Power::operator>(const Power& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_power > rhs.m_power);
}

Bool Power::operator>=(const Power& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_power >= rhs.m_power);
}

Bool Power::operator<(const Power& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_power < rhs.m_power);
}

Bool Power::operator<=(const Power& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_power <= rhs.m_power);
}

Power Power::operator+(const Power& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return Power(this->m_power + rhs.m_power);
}

Power Power::operator-(const Power& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);

    if (rhs.m_power > this->m_power)
    {
        throw dptf_exception("Invalid power subtraction requested.  Right side is greater than left side.");
    }
    else
    {
        return Power(this->m_power - rhs.m_power);
    }
}

std::ostream& operator<<(std::ostream& os, const Power& power)
{
    os << power.toString();
    return os;
}

Power::operator UInt32(void) const
{
    return m_power;
}

Bool Power::isValid() const
{
    return m_valid;
}

std::string Power::toString() const
{
    return std::to_string(m_power);
}

void Power::throwIfInvalid(const Power& power) const
{
    if (power.isValid() == false)
    {
        throw dptf_exception("Power is invalid.");
    }
}
