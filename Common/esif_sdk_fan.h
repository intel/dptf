/*******************************************************************************
** This file is provided under a dual BSD/GPLv2 license.  When using or
** redistributing this file, you may do so under either license.
**
** GPL LICENSE SUMMARY
**
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of version 2 of the GNU General Public License as published by the
** Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
** details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software  Foundation, Inc.,
** 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
** The full GNU General Public License is included in this distribution in the
** file called LICENSE.GPL.
**
** BSD LICENSE
**
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** * Redistributions of source code must retain the above copyright notice, this
**   list of conditions and the following disclaimer.
** * Redistributions in binary form must reproduce the above copyright notice,
**   this list of conditions and the following disclaimer in the documentation
**   and/or other materials provided with the distribution.
** * Neither the name of Intel Corporation nor the names of its contributors may
**   be used to endorse or promote products derived from this software without
**   specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#pragma once

#include "esif_sdk_data.h"

#pragma pack(push, 1)

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

struct EsifDataBinaryFcdcPackage //Has revision as first param
{	
	union esif_data_variant revision;
	union esif_data_variant minspeed;						 // ULONG - The value used to set the min fan speed percentage
	union esif_data_variant maxspeed;                        // ULONG - The value used to set the max fan speed percentage
};

#pragma pack(pop)
