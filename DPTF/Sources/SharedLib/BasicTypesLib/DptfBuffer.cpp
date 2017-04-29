/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "DptfBuffer.h"

DptfBuffer::DptfBuffer(void)
{
}

DptfBuffer::DptfBuffer(UInt32 sizeInBytes)
{
	allocate(sizeInBytes);
}

DptfBuffer::~DptfBuffer(void)
{
}

DptfBuffer DptfBuffer::fromExistingByteArray(UInt8* byteArray, UInt32 numberOfBytes)
{
	DptfBuffer buffer(numberOfBytes);
	for (UInt32 byteNumber = 0; byteNumber < numberOfBytes; byteNumber++)
	{
		buffer.set(byteNumber, byteArray[byteNumber]);
	}
	return buffer;
}

DptfBuffer DptfBuffer::fromExistingByteVector(std::vector<UInt8> byteVector)
{
	DptfBuffer buffer;
	buffer.m_buffer = byteVector;
	return buffer;
}

void DptfBuffer::allocate(UInt32 sizeInBytes)
{
	m_buffer.clear();
	m_buffer.resize(sizeInBytes, 0);
}

UInt8* DptfBuffer::get(void) const
{
	return (UInt8*)m_buffer.data();
}

const UInt8 DptfBuffer::get(UInt32 byteNumber) const
{
	if (byteNumber >= size())
	{
		throw dptf_exception("Requested byte number from DPTF Buffer is outside of the valid range.");
	}
	else
	{
		return m_buffer.at(byteNumber);
	}
}

void DptfBuffer::set(UInt32 byteNumber, UInt8 byteValue)
{
	if (byteNumber >= size())
	{
		throw dptf_exception("Requested byte number from DPTF Buffer is outside of the valid range.");
	}
	else
	{
		m_buffer.at(byteNumber) = byteValue;
	}
}

UInt32 DptfBuffer::size() const
{
	return (UInt32)m_buffer.size();
}

void DptfBuffer::trim(UInt32 sizeInBytes)
{
	if (sizeInBytes < size())
	{
		m_buffer.resize(sizeInBytes);
	}
}

Bool DptfBuffer::operator==(const DptfBuffer& rhs) const
{
	if (size() != rhs.size())
	{
		return false;
	}

	for (UInt32 pos = 0; pos < size(); pos++)
	{
		if (get(pos) != rhs.get(pos))
		{
			return false;
		}
	}

	return true;
}

void DptfBuffer::put(UInt32 offset, UInt8* data, UInt32 length)
{
	if ((offset + length) > size())
	{
		UInt32 difference = (offset + length) - size();
		m_buffer.resize(size() + difference, 0);
	}

	for (auto pos = offset; pos < (offset + length); pos++)
	{
		m_buffer[pos] = data[pos - offset];
	}
}

void DptfBuffer::append(const DptfBuffer& otherBuffer)
{
	auto currentSize = (UInt32)m_buffer.size();
	put(currentSize, otherBuffer.get(), otherBuffer.size());
}
