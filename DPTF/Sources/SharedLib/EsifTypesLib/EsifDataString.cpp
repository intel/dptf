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

#include "EsifDataString.h"
#include "esif_ccb_string.h"

#define RESPONSE_BUFFER_LEN_NORMAL 256 * 1024
#define RESPONSE_BUFFER_LEN_CAP 250 * 1024 * 1024

EsifDataString::EsifDataString(const std::string& data)
{
	initialize(data);
}

EsifDataString::EsifDataString(UIntN initialBufferSize)
{
	initialize(std::string(initialBufferSize, 0));
}

EsifDataString::EsifDataString(const EsifDataPtr esifDataPtr)
{
	if (esifDataPtr == nullptr)
	{
		throw dptf_exception("EsifDataPtr is null.");
	}

	if (esifDataPtr->type != ESIF_DATA_STRING)
	{
		throw dptf_exception("Received unexpected esifDataPtr->type.");
	}

	if (esifDataPtr->buf_ptr == nullptr)
	{
		throw dptf_exception("esifData->buf_ptr is null.");
	}

	char* buffer = static_cast<char*>(esifDataPtr->buf_ptr);
	UIntN bufferLength = esifDataPtr->buf_len;
	Bool nullTerminated = false;

	for (UIntN i = 0; i < bufferLength; i++)
	{
		if (buffer[i] == '\0')
		{
			nullTerminated = true;
			break;
		}
	}

	if (nullTerminated == false)
	{
		throw dptf_exception("Received ESIF_DATA_STRING without null terminator.");
	}

	initialize(std::string(static_cast<char*>(esifDataPtr->buf_ptr)));
}

EsifDataString::operator EsifData(void)
{
	return m_esifData;
}

EsifDataString::operator EsifDataPtr(void)
{
	return &m_esifData;
}

EsifDataString::operator std::string(void) const
{
	return m_esifDataValue;
}

void EsifDataString::initialize(const std::string& data)
{
	m_esifDataValue = data;

	m_esifData.type = esif_data_type::ESIF_DATA_STRING;
	m_esifData.buf_ptr = (void*)m_esifDataValue.c_str();
	m_esifData.buf_len = static_cast<UInt32>(data.size() + 1);
	m_esifData.data_len = static_cast<UInt32>(data.size() + 1);
}

eEsifError FillDataPtrWithString(EsifDataPtr dataPtr, std::string dataString)
{
	UIntN requiredBufferLength = static_cast<UIntN>(dataString.length() + 1);

	dataPtr->data_len = requiredBufferLength;

	if (dataPtr->buf_len >= requiredBufferLength)
	{
		dataPtr->type = ESIF_DATA_STRING;
		esif_ccb_strcpy((esif_string)dataPtr->buf_ptr, dataString.c_str(), requiredBufferLength);
		return ESIF_OK;
	}
	else
	{
		dataPtr->data_len =
			std::min(std::max((int)(requiredBufferLength * 2), RESPONSE_BUFFER_LEN_NORMAL), RESPONSE_BUFFER_LEN_CAP);
		return ESIF_E_NEED_LARGER_BUFFER;
	}
}
