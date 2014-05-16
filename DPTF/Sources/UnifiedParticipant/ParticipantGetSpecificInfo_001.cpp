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

#include "ParticipantGetSpecificInfo_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"

class PrimitiveAndInstance
{
public:
    esif_primitive_type primitive;
    UInt32 instance;
};

ParticipantGetSpecificInfo_001::ParticipantGetSpecificInfo_001(ParticipantServicesInterface* participantServicesInterface)
    : m_participantServicesInterface(participantServicesInterface)
{
    clearCachedData();
}

std::map<ParticipantSpecificInfoKey::Type, UIntN> ParticipantGetSpecificInfo_001::getParticipantSpecificInfo(
    UIntN participantIndex, const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo)
{
    std::map<ParticipantSpecificInfoKey::Type, UIntN> results;
    for (auto request = requestedInfo.cbegin(); request != requestedInfo.cend(); request++)
    {
        try
        {
            auto result = m_cachedData.find(*request);
            if (result == m_cachedData.end())
            {
                m_cachedData[*request] = readSpecificInfo(getPrimitiveAndInstanceForSpecificInfoKey(*request));
                result = m_cachedData.find(*request);
            }
            results[*request] = result->second;
        }
        catch (...)
        {
            // if the primitive isn't available in the cache we receive an exception and
            // we don't add this item to the map
        }
    }

    return results;
}

void ParticipantGetSpecificInfo_001::clearCachedData(void)
{
    m_cachedData.clear();
}

Temperature ParticipantGetSpecificInfo_001::readSpecificInfo(PrimitiveAndInstance primitiveAndInstance)
{
    return m_participantServicesInterface->primitiveExecuteGetAsTemperatureC(
        primitiveAndInstance.primitive, Constants::Esif::NoDomain, static_cast<UInt8>(primitiveAndInstance.instance));
}
PrimitiveAndInstance ParticipantGetSpecificInfo_001::getPrimitiveAndInstanceForSpecificInfoKey(
    ParticipantSpecificInfoKey::Type request)
{
    PrimitiveAndInstance primitiveAndInstance;
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
        break;
    }
    return primitiveAndInstance;
}

XmlNode* ParticipantGetSpecificInfo_001::getXml(UIntN domainIndex)
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
        tripRequest.push_back((ParticipantSpecificInfoKey::Type)ac);
    }

    std::map<ParticipantSpecificInfoKey::Type, UIntN> tripPoints = 
        getParticipantSpecificInfo(Constants::Invalid, tripRequest);

    XmlNode* root = XmlNode::createWrapperElement("specific_info");

    auto tripPoint = tripPoints.end();
    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::Critical)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("crt", StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("crt", StatusFormat::friendlyValue(Constants::Invalid)));
    }

    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::Hot)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("hot", StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("hot", StatusFormat::friendlyValue(Constants::Invalid)));
    }

    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::Warm)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("wrm", StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("wrm", StatusFormat::friendlyValue(Constants::Invalid)));
    }

    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::PSV)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("psv", StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("psv", StatusFormat::friendlyValue(Constants::Invalid)));
    }

    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::NTT)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("ntt", StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("ntt", StatusFormat::friendlyValue(Constants::Invalid)));
    }

    std::vector<std::stringstream> tripAc;
    UIntN count = 0;
    for (IntN ac = ParticipantSpecificInfoKey::AC0; ac <= ParticipantSpecificInfoKey::AC9; ac++)
    {
        std::stringstream acx;
        acx << "ac" << count++;

        if ((tripPoint = tripPoints.find((ParticipantSpecificInfoKey::Type)ac)) != tripPoints.end())
        {
            root->addChild(XmlNode::createDataElement(acx.str(), StatusFormat::friendlyValue(tripPoint->second)));
        }
        else
        {
            root->addChild(XmlNode::createDataElement(acx.str(), StatusFormat::friendlyValue(Constants::Invalid)));
        }
    }

    return root;
}