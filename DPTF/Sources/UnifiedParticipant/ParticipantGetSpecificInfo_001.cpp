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

#include "ParticipantGetSpecificInfo_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"

class PrimitiveAndInstance
{
public:
	esif_primitive_type primitive;
	UInt32 instance;
};

ParticipantGetSpecificInfo_001::ParticipantGetSpecificInfo_001(
	UIntN participantIndex,
	UIntN domainIndex,
	const std::shared_ptr<ParticipantServicesInterface>& participantServicesInterface)
	: ParticipantGetSpecificInfoBase(participantIndex, domainIndex, participantServicesInterface)
{
	onClearCachedData();
}

std::map<ParticipantSpecificInfoKey::Type, Temperature> ParticipantGetSpecificInfo_001::getParticipantSpecificInfo(
	UIntN participantIndex,
	const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo)
{
	std::map<ParticipantSpecificInfoKey::Type, Temperature> results;
	for (auto request : requestedInfo)
	{
		auto tripPointTemperature = Temperature(Constants::MaxUInt32);
		try
		{
			auto result = m_cachedData.find(request);
			if (result != m_cachedData.end())
			{
				tripPointTemperature = result->second;
			}
			else
			{
				tripPointTemperature = readSpecificInfo(getPrimitiveAndInstanceForSpecificInfoKey(request));
				m_cachedData[request] = tripPointTemperature;
			}
		}
		catch (...)
		{
			// if the primitive isn't available in the cache we receive an exception
		}

		results[request] = tripPointTemperature;
	}

	return results;
}

void ParticipantGetSpecificInfo_001::onClearCachedData()
{
	m_cachedData.clear();
}

Temperature ParticipantGetSpecificInfo_001::readSpecificInfo(PrimitiveAndInstance primitiveAndInstance) const
{
	auto tripPoint = getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
		primitiveAndInstance.primitive, Constants::Esif::NoDomain, static_cast<UInt8>(primitiveAndInstance.instance));
	tripPoint = Temperature::snapWithinAllowableTripPointRange(tripPoint);

	return tripPoint;
}

PrimitiveAndInstance ParticipantGetSpecificInfo_001::getPrimitiveAndInstanceForSpecificInfoKey(
	ParticipantSpecificInfoKey::Type request) const
{
	PrimitiveAndInstance primitiveAndInstance{};
	switch (request)
	{
	case ParticipantSpecificInfoKey::Type::Warm:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_WARM;
		primitiveAndInstance.instance = Constants::Esif::NoInstance;
		break;
	case ParticipantSpecificInfoKey::Type::Hot:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_HOT;
		primitiveAndInstance.instance = Constants::Esif::NoInstance;
		break;
	case ParticipantSpecificInfoKey::Type::Critical:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_CRITICAL;
		primitiveAndInstance.instance = Constants::Esif::NoInstance;
		break;
	case ParticipantSpecificInfoKey::Type::PSV:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_PASSIVE;
		primitiveAndInstance.instance = Constants::Esif::NoInstance;
		break;
	case ParticipantSpecificInfoKey::Type::NTT:
		primitiveAndInstance.primitive = esif_primitive_type::GET_NOTIFICATION_TEMP_THRESHOLD;
		primitiveAndInstance.instance = Constants::Esif::NoInstance;
		break;
	case ParticipantSpecificInfoKey::Type::AC0:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 0;
		break;
	case ParticipantSpecificInfoKey::Type::AC1:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 1;
		break;
	case ParticipantSpecificInfoKey::Type::AC2:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 2;
		break;
	case ParticipantSpecificInfoKey::Type::AC3:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 3;
		break;
	case ParticipantSpecificInfoKey::Type::AC4:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 4;
		break;
	case ParticipantSpecificInfoKey::Type::AC5:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 5;
		break;
	case ParticipantSpecificInfoKey::Type::AC6:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 6;
		break;
	case ParticipantSpecificInfoKey::Type::AC7:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 7;
		break;
	case ParticipantSpecificInfoKey::Type::AC8:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 8;
		break;
	case ParticipantSpecificInfoKey::Type::AC9:
		primitiveAndInstance.primitive = esif_primitive_type::GET_TRIP_POINT_ACTIVE;
		primitiveAndInstance.instance = 9;
		break;
	default:
		throw dptf_exception("Received unexpected Specific Info Key: " + std::to_string(request));
	}
	return primitiveAndInstance;
}

std::shared_ptr<XmlNode> ParticipantGetSpecificInfo_001::getXml(UIntN domainIndex)
{
	// Setup request vector
	std::vector<ParticipantSpecificInfoKey::Type> tripRequest;
	tripRequest.push_back(ParticipantSpecificInfoKey::Critical);
	tripRequest.push_back(ParticipantSpecificInfoKey::Hot);
	tripRequest.push_back(ParticipantSpecificInfoKey::Warm);
	tripRequest.push_back(ParticipantSpecificInfoKey::PSV);
	tripRequest.push_back(ParticipantSpecificInfoKey::NTT);

	for (IntN ac = ParticipantSpecificInfoKey::AC0; ac <= ParticipantSpecificInfoKey::AC9; ac++)
	{
		tripRequest.push_back(static_cast<ParticipantSpecificInfoKey::Type>(ac));
	}

	std::map<ParticipantSpecificInfoKey::Type, Temperature> tripPoints =
		getParticipantSpecificInfo(getParticipantIndex(), tripRequest);

	auto root = XmlNode::createWrapperElement("specific_info");

	auto tripPoint = tripPoints.end();
	if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::Critical)) != tripPoints.end())
	{
		root->addChild(XmlNode::createDataElement("crt", tripPoint->second.toString()));
	}
	else
	{
		root->addChild(XmlNode::createDataElement("crt", Constants::InvalidString));
	}

	if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::Hot)) != tripPoints.end())
	{
		root->addChild(XmlNode::createDataElement("hot", tripPoint->second.toString()));
	}
	else
	{
		root->addChild(XmlNode::createDataElement("hot", Constants::InvalidString));
	}

	if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::Warm)) != tripPoints.end())
	{
		root->addChild(XmlNode::createDataElement("wrm", tripPoint->second.toString()));
	}
	else
	{
		root->addChild(XmlNode::createDataElement("wrm", Constants::InvalidString));
	}

	if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::PSV)) != tripPoints.end())
	{
		root->addChild(XmlNode::createDataElement("psv", tripPoint->second.toString()));
	}
	else
	{
		root->addChild(XmlNode::createDataElement("psv", Constants::InvalidString));
	}

	if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::NTT)) != tripPoints.end())
	{
		root->addChild(XmlNode::createDataElement("ntt", tripPoint->second.toString()));
	}
	else
	{
		root->addChild(XmlNode::createDataElement("ntt", Constants::InvalidString));
	}

	UIntN count = 0;
	for (IntN ac = ParticipantSpecificInfoKey::AC0; ac <= ParticipantSpecificInfoKey::AC9; ac++)
	{
		std::stringstream acx;
		acx << "ac" << count++;

		if ((tripPoint = tripPoints.find(static_cast<ParticipantSpecificInfoKey::Type>(ac))) != tripPoints.end())
		{
			root->addChild(XmlNode::createDataElement(acx.str(), tripPoint->second.toString()));
		}
		else
		{
			root->addChild(XmlNode::createDataElement(acx.str(), Constants::InvalidString));
		}
	}

	return root;
}

std::string ParticipantGetSpecificInfo_001::getName()
{
	return "Get Specific Info Control";
}
