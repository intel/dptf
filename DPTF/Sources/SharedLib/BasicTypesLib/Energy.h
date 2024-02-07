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
#include <string>
#include "TimeSpan.h"

class DptfBuffer;

class Energy final
{
public:
	Energy();
	Energy(unsigned joules);
	Energy(unsigned counter, unsigned width, double unit);
	Energy(const DptfBuffer& buffer);

	bool isValid() const;
	std::string toStringWithoutUnit() const;

	bool operator==(const Energy& other) const;
	bool operator!=(const Energy& other) const;
	bool operator>(const Energy& other) const;
	bool operator>=(const Energy& other) const;
	bool operator<(const Energy& other) const;
	bool operator<=(const Energy& other) const;
	Energy operator+(const Energy& other) const;
	Energy operator-(const Energy& other) const;
	Power operator/(const TimeSpan& time) const;

	friend std::ostream& operator<<(std::ostream& os, const Energy& energy);
	operator DptfBuffer() const;
	operator std::string() const;
	operator bool() const;
	

private:
	bool m_valid;
	unsigned m_joules;

	static void throwIfInvalid(const Energy& energy);
	static void throwIfWidthLargerThanMax(unsigned width);
};
