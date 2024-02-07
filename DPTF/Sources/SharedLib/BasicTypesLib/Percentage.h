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

class Percentage final
{
public:
	Percentage(void); // Initialized to invalid by default
	Percentage(double percentage); // Passing in 0.99 results in 99%
	static Percentage createInvalid();
	static Percentage fromWholeNumber(UIntN wholeNumber);
	static Percentage fromSignedWholeNumber(Int32 signedWholeNumber);
	static Percentage fromCentiPercent(UInt64 centiPercent);
	static Percentage createFromDptfBuffer(const class DptfBuffer& buffer);

	Bool operator==(const Percentage& rhs) const;
	Bool operator!=(const Percentage& rhs) const;
	Bool operator>(const Percentage& rhs) const;
	Bool operator>=(const Percentage& rhs) const;
	Bool operator<(const Percentage& rhs) const;
	Bool operator<=(const Percentage& rhs) const;
	friend std::ostream& operator<<(std::ostream& os, const Percentage& percentage);
	operator double(void) const;

	Bool isValid() const;
	Int32 toSignedWholeNumber() const;
	UIntN toWholeNumber() const;
	UInt64 toCentiPercent() const;
	Percentage toCentiPercentInPercentage() const;
	std::string toString() const;
	std::string toStringWithPrecision(UInt32 precision) const;
	class DptfBuffer toDptfBuffer() const;

private:
	Bool m_valid;
	double m_percentage;

	Percentage(UInt64 percentage); // Make sure someone doesn't try to instantiate this class with a whole number.
	void throwIfInvalid(const Percentage& percentage) const;
};
