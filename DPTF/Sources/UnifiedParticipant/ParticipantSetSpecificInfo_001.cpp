/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

ParticipantSetSpecificInfo_001::ParticipantSetSpecificInfo_001(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface) :
    ParticipantSetSpecificInfoBase(participantIndex, domainIndex, participantServicesInterface)
{
}

void ParticipantSetSpecificInfo_001::setParticipantDeviceTemperatureIndication(UIntN participantIndex,
    const Temperature& temperature)
{
    getParticipantServices()->primitiveExecuteSetAsTemperatureC(
        esif_primitive_type::SET_DEVICE_TEMPERATURE_INDICATION, temperature);
}

void ParticipantSetSpecificInfo_001::setParticipantCoolingPolicy(UIntN participantIndex,
    const DptfBuffer& coolingPreference, CoolingPreferenceType::Type type)
{
    if (type == CoolingPreferenceType::_SCP)
    {
        getParticipantServices()->primitiveExecuteSet(esif_primitive_type::SET_COOLING_POLICY,
            ESIF_DATA_STRUCTURE, coolingPreference.get(), coolingPreference.size(), coolingPreference.size());
    }
    else if (type == CoolingPreferenceType::DSCP)
    {
        getParticipantServices()->primitiveExecuteSet(esif_primitive_type::SET_DPTF_COOLING_POLICY,
            ESIF_DATA_STRUCTURE, coolingPreference.get(), coolingPreference.size(), coolingPreference.size());
    }
    else
    {
        throw dptf_exception("Received unexpected CoolingPreferenceType.");
    }
}

void ParticipantSetSpecificInfo_001::setParticipantSpecificInfo(UIntN participantIndex, 
    ParticipantSpecificInfoKey::Type tripPoint, const Temperature& tripValue)
{
    esif_primitive_type primitiveType = esif_primitive_type::SET_TRIP_POINT_ACTIVE;
    UInt32 instance;

    switch (tripPoint)
    {
    case ParticipantSpecificInfoKey::AC0:
        instance = 0;
        break;
    case ParticipantSpecificInfoKey::AC1:
        instance = 1;
        break;
    case ParticipantSpecificInfoKey::AC2:
        instance = 2;
        break;
    case ParticipantSpecificInfoKey::AC3:
        instance = 3;
        break;
    case ParticipantSpecificInfoKey::AC4:
        instance = 4;
        break;
    case ParticipantSpecificInfoKey::AC5:
        instance = 5;
        break;
    case ParticipantSpecificInfoKey::AC6:
        instance = 6;
        break;
    case ParticipantSpecificInfoKey::AC7:
        instance = 7;
        break;
    case ParticipantSpecificInfoKey::AC8:
        instance = 8;
        break;
    case ParticipantSpecificInfoKey::AC9:
        instance = 9;
        break;
    default:
        throw dptf_exception("Received unexpected Specific Info Key: " + StlOverride::to_string(tripPoint));
        break;
    }

    return getParticipantServices()->primitiveExecuteSetAsTemperatureC(primitiveType, tripValue,
        Constants::Esif::NoDomain, static_cast<UInt8>(instance));
}

void ParticipantSetSpecificInfo_001::clearCachedData(void)
{
    // Do nothing.  We don't cache ParticipantSetSpecificInfo related data.
}

std::shared_ptr<XmlNode> ParticipantSetSpecificInfo_001::getXml(UIntN domainIndex)
{
    throw not_implemented();
}

std::string ParticipantSetSpecificInfo_001::getName(void)
{
    return "Set Specific Info Control (Version 1)";
}
