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

ParticipantGetSpecificInfo_001::ParticipantGetSpecificInfo_001(ParticipantServicesInterface* participantServicesInterface)
    : m_participantServicesInterface(participantServicesInterface)
{
    createSupportedKeysVector();
    clearCachedData();
}

std::map<ParticipantSpecificInfoKey::Type, UIntN> ParticipantGetSpecificInfo_001::getParticipantSpecificInfo(
    UIntN participantIndex, const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo)
{
    updateCacheIfCleared();

    std::map<ParticipantSpecificInfoKey::Type, UIntN> resultMap;

    for (auto it = requestedInfo.cbegin(); it != requestedInfo.cend(); it++)
    {
        try
        {
            UIntN temperature = m_cachedData.at(*it);
            resultMap.insert(std::pair<ParticipantSpecificInfoKey::Type, UIntN>(*it, temperature));
        }
        catch (...)
        {
            // if the primitive isn't available in m_cachedData we receive an exception and
            // we don't add this item to the map
        }
    }

    return resultMap;
}

void ParticipantGetSpecificInfo_001::clearCachedData(void)
{
    m_cachedData.clear();
    m_cacheDataCleared = true;
}

void ParticipantGetSpecificInfo_001::updateCacheIfCleared()
{
    if (m_cacheDataCleared == true)
    {
        updateCache();
    }
}

void ParticipantGetSpecificInfo_001::updateCache(void)
{
    m_cachedData.clear();

    // The supportedKeys vector contains all of the items that are supported by the participant.  We
    // update the cachedData map to cache everything we will need until we are called to refresh the data again.
    for (auto it = m_supportedKeys.cbegin(); it != m_supportedKeys.cend(); it++)
    {
        switch (*it)
        {
            case ParticipantSpecificInfoKey::Type::None:
                // ignore.
                break;
            case ParticipantSpecificInfoKey::Type::Warm:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_WARM, *it, m_cachedData);
                break;
            case ParticipantSpecificInfoKey::Type::Hot:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_HOT, *it, m_cachedData);
                break;
            case ParticipantSpecificInfoKey::Type::Critical:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_CRITICAL, *it, m_cachedData);
                break;
            case ParticipantSpecificInfoKey::Type::AC0:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 0);
                break;
            case ParticipantSpecificInfoKey::Type::AC1:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 1);
                break;
            case ParticipantSpecificInfoKey::Type::AC2:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 2);
                break;
            case ParticipantSpecificInfoKey::Type::AC3:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 3);
                break;
            case ParticipantSpecificInfoKey::Type::AC4:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 4);
                break;
            case ParticipantSpecificInfoKey::Type::AC5:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 5);
                break;
            case ParticipantSpecificInfoKey::Type::AC6:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 6);
                break;
            case ParticipantSpecificInfoKey::Type::AC7:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 7);
                break;
            case ParticipantSpecificInfoKey::Type::AC8:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 8);
                break;
            case ParticipantSpecificInfoKey::Type::AC9:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_ACTIVE, *it, m_cachedData, 9);
                break;
            case ParticipantSpecificInfoKey::Type::PSV:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_TRIP_POINT_PASSIVE, *it, m_cachedData);
                break;
            case ParticipantSpecificInfoKey::Type::NTT:
                RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type::GET_NOTIFICATION_TEMP_THRESHOLD, *it, m_cachedData);
                break;
            default:
                throw dptf_exception("Received unexpected ParticipantSpecificInfoKey::Type");
                break;
        }
    }

    m_cacheDataCleared = false;
}

void ParticipantGetSpecificInfo_001::RequestPrimitiveTemperatureAndAddToMap(esif_primitive_type primitive,
    ParticipantSpecificInfoKey::Type key, std::map<ParticipantSpecificInfoKey::Type, UIntN>& resultMap,
    UIntN instance)
{
    try
    {
        Temperature temp = m_participantServicesInterface->primitiveExecuteGetAsTemperatureC(primitive,
            Constants::Esif::NoDomain, static_cast<UInt8>(instance));
        resultMap.insert(std::pair<ParticipantSpecificInfoKey::Type, UIntN>(key, temp));
    }
    catch (...)
    {
        // if the primitive isn't available we receive an exception and we don't add this item to the map
    }
}

void ParticipantGetSpecificInfo_001::createSupportedKeysVector(void)
{
    // This is the complete list of keys that can be supported by ParticipantGetSpecificInfo.  When
    // a policy calls it will normally request a small subset.  However, when we update our cache
    // we go ahead and request everything supported and cache it.

    m_supportedKeys.clear();

    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::Warm);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::Hot);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::Critical);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC0);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC1);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC2);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC3);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC4);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC5);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC6);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC7);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC8);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::AC9);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::PSV);
    m_supportedKeys.push_back(ParticipantSpecificInfoKey::Type::NTT);
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

    std::map<ParticipantSpecificInfoKey::Type, UIntN> tripPoints = getParticipantSpecificInfo(Constants::Invalid, tripRequest);

    XmlNode* root = XmlNode::createWrapperElement("specific_info");

    auto tripPoint = tripPoints.end();
    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::Critical)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("crt",
            StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("crt",
            StatusFormat::friendlyValue(Constants::Invalid)));
    }

    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::Hot)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("hot",
            StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("hot",
            StatusFormat::friendlyValue(Constants::Invalid)));
    }

    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::Warm)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("wrm",
            StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("wrm",
            StatusFormat::friendlyValue(Constants::Invalid)));
    }

    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::PSV)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("psv",
            StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("psv",
            StatusFormat::friendlyValue(Constants::Invalid)));
    }

    if ((tripPoint = tripPoints.find(ParticipantSpecificInfoKey::NTT)) != tripPoints.end())
    {
        root->addChild(XmlNode::createDataElement("ntt",
            StatusFormat::friendlyValue(tripPoint->second)));
    }
    else
    {
        root->addChild(XmlNode::createDataElement("ntt",
            StatusFormat::friendlyValue(Constants::Invalid)));
    }

    std::vector<std::stringstream> tripAc;
    UIntN count = 0;
    for (IntN ac = ParticipantSpecificInfoKey::AC0; ac <= ParticipantSpecificInfoKey::AC9; ac++)
    {
        std::stringstream acx;
        acx << "ac" << count++;

        if ((tripPoint = tripPoints.find((ParticipantSpecificInfoKey::Type)ac)) != tripPoints.end())
        {
            root->addChild(XmlNode::createDataElement(acx.str(),
                StatusFormat::friendlyValue(tripPoint->second)));
        }
        else
        {
            root->addChild(XmlNode::createDataElement(acx.str(),
                StatusFormat::friendlyValue(Constants::Invalid)));
        }
    }

    return root;
}