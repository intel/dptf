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

#include "ParticipantSetSpecificInfo_001.h"

ParticipantSetSpecificInfo_001::ParticipantSetSpecificInfo_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
}

void ParticipantSetSpecificInfo_001::setParticipantDeviceTemperatureIndication(UIntN participantIndex,
    const Temperature& temperature)
{
    m_participantServicesInterface->primitiveExecuteSetAsTemperatureC(
        esif_primitive_type::SET_DEVICE_TEMPERATURE_INDICATION, temperature);
}

void ParticipantSetSpecificInfo_001::setParticipantCoolingPolicy(UIntN participantIndex,
    const CoolingPreference& coolingPreference)
{
    EsifDataBinaryScp esifData = coolingPreference.getEsifCompliantBinary();

    m_participantServicesInterface->primitiveExecuteSet(esif_primitive_type::SET_COOLING_POLICY,
        ESIF_DATA_STRUCTURE, static_cast<void*>(&esifData), sizeof(esifData), sizeof(esifData));
}

void ParticipantSetSpecificInfo_001::clearCachedData(void)
{
    // Do nothing.  We don't cache ParticipantSetSpecificInfo related data.
}

XmlNode* ParticipantSetSpecificInfo_001::getXml(UIntN domainIndex)
{
    throw not_implemented();
}