/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

ParticipantSetSpecificInfo_001::ParticipantSetSpecificInfo_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ParticipantSetSpecificInfoBase(participantIndex, domainIndex, participantServicesInterface)
{
	resetAllTripPoints();
}

ParticipantSetSpecificInfo_001::~ParticipantSetSpecificInfo_001()
{
}

void ParticipantSetSpecificInfo_001::setParticipantDeviceTemperatureIndication(
	UIntN participantIndex,
	const Temperature& temperature)
{
	getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
		esif_primitive_type::SET_DEVICE_TEMPERATURE_INDICATION, temperature);
}

void ParticipantSetSpecificInfo_001::setParticipantSpecificInfo(
	UIntN participantIndex,
	ParticipantSpecificInfoKey::Type tripPoint,
	const Temperature& tripValue)
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
	case ParticipantSpecificInfoKey::PSV:
		instance = 54;
		primitiveType = esif_primitive_type::SET_TRIP_POINT_PASSIVE;
		break;
	default:
		throw dptf_exception("Received unexpected Specific Info Key: " + std::to_string(tripPoint));
		break;
	}

	getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
		primitiveType,
		tripValue,
		Constants::Esif::NoDomain,
		static_cast<UInt8>(Constants::Esif::NoPersistInstanceOffset + instance));
}

void ParticipantSetSpecificInfo_001::onClearCachedData(void)
{
	// Do nothing.  We don't cache ParticipantSetSpecificInfo related data.
}

std::shared_ptr<XmlNode> ParticipantSetSpecificInfo_001::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string ParticipantSetSpecificInfo_001::getName(void)
{
	return "Set Specific Info Control";
}

void ParticipantSetSpecificInfo_001::resetAllTripPoints(void)
{
	resetPassiveTripPoint();
	UInt8 count = 0;
	for (IntN ac = ParticipantSpecificInfoKey::AC0; ac <= ParticipantSpecificInfoKey::AC9; ac++)
	{
		resetActiveTripPoint(count);
		count++;
	}
}

void ParticipantSetSpecificInfo_001::resetPassiveTripPoint(void)
{
	try
	{
		DptfBuffer tripPointBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_TRIP_POINT_PASSIVE, Constants::Esif::NoPersistInstance);
		getParticipantServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			tripPointBuffer.get(),
			tripPointBuffer.size(),
			tripPointBuffer.size(),
			0,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
	}
}

void ParticipantSetSpecificInfo_001::resetActiveTripPoint(UInt8 instance)
{
	try
	{
		DptfBuffer tripPointBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_TRIP_POINT_ACTIVE, Constants::Esif::NoPersistInstanceOffset + instance);
		getParticipantServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			tripPointBuffer.get(),
			tripPointBuffer.size(),
			tripPointBuffer.size(),
			0,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
	}
}
