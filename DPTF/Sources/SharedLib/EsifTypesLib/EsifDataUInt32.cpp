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

#include "EsifDataUInt32.h"

EsifDataUInt32::EsifDataUInt32(void)
{
	initialize(0);
}

EsifDataUInt32::EsifDataUInt32(UInt32 data)
{
	initialize(data);
}

EsifDataUInt32::EsifDataUInt32(const EsifDataPtr esifDataPtr)
{
	if (esifDataPtr == nullptr)
	{
		throw dptf_exception("EsifDataPtr is null.");
	}

	if (esifDataPtr->type != ESIF_DATA_UINT32)
	{
		throw dptf_exception("Received unexpected esifDataPtr->type.");
	}

	if (esifDataPtr->buf_ptr == nullptr)
	{
		throw dptf_exception("esifData->buf_ptr is null.");
	}

	if (esifDataPtr->buf_len < sizeof(UInt32))
	{
		throw dptf_exception("esifData->buf_len too small.");
	}

	if (esifDataPtr->data_len < sizeof(UInt32))
	{
		throw dptf_exception("esifData->data_len too small.");
	}

	initialize(*((UInt32*)esifDataPtr->buf_ptr));
}

EsifDataUInt32::operator EsifDataPtr(void)
{
	return &m_esifData;
}

EsifDataUInt32::operator UInt32(void) const
{
	return m_esifDataValue;
}

void EsifDataUInt32::initialize(UInt32 data)
{
	m_esifDataValue = data;

	m_esifData.type = esif_data_type::ESIF_DATA_UINT32;
	m_esifData.buf_ptr = &m_esifDataValue;
	m_esifData.buf_len = sizeof(m_esifDataValue);
	m_esifData.data_len = sizeof(m_esifDataValue);
}
