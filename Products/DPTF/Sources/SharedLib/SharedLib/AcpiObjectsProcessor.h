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

struct EsifDataBinaryPssPackage
{
    union esif_data_variant coreFrequency;      // ULONG - Frequency in MHz
    union esif_data_variant power;              // ULONG - Power dissipation in mW
    union esif_data_variant latency;            // ULONG - Transition latency in microseconds
    union esif_data_variant busMasterLatency;   // ULONG - Worst case latency that bus cannot access memory
    union esif_data_variant control;            // Context value for platform firmware to initiate performance state transition
    union esif_data_variant status;             // ULONG - Comparison value to use with PERF_STATUS register to ensure
                                                //  p-state transition was successful.
};

struct EsifDataBinaryTssPackage
{
    union esif_data_variant performancePercentage;  // ULONG - Percent of core CPU operating frequency (1-100)
    union esif_data_variant power;                  // ULONG - Power dissipation in mW
    union esif_data_variant latency;                // ULONG - Transition latency in microseconds
    union esif_data_variant control;                // Context value for platform firmware to initiate performance state transition
    union esif_data_variant status;                 // ULONG - Comparison value to use with PERF_STATUS register to ensure
                                                    //  t-state transition was successful.
};

struct EsifDataBinaryGfxPstateConfig
{
    union esif_data_variant maxRenderFrequency;     // ULONG - Gfx driver should not set the render frequency higher than this
};

const UIntN GFX_PSTATE_TRANSITION_LATENCY = 100000; // In microseconds (10^-6); = 0.1 seconds

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

struct EsifDataBinaryClpoPackage
{
    union esif_data_variant lpoEnable;                 // ULONG - If 0, LPO control not supported (even if available).
    union esif_data_variant startPstateIndex;          // ULONG - When the policy should initiate active control (P state index!)
    union esif_data_variant stepSize;                  // ULONG - Percentage indicating how many processors to offline
                                                       // E.g. 25% on a Dual core system with 4 logical processors, then policy would take away 1 logical processor at a time
    union esif_data_variant powerControlSetting;       // ULONG - If P0 (power limiting), 1 = SMT Control, 2 = Core off lining
    union esif_data_variant performanceControlSetting; // ULONG - If P1 (performance limiting), 1 = SMT Control, 2 = Core off lining
};