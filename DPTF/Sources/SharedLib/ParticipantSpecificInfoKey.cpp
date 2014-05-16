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

#include "ParticipantSpecificInfoKey.h"

namespace ParticipantSpecificInfoKey
{
    std::string ToString(ParticipantSpecificInfoKey::Type participantSpecificInfoKeyType)
    {
        switch (participantSpecificInfoKeyType)
        {
            case None:
                return "none";
            case Warm:
                return "warm";
            case Hot:
                return "hot";
            case Critical:
                return "critical";
            case AC0:
                return "ac0";
            case AC1:
                return "ac1";
            case AC2:
                return "ac2";
            case AC3:
                return "ac3";
            case AC4:
                return "ac4";
            case AC5:
                return "ac5";
            case AC6:
                return "ac6";
            case AC7:
                return "ac7";
            case AC8:
                return "ac8";
            case AC9:
                return "ac9";
            case PSV:
                return "passive";
            case NTT:
                return "ntt";
            default:
                throw dptf_exception("ParticipantSpecificInfoKey::Type is invalid");
        }
    }
}