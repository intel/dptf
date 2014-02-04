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

#include "DomainConfigTdpControlFactory.h"
#include "DomainConfigTdpControl_000.h"
#include "DomainConfigTdpControl_001.h"

DomainConfigTdpControlInterface* DomainConfigTdpControlFactory::createDomainConfigTdpControlObject(UIntN version,
    ParticipantServicesInterface* participantServicesInterface)
{
    switch (version)
    {
        case 0:
            return new DomainConfigTdpControl_000(participantServicesInterface);
            break;
        case 1:
            return new DomainConfigTdpControl_001(participantServicesInterface);
            break;
        default:
            std::stringstream message;
            message << "Received request for DomainConfigTdpControl version that isn't defined: " << version;
            throw dptf_exception(message.str());
            break;
    }
}