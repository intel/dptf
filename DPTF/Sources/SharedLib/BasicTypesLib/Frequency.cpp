/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "StringParser.h"

const UInt64 mhzToHzMultiplierInUint64 = 1000000;
const double mhzToHzMultiplierInDouble = 1000000.0;

Frequency::Frequency(void)
	: m_valid(false)
	, m_frequencyInHertz(0)
{
}

Frequency::Frequency(UInt64 frequencyInHertz)
	: m_valid(true)
	, m_frequencyInHertz(frequencyInHertz)
{
}

Frequency Frequency::createFromHertz(UInt64 frequencyInHertz)
{
	if (frequencyInHertz == Constants::Invalid)
	{
		return createInvalid();
	}

	return Frequency(frequencyInHertz);
}

Frequency Frequency::createFromMegahertz(UInt64 frequencyInMegahertz)
{
	if (frequencyInMegahertz == Constants::Invalid)
	{
		return createInvalid();
	}

	UInt64 frequencyInHertz = frequencyInMegahertz * mhzToHzMultiplierInUint64;
	return Frequency(frequencyInHertz);
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
		return (this->m_frequencyInHertz == rhs.m_frequencyInHertz);
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
	return (this->m_frequencyInHertz > rhs.m_frequencyInHertz);
}

Bool Frequency::operator>=(const Frequency& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	return (this->m_frequencyInHertz >= rhs.m_frequencyInHertz);
}

Bool Frequency::operator<(const Frequency& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	return (this->m_frequencyInHertz < rhs.m_frequencyInHertz);
}

Bool Frequency::operator<=(const Frequency& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	return (this->m_frequencyInHertz <= rhs.m_frequencyInHertz);
}

Frequency Frequency::operator+(const Frequency& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	return Frequency(this->m_frequencyInHertz + rhs.m_frequencyInHertz);
}

Frequency Frequency::operator-(const Frequency& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);

	if (rhs.m_frequencyInHertz > this->m_frequencyInHertz)
	{
		throw dptf_exception("Invalid frequency subtraction requested.  rhs > lhs.");
	}

	return Frequency(this->m_frequencyInHertz - rhs.m_frequencyInHertz);
}

Frequency Frequency::operator*(const Frequency& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	return Frequency(this->m_frequencyInHertz * rhs.m_frequencyInHertz);
}

Frequency Frequency::operator*(const int multiplier) const
{
	throwIfInvalid(*this);
	return Frequency(this->m_frequencyInHertz * multiplier);
}

std::ostream& operator<<(std::ostream& os, const Frequency& frequency)
{
	os << frequency.toString();
	return os;
}

Frequency::operator UInt64(void) const
{
	throwIfInvalid(*this);
	return m_frequencyInHertz;
}

Bool Frequency::isValid() const
{
	return m_valid;
}

std::string Frequency::toString() const
{
	if (isValid())
	{
		return std::to_string(m_frequencyInHertz);
	}
	else
	{
		return std::string(Constants::InvalidString);
	}
}

std::string Frequency::toStringAsMegahertz() const
{
	if (isValid())
	{
		return StringParser::removeTrailingZeros(std::to_string(m_frequencyInHertz / mhzToHzMultiplierInDouble));
	}
	else
	{
		return std::string(Constants::InvalidString);
	}
}

UInt64 Frequency::toIntAsMegahertz() const
{
	throwIfInvalid(*this);
	UInt64 asMegahertz = m_frequencyInHertz / mhzToHzMultiplierInUint64;
	return asMegahertz;
}

UInt64 Frequency::toIntAsHertz() const
{
	throwIfInvalid(*this);
	return m_frequencyInHertz;
}

void Frequency::throwIfInvalid(const Frequency& frequency) const
{
	if (frequency.isValid() == false)
	{
		throw dptf_exception("Frequency is not valid.");
	}
}