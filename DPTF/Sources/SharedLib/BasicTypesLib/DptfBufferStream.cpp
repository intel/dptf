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

#include "DptfBufferStream.h"

DptfBufferStream::DptfBufferStream(DptfBuffer& buffer)
	: m_buffer(buffer)
	, m_currentLocation(0)
{
}

UInt8 DptfBufferStream::readNextUInt8()
{
	throwIfReadIsPastEndOfBuffer(sizeof(UInt8));
	const UInt8* ptr = m_buffer.get() + m_currentLocation;
	const UInt8 value = *ptr;
	m_currentLocation += sizeof(UInt8);
	return value;
}

UInt16 DptfBufferStream::readNextUInt16()
{
	throwIfReadIsPastEndOfBuffer(sizeof(UInt16));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	const UInt16 value = *reinterpret_cast<UInt16*>(ptr);
	m_currentLocation += sizeof(UInt16);
	return value;
}

UInt32 DptfBufferStream::readNextUInt32()
{
	throwIfReadIsPastEndOfBuffer(sizeof(UInt32));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	const UInt32 value = *reinterpret_cast<UInt32*>(ptr);
	m_currentLocation += sizeof(UInt32);
	return value;
}

UInt64 DptfBufferStream::readNextUInt64()
{
	throwIfReadIsPastEndOfBuffer(sizeof(UInt64));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	const UInt64 value = *reinterpret_cast<UInt64*>(ptr);
	m_currentLocation += sizeof(UInt64);
	return value;
}

double DptfBufferStream::readNextDouble()
{
	throwIfReadIsPastEndOfBuffer(sizeof(double));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	const double value = *reinterpret_cast<double*>(ptr);
	m_currentLocation += sizeof(double);
	return value;
}

float DptfBufferStream::readNextFloat()
{
	throwIfReadIsPastEndOfBuffer(sizeof(float));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	const float value = *reinterpret_cast<float*>(ptr);
	m_currentLocation += sizeof(float);
	return value;
}

Bool DptfBufferStream::readNextBool()
{
	throwIfReadIsPastEndOfBuffer(sizeof(bool));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	const bool value = *reinterpret_cast<bool*>(ptr);
	m_currentLocation += sizeof(bool);
	return value;
}

Power DptfBufferStream::readNextPower()
{
	throwIfReadIsPastEndOfBuffer(sizeof(Power));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	const Power value = *reinterpret_cast<Power*>(ptr);
	m_currentLocation += sizeof(Power);
	return value;
}

Energy DptfBufferStream::readNextEnergy()
{
	throwIfReadIsPastEndOfBuffer(sizeof(Energy));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	const Energy value = *reinterpret_cast<Energy*>(ptr);
	m_currentLocation += sizeof(Energy);
	return value;
}

Temperature DptfBufferStream::readNextTemperature()
{
	const UInt32 temperatureDataSize = Temperature().toDptfBuffer().size();
	throwIfReadIsPastEndOfBuffer(temperatureDataSize);
	const UInt8* ptr = m_buffer.get() + m_currentLocation;
	const auto segment = DptfBuffer::fromExistingByteArray(ptr, temperatureDataSize);
	const Temperature value = Temperature::createFromDptfBuffer(segment);
	m_currentLocation += temperatureDataSize;
	return value;
}

void DptfBufferStream::resetReadPosition()
{
	m_currentLocation = 0;
}

void DptfBufferStream::movePositionBy(size_t bytes)
{
	throwIfReadIsPastEndOfBuffer(bytes);
	m_currentLocation += static_cast<UInt32>(bytes);
}

Bool DptfBufferStream::canReadNext(size_t bytes) const
{
	return ((m_currentLocation + bytes) <= m_buffer.size());
}

void DptfBufferStream::appendUInt8(UInt8 value) const
{
	m_buffer.append(&value, sizeof(value));
}

void DptfBufferStream::appendUInt16(UInt16 value) const
{
	m_buffer.append(reinterpret_cast<UInt8*>(&value), sizeof(value));
}

void DptfBufferStream::appendUInt32(UInt32 value) const
{
	m_buffer.append(reinterpret_cast<UInt8*>(&value), sizeof(value));
}

void DptfBufferStream::appendUInt64(UInt64 value) const
{
	m_buffer.append(reinterpret_cast<UInt8*>(&value), sizeof(value));
}

void DptfBufferStream::appendDouble(double value) const
{
	m_buffer.append(reinterpret_cast<UInt8*>(&value), sizeof(value));
}

void DptfBufferStream::appendFloat(float value) const
{
	m_buffer.append(reinterpret_cast<UInt8*>(&value), sizeof(value));
}

void DptfBufferStream::appendBool(bool value) const
{
	m_buffer.append(reinterpret_cast<UInt8*>(&value), sizeof(value));
}

const DptfBuffer& DptfBufferStream::getBuffer() const
{
	return m_buffer;
}

void DptfBufferStream::throwIfReadIsPastEndOfBuffer(size_t nextSize) const
{
	if ((m_currentLocation + nextSize) > m_buffer.size())
	{
		throw dptf_exception("Size of read requested from buffer stream is too large.");
	}
}
