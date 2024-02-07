/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

struct EsifDataBinaryTssPackage
{
	union esif_data_variant performancePercentage; // ULONG - Percent of core CPU operating frequency (1-100)
	union esif_data_variant power; // ULONG - Power dissipation in mW
	union esif_data_variant latency; // ULONG - Transition latency in microseconds
	union esif_data_variant control; // Context value for platform firmware to initiate performance state transition
	union esif_data_variant status; // ULONG - Comparison value to use with PERF_STATUS register to ensure
	//  t-state transition was successful.
};
