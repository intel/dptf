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

#include "DptfBufferStream.h"

DptfBufferStream::DptfBufferStream(DptfBuffer& buffer)
	: m_buffer(buffer)
	, m_currentLocation(0)
{
}

DptfBufferStream::~DptfBufferStream()
{
}

UInt8 DptfBufferStream::readNextUint8()
{
	throwIfReadIsPastEndOfBuffer(sizeof(UInt8));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	UInt8 value = *((UInt8*)ptr);
	m_currentLocation += sizeof(UInt8);
	return value;
}

UInt16 DptfBufferStream::readNextUint16()
{
	throwIfReadIsPastEndOfBuffer(sizeof(UInt16));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	UInt16 value = *((UInt16*)ptr);
	m_currentLocation += sizeof(UInt16);
	return value;
}

UInt32 DptfBufferStream::readNextUint32()
{
	throwIfReadIsPastEndOfBuffer(sizeof(UInt32));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	UInt32 value = *((UInt32*)ptr);
	m_currentLocation += sizeof(UInt32);
	return value;
}

UInt64 DptfBufferStream::readNextUint64()
{
	throwIfReadIsPastEndOfBuffer(sizeof(UInt64));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	UInt64 value = *((UInt64*)ptr);
	m_currentLocation += sizeof(UInt64);
	return value;
}

double DptfBufferStream::readNextDouble()
{
	throwIfReadIsPastEndOfBuffer(sizeof(double));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	double value = *((double*)ptr);
	m_currentLocation += sizeof(double);
	return value;
}

float DptfBufferStream::readNextFloat()
{
	throwIfReadIsPastEndOfBuffer(sizeof(float));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	float value = *((float*)ptr);
	m_currentLocation += sizeof(float);
	return value;
}

Bool DptfBufferStream::readNextBool()
{
	throwIfReadIsPastEndOfBuffer(sizeof(bool));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	bool value = *((bool*)ptr);
	m_currentLocation += sizeof(bool);
	return value;
}

Power DptfBufferStream::readNextPower()
{
	throwIfReadIsPastEndOfBuffer(sizeof(Power));
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	Power value = *((Power*)ptr);
	m_currentLocation += sizeof(Power);
	return value;
}

Temperature DptfBufferStream::readNextTemperature()
{
	UInt32 temperatureDataSize = Temperature().toDptfBuffer().size();
	throwIfReadIsPastEndOfBuffer(temperatureDataSize);
	UInt8* ptr = m_buffer.get() + m_currentLocation;
	auto segment = DptfBuffer::fromExistingByteArray(ptr, temperatureDataSize);
	Temperature value = Temperature::createFromDptfBuffer(segment);
	m_currentLocation += temperatureDataSize;
	return value;
}

void DptfBufferStream::resetReadPosition()
{
	m_currentLocation = 0;
}

Bool DptfBufferStream::canReadNext(size_t bytes) const
{
	return ((m_currentLocation + bytes) < m_buffer.size());
}

void DptfBufferStream::appendUint8(UInt8 value)
{
	m_buffer.append((UInt8*)&value, sizeof(value));
}

void DptfBufferStream::appendUint16(UInt16 value)
{
	m_buffer.append((UInt8*)&value, sizeof(value));
}

void DptfBufferStream::appendUint32(UInt32 value)
{
	m_buffer.append((UInt8*)&value, sizeof(value));
}

void DptfBufferStream::appendUint64(UInt64 value)
{
	m_buffer.append((UInt8*)&value, sizeof(value));
}

void DptfBufferStream::appendDouble(double value)
{
	m_buffer.append((UInt8*)&value, sizeof(value));
}

void DptfBufferStream::appendFloat(float value)
{
	m_buffer.append((UInt8*)&value, sizeof(value));
}

void DptfBufferStream::appendBool(bool value)
{
	m_buffer.append((UInt8*)&value, sizeof(value));
}

const DptfBuffer& DptfBufferStream::getBuffer() const
{
	return m_buffer;
}

void DptfBufferStream::throwIfReadIsPastEndOfBuffer(size_t nextSize)
{
	if ((m_currentLocation + nextSize) > m_buffer.size())
	{
		throw dptf_exception("Size of read requested from buffer stream is too large.");
	}
}
