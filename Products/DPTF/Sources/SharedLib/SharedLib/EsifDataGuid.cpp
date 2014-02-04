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

#include "EsifDataGuid.h"
#include "esif_ccb_memory.h"

EsifDataGuid::EsifDataGuid(void)
{
    initialize(Guid());
}

EsifDataGuid::EsifDataGuid(const Guid& data)
{
    initialize(data);
}

EsifDataGuid::EsifDataGuid(const esif_guid_t& esifGuid)
{
    initialize(esifGuid);
}

EsifDataGuid::EsifDataGuid(const esif::EsifData& esifData)
{
    if (esifData.type != ESIF_DATA_GUID)
    {
        throw dptf_exception("Received invalid EsifData parameter.  Expected ESIF_DATA_GUID.");
    }

    initialize(static_cast<const UInt8*>(esifData.buf_ptr));
}

EsifDataGuid::operator esif::EsifData(void)
{
    return m_esifData;
}

EsifDataGuid::operator esif::EsifDataPtr(void)
{
    return &m_esifData;
}

EsifDataGuid::operator Guid(void) const
{
    return Guid(m_guid);
}

void EsifDataGuid::initialize(const UInt8 guid[GuidSize])
{
    esif_ccb_memcpy(m_guid, guid, GuidSize);

    m_esifData.type = esif_data_type::ESIF_DATA_GUID;
    m_esifData.buf_ptr = m_guid;
    m_esifData.buf_len = GuidSize;
    m_esifData.data_len = GuidSize;
}