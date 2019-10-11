/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "EsifDataTime.h"

EsifDataTime::EsifDataTime(void)
{
	initialize(0);
}

EsifDataTime::EsifDataTime(const Int64 data)
{
	initialize(data);
}

EsifDataTime::operator EsifDataPtr(void)
{
	return &m_esifData;
}

TimeSpan EsifDataTime::createTimeSpanFromMilliseconds(void) const
{
	return TimeSpan::createFromMilliseconds(m_esifDataValue);
}

void EsifDataTime::initialize(Int64 data)
{
	m_esifDataValue = data;
	m_esifData.type = esif_data_type::ESIF_DATA_TIME;
	m_esifData.buf_ptr = &m_esifDataValue;
	m_esifData.buf_len = sizeof(m_esifDataValue);
	m_esifData.data_len = sizeof(m_esifDataValue);
}
