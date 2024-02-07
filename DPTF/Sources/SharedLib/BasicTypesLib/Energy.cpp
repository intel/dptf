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

#include "Energy.h"
#include "DptfBuffer.h"
#include "DptfBufferStream.h"

using namespace std;

static constexpr UInt32 MaxValidEnergyInJoules = 1000000; // 1,000,000 Joules

Energy::Energy()
	: m_valid(false)
	, m_joules(0)
{

}

Energy::Energy(unsigned joules)
	: m_valid(true)
	, m_joules(joules)
{
	if (m_joules > MaxValidEnergyInJoules)
	{
		throw dptf_exception("Energy value " + to_string(m_joules) + " out of valid range.");
	}
}

Energy::Energy(unsigned counter, unsigned width, double unit)
{
	throwIfWidthLargerThanMax(width);
	const auto mask = 1UL << width;
	counter = counter | mask;
	counter = counter & ~mask;
	m_joules = static_cast<unsigned>(counter * unit);
	m_valid = true;
}

Energy::Energy(const DptfBuffer& buffer)
{
	if (buffer.size() != sizeof(Energy))
	{
		throw dptf_exception("Invalid data length");
	}

	DptfBuffer bufferCopy = buffer;
	DptfBufferStream stream(bufferCopy);
	*this = stream.readNextEnergy();
}

bool Energy::isValid() const
{
	return m_valid;
}

std::string Energy::toStringWithoutUnit() const
{
	return to_string(m_joules);
}

Bool Energy::operator==(const Energy& other) const
{
	if (this->isValid() == true && other.isValid() == true)
	{
		return (this->m_joules == other.m_joules);
	}
	else if (this->isValid() == false && other.isValid() == false)
	{
		return true;
	}
	else
	{
		return false;
	}
}

Bool Energy::operator!=(const Energy& other) const
{
	return !(*this == other);
}

Bool Energy::operator>(const Energy& other) const
{
	throwIfInvalid(*this);
	throwIfInvalid(other);
	return (this->m_joules > other.m_joules);
}

Bool Energy::operator>=(const Energy& other) const
{
	throwIfInvalid(*this);
	throwIfInvalid(other);
	return (this->m_joules >= other.m_joules);
}

Bool Energy::operator<(const Energy& other) const
{
	throwIfInvalid(*this);
	throwIfInvalid(other);
	return this->m_joules < other.m_joules;
}

Bool Energy::operator<=(const Energy& other) const
{
	throwIfInvalid(*this);
	throwIfInvalid(other);
	return this->m_joules <= other.m_joules;
}

Energy Energy::operator+(const Energy& other) const
{
	throwIfInvalid(*this);
	throwIfInvalid(other);
	return {this->m_joules + other.m_joules};
}

Energy Energy::operator-(const Energy& other) const
{
	throwIfInvalid(*this);
	throwIfInvalid(other);

	if (other.m_joules > this->m_joules)
	{
		throw dptf_exception("Invalid energy subtraction requested.  Right side is greater than left side.");
	}

	return {this->m_joules - other.m_joules};
}

Power Energy::operator/(const TimeSpan& time) const
{
	if (time.isInvalid() || time == TimeSpan::createFromSeconds(0))
	{
		return Power::createInvalid();
	}
	const auto watts = static_cast<double>(m_joules) / time.asSeconds();
	return Power::createFromWatts(watts);
}

ostream& operator<<(ostream& os, const Energy& energy)
{
	os << energy.toStringWithoutUnit();
	return os;
}

Energy::operator DptfBuffer() const
{
	DptfBuffer buffer;
	buffer.append(reinterpret_cast<const UInt8*>(this), sizeof(Energy));
	return buffer;
}

Energy::operator std::string() const
{
	return toStringWithoutUnit() + "J"s;
}

Energy::operator bool() const
{
	return m_valid;
}

void Energy::throwIfInvalid(const Energy& energy)
{
	if (energy.isValid() == false)
	{
		throw dptf_exception("Energy is invalid.");
	}
}

void Energy::throwIfWidthLargerThanMax(unsigned width)
{
	constexpr unsigned maxCounterWidth = 32;
	if (width > maxCounterWidth)
	{
		throw dptf_exception("Invalid width"s);
	}
}