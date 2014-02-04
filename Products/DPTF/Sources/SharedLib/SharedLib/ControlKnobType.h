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

namespace ControlKnobType
{
    enum Type
    {
        Invalid = 0,
        PowerControlPl1 = 0x00010000,
        PowerControlPl2 = 0x00010001,
        PowerControlPl3 = 0x00010002,
        PerformanceControlPerfFrequency = 0x00020000,
        PerformanceControlThrottleFrequency = 0x00020001,
        DbptControlIccMax = 0x00030000,
        CoreControlLpo = 0x00040000,
        DisplayControlBrightness = 0x00050000,
        ActiveCoolingControlFanSpeed = 0x00060000,
        ConfigTdpControlTurboState = 0x00070000,
        TauControlPl1 = 0x00080000,
        TauControlPl2 = 0x00080001,
        TauControlPl3 = 0x00080002
    };

    std::string ToString(ControlKnobType::Type type);
}