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

#include "DomainRfProfileControlInterface.h"
#include "ComponentExtendedInterface.h"
#include "ParticipantServicesInterface.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//

class DomainRfProfileControl_000 final : public DomainRfProfileControlInterface,
    public ComponentExtendedInterface
{
public:

    DomainRfProfileControl_000(ParticipantServicesInterface* participantServicesInterface);

    // DomainRfProfileControlInterface
    virtual RfProfileCapabilities getRfProfileCapabilities(UIntN participantIndex,
        UIntN domainIndex) override final;
    virtual void setRfProfileCenterFrequency(UIntN participantIndex, UIntN domainIndex,
        const Frequency& centerFrequency) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;
};