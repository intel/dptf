/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include <vector>
#include <string>

class DptfBuffer
{
public:
	DptfBuffer(void);
	DptfBuffer(UInt32 sizeInBytes);
	~DptfBuffer(void);

	static DptfBuffer fromExistingByteArray(UInt8* byteArray, UInt32 numberOfBytes);
	static DptfBuffer fromExistingByteVector(std::vector<UInt8> byteVector);
	static DptfBuffer fromBool(Bool value);
	void allocate(UInt32 sizeInBytes);
	UInt8* get(void) const;
	const UInt8 get(UInt32 byteNumber) const;
	void set(UInt32 byteNumber, UInt8 byteValue);
	UInt32 size(void) const;
	void trim(UInt32 sizeInBytes);
	void put(UInt32 offset, UInt8* data, UInt32 length);
	void append(const DptfBuffer& otherBuffer);
	void append(UInt8 data);
	void append(UInt8* data, UInt32 sizeInBytes);
	UInt8 lastByte() const;
	std::string toString() const;

	Bool operator==(const DptfBuffer& rhs) const;
	UInt8 operator[](UInt32 byteNumber) const;

private:
	std::vector<UInt8> m_buffer;
};

std::ostream& operator<<(std::ostream& os, const DptfBuffer& buffer);
