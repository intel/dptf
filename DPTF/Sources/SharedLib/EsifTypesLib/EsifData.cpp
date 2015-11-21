/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#include "EsifData.h"

EsifDataContainer::EsifDataContainer(esif_data_type esifDataType, void* bufferPtr, UInt32 bufferLength, UInt32 dataLength)
{
    m_esifData.type = esifDataType;
    m_esifData.buf_ptr = bufferPtr;
    m_esifData.buf_len = bufferLength;
    m_esifData.data_len = dataLength;
}

esif_data_type EsifDataContainer::getEsifDataType(void) const
{
    return m_esifData.type;
}

void* EsifDataContainer::getBufferPtr(void) const
{
    return m_esifData.buf_ptr;
}

UInt32 EsifDataContainer::getBufferLength(void) const
{
    return m_esifData.buf_len;
}

UInt32 EsifDataContainer::getDataLength(void) const
{
    return m_esifData.data_len;
}

EsifDataContainer::operator EsifDataPtr(void)
{
    return &m_esifData;
}