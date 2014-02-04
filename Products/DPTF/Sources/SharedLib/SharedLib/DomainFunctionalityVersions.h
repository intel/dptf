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

struct DomainFunctionalityVersions final
{
    // By default all of the UInt8's will be set to 0
    DomainFunctionalityVersions(void);

    // Initialize based on array of capabilities received from ESIF
    DomainFunctionalityVersions(UInt8 capabilityBytes[]);

    // If the Uint8 contains 0, that means the domain doesn't implement the interface.
    // For anything > 0, this is the version number of the implementation it chooses.
    // The version will be used by the class factory to instantiate the correct object.
    // Since all of this is provided by ESIF, a domain can mix and match the different
    // capabilities and the participant is constructed at run time based on the DSP.
    UInt8 activeControlVersion;
    UInt8 configTdpControlVersion;
    UInt8 coreControlVersion;
    UInt8 displayControlVersion;
    UInt8 domainPriorityVersion;
    UInt8 performanceControlVersion;
    UInt8 powerControlVersion;
    UInt8 powerStatusVersion;
    UInt8 temperatureVersion;
    UInt8 utilizationVersion;
    UInt8 pixelClockControlVersion;
    UInt8 pixelClockStatusVersion;
    UInt8 rfProfileControlVersion;
    UInt8 rfProfileStatusVersion;
};