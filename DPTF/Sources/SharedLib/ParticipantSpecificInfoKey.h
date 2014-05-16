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

namespace ParticipantSpecificInfoKey
{
    enum Type
    {
        None,
        Warm,
        Hot,
        Critical,
        AC0,
        AC1,
        AC2,
        AC3,
        AC4,
        AC5,
        AC6,
        AC7,
        AC8,
        AC9,
        PSV,
        NTT,
        MaxSize
    };

    std::string ToString(ParticipantSpecificInfoKey::Type participantSpecificInfoKeyType);
}