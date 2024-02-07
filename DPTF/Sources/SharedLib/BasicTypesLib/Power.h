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

#pragma once

#include "Dptf.h"

//
// represents power in mW
//
class Power final
{
public:
	Power(void);
	Power(UInt32 power);
	static Power createInvalid();
	static Power createFromMilliwatts(UInt32 milliwatts);
	static Power createFromWatts(double watts);
	static Power createFromDptfBuffer(const class DptfBuffer& buffer);

	double asWatts() const;
	double asMilliwatts() const;

	Bool operator==(const Power& rhs) const;
	Bool operator!=(const Power& rhs) const;
	Bool operator>(const Power& rhs) const;
	Bool operator>=(const Power& rhs) const;
	Bool operator<(const Power& rhs) const;
	Bool operator<=(const Power& rhs) const;
	Power operator+(const Power& rhs) const;
	Power operator-(const Power& rhs) const;
	friend std::ostream& operator<<(std::ostream& os, const Power& power);
	operator UInt32(void) const;

	Bool isValid() const;
	std::string toString() const;
	std::string toStringAsWatts(int precision) const;
	std::string toStringAsMilliWatts(int precision) const;
	Int32 toInt32() const;
	class DptfBuffer toDptfBuffer() const;

private:
	Bool m_valid;
	UInt32 m_power;

	static void throwIfInvalid(const Power& power);
};
