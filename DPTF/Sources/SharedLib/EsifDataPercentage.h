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

class EsifDataPercentage final
{
public:

    EsifDataPercentage(void);
    EsifDataPercentage(const Percentage& data);

    operator esif::EsifDataPtr(void);
    operator Percentage(void) const;

private:

    // hide the copy constructor and assignment operator.
    EsifDataPercentage(const EsifDataPercentage& rhs);
    EsifDataPercentage& operator=(const EsifDataPercentage& rhs);

    // 93% is stored as 9300
    UInt32 m_esifDataValue;
    esif::EsifData m_esifData;

    static const UInt32 m_conversionValue = 10000;
    static const double m_roundingValue;

    void initialize(UInt32 data);
};