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

#include "DptfRequest.h"
#include "DptfBufferStream.h"

DptfRequest::DptfRequest()
	: m_requestType((DptfRequestType::Enum)Constants::Invalid)
	, m_participantIndex(Constants::Invalid)
	, m_domainIndex(Constants::Invalid)
{
}

DptfRequest::DptfRequest(DptfRequestType::Enum requestType)
	: m_requestType(requestType)
	, m_participantIndex(Constants::Invalid)
	, m_domainIndex(Constants::Invalid)
{
}

DptfRequest::DptfRequest(DptfRequestType::Enum requestType, UInt32 participantIndex)
	: m_requestType(requestType)
	, m_participantIndex(participantIndex)
	, m_domainIndex(Constants::Invalid)
{
}

DptfRequest::DptfRequest(DptfRequestType::Enum requestType, UInt32 participantIndex, UInt32 domainIndex)
	: m_requestType(requestType)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
{
}

DptfRequest::DptfRequest(
	DptfRequestType::Enum requestType,
	const DptfBuffer& data,
	UInt32 participantIndex,
	UInt32 domainIndex)
	: m_requestType(requestType)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_data(data)
{
}

DptfRequest::~DptfRequest()
{
}

DptfRequestType::Enum DptfRequest::getRequestType() const
{
	return m_requestType;
}

UInt32 DptfRequest::getParticipantIndex() const
{
	return m_participantIndex;
}

UInt32 DptfRequest::getDomainIndex() const
{
	return m_domainIndex;
}

const DptfBuffer& DptfRequest::getData() const
{
	return m_data;
}

const UInt32 DptfRequest::getDataAsUInt32() const
{
	if (m_data.size() != sizeof(UInt32))
	{
		throw dptf_exception("Data is not of UInt32 length.");
	}

	DptfBuffer bufferCopy = m_data;
	DptfBufferStream stream(bufferCopy);
	UInt32 dataAsUInt32 = stream.readNextUint32();
	return dataAsUInt32;
}

void DptfRequest::setData(const DptfBuffer& data)
{
	m_data = data;
}

void DptfRequest::setDataFromUInt32(const UInt32 data)
{
	DptfBuffer buffer;
	buffer.append((UInt8*)&data, sizeof(data));
	m_data = buffer;
}

Bool DptfRequest::operator==(const DptfRequest& rhs) const
{
	return (
		this->getRequestType() == rhs.getRequestType() && this->getParticipantIndex() == rhs.getParticipantIndex()
		&& this->getDomainIndex() == rhs.getDomainIndex() && this->getData() == rhs.getData());
};
