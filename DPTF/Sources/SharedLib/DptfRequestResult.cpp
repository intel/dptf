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

#include "DptfRequestResult.h"
#include "DptfBufferStream.h"

using namespace std;

DptfRequestResult::DptfRequestResult(Bool isSuccessful, const std::string& message, const DptfRequest& request)
	: m_isSuccessful(isSuccessful)
	, m_message(message)
	, m_data()
	, m_request(request)

{
}

DptfRequestResult::DptfRequestResult()
	: m_isSuccessful(false)
	, m_message(Constants::EmptyString)
	, m_data()
	, m_request()
{
}

Bool DptfRequestResult::isSuccessful() const
{
	return m_isSuccessful;
}

Bool DptfRequestResult::isFailure() const
{
	return !m_isSuccessful;
}

void DptfRequestResult::throwIfFailure() const
{
	if (isFailure())
	{
		throw dptf_exception(m_message);
	}
}

void DptfRequestResult::setData(const DptfBuffer& data)
{
	m_data = data;
}

void DptfRequestResult::setDataFromUInt32(const UInt32 data)
{
	DptfBuffer buffer;
	buffer.append((UInt8*)&data, sizeof(data));
	m_data = buffer;
}

void DptfRequestResult::setDataFromBool(const Bool data)
{
	DptfBuffer buffer;
	buffer.append((UInt8*)&data, sizeof(data));
	m_data = buffer;
}

const DptfBuffer& DptfRequestResult::getData() const
{
	return m_data;
}

UInt32 DptfRequestResult::getDataAsUInt32() const
{
	if (m_data.size() != sizeof(UInt32))
	{
		throw dptf_exception("Data is not of UInt32 length."s);
	}

	DptfBuffer bufferCopy = m_data;
	DptfBufferStream stream(bufferCopy);
	return stream.readNextUInt32();
}

Bool DptfRequestResult::getDataAsBool() const
{
	if (m_data.size() != sizeof(Bool))
	{
		throw dptf_exception("Data is not of Bool length."s);
	}

	DptfBuffer bufferCopy = m_data;
	DptfBufferStream stream(bufferCopy);
	const Bool dataAsBool = stream.readNextBool();
	return dataAsBool;
}

const DptfRequest& DptfRequestResult::getRequest() const
{
	return m_request;
}

const std::string DptfRequestResult::getMessage() const
{
	return m_message;
}