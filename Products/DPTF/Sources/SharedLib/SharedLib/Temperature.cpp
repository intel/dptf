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

#include "Temperature.h"

Temperature::Temperature(void)
    : m_temperature(0), m_valid(false)
{
}

Temperature::Temperature(UInt32 temperature)
    : m_temperature(temperature), m_valid(true)
{
    if (temperature > maxValidTemperature)
    {
        throw dptf_exception("Temperature out of valid range");
    }
}

Temperature Temperature::createInvalid()
{
    return Temperature();
}

Bool Temperature::operator==(const Temperature& rhs) const
{
    // Do not throw an exception if temperature is not valid.

    if (this->isValid() == true && rhs.isValid() == true)
    {
        return (this->m_temperature == rhs.m_temperature);
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

Bool Temperature::operator!=(const Temperature& rhs) const
{
    // Do not throw an exception if temperature is not valid.
    return !(*this == rhs);
}

Bool Temperature::operator>(const Temperature& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_temperature > rhs.m_temperature);
}

Bool Temperature::operator>=(const Temperature& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_temperature >= rhs.m_temperature);
}

Bool Temperature::operator<(const Temperature& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_temperature < rhs.m_temperature);
}

Bool Temperature::operator<=(const Temperature& rhs) const
{
    throwIfInvalid(*this);
    throwIfInvalid(rhs);
    return (this->m_temperature <= rhs.m_temperature);
}

std::ostream& operator<<(std::ostream& os, const Temperature& temperature)
{
    os << temperature.toString();
    return os;
}

Temperature::operator UInt32(void) const
{
    throwIfInvalid(*this);
    return m_temperature;
}

Bool Temperature::isValid() const
{
    return m_valid;
}

std::string Temperature::toString() const
{
    if (isValid())
    {
        return std::to_string(m_temperature); // TODO: adjust for when we get 10th degrees C
    }
    else
    {
        return std::string(Constants::InvalidString);
    }
}

void Temperature::throwIfInvalid(const Temperature& temperature) const
{
    if (temperature.isValid() == false)
    {
        throw dptf_exception("Temperature is not valid.");
    }
}