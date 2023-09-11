/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#include "DptfBufferStream.h"
#include <cmath>

Percentage::Percentage(void)
	: m_valid(false)
	, m_percentage(-1.0)
{
}

Percentage::Percentage(double percentage)
	: m_valid(true)
	, m_percentage(percentage)
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

Percentage Percentage::fromSignedWholeNumber(Int32 signedWholeNumber)
{
	double value = ((double)signedWholeNumber / 100.0);
	return Percentage(value);
}

Percentage Percentage::fromCentiPercent(UInt64 centiPercent)
{
	double value = ((double)centiPercent / 100.0) / 100.0;
	return Percentage(value);
}

Bool Percentage::operator==(const Percentage& rhs) const
{
	// Do not throw an exception if percentage is not valid.

	if (this->isValid() == true && rhs.isValid() == true)
	{
		return (this->toWholeNumber() == rhs.toWholeNumber());
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

Int32 Percentage::toSignedWholeNumber() const
{
	return (Int32)round(m_percentage * 100.0);
}

UIntN Percentage::toWholeNumber() const
{
	return (UIntN)round(m_percentage * 100.0);
}

UInt64 Percentage::toCentiPercent() const
{
	return (UInt64)(m_percentage * 100 * 100);
}

Percentage Percentage::toCentiPercentInPercentage() const
{
	return Percentage(m_percentage * 100 * 100);
}

std::string Percentage::toString() const
{
	return toStringWithPrecision(2);
}

std::string Percentage::toStringWithPrecision(UInt32 precision) const
{
	if (isValid())
	{
		std::stringstream stream;
		stream << std::setprecision(precision) << std::fixed << (m_percentage * 100);
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

Percentage Percentage::createFromDptfBuffer(const DptfBuffer& buffer)
{
	if (buffer.size() != (sizeof(m_valid) + sizeof(m_percentage)))
	{
		throw dptf_exception("Buffer given to Percentage class has invalid length.");
	}

	DptfBuffer bufferCopy = buffer;
	DptfBufferStream stream(bufferCopy);

	Percentage newPercentage;
	newPercentage.m_valid = stream.readNextBool();
	newPercentage.m_percentage = stream.readNextDouble();
	return newPercentage;
}

DptfBuffer Percentage::toDptfBuffer() const
{
	DptfBuffer buffer;
	buffer.append((UInt8*)&m_valid, sizeof(m_valid));
	buffer.append((UInt8*)&m_percentage, sizeof(m_percentage));
	return buffer;
}
