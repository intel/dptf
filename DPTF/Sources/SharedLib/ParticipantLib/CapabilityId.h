/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "esif_sdk_logging_data.h"

// This is the list of capabilities that participant can support.
namespace Capability
{
    enum Type
    {
        ActiveControl = 0,
        CtdpControl = 1,
        CoreControl = 2,
        DisplayControl = 3,
        DomainPriority = 4,
        PerformanceControl = 5,
        PowerControl = 6,
        PowerStatus = 7,
        TemperatureStatus = 8,
        UtilizationStatus = 9,
        PixelClockStatus = 10,
        PixelClockControl = 11,
        PlatformPowerStatus = 12,
        TemperatureThreshold = 13,
        RfProfileStatus = 14,
        RfProfileControl = 15,
        NetworkControl = 16,
        TransmitPowerControl = 17,
        CurrentControl = 18,
        HdcControl = 19,
        PSysControl = 20,
        Max
    };

    UInt32 ToCapabilityId(Capability::Type capabilityType);
    esif_data getEsifDataFromCapabilityData(EsifCapabilityData* capability);
}