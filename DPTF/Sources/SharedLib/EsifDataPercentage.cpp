/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#include "EsifDataPercentage.h"

const double EsifDataPercentage::m_roundingValue = .00005;

EsifDataPercentage::EsifDataPercentage(void)
{
    initialize(0);
}

EsifDataPercentage::EsifDataPercentage(const Percentage& data)
{
    // In this case we have to convert the float to an unsigned int
    // to satisfy ESIF.  For 93%, the float would store .9300, and
    // we convert it to an unsigned int with 9300 for ESIF.

    double doubleValue = ((data + m_roundingValue) * m_conversionValue);
    UInt32 uint32Value = static_cast<UInt32>(doubleValue);

    initialize(uint32Value);
}

EsifDataPercentage::operator esif::EsifDataPtr(void)
{
    return &m_esifData;
}

EsifDataPercentage::operator Percentage(void) const
{
    // In this case we will have 93% stored as 9300.  We convert it to
    // .9300 which is used internally in our Percentage class.

    double doubleValue = static_cast<double>(m_esifDataValue) / m_conversionValue;

    return Percentage(doubleValue);
}

void EsifDataPercentage::initialize(UInt32 data)
{
    m_esifDataValue = data;

    m_esifData.type = esif_data_type::ESIF_DATA_PERCENT;
    m_esifData.buf_ptr = &m_esifDataValue;
    m_esifData.buf_len = sizeof(m_esifDataValue);
    m_esifData.data_len = sizeof(m_esifDataValue);
}