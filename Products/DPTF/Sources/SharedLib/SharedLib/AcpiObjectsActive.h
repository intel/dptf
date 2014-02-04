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

#include "esif_data_variant.h"

#pragma pack(push, 1)

struct EsifDataBinaryArtPackage
{
    union esif_data_variant sourceDevice;           // ObjectReference - 64 byte string
    union esif_data_variant targetDevice;           // ObjectReference - 64 byte string
    union esif_data_variant weight;                 // ULONG - The source device's contribution to the platform's target device cooing capacity.
    union esif_data_variant ac0MaxFanSpeed;         // ULONG - Indicates max fan speed when AC0 is exceeded.
    union esif_data_variant ac1MaxFanSpeed;         // ULONG - Indicates max fan speed when AC1 is exceeded.
    union esif_data_variant ac2MaxFanSpeed;         // ULONG - Indicates max fan speed when AC2 is exceeded.
    union esif_data_variant ac3MaxFanSpeed;         // ULONG - Indicates max fan speed when AC3 is exceeded.
    union esif_data_variant ac4MaxFanSpeed;         // ULONG - Indicates max fan speed when AC4 is exceeded.
    union esif_data_variant ac5MaxFanSpeed;         // ULONG - Indicates max fan speed when AC5 is exceeded.
    union esif_data_variant ac6MaxFanSpeed;         // ULONG - Indicates max fan speed when AC6 is exceeded.
    union esif_data_variant ac7MaxFanSpeed;         // ULONG - Indicates max fan speed when AC7 is exceeded.
    union esif_data_variant ac8MaxFanSpeed;         // ULONG - Indicates max fan speed when AC8 is exceeded.
    union esif_data_variant ac9MaxFanSpeed;         // ULONG - Indicates max fan speed when AC9 is exceeded.
};

#pragma pack(pop)