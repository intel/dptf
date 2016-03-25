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
#include "esif_sdk_data.h"

struct EsifDataBinaryPpccPackage
{
    union esif_data_variant powerLimitIndex;   // ULONG - Index 0 = PL1, Index 1 = PL2
    union esif_data_variant powerLimitMinimum; // ULONG - Min power allowed for this limit. (In mW)
    union esif_data_variant powerLimitMaximum; // ULONG - Max power allowed for this limit. (In mW)
                                               //  If the min == max, there is no power programmability for this limit
    union esif_data_variant timeWindowMinimum; // ULONG - Min time window allowed for this limit. (In mS)
    union esif_data_variant timeWindowMaximum; // ULONG - Max time window allowed for this limit. (In mS)
                                               //  If the min == max, there is no time limit programmability for this limit
    union esif_data_variant stepSize;          // Step size for the turbo power limit (In mW)
};