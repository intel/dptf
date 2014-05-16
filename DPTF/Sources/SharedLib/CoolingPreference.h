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
#include "CoolingMode.h"
#include "CoolingModeAcousticLimit.h"
#include "CoolingModePowerLimit.h"
#include "XmlNode.h"

struct EsifDataBinaryScp
{
    UInt32 mode;
    UInt32 acousticLimit;
    UInt32 powerLimit;
};

class CoolingPreference final
{
public:

    CoolingPreference(CoolingMode::Type coolingMode, CoolingModeAcousticLimit::Type coolingModeAcousticLimit,
        CoolingModePowerLimit::Type coolingModePowerLimit);
    EsifDataBinaryScp getEsifCompliantBinary(void) const;

    Bool operator==(const CoolingPreference& rhs) const;
    Bool operator!=(const CoolingPreference& rhs) const;

    XmlNode* getXml() const;

private:

    CoolingMode::Type m_coolingMode;

    // Value between 1 (no tolerance) - 5 (max tolerance) (default: 0 means no value specified)
    CoolingModeAcousticLimit::Type m_coolingModeAcousticLimit;

    // Value between 1 (no tolerance) - 5 (max tolerance) (default: 0 means no value specified)
    CoolingModePowerLimit::Type m_coolingModePowerLimit;
};