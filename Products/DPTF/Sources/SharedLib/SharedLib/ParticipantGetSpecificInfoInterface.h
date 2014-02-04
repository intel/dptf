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
#include "ParticipantSpecificInfoKey.h"
#include <map>
#include <vector>

class ParticipantGetSpecificInfoInterface
{
public:

    virtual ~ParticipantGetSpecificInfoInterface()
    {
    };

    // This function is used to request policy specific information from a participant.
    // For the input parameters, pass in the destination participant index and a vector containing
    // all keys of interest.  The participant will return a map containing each key and associated
    // value.
    virtual std::map<ParticipantSpecificInfoKey::Type, UIntN> getParticipantSpecificInfo(UIntN participantIndex,
        const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo) = 0;
};