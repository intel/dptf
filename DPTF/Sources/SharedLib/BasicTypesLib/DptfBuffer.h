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
#include <vector>
#include <string>

class DptfBuffer
{
public:
	DptfBuffer();
	DptfBuffer(size_t sizeInBytes);
	virtual ~DptfBuffer() = default;

	DptfBuffer(const DptfBuffer& other) = default;
	DptfBuffer& operator=(const DptfBuffer& other) = default;
	DptfBuffer(DptfBuffer&& other) = default;
	DptfBuffer& operator=(DptfBuffer&& other) = default;

	static DptfBuffer fromExistingByteArray(const UInt8* byteArray, UInt32 numberOfBytes);
	static DptfBuffer fromExistingByteVector(const std::vector<UInt8>& byteVector);
	static DptfBuffer fromBool(Bool value);
	static DptfBuffer fromString(const std::string& value);
	void allocate(size_t sizeInBytes);
	[[nodiscard]] UInt8* get() const;
	[[nodiscard]] UInt8 get(UInt32 byteNumber) const;
	[[nodiscard]] DptfBuffer get(UInt32 offset, UInt32 numberOfBytes) const;
	void set(UInt32 byteNumber, UInt8 byteValue);
	[[nodiscard]] UInt32 size() const;
	void trim(UInt32 sizeInBytes);
	void put(UInt32 offset, const UInt8* data, UInt32 length);
	void append(const DptfBuffer& otherBuffer);
	void append(UInt8 data);
	void append(const UInt8* data, UInt32 sizeInBytes);
	[[nodiscard]] UInt8 lastByte() const;
	[[nodiscard]] std::string toString() const;
	[[nodiscard]] Bool notEmpty() const;

	Bool operator==(const DptfBuffer& rhs) const;
	UInt8 operator[](UInt32 byteNumber) const;
	operator const std::vector<UInt8>&() const;

private:
	std::vector<UInt8> m_buffer;
	void throwIfOutsideBuffer(UInt32 byteNumber) const;
};

std::ostream& operator<<(std::ostream& os, const DptfBuffer& buffer);
