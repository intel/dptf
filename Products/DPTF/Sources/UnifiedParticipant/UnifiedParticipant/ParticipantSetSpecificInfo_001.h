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

#include "ParticipantSetSpecificInfoInterface.h"
#include "ComponentExtendedInterface.h"
#include "ParticipantServicesInterface.h"

class ParticipantSetSpecificInfo_001 final : public ParticipantSetSpecificInfoInterface,
    public ComponentExtendedInterface
{
public:

    ParticipantSetSpecificInfo_001(ParticipantServicesInterface* participantServicesInterface);

    // ParticipantSetSpecificInfoInterface
    virtual void setParticipantDeviceTemperatureIndication(UIntN participantIndex,
        const Temperature& temperature) override final;
    virtual void setParticipantCoolingPolicy(UIntN participantIndex,
        const CoolingPreference& coolingPreference) override final;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override final;
    virtual XmlNode* getXml(UIntN domainIndex) override final;

private:

    ParticipantServicesInterface* m_participantServicesInterface;
};