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
#include <string>
#include "esif_data.h"
#include "esif_rc.h"

class EsifDataString final
{
public:

    // create based on an STL string
    EsifDataString(const std::string& data);

    // create empty based on initialBufferSize
    EsifDataString(UIntN initialBufferSize);

    // create from esif data
    EsifDataString(const esif::EsifData& esifData);

    operator esif::EsifData(void);
    operator esif::EsifDataPtr(void);
    operator std::string(void) const;

private:

    // hide the copy constructor and assignment operator.
    EsifDataString(const EsifDataString & rhs);
    EsifDataString& operator=(const EsifDataString& rhs);

    std::string m_esifDataValue;
    esif::EsifData m_esifData;

    void initialize(const std::string& data);
};

eEsifError FillDataPtrWithString(esif::EsifDataPtr dataPtr, std::string dataString);