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

#include "ParticipantSetSpecificInfo_000.h"

ParticipantSetSpecificInfo_000::ParticipantSetSpecificInfo_000(ParticipantServicesInterface* participantServicesInterface)
{
    // Do nothing.  Not an error.
}

void ParticipantSetSpecificInfo_000::setParticipantDeviceTemperatureIndication(UIntN participantIndex,
    const Temperature& temperature)
{
    throw not_implemented();
}

void ParticipantSetSpecificInfo_000::setParticipantCoolingPolicy(UIntN participantIndex,
    const CoolingPreference& coolingPreference)
{
    throw not_implemented();
}

void ParticipantSetSpecificInfo_000::clearCachedData(void)
{
    // Do nothing.  Not an error.
}

XmlNode* ParticipantSetSpecificInfo_000::getXml(UIntN domainIndex)
{
    throw not_implemented();
}