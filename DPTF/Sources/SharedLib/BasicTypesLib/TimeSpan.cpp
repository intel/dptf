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

#include "TimeSpan.h"
#include "StatusFormat.h"
using namespace StatusFormat;
using namespace std;

constexpr UInt64 MicrosecondsPerMillisecond = 1000;
constexpr UInt64 MicrosecondsPerTenthSecond = 100000;
constexpr UInt64 MicrosecondsPerSecond = 1000000;
constexpr UInt64 MicrosecondsPerMinute = 60000000;
constexpr UInt64 MicrosecondsPerHour = 3600000000;

TimeSpan::TimeSpan()
	: m_valid(false)
	, m_microseconds(0)
{
}

TimeSpan TimeSpan::createInvalid()
{
	return {};
}

TimeSpan TimeSpan::createFromMicroseconds(Int64 microseconds)
{
	TimeSpan span;
	span.m_microseconds = microseconds;
	span.m_valid = true;
	return span;
}

TimeSpan TimeSpan::createFromMilliseconds(Int64 milliseconds)
{
	TimeSpan span;
	span.m_microseconds = milliseconds * MicrosecondsPerMillisecond;
	span.m_valid = true;
	return span;
}

TimeSpan TimeSpan::createFromTenthSeconds(Int64 tenthSeconds)
{
	TimeSpan span;
	span.m_microseconds = tenthSeconds * MicrosecondsPerTenthSecond;
	span.m_valid = true;
	return span;
}

TimeSpan TimeSpan::createFromSeconds(Int64 seconds)
{
	TimeSpan span;
	span.m_microseconds = seconds * MicrosecondsPerSecond;
	span.m_valid = true;
	return span;
}

TimeSpan TimeSpan::createFromSeconds(const string& seconds)
{
	TimeSpan span;
	span.m_microseconds = stoi(seconds) * MicrosecondsPerSecond;
	span.m_valid = true;
	return span;
}

TimeSpan TimeSpan::createFromMinutes(Int64 minutes)
{
	TimeSpan span;
	span.m_microseconds = minutes * MicrosecondsPerMinute;
	span.m_valid = true;
	return span;
}

TimeSpan TimeSpan::createFromHours(Int64 hours)
{
	TimeSpan span;
	span.m_microseconds = hours * MicrosecondsPerHour;
	span.m_valid = true;
	return span;
}

Int64 TimeSpan::asMicroseconds() const
{
	throwIfInvalid();
	return m_microseconds;
}

double TimeSpan::asMilliseconds() const
{
	throwIfInvalid();
	const double milliseconds = static_cast<double>(m_microseconds) / static_cast<double>(MicrosecondsPerMillisecond);
	return milliseconds;
}

UInt64 TimeSpan::asMillisecondsUInt() const
{
	throwIfInvalid();
	return static_cast<UInt64>(asMilliseconds());
}

Int64 TimeSpan::asMillisecondsInt() const
{
	throwIfInvalid();
	return static_cast<Int64>(asMilliseconds());
}

Int64 TimeSpan::asTenthSecondsInt() const
{
	throwIfInvalid();
	return static_cast<Int64>(asTenthSeconds());
}

double TimeSpan::asTenthSeconds() const
{
	throwIfInvalid();
	const double tenthSeconds = static_cast<double>(m_microseconds) / static_cast<double>(MicrosecondsPerTenthSecond);
	return tenthSeconds;
}

double TimeSpan::asSeconds() const
{
	throwIfInvalid();
	const double seconds = static_cast<double>(m_microseconds) / static_cast<double>(MicrosecondsPerSecond);
	return seconds;
}

UInt64 TimeSpan::asSecondsUInt() const
{
	throwIfInvalid();
	return static_cast<UInt64>(asSeconds());
}

double TimeSpan::asMinutes() const
{
	throwIfInvalid();
	const double minutes = static_cast<double>(m_microseconds) / static_cast<double>(MicrosecondsPerMinute);
	return minutes;
}

double TimeSpan::asHours() const
{
	throwIfInvalid();
	const double hours = static_cast<double>(m_microseconds) / static_cast<double>(MicrosecondsPerHour);
	return hours;
}

Bool TimeSpan::isValid() const
{
	return m_valid;
}

Bool TimeSpan::isInvalid() const
{
	return !m_valid;
}

TimeSpan TimeSpan::operator+(const TimeSpan& rhs) const
{
	throwIfInvalid();
	rhs.throwIfInvalid();
	return createFromMicroseconds(m_microseconds + rhs.m_microseconds);
}

TimeSpan TimeSpan::operator+=(const TimeSpan& rhs) const
{
	return *this + rhs;
}

TimeSpan TimeSpan::operator-(const TimeSpan& rhs) const
{
	throwIfInvalid();
	rhs.throwIfInvalid();
	return createFromMicroseconds(m_microseconds - rhs.m_microseconds);
}

TimeSpan TimeSpan::operator-=(const TimeSpan& rhs) const
{
	return *this - rhs;
}

TimeSpan TimeSpan::operator*(Int64 multiplier) const
{
	throwIfInvalid();
	return createFromMicroseconds(m_microseconds * multiplier);
}

TimeSpan TimeSpan::operator/(Int64 divider) const
{
	throwIfInvalid();
	return createFromMicroseconds(m_microseconds / divider);
}

Bool TimeSpan::operator==(const TimeSpan& rhs) const
{
	return m_microseconds == rhs.m_microseconds;
}

Bool TimeSpan::operator!=(const TimeSpan& rhs) const
{
	throwIfInvalid();
	rhs.throwIfInvalid();
	return m_microseconds != rhs.m_microseconds;
}

Bool TimeSpan::operator<(const TimeSpan& rhs) const
{
	throwIfInvalid();
	rhs.throwIfInvalid();
	return m_microseconds < rhs.m_microseconds;
}

Bool TimeSpan::operator>(const TimeSpan& rhs) const
{
	throwIfInvalid();
	rhs.throwIfInvalid();
	return m_microseconds > rhs.m_microseconds;
}

Bool TimeSpan::operator<=(const TimeSpan& rhs) const
{
	throwIfInvalid();
	rhs.throwIfInvalid();
	return m_microseconds <= rhs.m_microseconds;
}

Bool TimeSpan::operator>=(const TimeSpan& rhs) const
{
	throwIfInvalid();
	rhs.throwIfInvalid();
	return m_microseconds >= rhs.m_microseconds;
}

string TimeSpan::toStringMicroseconds() const
{
	stringstream stream;
	if (isInvalid())
	{
		stream << Constants::InvalidString;
	}
	else
	{
		stream << m_microseconds;
	}
	return stream.str();
}

string TimeSpan::toStringMilliseconds() const
{
	stringstream stream;
	if (isInvalid())
	{
		stream << Constants::InvalidString;
	}
	else
	{
		stream << asMillisecondsInt();
	}
	return stream.str();
}

string TimeSpan::toStringSeconds(UInt32 precision) const
{
	stringstream stream;
	if (isInvalid())
	{
		stream << Constants::InvalidString;
	}
	else
	{
		stream << friendlyValueWithPrecision(asSeconds(), precision);
	}
	return stream.str();
}

void TimeSpan::throwIfInvalid() const
{
	if (isInvalid())
	{
		throw dptf_exception("TimeSpan is not valid.");
	}
}
