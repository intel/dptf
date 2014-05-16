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

#include "DomainPerformanceControlFactory.h"
#include "DomainPerformanceControl_000.h"
#include "DomainPerformanceControl_001.h"
#include "DomainPerformanceControl_002.h"
#include "DomainPerformanceControl_003.h"

DomainPerformanceControlInterface* DomainPerformanceControlFactory::createDomainPerformanceControlObject(
    UIntN version, ParticipantServicesInterface* participantServicesInterface)
{
    switch (version)
    {
        case 0:
            return new DomainPerformanceControl_000(participantServicesInterface);
            break;
        case 1: // Generic Participant
            return new DomainPerformanceControl_001(participantServicesInterface);
            break;
        case 2: // Processor participant (CPU domain)
            return new DomainPerformanceControl_002(participantServicesInterface);
            break;
        case 3: // Processor participant (GFX domain, Interface V1)
            return new DomainPerformanceControl_003(participantServicesInterface);
            break;
        default:
            std::stringstream message;
            message << "Received request for DomainPerformanceControl version that isn't defined: " << version;
            throw dptf_exception(message.str());
            break;
    }
}