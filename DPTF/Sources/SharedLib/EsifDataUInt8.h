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
#include "esif_data.h"

class EsifDataUInt8 final
{
public:

    EsifDataUInt8(void);
    EsifDataUInt8(UInt8 data);
    EsifDataUInt8(const esif::EsifDataPtr esifDataPtr);

    operator esif::EsifDataPtr(void);
    operator UInt8(void) const;

private:

    // hide the copy constructor and assignment operator.
    EsifDataUInt8(const EsifDataUInt8& rhs);
    EsifDataUInt8& operator=(const EsifDataUInt8& rhs);

    UInt8 m_esifDataValue;
    esif::EsifData m_esifData;

    void initialize(UInt8 data);
};