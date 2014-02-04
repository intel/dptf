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

struct EsifDataBinaryPpssPackage
{
    union esif_data_variant performancePercentage;      //ULONG
    union esif_data_variant power;                      //ULONG - Power dissipation in mW
    union esif_data_variant latency;                    //ULONG - Transition latency in microseconds
    union esif_data_variant isLinear;                   //BOOLEAN - Is/Is not linear control state
    union esif_data_variant control;                    //Context value for platform firmware to initiate performance state transition
    union esif_data_variant rawPerformance;             //ULONG - Performance for particular state, use with raw units
    union esif_data_variant rawUnits;                   //CHAR ARRAY[8] - Units for raw performance value (e.g. "MHz")
    union esif_data_variant reserved;                   //??
};

#pragma pack(pop)