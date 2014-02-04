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

#pragma once

#include "Dptf.h"
#include "Guid.h"
#include "esif_data.h"

class EsifDataGuid final
{
public:

    static const UIntN GuidSize = 16;

    EsifDataGuid(void);
    EsifDataGuid(const Guid& data);
    EsifDataGuid(const esif_guid_t& esifGuid);
    EsifDataGuid(const esif::EsifData& esifData);

    operator esif::EsifData(void);
    operator esif::EsifDataPtr(void);
    operator Guid(void) const;

private:

    // hide the copy constructor and assignment operator.
    EsifDataGuid(const EsifDataGuid& rhs);
    EsifDataGuid& operator=(const EsifDataGuid& rhs);

    UInt8 m_guid[GuidSize];
    esif::EsifData m_esifData;

    void initialize(const UInt8 guid[GuidSize]);
};