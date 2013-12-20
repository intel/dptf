/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

#include "BasicTypes.h"

namespace Constants
{
    const UInt32 Invalid = 0xFFFFFFFFU;
    const UIntN DefaultBufferSize = 4096;
    const UIntN GuidSize = 16;

    namespace Participants
    {
        const UIntN MaxParticipantEstimate = 20;
    }

    namespace Esif
    {
        // NoParticipant and NoDomain must be set higher than any reasonable index value
        const UIntN NoParticipant = 1999;
        const UIntN NoDomain = 1999;

        // NoInstance is defined by ESIF to be 255
        const UInt8 NoInstance = 255;
    }
}