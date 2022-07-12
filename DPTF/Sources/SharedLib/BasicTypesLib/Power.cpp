/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "DptfBufferStream.h"
#include <cmath>

static const UInt32 MaxValidPower = 10000000; // 10,000 watts
const double MilliwattsPerWatt = 1000.0;

Power::Power(void)
	: m_valid(false)
	, m_power(0)
{
}

Power::Power(UInt32 power)
	: m_valid(true)
	, m_power(power)
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

Power Power::createFromMilliwatts(UInt32 milliwatts)
{
	if (milliwatts > MaxValidPower)
	{
		throw dptf_exception("Power value " + std::to_string(milliwatts) + " out of valid range.");
	}
	Power power;
	power.m_power = milliwatts;
	power.m_valid = true;
	return power;
}

Power Power::createFromWatts(double watts)
{
	UInt32 milliwatts = (UInt32)(round(watts * MilliwattsPerWatt));
	return Power::createFromMilliwatts(milliwatts);
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
	if (isValid())
	{
		return std::to_string(m_power);
	}

	return Constants::InvalidString;
}

void Power::throwIfInvalid(const Power& power) const
{
	if (power.isValid() == false)
	{
		throw dptf_exception("Power is invalid.");
	}
}

Int32 Power::toInt32() const
{
	return Int32(m_power);
}

DptfBuffer Power::toDptfBuffer() const
{
	DptfBuffer buffer;
	buffer.append((UInt8*)this, sizeof(Power));
	return buffer;
}

Power Power::createFromDptfBuffer(const DptfBuffer& buffer)
{
	if (buffer.size() != sizeof(Power))
	{
		throw dptf_exception("Buffer given to Power class has invalid length.");
	}

	DptfBuffer bufferCopy = buffer;
	DptfBufferStream stream(bufferCopy);
	Power newRequest = stream.readNextPower();
	return newRequest;
}

double Power::asWatts() const
{
	throwIfInvalid(*this);
	double watts = (double)m_power / MilliwattsPerWatt;
	return watts;
}
