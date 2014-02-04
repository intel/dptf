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

struct EsifDataBinaryFifPackage //Has revision as first param
{
    union esif_data_variant revision;
    union esif_data_variant hasFineGrainControl;            //BOOLEAN - If non-zero, supports _FSL evaluation with 0-100 range
                                                            // Else, must use control value in _FPS to program _FSL
    union esif_data_variant stepSize;                       //Context value for platform firmware to initiate performance state transition
    union esif_data_variant supportsLowSpeedNotification;   //ULONG - Performance for particular state, use with raw units
};

struct EsifDataBinaryFpsPackage //Has revision as first param
{
    union esif_data_variant control;                        //ULONG - The value used to set the fan speed in _FSL.
    union esif_data_variant tripPoint;                      //ULONG - The active cooling trip point number corresponding with this performance state.
    union esif_data_variant speed;                          //ULONG - Fan RPM
    union esif_data_variant noiseLevel;                     //ULONG - Audible noise in 10ths of decibels.
    union esif_data_variant power;                          //ULONG - Power consumption in mW.
};

struct EsifDataBinaryFstPackage //Has revision as first param
{
    union esif_data_variant revision;
    union esif_data_variant control;                        // ULONG - The value used to set the fan speed in _FSL.
    union esif_data_variant speed;                          // ULONG - The active cooling trip point number corresponding with this performance state.
};