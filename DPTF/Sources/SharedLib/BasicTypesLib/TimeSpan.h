/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#pragma once
#include "Dptf.h"

class TimeSpan
{
public:
	TimeSpan();
	~TimeSpan();

	static TimeSpan createInvalid();
	static TimeSpan createFromMicroseconds(Int64 microseconds);
	static TimeSpan createFromMilliseconds(Int64 milliseconds);
	static TimeSpan createFromTenthSeconds(Int64 tenthSeconds);
	static TimeSpan createFromSeconds(Int64 seconds);
	static TimeSpan createFromMinutes(Int64 minutes);
	static TimeSpan createFromHours(Int64 hours);

	Int64 asMicroseconds() const;
	double asMilliseconds() const;
	UInt64 asMillisecondsUInt() const;
	Int64 asMillisecondsInt() const;
	Int64 asTenthSecondsInt() const;
	double asTenthSeconds() const;
	double asSeconds() const;
	double asMinutes() const;
	double asHours() const;

	Bool isValid() const;
	Bool isInvalid() const;

	TimeSpan operator+(const TimeSpan& rhs) const;
	TimeSpan operator+=(const TimeSpan& rhs) const;
	TimeSpan operator-(const TimeSpan& rhs) const;
	TimeSpan operator-=(const TimeSpan& rhs) const;
	TimeSpan operator*(Int64 multiplier) const;
	TimeSpan operator/(Int64 divider) const;
	Bool operator==(const TimeSpan& rhs) const;
	Bool operator!=(const TimeSpan& rhs) const;
	Bool operator<(const TimeSpan& rhs) const;
	Bool operator>(const TimeSpan& rhs) const;
	Bool operator<=(const TimeSpan& rhs) const;
	Bool operator>=(const TimeSpan& rhs) const;

	std::string toStringMicroseconds() const;
	std::string toStringMilliseconds() const;
	std::string toStringSeconds(UInt32 precision = 1) const;

private:
	Bool m_valid;
	Int64 m_microseconds;

	void throwIfInvalid() const;
};
