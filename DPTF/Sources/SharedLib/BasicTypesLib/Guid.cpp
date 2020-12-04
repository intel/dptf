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

#include "Guid.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"
#include <algorithm>

Guid::Guid(void)
	: m_valid(false)
{
	esif_ccb_memset(m_guid, 0, GuidSize);
}

Guid::Guid(const UInt8 guid[GuidSize])
	: m_valid(true)
{
	esif_ccb_memcpy(m_guid, guid, GuidSize);
}

Guid::Guid(
	UInt8 value00,
	UInt8 value01,
	UInt8 value02,
	UInt8 value03,
	UInt8 value04,
	UInt8 value05,
	UInt8 value06,
	UInt8 value07,
	UInt8 value08,
	UInt8 value09,
	UInt8 value10,
	UInt8 value11,
	UInt8 value12,
	UInt8 value13,
	UInt8 value14,
	UInt8 value15)
	: m_valid(true)
{
	m_guid[0] = value00;
	m_guid[1] = value01;
	m_guid[2] = value02;
	m_guid[3] = value03;
	m_guid[4] = value04;
	m_guid[5] = value05;
	m_guid[6] = value06;
	m_guid[7] = value07;
	m_guid[8] = value08;
	m_guid[9] = value09;
	m_guid[10] = value10;
	m_guid[11] = value11;
	m_guid[12] = value12;
	m_guid[13] = value13;
	m_guid[14] = value14;
	m_guid[15] = value15;
}

Guid Guid::createInvalid()
{
	return Guid();
}

Bool Guid::operator==(const Guid& rhs) const
{	
	UIntN mySize = sizeof(this->m_guid)/sizeof(this->m_guid[0]);
	UIntN rhsSize = sizeof(rhs.m_guid)/sizeof(rhs.m_guid[0]);

	if (mySize != rhsSize)
	{
		return false;
	}
	
	// FIXME: this can be switched to memcmp once implemented in esif/ccb
	for (UIntN i = 0; i < mySize && i < rhsSize; ++i)
	{
		if (this->m_guid[i] != rhs.m_guid[i])
		{
			return false;
		}
	}
	return true;
}

Bool Guid::operator!=(const Guid& rhs) const
{
	return !(*this == rhs);
}

std::ostream& operator<<(std::ostream& os, const Guid& guid)
{
	os << guid.toString();
	return os;
}

Guid::operator const UInt8*(void)const
{
	throwIfInvalid(*this);
	return m_guid;
}

Bool Guid::isValid(void) const
{
	return m_valid;
}

std::string Guid::toString() const
{
	std::stringstream guidTextStream;
	guidTextStream << std::hex << std::setfill('0');
	for (UIntN i = 0; i < GuidSize; i++)
	{
		if (isValid())
		{
			guidTextStream << std::setw(2) << (IntN)(UInt8)m_guid[i];
		}
		else
		{
			guidTextStream << "X";
		}

		if (i < GuidSize - 1)
		{
			guidTextStream << "-";
		}
	}

	std::string guidText = guidTextStream.str();
	std::transform(guidText.begin(), guidText.end(), guidText.begin(), [](int c) -> char { return (char)::toupper(c); } );
	return guidText;
}

Guid Guid::fromString(const std::string guidString)
{
	// It is expected that the input string is in such format:
	// 7516b95f-f776-4464-8c53-06167f40cc99

	UInt16 guid[GuidSize] = {0};
	UInt8 guidBytes[GuidSize] = {0};

	const Int32 returnCount = esif_ccb_sscanf(
		guidString.c_str(),
		"%2hx%2hx%2hx%2hx-%2hx%2hx-%2hx%2hx-%2hx%2hx-%2hx%2hx%2hx%2hx%2hx%2hx",
		&guid[0],
		&guid[1],
		&guid[2],
		&guid[3],
		&guid[4],
		&guid[5],
		&guid[6],
		&guid[7],
		&guid[8],
		&guid[9],
		&guid[10],
		&guid[11],
		&guid[12],
		&guid[13],
		&guid[14],
		&guid[15]);

	if (returnCount != GuidSize)
	{
		return Guid(); // return invalid Guid
	}

	// convert shorts to bytes
	for (size_t i = 0; i < GuidSize; ++i)
	{
		guidBytes[i] = static_cast<UInt8>(guid[i]);
	}

	return Guid(guidBytes);
}

void Guid::throwIfInvalid(const Guid& guid) const
{
	if (guid.isValid() == false)
	{
		throw dptf_exception("Guid is invalid.");
	}
}

void Guid::copyToBuffer(UInt8 buffer[GuidSize]) const
{
	for (UInt32 pos = 0; pos < GuidSize; pos++)
	{
		buffer[pos] = m_guid[pos];
	}
}
