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

#pragma pack(push, 1)

struct EsifDataBinaryTrtPackage
{
	union esif_data_variant sourceDevice; // ObjectReference - 64 byte string
	union esif_data_variant targetDevice; // ObjectReference - 64 byte string
	union esif_data_variant thermalInfluence; // ULONG
	union esif_data_variant thermalSamplingPeriod; // ULONG
	union esif_data_variant reserved1; // ULONG
	union esif_data_variant reserved2; // ULONG
	union esif_data_variant reserved3; // ULONG
	union esif_data_variant reserved4; // ULONG
};

#pragma pack(pop)
